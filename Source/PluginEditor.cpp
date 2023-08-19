/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
XcytheLookAndFeel_v1::XcytheLookAndFeel_v1()
{
    font = juce::Typeface::createSystemTypefaceFor(BinaryData::MagistralTT_ttf, BinaryData::MagistralTT_ttfSize);
    font.setHeight(FONT_HEIGHT);
}

void XcytheLookAndFeel_v1::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPosProportional, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider)
{
    float lineThickness{ 2.0f };
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(lineThickness / 2);
    g.setColour(juce::Colours::white);

    juce::Point<float> center{ bounds.getCentre() };
    float radius{ width > height ? (bounds.getHeight() / 2) : (bounds.getWidth() / 2) };
    juce::Path circumference;
    circumference.addEllipse(bounds);
    // Рисуем одиночный шип
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
    float correction = 0.35f;// JUCE_LIVE_CONSTANT(50) * 0.01;
    g.addTransform(juce::AffineTransform::rotation(
        juce::jmap(sliderPosProportional, rotaryStartAngle * correction, rotaryEndAngle * correction), center.x, center.y));
    g.setColour(juce::Colours::darkgrey);
    // размножаем шипы на 8 штук по всей окружности
    for (int i = 1; i <= 8; ++i)
    {
        spike.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi * 0.25f, center.x, center.y));
        g.fillPath(spike);
    }
    g.reduceClipRegion(circumference);
    correction = 0.27f;// JUCE_LIVE_CONSTANT(27) * 0.01;
    g.addTransform(juce::AffineTransform::rotation(
        juce::jmap(sliderPosProportional, rotaryStartAngle * correction, rotaryEndAngle * correction), center.x, center.y));
    g.addTransform(juce::AffineTransform::scale(1.7f - sliderPosProportional * 0.7f,
                                                1.7f - sliderPosProportional * 0.7f,
                                                center.x,
                                                center.y));
    // размножаем шипы на 8 штук по всей окружности
    g.setColour(juce::Colours::white);
    for (int i = 1; i <= 8; ++i)
    {
        spike.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi * 0.25f, center.x, center.y));
        g.fillPath(spike);
    }
}

void XcytheLookAndFeel_v1::drawToggleButton(juce::Graphics& g, juce::ToggleButton& togglebutton,
                                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    float lineThickness{ 1.0f };
    auto bounds{ togglebutton.getLocalBounds().toFloat().reduced(lineThickness * 0.5f) };
    juce::Path contour{ createFrame(bounds, FrameOrientation::None) };
    // определяем цвет в зависимости от состояния кнопки
    auto baseColor{ juce::Colours::darkgrey.withMultipliedSaturation(
                        togglebutton.hasKeyboardFocus(true) ? 1.3f : 1.0f)
                        .withMultipliedAlpha(togglebutton.getToggleState() ? 1.0f : 0.5f)};
    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
    {
        baseColor = baseColor.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);
    }
    g.setColour(baseColor);
    g.fillPath(contour);
    g.setColour(juce::Colours::white);
    g.strokePath(contour, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved));
    g.setFont(font);
    g.drawText(togglebutton.getButtonText(), bounds, juce::Justification::centred);
}

void XcytheLookAndFeel_v1::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    float lineThickness{ 1.0f };
    auto bounds{ button.getLocalBounds().toFloat().reduced(lineThickness * 0.5f) };
    auto orientation = FrameOrientation::None;
    if (button.getButtonText() == "<") { orientation = FrameOrientation::Left; }
    else if (button.getButtonText() == ">") { orientation = FrameOrientation::Right; }
    juce::Path contour{ createFrame(bounds, orientation) };
    // определяем цвет в зависимости от состояния кнопки
    auto baseColor{ juce::Colours::darkgrey.withMultipliedSaturation(
                        button.hasKeyboardFocus(true) ? 1.3f : 1.0f)
                        .withMultipliedAlpha(button.getToggleState() ? 1.0f : 0.5f) };
    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
    {
        baseColor = baseColor.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);
    }
    g.setColour(baseColor);
    g.fillPath(contour);
    g.setColour(juce::Colours::white);
    g.strokePath(contour, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved));
    g.setFont(font);
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

