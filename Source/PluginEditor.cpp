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
                                            float rotaryEndAngle, juce::Slider&)
{
    float arcThickness{ 6.0f };
    float radius{ juce::jmin<float>(static_cast<float>(width), static_cast<float>(height)) };
    auto bounds{ juce::Rectangle<int>(x, y, width, height).toFloat().withSizeKeepingCentre(radius, radius).reduced(arcThickness * 0.5f) };
    auto knobBounds{ bounds.reduced(arcThickness + 2) };
    // заливка ручки
    juce::ColourGradient gradient;
    gradient.addColour(0.0, juce::Colours::black.contrasting(0.25f));
    gradient.addColour(0.1, juce::Colours::black.contrasting(0.25f));
    gradient.addColour(0.9, juce::Colours::black.contrasting(0.12f));
    gradient.point1 = knobBounds.getTopLeft();
    gradient.point2 = knobBounds.getBottomLeft();
    g.setGradientFill(gradient);
    g.fillEllipse(knobBounds);
    gradient.clearColours();
    // добавление точки на ручке
    auto remappedAngle{ juce::jmap(sliderPosProportional, rotaryStartAngle, rotaryEndAngle) };
    auto dot{ juce::Rectangle<float>(0.0f, 0.0f, arcThickness - 2.0f, arcThickness - 2.0f) };
    auto dotPosition{ juce::Point<float>(bounds.getCentreX(), bounds.getCentreY())
        .getPointOnCircumference(knobBounds.getHeight() * 0.5f - 10.0f, remappedAngle) };
    dot.setCentre(dotPosition.x, dotPosition.y);
    g.setColour(juce::Colours::orange);
    g.fillEllipse(dot);
    juce::Path glowPath;
    glowPath.addRectangle(dot);
    juce::DropShadow dotGlow{ juce::Colours::orange, 5, juce::Point<int>(0, 0) };
    dotGlow.drawForPath(g, glowPath);
    // добавление арок вокруг ручки
    juce::Path path;
    float startAngle{ 0.0f };
    float endAngle{ 0.0f };
    for (int i = 0; i < 6; ++i)
    {
        startAngle = rotaryStartAngle + i * juce::MathConstants<float>::pi * 0.25f + 0.05f;
        endAngle = rotaryStartAngle + (i + 1) * juce::MathConstants<float>::pi * 0.25f - 0.05f;
        path.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                           bounds.getWidth() * 0.5f, bounds.getHeight() * 0.5f,
                           0.0f, startAngle, endAngle, true);
    }
    g.setColour(juce::Colours::black.contrasting(0.3f));
    g.strokePath(path, juce::PathStrokeType(arcThickness));
    juce::Path clipPath;
    clipPath.addPieSegment(bounds.expanded(arcThickness * 2), rotaryStartAngle, remappedAngle, 0.0f);
    g.reduceClipRegion(clipPath);
    g.setColour(juce::Colours::orange);
    g.strokePath(path, juce::PathStrokeType(arcThickness));
}

void XcytheLookAndFeel_v1::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    /* Переопределение данной функции необходимо по следующей причине.
    При создании кнопки внутри окна AlertWindow, её конструирование происходит
    на основе вызова функции paintButton(), задействующей внутри себя вызов
    функций drawButtonBackground() и drawButtonText() класса LookAndFeel.
    Для обычных кнопок в MainComponent данный метод, судя по всему,
    не используется автоматически, либо используется с текущим LookAndFeel.
    В случае AlertWindow происходит дублирование текста на кнопке из-за 
    отрисовки текста внутри метода drawButtonBackground(). 
    Судя по всему, кнопки внутри AlertWindow и MainComponent относятся к 
    разным классам. Прописывание отрисовки текста в данной функции не 
    влечёт за собой отрисовку текста на основных кнопках, но при этом 
    добавляет его на кнопки AlertWindow. Поэтому имплементация этой 
    функции пустая, а отрисовка текста перенесена в drawButtonInternal(),
    что само по себе, вероятно, семантически не корректно. */
}

