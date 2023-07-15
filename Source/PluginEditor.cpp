/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
void XcytheLookAndFeel_v1::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPosProportional, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider)
{
    float lineThickness{ 2.0f };
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(lineThickness / 2);
    g.setColour(juce::Colours::white);
    g.drawRect(bounds);
    g.drawEllipse(bounds.toFloat().reduced(lineThickness / 2), lineThickness);
    g.drawText(static_cast<juce::String>(static_cast<int>(sliderPosProportional * 100)), bounds.reduced(30), juce::Justification::centred);

    juce::Point<float> center{ bounds.getCentre() };
    float radius{ width > height ? (bounds.getHeight() / 2) : (bounds.getWidth() / 2) };
    juce::Path circumference;
    circumference.addEllipse(bounds);
    // Рисуем одиночный шип
    //juce::Point<float> pivot{ center.translated(0.0f, -radius * 0.3f) };
    juce::Point<float> peak{ center.translated(0.0f, -radius * 0.6f) };
    // angle = 1.0f и 1.25f взятs при расчёте угла = длина дуги / радиус
    // значения дуги и радиуса взяты относительно дизайна слайдера.
    juce::Point<float> p1{ center.getPointOnCircumference(radius, -1.00f) };
    juce::Point<float> p2{ center.getPointOnCircumference(radius, -1.25f) };
    juce::Path spike;
    spike.startNewSubPath(p1);
    /* Для использования функции quadraticTo необходимо контрольная точка.
    Поскольку она нужна для каждой дуги своя, необходимо её создание
    привязать к точкам p1 и p2. Значения смещения для контрольных точек
    подобраны с использованием JUCE_LIVE_CONSTANT. */
    spike.quadraticTo(p1.withX(p1.x + radius * 0.35f).withY(p1.y - radius * 0.15f), peak);
    spike.quadraticTo(p2.withX(p2.x + radius * 0.30f).withY(p2.y - radius * 0.25f), p2);
    spike.addCentredArc(center.x, center.y, radius, radius, 0.0f, -1.25f, -1.00f);
    spike.closeSubPath();
    // размножаем шипы на 8 штук по всей окружности
    float correction = JUCE_LIVE_CONSTANT(50) * 0.01;
    g.reduceClipRegion(circumference);
    g.addTransform(juce::AffineTransform::rotation(
        juce::jmap(sliderPosProportional, rotaryStartAngle * correction, rotaryEndAngle * correction), center.x, center.y));
    g.addTransform(juce::AffineTransform::scale(1.7f - sliderPosProportional * 0.7f,
        1.7f - sliderPosProportional * 0.7f,
        center.x,
        center.y));
    for (int i = 1; i <= 8; ++i)
    {
        spike.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi * 0.25f, center.x, center.y));
        g.fillPath(spike);
    }
}
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
    gainSlider.setRotaryParameters(rotaryParameters);
    gainSlider.setRange(-12.0, 12.0);
    gainSlider.setValue(-6.0);
    gainSlider.onValueChange = [this]() { audioProcessor.controllerLayout.setGainLevelInDecibels(gainSlider.getValue()); };
    gainSlider.setLookAndFeel(&newLNF);
    addAndMakeVisible(gainSlider);

    clipSlider.setRotaryParameters(rotaryParameters);
    clipSlider.setRange(1.0, 10.0);
    clipSlider.setValue(1.0);
    clipSlider.onValueChange = [this]()
    {
        audioProcessor.clipHolder.getClipper()->updateMultiplier(clipSlider.getValue());
    };
    clipSlider.setLookAndFeel(&newLNF);
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
    clipperBox.setBounds(bounds.withX(clipSlider.getBounds().getRight() + 10));

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