void XcytheLookAndFeel_v1::drawComboBox(juce::Graphics& g, int width, int height, bool,
                  int, int, int, int, juce::ComboBox& box)
{
    float lineThickness{ 1.0f };
    auto bounds{ box.getLocalBounds().toFloat().reduced(lineThickness * 0.5f) };
    juce::Path contour{ createFrame(bounds, FrameOrientation::None) };
    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    g.fillPath(contour);
    g.setColour(juce::Colours::white);
    g.strokePath(contour, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved));
}

void XcytheLookAndFeel_v1::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(box.getLocalBounds().reduced(1));
    label.setFont(font);
    label.setJustificationType(juce::Justification::centred);
}

void XcytheLookAndFeel_v1::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                             const bool isSeparator, const bool isActive,
                                             const bool isHighlighted, const bool isTicked,
                                             const bool hasSubMenu, const juce::String& text,
                                             const juce::String& shortcutKeyText,
                                             const juce::Drawable* icon, const juce::Colour* const textColourToUse)
{
    if (isSeparator)
    {
        auto r = area.reduced(12, 0); // 12 для выравнивания под фаску рамки
        r.removeFromTop(juce::roundToInt((static_cast<float>(r.getHeight()) * 0.5f) - 0.5f));

        g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.3f));
        g.fillRect(r.removeFromTop(1));
        return;
    }
    auto textColour = (textColourToUse == nullptr ? findColour(juce::PopupMenu::textColourId)
                       : *textColourToUse);
    auto r = area;
    juce::Path contour{ createFrame(r.toFloat().reduced(1.5f, 0.5f), FrameOrientation::None) };
    if (isHighlighted && isActive)
    {
        g.setColour(juce::Colours::grey);
        g.strokePath(contour, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved));
        g.setColour(findColour(juce::PopupMenu::highlightedTextColourId)); // нужно менять цвет для текста
    }
    else
    {
        g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
    }
    g.setFont(font);
    if (isTicked)
    {
        g.setColour(juce::Colours::white);
        g.strokePath(contour, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved));
    }
    g.drawFittedText(text, r, juce::Justification::centred, 1);
    if (shortcutKeyText.isNotEmpty())
    {
        auto f2 = font;
        f2.setHeight(f2.getHeight() * 0.75f);
        f2.setHorizontalScale(0.95f);
        g.setFont(f2);
        g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
    }
}

void XcytheLookAndFeel_v1::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    auto background{ juce::Colours::black.contrasting(0.15f) };
    g.fillAll(background);   
    #if ! JUCE_MAC
        g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.6f));
        g.drawRect(0, 0, width, height);
    #endif
}