void XcytheLookAndFeel_v1::drawToggleButton(juce::Graphics& g, juce::ToggleButton& togglebutton,
                                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    drawButtonInternal(g, togglebutton.getLocalBounds().toFloat(), FrameOrientation::None,togglebutton.getButtonText(),
                       togglebutton.getToggleState(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void XcytheLookAndFeel_v1::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto orientation = FrameOrientation::None;
    if (button.getButtonText() == "<") { orientation = FrameOrientation::Left; }
    else if (button.getButtonText() == ">") { orientation = FrameOrientation::Right; }
    drawButtonInternal(g, button.getLocalBounds().toFloat(), orientation, button.getButtonText(),
                       button.getToggleState(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void XcytheLookAndFeel_v1::drawButtonInternal(juce::Graphics& g, juce::Rectangle<float> bounds,
                                              FrameOrientation orientation, const juce::String& text, bool toggleState,
                                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    float lineThickness{ 1.0f };
    bounds.reduce(lineThickness * 0.5f, lineThickness * 0.5f);
    juce::Path contour{ createFrame(bounds, orientation) };
    // определяем цвет в зависимости от состояния кнопки
    auto baseColor{ juce::Colours::darkgrey.withAlpha(0.5f) };
    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
    {
        baseColor = baseColor.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);
    }
    g.setColour(baseColor);
    g.fillPath(contour);
    g.setColour(juce::Colours::white);
    g.setFont(font);
    g.drawText(text, bounds, juce::Justification::centred);
    if (toggleState) { g.setColour(juce::Colours::orange); }
    g.strokePath(contour, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved));
}

void XcytheLookAndFeel_v1::drawComboBox(juce::Graphics& g, int, int, bool,
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
                                             const bool, const juce::String& text,
                                             const juce::String& shortcutKeyText,
                                             const juce::Drawable*, const juce::Colour* const textColourToUse)
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

void XcytheLookAndFeel_v1::drawAlertBox(juce::Graphics& g, juce::AlertWindow& alert,
                                        const juce::Rectangle<int>& textArea, juce::TextLayout& textLayout)
{
    float cornerSize{ 6.0f };
    auto bounds{ alert.getLocalBounds().toFloat() };
    juce::ColourGradient gradient;
    gradient.addColour(0.00, juce::Colours::black.contrasting(0.20f));
    gradient.addColour(0.25, juce::Colours::black.contrasting(0.05f));
    gradient.addColour(0.75, juce::Colours::black.contrasting(0.05f));
    gradient.addColour(1.00, juce::Colours::black.contrasting(0.20f));
    gradient.point1 = bounds.getBottomLeft();
    gradient.point2 = bounds.getTopLeft();
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, cornerSize);
    gradient.clearColours();
    g.setColour(juce::Colours::orange);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    g.setColour(juce::Colours::white);
    bounds.reduce(10.0f, 10.0f);
    textLayout.draw(g, bounds);
}

juce::Font XcytheLookAndFeel_v1::getTextButtonFont(juce::TextButton& button, int buttonHeight) { return getAlertWindowTitleFont(); }

juce::Font XcytheLookAndFeel_v1::getAlertWindowTitleFont() { return font.withHeight(22.0f).withStyle(juce::Font::bold); }

juce::Font XcytheLookAndFeel_v1::getAlertWindowMessageFont() { return font.withHeight(18.0f); }

juce::Font XcytheLookAndFeel_v1::getAlertWindowFont() { return font.withHeight(16.0f); }

int XcytheLookAndFeel_v1::getAlertWindowButtonHeight() { return 22; }

juce::Array<int> XcytheLookAndFeel_v1::getWidthsForTextButtons(juce::AlertWindow& aw, const juce::Array<juce::TextButton*>& tb)
{
    juce::Array<int> buttonWidths;
    for (int i = 0; i < tb.size(); ++i) { buttonWidths.add(100); }
    return buttonWidths;
}

juce::Path XcytheLookAndFeel_v1::createFrame(const juce::Rectangle<float>& bounds, FrameOrientation orientation)
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
    g.setColour(juce::Colours::orange);
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
    addAndMakeVisible(presetNameLabel); // определяется за полупрозрачным комбобоксом

    previousButton.setButtonText("<");
    previousButton.setLookAndFeel(&lnf);
    addAndMakeVisible(previousButton);
    previousButton.onClick = [&]()
    {
        const int index{ manager.previousPreset() };
        presetNameLabel.setText(manager.currentPreset.toString(), juce::NotificationType::dontSendNotification);
    };

    nextButton.setButtonText(">");
    nextButton.setLookAndFeel(&lnf);
    addAndMakeVisible(nextButton);
    nextButton.onClick = [&]()
    {
        const int index{ manager.nextPreset() };
        presetNameLabel.setText(manager.currentPreset.toString(), juce::NotificationType::dontSendNotification);
    };

    updatePresetMenu();
    presetMenu.setTextWhenNothingSelected("");
    const auto presetMenuIndex{ manager.presetList.indexOf(manager.currentPreset.toString()) };
    juce::String initialPresetName;
    if (presetMenuIndex == -1) { initialPresetName = juce::String("-init-"); }
    else { initialPresetName = manager.currentPreset.toString(); }
    presetNameLabel.setText(initialPresetName, juce::NotificationType::dontSendNotification);
    presetMenu.setLookAndFeel(&lnf);
    addAndMakeVisible(presetMenu);
    presetMenu.onChange = [this]()
    {
        switch (presetMenu.getSelectedId())
        {
        case (PresetMenuIDs::NoSelect): break;
        case (PresetMenuIDs::New):
        {
            presetNameLabel.setText(juce::String("-init-"), juce::NotificationType::dontSendNotification);
            presetMenu.setSelectedId(0, juce::NotificationType::dontSendNotification);
            manager.newPreset();
            break;
        }
        case (PresetMenuIDs::Save):
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
                    presetNameLabel.setText(manager.currentPreset.toString(), juce::NotificationType::dontSendNotification);
                    presetMenu.setSelectedId(0, juce::NotificationType::dontSendNotification);
                });
            break;
        }
        case (PresetMenuIDs::Load):
        {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Please choose the preset to load",
                PresetManager::defaultDir,
                "*." + PresetManager::extention);
            fileChooser->launchAsync(juce::FileBrowserComponent::openMode, [this](const juce::FileChooser& chooser)
                {
                    const auto fileToLoad{ chooser.getResult() };
                    manager.loadPreset(fileToLoad.getFileNameWithoutExtension());
                    manager.currentPreset.setValue(manager.currentPreset.toString());
                    presetNameLabel.setText(manager.currentPreset.toString(), juce::NotificationType::dontSendNotification);
                    presetMenu.setSelectedId(0, juce::NotificationType::dontSendNotification);
                });
            break;
        }
        case (PresetMenuIDs::Delete):
        {
            if (presetNameLabel.getText() == "-init-")
            {
                presetMenu.setSelectedId(0);
                return;
            }
            juce::MessageBoxOptions mbo
            {
                juce::MessageBoxOptions()
                .withTitle("")
                .withMessage("Do you want to delete current preset?")
                .withIconType(juce::MessageBoxIconType::NoIcon)
                .withAssociatedComponent(nullptr)
            };
            /* AlertWindow определяется как unique_ptr для того,
            чтобы вызвать внутри лямбды ModalCallbackFunction.
            ПРИМЕЧАНИЕ! Сначала применяется LookAndFeel, затем
            добавляются кнопки, потому что функция addButton()
            внутри вызывает текущий LookAndFeel, в котором определена
            ширина кнопок для корректного размещения в окне. */
            aw = std::make_unique<juce::AlertWindow>(mbo.getTitle(),
                mbo.getMessage(),
                mbo.getIconType(),
                mbo.getAssociatedComponent());
            aw->setLookAndFeel(&lnf);
            aw->addButton("Yes", 1);
            aw->addButton("No", 0);
            aw->enterModalState(true, juce::ModalCallbackFunction::create([this](int result)
                {
                    presetMenu.setSelectedId(0, juce::NotificationType::dontSendNotification);
                    if (result == 1)
                    {
                        manager.deletePreset(presetNameLabel.getText());
                        updatePresetMenu();
                        presetNameLabel.setText("-init-", juce::NotificationType::dontSendNotification);
                    }
                    aw->exitModalState(result);
                    aw->setVisible(false);
                }));
            break;
        }
        default:
        {
            auto presetName{ presetMenu.getItemText(presetMenu.getSelectedItemIndex()) };
            manager.loadPreset(presetName);
            manager.currentPreset.setValue(presetName);
            presetNameLabel.setText(presetName, juce::NotificationType::dontSendNotification);
            presetMenu.setSelectedId(0, juce::NotificationType::dontSendNotification);
            break;
        }
        } // end switch
    };
}

