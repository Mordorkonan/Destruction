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
    setSize (600, 300);

    juce::Slider::RotaryParameters rotaryParameters;
    rotaryParameters.startAngleRadians = juce::MathConstants<float>::twoPi - 3 * 0.25f * juce::MathConstants<float>::pi;
    rotaryParameters.endAngleRadians = juce::MathConstants<float>::twoPi + 3 * 0.25f * juce::MathConstants<float>::pi;
    rotaryParameters.stopAtEnd = true;
    gainSlider.setRotaryParameters(rotaryParameters);
    gainSlider.setRange(-18.0, 18.0);
    gainSlider.setValue(-18.0);
    gainSlider.onValueChange = [this]() { audioProcessor.controllerLayout.setGainLevelInDecibels(gainSlider.getValue()); };
    addAndMakeVisible(gainSlider);

    clipSlider.setRotaryParameters(rotaryParameters);
    clipSlider.setRange(1.0, 10.0);
    clipSlider.setValue(1.0);
    clipSlider.onValueChange = [this]() { audioProcessor.clipper.updateMultiplier(static_cast<float>(clipSlider.getValue())); };
    addAndMakeVisible(clipSlider);
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
    auto bounds{ juce::Rectangle<int>(75, 75, 100, 100) };
    gainSlider.setBounds(bounds);
    clipSlider.setBounds(bounds.withX(bounds.getRight() + 10));

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
