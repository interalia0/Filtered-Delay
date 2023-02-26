/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FilteredDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FilteredDelayAudioProcessorEditor (FilteredDelayAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~FilteredDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FilteredDelayAudioProcessor& audioProcessor;
    
    juce::Rectangle<int> delaySection;
    juce::Rectangle<int> filterSection;
    juce::Rectangle<int> modSection;
    
    juce::ToggleButton syncButton;
    juce::ComboBox syncRateMenu;
    juce::Slider rateDial;
    juce::Slider mixDial;
    juce::Slider feedbackDial;
    juce::Slider widthDial;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncButtonAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncRateMenuAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateDialAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixDialAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackDialAttachment;
    
    juce::Slider cutoffDial;
    juce::Slider resonanceDial;
    juce::ComboBox filterTypeMenu;
    
    juce::ToggleButton modBypassButton;
    juce::Slider modRate;
    juce::Slider modDepth;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilteredDelayAudioProcessorEditor)
};