juce::Label* PresetPanel::getPresetNameLabel() { return &presetNameLabel; }

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
    presetNameLabel.setBounds(presetMenu.getBounds());
}
//==============================================================================
void Plate::paint(juce::Graphics& g)
{
    auto bounds{ getLocalBounds().toFloat() };
    juce::ColourGradient gradient;
    gradient.addColour(0.00, juce::Colours::black.contrasting(0.20f));
    gradient.addColour(0.25, juce::Colours::black.contrasting(0.05f));
    gradient.addColour(0.75, juce::Colours::black.contrasting(0.05f));
    gradient.addColour(1.00, juce::Colours::black.contrasting(0.20f));
    gradient.point1 = bounds.getBottomLeft();
    gradient.point2 = bounds.getTopLeft();
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 6.0f);
    gradient.clearColours();
}
//==============================================================================
DestructionAudioProcessorEditor::DestructionAudioProcessorEditor (DestructionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), presetPanel(newLNF, audioProcessor.getPresetManager())
{
    setSize (660, 210);
    juce::Font font{ juce::Typeface::createSystemTypefaceFor(BinaryData::MagistralTT_ttf, BinaryData::MagistralTT_ttfSize) };
    font.setHeight(18.0f);
    addAndMakeVisible(presetPanel);
    addAndMakeVisible(sliderPlate);
    addAndMakeVisible(graphPlate);
    addAndMakeVisible(pluginName);
    addAndMakeVisible(version);

    auto presetNameLabel{ presetPanel.getPresetNameLabel() };
    presetNameLabel->setFont(font);
    presetNameLabel->setJustificationType(juce::Justification::centred);
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
    //==================================================
    // header settings
    logo = juce::ImageCache::getFromMemory(BinaryData::Logo_transparent_png, BinaryData::Logo_transparent_pngSize);
    font.setHeight(26.0f);
    pluginName.setFont(font.withStyle(juce::Font::FontStyleFlags::italic));
    pluginName.setText(juce::String(ProjectInfo::projectName).toUpperCase(), juce::NotificationType::dontSendNotification);
    pluginName.setJustificationType(juce::Justification::centredLeft);
    version.setFont(font.withHeight(FONT_HEIGHT).withStyle(juce::Font::FontStyleFlags::plain));
    version.setText(juce::String("v.") + juce::String(ProjectInfo::versionString), juce::NotificationType::dontSendNotification);
    version.setJustificationType(juce::Justification::centred);
}

