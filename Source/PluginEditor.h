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
class DistortionTestAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DistortionTestAudioProcessorEditor (DistortionTestAudioProcessor&);
    ~DistortionTestAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider inputGainSlider{ juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                  juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Slider outputGainSlider{ juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                   juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::Slider clipSlider{ juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                             juce::Slider::TextEntryBoxPosition::TextBoxBelow };
    juce::ComboBox clipperBox{ "Clippers" };
    juce::ToggleButton linkButton{ "Link" };
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DistortionTestAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionTestAudioProcessorEditor)
};
