/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FilteredDelayAudioProcessorEditor::FilteredDelayAudioProcessorEditor (FilteredDelayAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1000, 500);
    
    addAndMakeVisible(syncButton);
    addAndMakeVisible(syncRateMenu);
    addAndMakeVisible(rateDial);
    addAndMakeVisible(mixDial);
    addAndMakeVisible(feedbackDial);
    addAndMakeVisible(widthDial);
    addAndMakeVisible(filterTypeMenu);
    addAndMakeVisible(cutoffDial);
    addAndMakeVisible(resonanceDial);
    addAndMakeVisible(modBypassButton);
    addAndMakeVisible(modRate);
    addAndMakeVisible(modDepth);
    
    syncButtonAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(vts, "BPM_SYNC", syncButton));
    syncRateMenuAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(vts, "SYNC_RATE_CHOICE", syncRateMenu));
//    rateDialAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "RATE", rateDial));
    mixDialAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "MIX", mixDial));
    feedbackDialAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(vts, "FEEDBACK", feedbackDial));

    
    rateDial.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    rateDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);
    feedbackDial.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    feedbackDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 20);

    
    
    
    

    

    
}

FilteredDelayAudioProcessorEditor::~FilteredDelayAudioProcessorEditor()
{
}

//==============================================================================
void FilteredDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    juce::Image background = juce::ImageCache::getFromMemory (BinaryData::background2_png, BinaryData::background2_pngSize);
        g.drawImageAt (background, 0, 0);
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
//    g.setColour(juce::Colours::red);
//    g.fillRect(delaySection);
//    g.setColour(juce::Colours::green);
//    g.fillRect(filterSection);
//    g.setColour(juce::Colours::blue);
//    g.fillRect(modSection);
    

}

void FilteredDelayAudioProcessorEditor::resized()
{
    const int border = 20;
    const int topRowSliderWidth = 200 - 30;
    const int topRowSliderHeight = 250 - border - 40;
    const int menuWidth = 100;
    delaySection.setBounds(0, 0, 400, 500);
    filterSection.setBounds(400, 0, 400, 500);
    modSection.setBounds(800, 0, 200, 500);
    
    syncRateMenu.setBounds(delaySection.getWidth() / 2 + 35, delaySection.getHeight() / 2 - 20, menuWidth, 30);
    syncButton.setBounds(90, delaySection.getHeight() / 2 - 20, 30, 30);
    rateDial.setBounds(delaySection.getX() + border, delaySection.getY() + border, topRowSliderWidth, topRowSliderHeight);
    feedbackDial.setBounds(delaySection.getWidth() / 2, delaySection.getY() + border, topRowSliderWidth, topRowSliderHeight);

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