DestructionAudioProcessorEditor::~DestructionAudioProcessorEditor()
{
}

//==============================================================================
void DestructionAudioProcessorEditor::paint (juce::Graphics& g)
{
    /* Для отрисовки фона заголовка и тела окна плагина использован
    класс std::map<key, T>. Это карта, содержащая значения и их ключи.
    Заполнение карты производится с помощью функции emplace(),
    принимающей в себя объект типа std::pair<key, T>, который можно
    сгенерировать, используя функцию std::make_pair. Типы, указанные
    в шаблоне и передаваемые в функцию должны соответствовать. */
    auto bounds{ getLocalBounds().toFloat() };
    juce::ColourGradient gradient;
    std::map<double, juce::Colour> colors;
    colors.emplace(std::make_pair(0.0, juce::Colours::black.contrasting(0.2f)));
    colors.emplace(std::make_pair(0.5, juce::Colours::black));
    colors.emplace(std::make_pair(1.0, juce::Colours::black.contrasting(0.2f)));
    drawBackground(g, gradient, bounds.removeFromTop(40), colors);
    colors.clear();
    colors.emplace(std::make_pair(0.0, juce::Colours::black.contrasting(0.2f)));
    colors.emplace(std::make_pair(0.2, juce::Colours::black));
    drawBackground(g, gradient, bounds, colors);
    colors.clear();
    colors.emplace(std::make_pair(0.0, juce::Colours::orange.withAlpha(0.0f)));
    colors.emplace(std::make_pair(1.0, juce::Colours::orange.withAlpha(0.8f)));
    drawBackground(g, gradient, bounds.withHeight(8.0f), colors);

    juce::Rectangle<float> logoBounds{ 0.0f, 0.0f, 50.0f, 40.0f };
    g.drawImage(logo, logoBounds, juce::RectanglePlacement::centred, false);

    sliderPlateShadow = std::make_unique<juce::DropShadow>(juce::Colours::orange, 8, juce::Point<int>(5, 5));
    graphPlateShadow  = std::make_unique<juce::DropShadow>(juce::Colours::orange, 8, juce::Point<int>(5, 5));
    juce::Path path;
    drawShadows(g, path, sliderPlate.getBounds().reduced(3), sliderPlateShadow);
    drawShadows(g, path, graphPlate.getBounds().reduced(3), graphPlateShadow);
}