juce::Path XcytheLookAndFeel_v1::createFrame(juce::Rectangle<float>& bounds, FrameOrientation orientation)
{
    /* Метод для отрисовки 6-угольного контура кнопок и комбобоксов.
    Примечание! передаваемые баунды учитывают половину толщины линии,
    т.е. для толщины = 1 пикселю, баунды будут равны:
    juce::Rectangle<float>(0.5, 0.5, getHeight() - 0.5, getWidth() - 0.5)
    В связи с этим при выстраивании контура к параметру chamfer 
    добавляется bounds.getY() для учёта смещения по вертикали. */
    juce::Path contour;
    auto chamfer{ bounds.getHeight() * 0.5f };
    if (orientation == FrameOrientation::None)
    {
        contour.startNewSubPath(bounds.getX() + chamfer, bounds.getY());
    }
    else
    {
        contour.startNewSubPath(bounds.getX(), bounds.getY());
    }
    contour.lineTo(bounds.getRight() - chamfer, bounds.getY());
    contour.lineTo(bounds.getRight(), bounds.getY() + chamfer);
    contour.lineTo(bounds.getRight() - chamfer, bounds.getBottom());
    if (orientation == FrameOrientation::None)
    {
        contour.lineTo(bounds.getX() + chamfer, bounds.getBottom());
        contour.lineTo(bounds.getX(), bounds.getY() + chamfer);
    }
    else
    {
        contour.lineTo(bounds.getX(), bounds.getBottom());
        contour.lineTo(bounds.getX() + chamfer, bounds.getY() + chamfer);
    }
    contour.closeSubPath();
    /* Для отражения ориентированного контура для кнопок nextPreset,
    prevPreset используется преобразование через juce::AffineTransform.
    При этом необходимо учитывать, что поворот производится относительно
    начала координат, при этом пиксель, находившийся в координатах 0;0,
    будет находиться в -1;-1, поэтому при использовании функции
    juce::AffineTransform::translation нужно к размерам баундов
    добавлять 1 для компенсации разворота. */
    if (orientation == FrameOrientation::Left)
    {
        contour.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi));
        contour.applyTransform(juce::AffineTransform::translation(bounds.getWidth() + 1, bounds.getHeight() + 1));
    }    
    return contour;
}
//==============================================================================
void XcytheRotarySlider::initialize(juce::Slider::RotaryParameters& rp, double minValue, double maxValue, double defaultValue,
                                    juce::String suffix, juce::String newName, juce::LookAndFeel* lnf, juce::Font& font)
{
    slider.setRotaryParameters(rp);
    slider.setRange(minValue, maxValue);
    slider.setValue(defaultValue);
    slider.setDoubleClickReturnValue(true, defaultValue);
    slider.setLookAndFeel(lnf);
    nameText.setFont(font);
    nameText.setText(newName, juce::NotificationType::dontSendNotification);
    nameText.setJustificationType(juce::Justification::centredTop);
    valueText.setFont(font);
    valueText.setText(juce::String(slider.getValue(), 1) + suffix, juce::NotificationType::dontSendNotification);
    valueText.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(slider);
    addAndMakeVisible(nameText);
    addAndMakeVisible(valueText);
}

void XcytheRotarySlider::resized()
{
    auto bounds{ getLocalBounds() };
    nameText.setBounds(bounds.removeFromTop(LABEL_HEIGHT));
    valueText.setBounds(bounds.removeFromBottom(LABEL_HEIGHT));
    slider.setBounds(bounds);
}
//==============================================================================
void TransientFunctionGraph::initialize(Clipper<float>* clipper) { currentClipper = clipper; }

void TransientFunctionGraph::update() { needUpdate = true; }

void TransientFunctionGraph::timerCallback()
{
    if (needUpdate)
    {
        needUpdate = false;
        time = juce::Time::currentTimeMillis();
        repaint();
    }
}

void TransientFunctionGraph::drawBackground()
{
    // рендеринг статических данных в изображение для вызова в paint
    auto bounds{ getLocalBounds().toFloat() };
    bkgd = juce::Image(juce::Image::PixelFormat::ARGB,
                       static_cast<int>(bounds.getWidth()),
                       static_cast<int>(bounds.getHeight()),
                       true);
    juce::Graphics g{ bkgd }; // сначала создаём изображение с баундами
    bounds.reduce(lineThickness * 0.5f, lineThickness * 0.5f); // затем режем баунды по толщине линии, чтобы влез контур
    g.addTransform(juce::AffineTransform::scale(juce::Desktop::getInstance().getGlobalScaleFactor()));
    g.setColour(juce::Colours::transparentWhite);
    g.fillAll();
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(bounds, cornerSize);
    g.setColour(juce::Colours::darkgrey);
    g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY());
    g.drawLine(bounds.getCentreX(), bounds.getY(), bounds.getCentreX(), bounds.getBottom());
    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(bounds, cornerSize, lineThickness);
}

