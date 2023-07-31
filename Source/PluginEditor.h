/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define FONT_HEIGHT 18.0f
#define LABEL_HEIGHT 25
//==============================================================================
class XcytheLookAndFeel_v1 : public juce::LookAndFeel_V4
{
public:
    XcytheLookAndFeel_v1();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& togglebutton,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox& box) override;
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           const bool isSeparator, const bool isActive,
                           const bool isHighlighted, const bool isTicked,
                           const bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* const textColourToUse) override;
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    juce::Path createFrame(juce::Rectangle<float>& bounds);
private:
    juce::Font font;
};
//==============================================================================
class XcytheRotarySlider : public juce::Component
{
public:
    void initialize(juce::Slider::RotaryParameters& rp, double minValue, double maxValue, double defaultValue,
                    juce::String suffix, juce::String newName, juce::LookAndFeel* lnf, juce::Font& font);
    void resized() override;
    juce::Label nameText{ "name" };
    juce::Label valueText{ "value" };
    juce::Slider slider{ juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                         juce::Slider::TextEntryBoxPosition::NoTextBox };
};
//==============================================================================
class TransientFunctionGraph : public juce::Component, public juce::Timer
{
public:
    void initialize(Clipper<float>* clipper);
    void drawBackground();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void update();
    juce::Label label{ "name", "TRANSFER FUNCTION" };
private:
    Clipper<float>* currentClipper{ nullptr };
    juce::uint64 time{ 0 };
    juce::Image bkgd;
    bool needUpdate{ false };
    float lineThickness{ 2.0f };
    float cornerSize{ 4.0f };
};
//==============================================================================
class DistortionTestAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DistortionTestAudioProcessorEditor (DistortionTestAudioProcessor&);
    ~DistortionTestAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    XcytheLookAndFeel_v1 newLNF;
    XcytheRotarySlider inputGainSlider;
    XcytheRotarySlider outputGainSlider;
    XcytheRotarySlider clipSlider;
    juce::ComboBox clipperBox{ "Clippers" };
    juce::ToggleButton linkButton{ "Link" };
    juce::ToggleButton bypassButton{ "Bypass" };

    DistortionTestAudioProcessor& audioProcessor;
    TransientFunctionGraph graph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionTestAudioProcessorEditor)
};