void DestructionAudioProcessorEditor::resized()
{
    int spacing{ 5 };
    int buttonHeight{ 22 };
    int plateReduction{ 10 };
    auto bounds{ getLocalBounds() };
    auto headerBounds{ bounds.removeFromTop(40) }; // под лого и название
    auto plateBounds{ bounds };    
    sliderPlate.setBounds(plateBounds.removeFromRight(bounds.proportionOfWidth(0.5)).reduced(plateReduction));
    graphPlate.setBounds(plateBounds.reduced(plateReduction));
    // заполняем sliderPlate
    auto staticBounds = plateBounds = sliderPlate.getBounds().reduced(spacing);
    outputGainSlider.setBounds(plateBounds.removeFromRight(staticBounds.proportionOfWidth(0.33)).reduced(spacing));
    clipSlider.setBounds(plateBounds.removeFromRight(staticBounds.proportionOfWidth(0.33)).reduced(spacing));
    inputGainSlider.setBounds(plateBounds.reduced(spacing));
    // заполняем graphPlate
    staticBounds = plateBounds = graphPlate.getBounds().reduced(spacing);
    graph.setBounds(plateBounds.removeFromTop(plateBounds.getHeight() - buttonHeight - 2 * spacing).reduced(spacing));
    linkButton.setBounds(plateBounds.removeFromRight(staticBounds.proportionOfWidth(0.3)).reduced(spacing));
    bypassButton.setBounds(plateBounds.removeFromRight(staticBounds.proportionOfWidth(0.3)).reduced(spacing));
    clipperBox.setBounds(plateBounds.reduced(spacing));
    presetPanel.setBounds(headerBounds.removeFromRight(headerBounds.proportionOfWidth(0.5)).reduced(9));
    headerBounds.removeFromLeft(50 + 10); // под лого
    pluginName.setBounds(headerBounds.removeFromLeft(200));
    version.setBounds(headerBounds);
}

void DestructionAudioProcessorEditor::drawBackground(juce::Graphics& g,
                                                        juce::ColourGradient& gradient,
                                                        const juce::Rectangle<float>& bounds,
                                                        std::map<double, juce::Colour>& colors)
{
    for (const auto& [proportion, color] : colors) { gradient.addColour(proportion, color); }        
    gradient.point1 = bounds.getBottomLeft();
    gradient.point2 = bounds.getTopLeft();
    g.setGradientFill(gradient);
    g.fillRect(bounds);
    gradient.clearColours();
}

void DestructionAudioProcessorEditor::drawShadows(juce::Graphics& g,
                                                  juce::Path& path,
                                                  const juce::Rectangle<int>& bounds,
                                                  std::unique_ptr<juce::DropShadow>& shadow)
{
    path.addRoundedRectangle(bounds, 6.0f);
    shadow->drawForPath(g, path);
    path.clear();
}