void TransientFunctionGraph::paint(juce::Graphics& g)
{
    auto bounds{ getLocalBounds().toFloat().withTrimmedTop(LABEL_HEIGHT) };
    g.drawImage(bkgd, bounds);
    bounds.reduce(cornerSize, cornerSize);
    int resolution{ 400 }; // кратна ширине графика передаточной функции
    float x{ 0.0f };
    float y{ 0.0f };
    float normalizedX{ 0.0f };
    float normalizedY{ 0.0f };
    juce::Path graph;
    for (int i = 0; i < resolution; ++i)
    {
        x = static_cast<float>(i);
        normalizedX = juce::jmap(x, 0.0f, static_cast<float>(resolution) - 1.0f, -1.0f, 1.0f);
        y = currentClipper->process(normalizedX);
        normalizedX = juce::jmap(normalizedX, -1.0f, 1.0f, bounds.getX(), bounds.getRight());
        normalizedY = juce::jmap(y, -1.0f, 1.0f, bounds.getBottom(), bounds.getY());
        if (i == 0) { graph.startNewSubPath(normalizedX, normalizedY); }
        else { graph.lineTo(normalizedX, normalizedY); }
    }
    g.setColour(juce::Colours::white);
    g.strokePath(graph, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved));
}

void TransientFunctionGraph::resized()
{
    auto bounds{ getLocalBounds() };
    label.setBounds(bounds.removeFromTop(LABEL_HEIGHT));
    label.setJustificationType(juce::Justification::centredTop);
    drawBackground();
}
//==============================================================================
PresetPanel::PresetPanel(juce::LookAndFeel& _lnf, PresetManager& pm) : lnf(_lnf), manager(pm)
{
    previousButton.setButtonText("<");
    previousButton.setLookAndFeel(&lnf);
    addAndMakeVisible(previousButton);
    previousButton.onClick = [&]()
    {
        const int index{ manager.previousPreset() };
        presetMenu.setSelectedItemIndex(index, juce::NotificationType::dontSendNotification);
    };

    nextButton.setButtonText(">");
    nextButton.setLookAndFeel(&lnf);
    addAndMakeVisible(nextButton);
    nextButton.onClick = [&]()
    {
        const int index{ manager.nextPreset() };
        presetMenu.setSelectedItemIndex(index, juce::NotificationType::dontSendNotification);
    };

    updatePresetMenu();
    presetMenu.setTextWhenNothingSelected("-init-");
    presetMenu.setTextWhenNoChoicesAvailable("No saved presets");
    const auto presetMenuIndex{ manager.presetList.indexOf(manager.currentPreset.toString()) };
    if (presetMenuIndex == -1) { presetMenu.setSelectedItemIndex(presetMenuIndex); }
    else { presetMenu.setSelectedItemIndex(presetMenuIndex + manager.presetListIdOffset); }    
    presetMenu.setLookAndFeel(&lnf);
    addAndMakeVisible(presetMenu);
    presetMenu.onChange = [&]()
    {
        if (presetMenu.getSelectedId() == PresetMenuIDs::New)
        {
            presetMenu.setSelectedId(0, juce::NotificationType::dontSendNotification);
            manager.newPreset();
        }
        else if (presetMenu.getSelectedId() == PresetMenuIDs::Save)
        {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Please enter the name of the preset to save",
                PresetManager::defaultDir,
                "*." + PresetManager::extention);
            fileChooser->launchAsync(juce::FileBrowserComponent::saveMode, [this](const juce::FileChooser& chooser)
                {
                    const auto fileToSave{ chooser.getResult() };
                    manager.savePreset(fileToSave.getFileNameWithoutExtension());
                    updatePresetMenu();
                    presetMenu.setSelectedItemIndex(manager.presetList.indexOf(
                        manager.currentPreset.toString()) + manager.presetListIdOffset);
                });

        }
        else if (presetMenu.getSelectedId() == PresetMenuIDs::Load)
        {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Please choose the preset to load",
                PresetManager::defaultDir,
                "*." + PresetManager::extention);
            fileChooser->launchAsync(juce::FileBrowserComponent::openMode, [this](const juce::FileChooser& chooser)
                {
                    const auto fileToLoad{ chooser.getResult() };
                    manager.loadPreset(fileToLoad.getFileNameWithoutExtension());
                    presetMenu.setSelectedItemIndex(manager.presetList.indexOf(
                        manager.currentPreset.toString()) + manager.presetListIdOffset);
                });
        }
        else if (presetMenu.getSelectedId() == PresetMenuIDs::Delete)
        {
            presetMenu.setSelectedId(0, juce::NotificationType::dontSendNotification);
            manager.deletePreset(manager.currentPreset.toString());
            updatePresetMenu();
        }
        else // пресеты
        {
            manager.loadPreset(presetMenu.getItemText(presetMenu.getSelectedItemIndex()));
        }
    };
}

