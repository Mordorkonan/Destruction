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
enum PresetMenuIDs { NoSelect, New, Save, Load, Delete, PresetList };
enum FrameOrientation { None, Left, Right };
//==============================================================================
class XcytheLookAndFeel_v1 : public juce::LookAndFeel_V4
{
public:
    XcytheLookAndFeel_v1();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& togglebutton,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawButtonInternal(juce::Graphics& g, juce::Rectangle<float> bounds,
                            FrameOrientation orientation, const juce::String& text, bool toggleState,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown);
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
    void drawAlertBox(juce::Graphics& g, juce::AlertWindow& alert,
                      const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout) override;
    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
    juce::Font getAlertWindowTitleFont() override;
    juce::Font getAlertWindowMessageFont() override;
    juce::Font getAlertWindowFont() override;
    int getAlertWindowButtonHeight() override;
    juce::Array<int> getWidthsForTextButtons(juce::AlertWindow&, const juce::Array<juce::TextButton*>&) override;
    juce::Path createFrame(const juce::Rectangle<float>& bounds, FrameOrientation orientation);
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
class PresetPanel : public juce::Component
{
public:
    PresetPanel(juce::LookAndFeel& _lnf, PresetManager& manager);
    juce::Label* getPresetNameLabel();
    void updatePresetMenu();
    void resized() override;
private:
    juce::Label presetNameLabel{ "Preset Name", "-init-"};
    juce::LookAndFeel& lnf;
    juce::TextButton previousButton{ "Previous" };
    juce::TextButton nextButton{ "Next" };
    juce::ComboBox presetMenu;
    PresetManager& manager;
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<juce::AlertWindow> aw;
};
//==============================================================================
class Plate : public juce::Component
{
    void paint(juce::Graphics& g) override;
};
//==============================================================================
class DestructionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DestructionAudioProcessorEditor (DestructionAudioProcessor&);
    ~DestructionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void drawShadows(juce::Graphics& g,
                     juce::Path& path,
                     const juce::Rectangle<int>& bounds,
                     std::unique_ptr<juce::DropShadow>& shadow);
    void drawBackground(juce::Graphics& g,
                        juce::ColourGradient& gradient,
                        const juce::Rectangle<float>& bounds,
                        std::map<double, juce::Colour>& colors);

private:
    XcytheLookAndFeel_v1 newLNF;
    XcytheRotarySlider inputGainSlider;
    XcytheRotarySlider outputGainSlider;
    XcytheRotarySlider clipSlider;
    juce::ComboBox clipperBox{ "Clippers" };
    juce::ToggleButton linkButton{ "Link" };
    juce::ToggleButton bypassButton{ "Bypass" };
    juce::Label pluginName{ "Destruction" };
    juce::Label version{ "Version"};
    juce::Image logo;

    DestructionAudioProcessor& audioProcessor;
    TransientFunctionGraph graph;
    PresetPanel presetPanel;
    Plate graphPlate, sliderPlate;
    std::unique_ptr<juce::DropShadow> graphPlateShadow, sliderPlateShadow;

    std::unique_ptr<APVTS::SliderAttachment> inputGainAttach;
    std::unique_ptr<APVTS::SliderAttachment> outputGainAttach;
    std::unique_ptr<APVTS::SliderAttachment> clipAttach;
    std::unique_ptr<APVTS::ButtonAttachment> bypassAttach;
    std::unique_ptr<APVTS::ButtonAttachment> linkAttach;
    std::unique_ptr<APVTS::ComboBoxAttachment> clipperBoxAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DestructionAudioProcessorEditor)
};
