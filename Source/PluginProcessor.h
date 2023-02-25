/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class FilteredDelayAudioProcessor  : public juce::AudioProcessor,
                                     public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    FilteredDelayAudioProcessor();
    ~FilteredDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
   
    juce::AudioProcessorValueTreeState treeState;
    

private:

// Delay
    static constexpr auto maxDelaySamples = 192000;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> delayL {maxDelaySamples};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> delayR {maxDelaySamples};
    
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>, 2> delayTimeValue;
    float delayTimeInSamples {0};
    float delayOffsetValue {0.0f};
    std::array<float, 2> lastDelayOutputL;
    std::array<float, 2> lastDelayOutputR;
    std::array<juce::LinearSmoothedValue<float>, 2> delayFeedbackVolume;
    
    
    juce::AudioPlayHead* playHead;
    juce::AudioPlayHead::CurrentPositionInfo cpi;
    double bpm{0};

    const std::array<float, 13> subdivisions{ 0.25f, (0.5f/3.0f), 0.375f, 0.5f, (1.0f/3.0f), 0.75f, 1.0f, (2.0f/3.0f), 1.5f, 2.0f, (4.0f/3.0f),3.0f, 4.0f };
    
// Mixer
    juce::dsp::DryWetMixer<float> mixer;


// Filter
    
    juce::dsp::StateVariableTPTFilter<float> filter;
    
// Modulation
    
    juce::dsp::Chorus<float> mod;


// ValueTree
    
    
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilteredDelayAudioProcessor)
};