void PresetPanel::updatePresetMenu()
{
    presetMenu.clear(juce::NotificationType::dontSendNotification);
    presetMenu.addItem("New preset", PresetMenuIDs::New);
    presetMenu.addItem("Save preset...", PresetMenuIDs::Save);
    presetMenu.addItem("Load preset...", PresetMenuIDs::Load);
    presetMenu.addItem("Delete preset", PresetMenuIDs::Delete);
    presetMenu.addSeparator();
    presetMenu.addItemList(manager.presetList, PresetMenuIDs::PresetList);
}

void PresetPanel::resized()
{
    auto bounds{ getLocalBounds() };
    const auto prop{ bounds };
    const int componentWidth{ 51 };
    const int spacing{ 6 };
    previousButton.setBounds(bounds.removeFromLeft(componentWidth));
    presetMenu.setBounds(bounds.removeFromLeft(componentWidth * 4));
    nextButton.setBounds(bounds.removeFromLeft(componentWidth));
}
//==============================================================================
DistortionTestAudioProcessorEditor::DistortionTestAudioProcessorEditor (DistortionTestAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), presetPanel(newLNF, audioProcessor.getPresetManager())
{
    setSize (660, 210);
    juce::Font font{ juce::Typeface::createSystemTypefaceFor(BinaryData::MagistralTT_ttf, BinaryData::MagistralTT_ttfSize) };
    font.setHeight(18.0f);
    addAndMakeVisible(presetPanel);
    //==================================================
    // clipperBox settings
    clipperBox.addItem("Hard Clip", hard);
    clipperBox.addItem("Soft Clip", soft);
    clipperBox.addItem("Fold Back", foldback);
    clipperBox.addItem("Sine Fold", sinefold);
    clipperBox.addItem("Linear Fold", linearfold);
    clipperBox.setSelectedItemIndex(1);
    clipperBox.onChange = [this]()
    {
        audioProcessor.clipHolder.setClipper(clipperBox.getSelectedItemIndex());
        audioProcessor.clipHolder.getClipper()->updateMultiplier(clipSlider.slider.getValue());
        graph.initialize(audioProcessor.clipHolder.getClipper());
        graph.update();
    };
    clipperBox.setLookAndFeel(&newLNF);
    addAndMakeVisible(clipperBox);
    //==================================================
    // rotary parameters
    juce::Slider::RotaryParameters rotaryParameters;
    rotaryParameters.startAngleRadians = juce::MathConstants<float>::twoPi - 3 * 0.25f * juce::MathConstants<float>::pi;
    rotaryParameters.endAngleRadians = juce::MathConstants<float>::twoPi + 3 * 0.25f * juce::MathConstants<float>::pi;
    rotaryParameters.stopAtEnd = true;
    //==================================================
    // sliders settings
    inputGainSlider.initialize(rotaryParameters, -12.0, 12.0, 0.0, "dB", "IN", &newLNF, font);
    outputGainSlider.initialize(rotaryParameters, -12.0, 12.0, 0.0, "dB", "OUT", &newLNF, font);
    clipSlider.initialize(rotaryParameters, 1.0, 10.0, 1.0, "", "CLIP", &newLNF, font);
    inputGainSlider.slider.onValueChange = [this]()
    {
        double newValue{ inputGainSlider.slider.getValue() };
        inputGainSlider.valueText.setText(juce::String(newValue, 1) + " dB",
                                          juce::NotificationType::dontSendNotification);
        audioProcessor.gainController.setInputGainLevelInDb(newValue);
        if (linkButton.getToggleState())
        {
            outputGainSlider.slider.setValue(-newValue);
        }
    };
    outputGainSlider.slider.onValueChange = [this]()
    {
        double newValue{ outputGainSlider.slider.getValue() };
        outputGainSlider.valueText.setText(juce::String(newValue, 1) + " dB",
                                           juce::NotificationType::dontSendNotification);
        audioProcessor.gainController.setOutputGainLevelInDb(newValue);
        if (linkButton.getToggleState())
        {
            inputGainSlider.slider.setValue(-newValue);
        }
    };
    clipSlider.slider.onValueChange = [this]()
    {
        double newValue{ clipSlider.slider.getValue() };
        clipSlider.valueText.setText(juce::String(newValue, 1), juce::NotificationType::dontSendNotification);
        audioProcessor.clipHolder.getClipper()->updateMultiplier(newValue);
        graph.update();
    };
    addAndMakeVisible(inputGainSlider);
    addAndMakeVisible(outputGainSlider);
    addAndMakeVisible(clipSlider);
    //==================================================
    // linkButton settings
    linkButton.setToggleState(true, juce::NotificationType::sendNotification);
    linkButton.onStateChange = [this]()
    {
        if (linkButton.getToggleState())
        {
            if (inputGainSlider.slider.getValue() < 0)
            {
                inputGainSlider.slider.setValue(-outputGainSlider.slider.getValue());
            }
            else { outputGainSlider.slider.setValue(-inputGainSlider.slider.getValue()); }
        }
    };
    linkButton.setLookAndFeel(&newLNF);
    addAndMakeVisible(linkButton);
    //==================================================
    // bypasskButton settings
    bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
    bypassButton.onStateChange = [this]() { audioProcessor.gainController.setBypassState(bypassButton.getToggleState()); };
    bypassButton.setLookAndFeel(&newLNF);
    addAndMakeVisible(bypassButton);
    //==================================================
    // graph settings
    graph.startTimerHz(60);
    graph.initialize(audioProcessor.clipHolder.getClipper());
    graph.label.setFont(font);
    graph.addAndMakeVisible(graph.label);
    addAndMakeVisible(graph);
    //==================================================
    // attachment settings
    inputGainAttach = std::make_unique<APVTS::SliderAttachment>(audioProcessor.apvts, "Input Gain", inputGainSlider.slider);
    outputGainAttach = std::make_unique<APVTS::SliderAttachment>(audioProcessor.apvts, "Output Gain", outputGainSlider.slider);
    clipAttach = std::make_unique<APVTS::SliderAttachment>(audioProcessor.apvts, "Clip", clipSlider.slider);
    bypassAttach = std::make_unique<APVTS::ButtonAttachment>(audioProcessor.apvts, "Bypass", bypassButton);
    linkAttach = std::make_unique<APVTS::ButtonAttachment>(audioProcessor.apvts, "Link", linkButton);
    clipperBoxAttach = std::make_unique<APVTS::ComboBoxAttachment>(audioProcessor.apvts, "Clipper Type", clipperBox);
}

