/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FilteredDelayAudioProcessor::FilteredDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "Parameters", createParameters())
#endif
{
    treeState.addParameterListener ("BPM_SYNC", this);
    treeState.addParameterListener ("SYNC_RATE_CHOICE", this);
    treeState.addParameterListener ("RATE", this);
    treeState.addParameterListener ("FEEDBACK", this);
    treeState.addParameterListener ("WIDTH", this);
    treeState.addParameterListener ("MIX", this);
    treeState.addParameterListener ("FILTER_TYPE", this);
    treeState.addParameterListener ("CUTOFF", this);
    treeState.addParameterListener ("RESONANCE", this);
    treeState.addParameterListener ("MOD_BP", this);
    treeState.addParameterListener ("MOD_RATE", this);
    treeState.addParameterListener ("MOD_DEPTH", this);
    treeState.addParameterListener ("MOD_FB", this);
  
}

FilteredDelayAudioProcessor::~FilteredDelayAudioProcessor()
{
    treeState.removeParameterListener ("BPM_SYNC", this);
    treeState.removeParameterListener ("SYNC_RATE_CHOICE", this);
    treeState.removeParameterListener ("RATE", this);
    treeState.removeParameterListener ("FEEDBACK", this);
    treeState.removeParameterListener ("WIDTH", this);
    treeState.removeParameterListener ("MIX", this);
    treeState.removeParameterListener ("FILTER_TYPE", this);
    treeState.removeParameterListener ("CUTOFF", this);
    treeState.removeParameterListener ("RESONANCE", this);
    treeState.removeParameterListener ("MOD_BP", this);
    treeState.removeParameterListener ("MOD_RATE", this);
    treeState.removeParameterListener ("MOD_DEPTH", this);
    treeState.removeParameterListener ("MOD_FB", this);
}


//==============================================================================
const juce::String FilteredDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FilteredDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FilteredDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FilteredDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FilteredDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FilteredDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FilteredDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FilteredDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FilteredDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void FilteredDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FilteredDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    filter.reset();
    mixer.reset();
    delayL.reset();
    delayR.reset();
    mod.reset();
    
    mixer.prepare(spec);
    delayL.prepare(spec);
    delayR.prepare(spec);
    filter.prepare(spec);
    mod.prepare(spec);
    
    mixer.setWetMixProportion(0);
    mod.setCentreDelay(15.0f);
    mod.setMix(0);
    mod.setFeedback(0);
    
    for (auto& volume : delayFeedbackVolume)
        volume.reset (spec.sampleRate, 0.05);
    
    delayTimeSmoothedValue.reset(sampleRate, 0.0025f);
    delayOffsetSmoothedValue.reset(sampleRate, 0.0025f);
  

    std::fill (lastDelayOutputL.begin(), lastDelayOutputL.end(), 0.0f);
    std::fill (lastDelayOutputR.begin(), lastDelayOutputR.end(), 0.0f);
}

void FilteredDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FilteredDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FilteredDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
   juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    juce::RangedAudioParameter* bpmSyncParameter = treeState.getParameter("BPM_SYNC");
    juce::RangedAudioParameter* syncRateChoiceParameter = treeState.getParameter("SYNC_RATE_CHOICE");
    
    const int subdivisionIndex = static_cast<int>(syncRateChoiceParameter->getValue() * subdivisions.size());
    const float selectedSubdivisionValue = subdivisions[subdivisionIndex];
    
    double bpm {120};
        if (auto bpmFromHost = *getPlayHead()->getPosition()->getBpm())
            bpm = bpmFromHost;

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
     
    const auto numChannels = juce::jmax (totalNumInputChannels, totalNumOutputChannels);

    auto audioBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, (size_t) numChannels);
    auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
    const auto& input = context.getInputBlock();
    const auto& output = context.getOutputBlock();

    mixer.pushDrySamples(input);
    
//    Modulation
    mod.process(context);
    
    for (size_t channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *samplesIn = input.getChannelPointer(channel);
        auto *samplesOut = output.getChannelPointer(channel);
        
        delayOffsetSmoothedValue.setTargetValue(delayOffsetInSamples);
        
 
//          Check for synced or free delay time
        if (bpmSyncParameter->getValue())
        {
            float delayTimeInSamples = (60.0 / bpm) * selectedSubdivisionValue  * getSampleRate();
            float delayTimeInMillisec =  delayTimeInSamples / getSampleRate() * 1000.0f;
            treeState.getParameter("RATE")->beginChangeGesture();
            treeState.getParameter("RATE")->setValueNotifyingHost(juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f).convertTo0to1(delayTimeInMillisec));
            treeState.getParameter("RATE")->endChangeGesture();
            
            delayTimeSmoothedValue.setTargetValue(delayTimeInSamples);
            delayL.setDelay(delayTimeSmoothedValue.getNextValue() + delayOffsetSmoothedValue.getNextValue());
            delayR.setDelay(delayTimeSmoothedValue.getNextValue());
        }
        else
        {
            delayTimeSmoothedValue.setTargetValue(delayTimeInSamples);
            delayL.setDelay(delayTimeSmoothedValue.getNextValue() + delayOffsetSmoothedValue.getNextValue());
            delayR.setDelay(delayTimeSmoothedValue.getNextValue());
        }

