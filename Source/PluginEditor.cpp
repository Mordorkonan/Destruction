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

    clipperBox.addItem("Hard Clip", hard);
    clipperBox.addItem("Soft Clip", soft);
    clipperBox.addItem("Fold Back", foldback);
    clipperBox.addItem("Sine Fold", sinefold);
    clipperBox.addItem("Linear Fold", linearfold);
    clipperBox.setSelectedItemIndex(1);
    clipperBox.onChange = [this]()
    {
        audioProcessor.clipHolder.setClipper(clipperBox.getSelectedItemIndex());
        audioProcessor.clipHolder.getClipper()->updateMultiplier(clipSlider.getValue());
    };
    addAndMakeVisible(clipperBox);

    juce::Slider::RotaryParameters rotaryParameters;
    rotaryParameters.startAngleRadians = juce::MathConstants<float>::twoPi - 3 * 0.25f * juce::MathConstants<float>::pi;
    rotaryParameters.endAngleRadians = juce::MathConstants<float>::twoPi + 3 * 0.25f * juce::MathConstants<float>::pi;
    rotaryParameters.stopAtEnd = true;

    inputGainSlider.setRotaryParameters(rotaryParameters);
    inputGainSlider.setRange(-12.0, 12.0);
    inputGainSlider.setValue(0.0);
    inputGainSlider.setDoubleClickReturnValue(true, 0.0);
    inputGainSlider.onValueChange = [this]()
    {
        audioProcessor.gainController.setInputGainLevelInDb(inputGainSlider.getValue());
        if (linkButton.getToggleState())
        {
            outputGainSlider.setValue(-inputGainSlider.getValue());
        }
    };
    addAndMakeVisible(inputGainSlider);

    outputGainSlider.setRotaryParameters(rotaryParameters);
    outputGainSlider.setRange(-12.0, 12.0);
    outputGainSlider.setValue(0.0);
    outputGainSlider.setDoubleClickReturnValue(true, 0.0);
    outputGainSlider.onValueChange = [this]()
    {
        audioProcessor.gainController.setOutputGainLevelInDb(outputGainSlider.getValue());
        if (linkButton.getToggleState())
        {
            inputGainSlider.setValue(-outputGainSlider.getValue());
        }
    };
    addAndMakeVisible(outputGainSlider);

    clipSlider.setRotaryParameters(rotaryParameters);
    clipSlider.setRange(1.0, 10.0);
    clipSlider.setValue(1.0);
    clipSlider.setDoubleClickReturnValue(true, 1.0);
    clipSlider.onValueChange = [this]()
    {
        audioProcessor.clipHolder.getClipper()->updateMultiplier(clipSlider.getValue());
    };
    addAndMakeVisible(clipSlider);

    linkButton.setToggleState(true, juce::NotificationType::sendNotification);
    linkButton.onStateChange = [this]()
    {
        if (linkButton.getToggleState())
        {
            if (inputGainSlider.getValue() < 0)
            {
                inputGainSlider.setValue(-outputGainSlider.getValue());
            }
            else { outputGainSlider.setValue(-inputGainSlider.getValue()); }            
        }
    };
    addAndMakeVisible(linkButton);
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
    inputGainSlider.setBounds(bounds);
    clipSlider.setBounds(bounds.withX(bounds.getRight() + 10));
    outputGainSlider.setBounds(bounds.withX(clipSlider.getBounds().getRight() + 10));
    clipperBox.setBounds(bounds.withHeight(20).withX(outputGainSlider.getBounds().getRight() + 10));
    linkButton.setBounds(clipperBox.getBounds().withY(clipperBox.getBounds().getBottom() + 10));

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
