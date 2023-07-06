/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistortionTestAudioProcessorEditor::DistortionTestAudioProcessorEditor (DistortionTestAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    gainSlider.onValueChange = [this]() { audioProcessor.setGainLevelInDecibels(gainSlider.getValue()); };
    juce::Slider::RotaryParameters rotaryParameters;
    rotaryParameters.startAngleRadians = juce::MathConstants<float>::twoPi - 3 * 0.25 * juce::MathConstants<float>::pi;
    rotaryParameters.endAngleRadians = juce::MathConstants<float>::twoPi + 3 * 0.25 * juce::MathConstants<float>::pi;
    rotaryParameters.stopAtEnd = true;
    gainSlider.setRotaryParameters(rotaryParameters);
    gainSlider.setRange(-18.0, 18.0);
    addAndMakeVisible(gainSlider);
}

DistortionTestAudioProcessorEditor::~DistortionTestAudioProcessorEditor()
{
}

//==============================================================================
void DistortionTestAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void DistortionTestAudioProcessorEditor::resized()
{
    gainSlider.setBounds(75, 75, 150, 150);
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