DistortionTestAudioProcessorEditor::~DistortionTestAudioProcessorEditor()
{
}

//==============================================================================
void DistortionTestAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void DistortionTestAudioProcessorEditor::resized()
{
    int spacing{ 10 };
    int buttonHeight{ 22 };
    int componentWidth{ 100 };
    auto bounds{ getLocalBounds() };
    auto header{ bounds.removeFromTop(40) }; // под лого и название
    bounds.reduce(spacing, spacing);
    outputGainSlider.setBounds(bounds.removeFromRight(componentWidth));
    bounds.removeFromRight(spacing);
    clipSlider.setBounds(bounds.removeFromRight(componentWidth));
    bounds.removeFromRight(spacing);
    inputGainSlider.setBounds(bounds.removeFromRight(componentWidth));
    bounds.removeFromRight(spacing);
    graph.setBounds(bounds.removeFromTop(bounds.getHeight() - spacing - buttonHeight));
    bounds.removeFromTop(spacing);
    clipperBox.setBounds(bounds.removeFromLeft(componentWidth));
    bounds.removeFromLeft(spacing);
    bypassButton.setBounds(bounds.removeFromLeft(componentWidth));
    bounds.removeFromLeft(spacing);
    linkButton.setBounds(bounds.removeFromLeft(componentWidth));
    presetPanel.setBounds(header.removeFromRight(getWidth() * 0.5f).reduced(9));
}