//        Left delay line
        
        if (channel == 0)
        {
            for (size_t sample = 0; sample < input.getNumSamples(); ++sample)
            {
                auto input = samplesIn[sample] - lastDelayOutputL[channel];
                delayL.pushSample((int) channel, input);
                
                auto wetL = delayL.popSample((int)channel);
                wetL = filter.processSample((int)channel, wetL);
                
                samplesOut[sample] = wetL + lastDelayOutputL[channel];
                lastDelayOutputL[channel] = samplesOut[sample] * delayFeedbackVolume[channel].getNextValue() * 0.5f;
            }
        }
        
//        Right delay line
        
        if (channel == 1)
        {
            for (size_t sample = 0; sample < input.getNumSamples(); ++sample)
            {
                auto input = samplesIn[sample] - lastDelayOutputR[channel];
                delayR.pushSample((int) channel, input);
                
                auto wetR = delayR.popSample((int)channel);
                wetR = filter.processSample((int)channel, wetR);
                
                samplesOut[sample] = wetR + lastDelayOutputR[channel];
                lastDelayOutputR[channel] = samplesOut[sample] * delayFeedbackVolume[channel].getNextValue() * 0.5f;
            }
        }
    }
    mixer.mixWetSamples(output);
}

//==============================================================================
bool FilteredDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FilteredDelayAudioProcessor::createEditor()
{
//    return new FilteredDelayAudioProcessorEditor (*this, treeState);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void FilteredDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FilteredDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FilteredDelayAudioProcessor();
}
void FilteredDelayAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    
    using filterType = juce::dsp::StateVariableTPTFilterType;
  
    if (parameterID == "RATE")
    {
        delayTimeInSamples = newValue / 1000.0 * getSampleRate();
    }
    
    if (parameterID == "MIX")
    {
        mixer.setWetMixProportion (newValue);
    }
    
    if (parameterID == "FEEDBACK")
    {
        const auto feedbackGain = newValue;

        for (auto& volume : delayFeedbackVolume)
            volume.setTargetValue (feedbackGain);
    }
    
    if (parameterID == "WIDTH")
    {
        delayOffsetInSamples = newValue / 1000.0 * getSampleRate();
        
    }

    if (parameterID == "FILTER_TYPE")
    {
        if (newValue == 0)
            filter.setType(filterType::lowpass);

        if (newValue == 1)
            filter.setType(filterType::highpass);

        if (newValue == 2)
            filter.setType(filterType::bandpass);

    }

    if (parameterID == "CUTOFF")
    {
        filter.setCutoffFrequency (newValue);
    }
    
    if (parameterID == "RESONANCE")
    {
        filter.setResonance(newValue);
    }

    if (parameterID == "MOD_BP")
    {
        if (newValue == false)
            mod.setMix(1.0f);
        if (newValue == true)
            mod.setMix(0.0f);
    }

    if (parameterID == "MOD_RATE")
    {
        mod.setRate(newValue);
    }

    if (parameterID == "MOD_DEPTH")
    {
        mod.setDepth(newValue);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout FilteredDelayAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    
    using Range = juce::NormalisableRange<float>;
    using pID = juce::ParameterID;
    
    params.add (std::make_unique<juce::AudioParameterBool>  (pID {"BPM_SYNC", 1}, "Bpm Sync", true));
    params.add (std::make_unique<juce::AudioParameterChoice>(pID {"SYNC_RATE_CHOICE", 1}, "Sync Rate Choice", juce::StringArray{ "16th", "16th Triplet", "16th Dotted",
                                                                  "8th", "8th Triplet", "8th Dotted", "Quarter", "Quarter Triplet", "Quarter Dotted",
                                                                  "Half", "Half Triplet", "Half Dotted", "Whole"}, 3));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"RATE", 1}, "Rate", Range {1.0f, 2000.0f, 1.0}, 0, "ms"));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"FEEDBACK", 1}, "Feedback", Range {0.0f, 1.0f, 0.01f}, 0.25f, "%"));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"WIDTH", 1}, "Width", Range {0.0f, 5.0f, 0.1f}, 0.0f, "ms"));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"MIX", 1}, "Mix", Range { 0.0f, 1.0f, 0.01f }, 0.0f, "%"));
    params.add (std::make_unique<juce::AudioParameterChoice>(pID {"FILTER_TYPE", 1}, "Filter Type", juce::StringArray("Lowpass", "Highpass", "Bandpass"), 0));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"CUTOFF", 1}, "Cutoff", Range {20.0f, 20000.0f, 1.0f, 0.2f}, 1000.f, "Hz"));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"RESONANCE", 1}, "Resonance", Range {0.0f, 2.0f, 0.1f}, 0.707f));
    params.add (std::make_unique<juce::AudioParameterBool>  (pID {"MOD_BP", 1}, "Mod Bypass", true));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"MOD_RATE", 1}, "Mod Rate", Range{0.05f, 5.0f, 0.01f}, 1.0f, "Hz"));
    params.add (std::make_unique<juce::AudioParameterFloat> (pID {"MOD_DEPTH", 1}, "Mod Depth", Range{0.0f, 1.0f, 0.01f}, 0.15f));
    return params;
}

