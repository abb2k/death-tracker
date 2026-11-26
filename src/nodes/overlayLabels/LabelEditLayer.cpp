#include "LabelEditLayer.hpp"

#include <regex>

#include <utils/Save.hpp>
#include <utils/Settings.hpp>
#include <utils/Dev.hpp>

#include <utils/RichTextKeys.hpp>
#include <nodes/DTLayer.hpp>
#include <nodes/Tutorial/TutorialButton.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>

#include <managers/AuthManager.hpp>

#include <nodes/overlayLabels/LabelEditSpecialKeyEditorPopup.hpp>
#include <nodes/overlayLabels/OverlayLabelCell.hpp>
#include <nodes/general/SimpleToggler.hpp>

#include <nodes/general/RichTextArea.hpp>
#include <managers/StatsManager.hpp>

LabelEditLayer* LabelEditLayer::create() {
    auto ret = new LabelEditLayer();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool LabelEditLayer::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!Popup::init(winSize.width - 50, winSize.height - 50, "geode.loader/GE_square01.png"))
        return false;

    m_previewArea = CCClippingNode::create();
    m_previewArea->setZOrder(1);
    m_mainLayer->addChildAtPosition(m_previewArea, Anchor::TopRight, {-15, -15});

    CCPoint previewSize = {m_size.width / 1.5f, m_size.height / 1.75f};

    m_previewArea->setContentSize(previewSize);
    m_previewArea->setAnchorPoint({1, 1});

    auto stencil = CCLayerColor::create();
    stencil->setContentSize(m_previewArea->getContentSize());
    stencil->setAnchorPoint({0, 0});
    stencil->setPosition({0, 0});
    stencil->ignoreAnchorPointForPosition(false);

    m_previewArea->setStencil(stencil);
    m_mainLayer->addChild(stencil);

    m_previewAreaMenu = CCMenu::create();
    m_previewAreaMenu->setContentSize(m_previewArea->getContentSize());
    m_previewAreaMenu->setZOrder(1);
    m_previewArea->addChildAtPosition(m_previewAreaMenu, Anchor::Center);

    createAnchorButton(Anchor::BottomLeft, "BL");
    createAnchorButton(Anchor::TopLeft, "TL");
    createAnchorButton(Anchor::TopRight, "TR");
    createAnchorButton(Anchor::BottomRight, "BR");
    createAnchorButton(Anchor::Bottom, "B");
    createAnchorButton(Anchor::Top, "T");
    createAnchorButton(Anchor::Right, "R");
    createAnchorButton(Anchor::Left, "L");

    auto outline = CCLayerColor::create({0, 0, 0, 255});
    outline->setPosition(m_previewArea->getPosition() - m_previewArea->getContentSize() / 2);
    outline->setContentSize(m_previewArea->getContentSize() + ccp(4.5f, 4.5f));
    outline->ignoreAnchorPointForPosition(false);
    m_mainLayer->addChild(outline);

    ccColor3B bgColor = {40, 125, 255};

    auto bg = CCSprite::create("game_bg_01_001.png");
    bg->setPosition(m_previewArea->getContentSize() / 2);
    bg->setColor(bgColor);
    m_previewArea->addChild(bg);

    auto bg1 = CCSprite::create("game_bg_01_001.png");
    bg1->setAnchorPoint({1, .5f});
    bg1->setColor(bgColor);
    bg->addChildAtPosition(bg1, Anchor::Left);

    auto bg2 = CCSprite::create("game_bg_01_001.png");
    bg2->setAnchorPoint({0, .5f});
    bg2->setColor(bgColor);
    bg->addChildAtPosition(bg2, Anchor::Right);

    auto ground = GJGroundLayer::create(1, 0);
    ground->setPosition({0, 30});
    ground->setScale(0.64f);
    m_previewArea->addChild(ground);

    constexpr float settingsPanelHeight = 78.f;
    auto labelListSize = CCSize{
        m_size.width / 4.f,
        m_size.height - settingsPanelHeight - 40.f
    };
    m_scroll = ScrollLayer::create(labelListSize);
    m_scroll->setAnchorPoint({0, 1});
    m_scroll->ignoreAnchorPointForPosition(false);
    m_scroll->setZOrder(1);
    m_mainLayer->addChildAtPosition(m_scroll, Anchor::TopLeft, {15, -15});

    m_scroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_scroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisOverflow(false)
        ->ignoreInvisibleChildren(true)
    );

    auto scrollBG = CCScale9Sprite::create("square02_small.png");
    scrollBG->setPosition(m_scroll->getPosition() + ccp(m_scroll->getScaledContentWidth() / 2, -m_scroll->getScaledContentHeight() / 2));
    scrollBG->setContentSize(m_scroll->getContentSize() + ccp(5, 5));
    scrollBG->setOpacity(150);
    m_mainLayer->addChild(scrollBG);

    m_labelsScrollbar = Scrollbar::create(m_scroll);
    m_labelsScrollbar->setAnchorPoint({0, .5f});
    m_labelsScrollbar->ignoreAnchorPointForPosition(false);
    m_labelsScrollbar->setPosition({
        m_scroll->getPositionX() + m_scroll->getScaledContentWidth() + 4,
        m_scroll->getPositionY() - m_scroll->getScaledContentHeight() / 2
    });
    m_mainLayer->addChild(m_labelsScrollbar);

    auto settingsMenu = CCMenu::create();
    settingsMenu->setContentSize({labelListSize.width, settingsPanelHeight});
    settingsMenu->setAnchorPoint({0, 0});
    settingsMenu->ignoreAnchorPointForPosition(false);
    settingsMenu->setZOrder(1);
    m_mainLayer->addChildAtPosition(settingsMenu, Anchor::BottomLeft, {15, 15});

    auto settingsBG = CCScale9Sprite::create("square02_small.png");
    settingsBG->setPosition(settingsMenu->getPosition() + settingsMenu->getContentSize() / 2);
    settingsBG->setContentSize(settingsMenu->getContentSize() + ccp(5, 5));
    settingsBG->setOpacity(150);
    m_mainLayer->addChild(settingsBG);

    auto addGlobalSettingRow = [&](const char* title, float y, TextInput*& input, Slider*& slider, auto sliderCallback) {
        auto label = geode::Label::create(title, "bigFont.fnt");
        label->setScale(.28f);
        label->setAnchorPoint({0, .5f});
        label->setPosition({5, y});
        settingsMenu->addChild(label);
        
        input = TextInput::create(60, title);
        input->setScale(.45f);
        input->setAnchorPoint({1, .5f});
        input->setCommonFilter(CommonFilter::Float);
        input->setPosition({settingsMenu->getContentWidth() - 5, y});
        settingsMenu->addChild(input);

        slider = Slider::create(this, sliderCallback);
        slider->setScale(.5f);
        slider->setPosition({
            settingsMenu->getContentWidth() / 2,
            y - slider->m_sliderBar->getContentHeight() / 2 - input->getScaledContentHeight() / 2 - 2.5f
        });
        slider->setContentSize({0, 0});
        settingsMenu->addChild(slider);
    };

    addGlobalSettingRow("Offset:", 70, m_offsetInput, m_offsetSlider, menu_selector(LabelEditLayer::onOffsetSliderChanged));
    addGlobalSettingRow("Size:", 44, m_sizeInput, m_sizeSlider, menu_selector(LabelEditLayer::onSizeSliderChanged));
    addGlobalSettingRow("Opacity:", 19, m_opacityInput, m_opacitySlider, menu_selector(LabelEditLayer::onOpacitySliderChanged));
    m_opacityInput->setCommonFilter(CommonFilter::Uint);

    m_offsetInput->setCallback([this](const std::string& value) {
        if (m_updatingControls) return;

        auto parsedValue = geode::utils::numFromString<float>(value);
        if (parsedValue.isErr()) return;

        auto offset = parsedValue.unwrap();
        if (offset < m_OFFSET_SLIDER_MIN || offset > m_OFFSET_SLIDER_MAX) return;
        m_updatingControls = true;
        m_offsetSlider->setValue((offset - m_OFFSET_SLIDER_MIN) / (m_OFFSET_SLIDER_MAX - m_OFFSET_SLIDER_MIN));
        m_updatingControls = false;

        auto settings = Save::getOverlayLabels();
        if (settings.m_offset.x == offset && settings.m_offset.y == offset) return;
        settings.m_offset = ccp(offset, offset);
        Save::setOverlayLabels(settings);
    });

    m_sizeInput->setCallback([this](const std::string& value) {
        if (m_updatingControls) return;

        auto parsedValue = geode::utils::numFromString<float>(value);
        if (parsedValue.isErr()) return;
        auto size = parsedValue.unwrap();
        if (size < OverlayLabelSettings::MIN_SIZE || size > OverlayLabelSettings::MAX_SIZE) return;
        setCurrentLabelSize(size, false);
    });

    m_opacityInput->setCallback([this](const std::string& value) {
        if (m_updatingControls) return;

        auto parsedValue = geode::utils::numFromString<int>(value);
        if (parsedValue.isErr()) return;
        auto opacity = parsedValue.unwrap();
        if (opacity < OverlayLabelSettings::MIN_OPACITY || opacity > OverlayLabelSettings::MAX_OPACITY) return;
        setCurrentLabelOpacity(opacity, false);
    });

    auto initialSettings = Save::getOverlayLabels();
    auto initialOffset = std::clamp(
        (initialSettings.m_offset.x + initialSettings.m_offset.y) / 2.f,
        m_OFFSET_SLIDER_MIN,
        m_OFFSET_SLIDER_MAX
    );
    m_updatingControls = true;
    m_offsetInput->setString(fmt::format("{:.1f}", initialOffset));
    m_sizeInput->setString(fmt::format("{:.2f}", initialSettings.m_size));
    m_opacityInput->setString(fmt::format("{}", initialSettings.m_opacity));
    m_offsetSlider->setValue(
        (initialOffset - m_OFFSET_SLIDER_MIN) / (m_OFFSET_SLIDER_MAX - m_OFFSET_SLIDER_MIN)
    );
    m_sizeSlider->setValue(
        (initialSettings.m_size - OverlayLabelSettings::MIN_SIZE) /
        (OverlayLabelSettings::MAX_SIZE - OverlayLabelSettings::MIN_SIZE)
    );
    m_opacitySlider->setValue(
        static_cast<float>(initialSettings.m_opacity - OverlayLabelSettings::MIN_OPACITY) /
        (OverlayLabelSettings::MAX_OPACITY - OverlayLabelSettings::MIN_OPACITY)
    );
    m_updatingControls = false;

    if (initialSettings.m_offset.x != initialOffset || initialSettings.m_offset.y != initialOffset) {
        initialSettings.m_offset = ccp(initialOffset, initialOffset);
        Save::setOverlayLabels(initialSettings);
    }

    m_noLabelsText = geode::Label::create("No Labels\nCreated!", "bigFont.fnt");
    m_noLabelsText->setScale(.5f);
    m_noLabelsText->setPosition(scrollBG->getPosition());
    m_noLabelsText->setAlignment(geode::Label::Alignment::Center);
    m_mainLayer->addChild(m_noLabelsText);

    auto sideMenu = CCMenu::create();
    sideMenu->setPosition({0, 0});
    sideMenu->setZOrder(1);
    m_mainLayer->addChild(sideMenu);

    auto addLabelButtonSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    addLabelButtonSpr->setScale(.75f);
    auto addLabelButton = CCMenuItemSpriteExtra::create(
        addLabelButtonSpr,
        this,
        menu_selector(LabelEditLayer::onLabelAdded)
    );
    addLabelButton->setPosition(scrollBG->getPosition() + ccp(
        scrollBG->getScaledContentWidth() / 2,
        scrollBG->getScaledContentHeight() / 2
    ));
    sideMenu->addChild(addLabelButton);

    auto editBG = CCScale9Sprite::create("square02_small.png");
    editBG->setContentSize({
        previewSize.x,
        m_size.height - previewSize.y - 35
    });
    editBG->setOpacity(150);
    editBG->setAnchorPoint({1, 0});
    m_mainLayer->addChildAtPosition(editBG, Anchor::BottomRight, {-15, 15});

    m_editMenu = CCMenu::create();
    m_editMenu->setPosition(editBG->getPosition());
    m_editMenu->setAnchorPoint(editBG->getAnchorPoint());
    m_editMenu->setContentSize(editBG->getContentSize());
    m_editMenu->ignoreAnchorPointForPosition(false);
    m_mainLayer->addChild(m_editMenu);

    m_freeModeToggle = SimpleToggler::createWithDefaults(
        .5f
    );
    m_freeModeToggle->setCallback([this](bool state) {
        if (m_updatingControls || !m_currentEdit) return;

        auto label = m_currentEdit->getData();
        label.m_freeMode = state;
        m_currentEdit->setData(label);

        m_updatingControls = true;
        m_disabledAnchorSlidersFG->setVisible(!label.m_freeMode);
        if (label.m_freeMode) {
            m_xSlider->setValue(label.m_posX);
            m_ySlider->setValue(label.m_posY);
            Dev::setSliderEnabled(m_xSlider, true);
            Dev::setSliderEnabled(m_ySlider, true);
        }
        else {
            Dev::setSliderEnabled(m_xSlider, false);
            Dev::setSliderEnabled(m_ySlider, false);
        }
        m_updatingControls = false;

        updateAnchorButtons();
        updateSavedLabels();
    });
    m_editMenu->addChildAtPosition(m_freeModeToggle, Anchor::TopLeft, {m_editMenu->getContentWidth() / 4.5f, -m_freeModeToggle->getScaledContentHeight() / 2 - 5});

    auto freeModeToggleLabel = geode::Label::create("Free Mode", "bigFont.fnt");
    freeModeToggleLabel->setPosition(m_freeModeToggle->getPosition() + ccp(
        m_freeModeToggle->getScaledContentWidth() / 2 + 5,
        0
    ));
    freeModeToggleLabel->setAnchorPoint({0, .5f});
    freeModeToggleLabel->setScale(.5f);
    m_editMenu->addChild(freeModeToggleLabel);

    auto sliderHolder = CCNode::create();
    sliderHolder->setContentSize({m_editMenu->getContentWidth() / 1.6f, 20});
    sliderHolder->setAnchorPoint({.5f, .5f});
    m_editMenu->addChildAtPosition(sliderHolder, Anchor::TopLeft, {
        m_editMenu->getContentWidth() / 3.f,
        -sliderHolder->getScaledContentHeight() / 2 - freeModeToggleLabel->getScaledContentHeight() - 10
    });

    auto xLabel = geode::Label::create("X:", "bigFont.fnt");
    xLabel->setScale(.5f);
    xLabel->setAnchorPoint({0, .5f});
    sliderHolder->addChildAtPosition(xLabel, Anchor::Left);

    m_xSlider = Slider::create(this, menu_selector(LabelEditLayer::onSliderX));
    m_xSlider->setScale(.45f);
    m_xSlider->setPosition(xLabel->getPosition() + ccp(
        xLabel->getScaledContentWidth() + 15 + m_xSlider->m_sliderBar->getContentWidth() / m_xSlider->getScale() / 2,
        -1
    ));
    m_xSlider->setContentSize({0, 0});
    sliderHolder->addChild(m_xSlider);

    auto yLabel = geode::Label::create("Y:", "bigFont.fnt");
    yLabel->setScale(.5f);
    yLabel->setAnchorPoint({0, .5f});
    sliderHolder->addChildAtPosition(yLabel, Anchor::Left, {xLabel->getScaledContentWidth() + 15 + m_xSlider->m_sliderBar->getContentWidth() / m_xSlider->getScale() + 15, 0});

    m_ySlider = Slider::create(this, menu_selector(LabelEditLayer::onSliderY));
    m_ySlider->setScale(.45f);
    m_ySlider->setPosition(yLabel->getPosition() + ccp(
        yLabel->getScaledContentWidth() + 15 + m_ySlider->m_sliderBar->getContentWidth() / m_ySlider->getScale() / 2,
        -1
    ));
    m_ySlider->setContentSize({0, 0});
    sliderHolder->addChild(m_ySlider);

    m_disabledAnchorSlidersFG = CCScale9Sprite::create("square02_small.png");
    m_disabledAnchorSlidersFG->setZOrder(1);
    m_disabledAnchorSlidersFG->setAnchorPoint({0, .5f});
    m_disabledAnchorSlidersFG->setContentSize({
        (m_xSlider->m_sliderBar->getContentWidth() / m_xSlider->getScale()) * 2 + 15 * 2 + xLabel->getScaledContentWidth() * 2 + yLabel->getScaledContentWidth() * 2,
        m_ySlider->m_sliderBar->getContentHeight() / m_ySlider->getScale() + 5
    });
    m_disabledAnchorSlidersFG->setScale(.45f);
    m_disabledAnchorSlidersFG->setContentSize(m_disabledAnchorSlidersFG->getContentSize() / m_disabledAnchorSlidersFG->getScale());
    m_disabledAnchorSlidersFG->setOpacity(175);
    m_disabledAnchorSlidersFG->setPositionX(-2);
    m_disabledAnchorSlidersFG->setPositionY(sliderHolder->getContentHeight() / 2);
    sliderHolder->addChild(m_disabledAnchorSlidersFG);

    Dev::setSliderEnabled(m_xSlider, false);
    Dev::setSliderEnabled(m_ySlider, false);

    auto nameLabel = geode::Label::create("Name:", "bigFont.fnt");
    nameLabel->setScale(.5f);
    m_editMenu->addChildAtPosition(nameLabel, Anchor::TopRight, {-m_editMenu->getContentWidth() / 6, -nameLabel->getScaledContentHeight() / 2 - 5});

    m_nameInput = TextInput::create(80, "Name");
    m_nameInput->setScale(.75f);
    m_nameInput->setCommonFilter(CommonFilter::Name);
    m_nameInput->setMaxCharCount(12);
    m_nameInput->setCallback([this](const std::string& name) {
        if (m_updatingControls || !m_currentEdit) return;

        auto label = m_currentEdit->getData();
        label.m_labelName = name;
        m_currentEdit->setData(label);
        updateSavedLabels();
    });
    m_editMenu->addChildAtPosition(m_nameInput, Anchor::TopRight, {
        -m_editMenu->getContentWidth() / 6,
        -m_nameInput->getScaledContentHeight() / 2 - nameLabel->getScaledContentHeight() - 10
    });

    auto textLabel = geode::Label::create("Text:", "bigFont.fnt");
    textLabel->setScale(.5f);
    textLabel->setAnchorPoint({0, .5f});
    m_editMenu->addChildAtPosition(textLabel, Anchor::BottomLeft, {10, 15});

    auto richTextBtnSpr = ButtonSprite::create(
        "{ }",
        40,
        40,
        1,
        true,
        "goldFont.fnt",
        "GJ_button_03.png",
        40
    );
    richTextBtnSpr->setScale(.5f);
    richTextBtnSpr->setCascadeOpacityEnabled(true);
    m_richTextBtn = CCMenuItemSpriteExtra::create(
        richTextBtnSpr,
        this,
        menu_selector(LabelEditLayer::onRichTextBtn)
    );
    m_editMenu->addChild(m_richTextBtn);

    m_textInput = TextInput::create(
        (m_editMenu->getScaledContentWidth() - textLabel->getPositionX() - textLabel->getScaledContentWidth() - m_richTextBtn->getScaledContentWidth() - 25) / .75f,
        "Text"
    );
    m_textInput->setScale(.75f);
    m_textInput->setAnchorPoint({0, .5f});
    m_textInput->setPosition(textLabel->getPosition() + ccp(
        textLabel->getScaledContentWidth() + 5,
        0
    ));
    m_textInput->setCallback([this](const std::string& text) {
        if (m_updatingControls || !m_currentEdit) return;

        auto label = m_currentEdit->getData();
        label.m_text = text;
        m_currentEdit->setData(label);
        updateSavedLabels();
    });
    m_textInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_=<>. {}()%:");
    m_editMenu->addChild(m_textInput);

    m_richTextBtn->setPosition(m_textInput->getPosition() + ccp(
        m_textInput->getScaledContentWidth() + m_richTextBtn->getScaledContentWidth() / 2 + 5,
        0
    ));

    m_textInput->setFocusCallback([this](TextInput::FocusState focused) {
        if (focused == TextInput::FocusState::Focused) {
            m_prevCursorPos = m_textInput->getCursorPos();
            m_doUpdateCursorPos = true;
        } else {
            m_doUpdateCursorPos = false;
        }
    });

    m_noLabelEditedOverlay = CCScale9Sprite::create("square02_small.png");
    m_noLabelEditedOverlay->setContentSize(m_editMenu->getContentSize());
    m_noLabelEditedOverlay->setAnchorPoint(m_editMenu->getAnchorPoint());
    m_noLabelEditedOverlay->ignoreAnchorPointForPosition(false);
    m_noLabelEditedOverlay->setPosition(m_editMenu->getPosition());
    m_noLabelEditedOverlay->setOpacity(180);
    m_mainLayer->addChild(m_noLabelEditedOverlay);

    m_noLabelEditedText = geode::Label::create("No label edited", "bigFont.fnt");
    m_noLabelEditedText->setScale(.6f);
    m_noLabelEditedText->setAnchorPoint({.5f, .5f});
    m_noLabelEditedText->setPosition({
        m_editMenu->getContentWidth() / 2,
        m_editMenu->getContentHeight() / 2
    });
    m_noLabelEditedOverlay->addChild(m_noLabelEditedText);
    setLabelEditEnabled(false);

    auto labelSettings = Save::getOverlayLabels();
    for (auto const& label : labelSettings.m_labels) {
        auto labelCell = OverlayLabelCell::create(m_scroll->getContentWidth(), label);
        labelCell->onEditCallback([this](auto cell) { onCellEdit(cell); });
        labelCell->onDeleteCallback([this](auto cell) { onCellDelete(cell); });
        labelCell->onChangedCallback([this](auto cell) { onCellChanged(cell); });
        labelCell->onVisibilityChangedCallback([this](auto) {
            m_scroll->m_contentLayer->updateLayout();
            updateNoLabelsText();
        });
        m_scroll->m_contentLayer->addChild(labelCell);
        m_labels.push_back(labelCell);
    }
    m_scroll->m_contentLayer->updateLayout();
    updateNoLabelsText();

    m_overlayLabelsChangedListener = Save::OverlayLabelsChangedEvent().listen([&](){
        RefreshPreviewLabels();
        
        return ListenerResult::Propagate;
    });

    RefreshPreviewLabels();

    auto labelTutorial = TutorialButton::create(.75f, "label-edit", [this, addLabelButton, settingsBG, nameLabel, textLabel, xLabel, yLabel, freeModeToggleLabel](DTTutorialLayer* tutorial) {
        tutorial->appendDialogue("Welcome to the <cy>overlay label</c> editor!", TutorialCharacterFace::TCFHappy, {255, 255, 255})
            ->appendDialogue("Here is a list of <cy>all your labels</c>!! Add ones with the <cg>plus button</c>", TutorialCharacterFace::TCFNormal, {255, 255, 255})
            ->joinHighlight(m_scroll)
            ->joinHighlight(addLabelButton)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f);
        
        tutorial->appendDialogue("You can <cg>enable</c>/<cr>disable</c>, <cy>edit the settings</c>, or even <cr>delete</c> labels", TutorialCharacterFace::TCFHappy, {255, 255, 255});
        
        for (auto& label : m_labels) {
            tutorial->joinHighlight(label->getEnableLabel());
            tutorial->joinHighlight(label->getEnabledToggle());

            tutorial->joinHighlight(label->getEditButton(), .7f);
            tutorial->joinTextToHighlight("Edit", .25f);

            tutorial->joinHighlight(label->getDeleteButton(), 1.4f);
            tutorial->joinTextToHighlight("Delete", .25f);
        }

        tutorial->appendDialogue("You can also set a label to <cf>only work on this level</c>, or <cy>order</c> them differently", TutorialCharacterFace::TCFNormal, {255, 255, 255});

        for (auto& label : m_labels) {
            tutorial->joinHighlight(label->getLevelSpecificLabel());
            tutorial->joinHighlight(label->getLevelSpecificToggle());

            tutorial->joinHighlight(label->getOrderInput(), 1);
            tutorial->joinHighlight(label->getOrderLabel(), 1);
        }

        tutorial->appendDialogue("In the <cf>preview window</c> you can see an approximation of how the <cy>labels will look</c>", TutorialCharacterFace::TCFNormal, {255, 255, 255})
            ->joinHighlight(m_previewArea)
            ->joinTextToHighlight("Preview area", .5f)
            ->joinTransform(TutorialBoxPlacement::TBPBottomRight, .75f)

            ->appendDialogue("Set the selected labels <cy>name</c> and <cy>displayed text</c> here", TutorialCharacterFace::TCFNormal, {255, 255, 255})
            ->joinHighlight(m_nameInput)
            ->joinHighlight(nameLabel)
            ->joinHighlight(m_textInput)
            ->joinHighlight(textLabel)
            ->joinTransform(TutorialBoxPlacement::TBPTopRight, .75f)

            ->appendDialogue("And use the <cd>create key button</c> to use dynamic values to display!", TutorialCharacterFace::TCFHappy, {255, 255, 255})
            ->joinHighlight(m_richTextBtn)
            ->joinTextToHighlight("Create Key", .25f)

            ->appendDialogue("You can edit the position using the <cy>anchor buttons</c> in the preview window OR use <cy>\"free mode\"</c>", TutorialCharacterFace::TCFNormal, {255, 255, 255})
            ->joinHighlight(m_freeModeToggle)
            ->joinHighlight(m_xSlider->m_groove)
            ->joinHighlight(m_ySlider->m_groove)
            ->joinHighlight(m_xSlider->m_touchLogic->m_thumb)
            ->joinHighlight(m_ySlider->m_touchLogic->m_thumb)
            ->joinHighlight(xLabel)
            ->joinHighlight(yLabel)
            ->joinHighlight(freeModeToggleLabel)

            ->appendDialogue("There are some settings that are <cg>global</c> to <cg>all labels</c> that you can edit over here", TutorialCharacterFace::TCFNormal, {255, 255, 255})
            ->joinHighlight(settingsBG)
            ->joinTransform(TutorialBoxPlacement::TBPTopLeft, .75f)
            
            ->appendDialogue("Hope you find overlay labels useful :D", TutorialCharacterFace::TCFHappy, {255, 255, 255})
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    labelTutorial->setPosition(m_size);
    m_buttonMenu->addChild(labelTutorial);

    this->setTouchEnabled(true);

    scheduleUpdate();

    return true;
}

void LabelEditLayer::onLabelAdded(CCObject*) {
    auto newLabel = OverlayLabel{};
    newLabel.m_labelName = "New Label";

    auto newLabelCell = OverlayLabelCell::create(m_scroll->getContentWidth(), newLabel);
    newLabelCell->onEditCallback([this](auto cell) { onCellEdit(cell); });
    newLabelCell->onDeleteCallback([this](auto cell) { onCellDelete(cell); });
    newLabelCell->onChangedCallback([this](auto cell) { onCellChanged(cell); });
    newLabelCell->onVisibilityChangedCallback([this](auto) {
        m_scroll->m_contentLayer->updateLayout();
        updateNoLabelsText();
    });

    m_scroll->m_contentLayer->addChild(newLabelCell);
    for (auto& label : m_labels)
    {
        label->updateLevelVisibility();
    }
    
    m_scroll->m_contentLayer->updateLayout();

    m_labels.push_back(newLabelCell);
    onCellEdit(newLabelCell);

    updateNoLabelsText();

    updateSavedLabels();
}


void LabelEditLayer::onCellDelete(OverlayLabelCell* cell) {
    auto labelIt = std::find(m_labels.begin(), m_labels.end(), cell);
    if (labelIt == m_labels.end()) return;

    if (m_currentEdit == cell) {
        m_currentEdit = nullptr;
        m_updatingControls = true;
        m_nameInput->setString("");
        m_textInput->setString("");
        m_freeModeToggle->toggle(false);
        m_xSlider->setValue(0);
        Dev::setSliderEnabled(m_xSlider, false);
        m_ySlider->setValue(0);
        Dev::setSliderEnabled(m_ySlider, false);
        m_updatingControls = false;
        m_disabledAnchorSlidersFG->setVisible(true);
        setLabelEditEnabled(false);
        m_prevCursorPos = std::nullopt;
        m_doUpdateCursorPos = false;
    }

    m_labels.erase(labelIt);

    cell->setVisible(false);
    cell->removeMeAndCleanup();

    for (auto& label : m_labels)
    {
        label->updateLevelVisibility();
    }

    m_scroll->m_contentLayer->updateLayout();

    updateNoLabelsText();

    updateSavedLabels();
}

void LabelEditLayer::onCellChanged(OverlayLabelCell*) {
    updateSavedLabels();
}

void LabelEditLayer::onCellEdit(OverlayLabelCell* cell) {
    if (std::find(m_labels.begin(), m_labels.end(), cell) == m_labels.end()) return;

    if (m_currentEdit == cell) {
        m_currentEdit->setEditing(false);
        m_currentEdit = nullptr;
        m_updatingControls = true;
        m_nameInput->setString("");
        m_textInput->setString("");
        m_freeModeToggle->toggle(false);
        m_xSlider->setValue(0);
        Dev::setSliderEnabled(m_xSlider, false);
        m_ySlider->setValue(0);
        Dev::setSliderEnabled(m_ySlider, false);
        m_updatingControls = false;
        m_disabledAnchorSlidersFG->setVisible(true);
        setLabelEditEnabled(false);
        RefreshPreviewLabels();
        m_prevCursorPos = std::nullopt;
        m_doUpdateCursorPos = false;
        return;
    }

    if (m_currentEdit)
        m_currentEdit->setEditing(false);

    m_currentEdit = cell;
    m_currentEdit->setEditing(true);
    setLabelEditEnabled(true);

    auto const& label = cell->getData();
    m_updatingControls = true;
    m_nameInput->setString(label.m_labelName);
    m_textInput->setString(label.m_text);
    m_freeModeToggle->toggle(label.m_freeMode);
    m_xSlider->setValue(label.m_posX);
    m_ySlider->setValue(label.m_posY);
    m_updatingControls = false;
    m_disabledAnchorSlidersFG->setVisible(!label.m_freeMode);
    if (!label.m_freeMode) {
        Dev::setSliderEnabled(m_xSlider, false);
        Dev::setSliderEnabled(m_ySlider, false);
    }
    else {
        Dev::setSliderEnabled(m_xSlider, true);
        Dev::setSliderEnabled(m_ySlider, true);
    }

    RefreshPreviewLabels();
}

void LabelEditLayer::setLabelEditEnabled(bool enabled) {
    m_editMenu->setEnabled(enabled);
    m_nameInput->setEnabled(enabled);
    m_textInput->setEnabled(enabled);
    updateAnchorButtons();
    if (!enabled) {
        Dev::setSliderEnabled(m_xSlider, false);
        Dev::setSliderEnabled(m_ySlider, false);
    }
    m_noLabelEditedOverlay->setVisible(!enabled);
    m_noLabelEditedText->setVisible(!enabled);
}

void LabelEditLayer::updateNoLabelsText() {
    auto hasVisibleLabels = std::any_of(m_labels.begin(), m_labels.end(), [](auto* label) {
        return label->isVisible();
    });
    m_noLabelsText->setVisible(!hasVisibleLabels);
}

void LabelEditLayer::updateAnchorButtons() {
    auto label = m_currentEdit ? m_currentEdit->getData() : OverlayLabel{};
    for (const auto& [buttonObject, anchor] : m_anchorButtons) {
        auto button = static_cast<CCMenuItemSpriteExtra*>(buttonObject);
        auto enabled = m_currentEdit && !label.m_freeMode && anchor != label.m_anchor;
        button->setEnabled(enabled);
        button->setVisible(enabled);
    }
}

void LabelEditLayer::onSliderX(CCObject*) {
    if (m_updatingControls || !m_currentEdit || !m_currentEdit->getData().m_freeMode) return;

    auto label = m_currentEdit->getData();
    label.m_posX = m_xSlider->getValue();
    m_currentEdit->setData(label);
    updateSavedLabels();
}

void LabelEditLayer::onSliderY(CCObject*) {
    if (m_updatingControls || !m_currentEdit || !m_currentEdit->getData().m_freeMode) return;

    auto label = m_currentEdit->getData();
    label.m_posY = m_ySlider->getValue();
    m_currentEdit->setData(label);
    updateSavedLabels();
}

void LabelEditLayer::onOffsetSliderChanged(CCObject*) {
    if (m_updatingControls) return;

    auto offset = m_OFFSET_SLIDER_MIN + (m_OFFSET_SLIDER_MAX - m_OFFSET_SLIDER_MIN) * m_offsetSlider->getValue();
    m_updatingControls = true;
    m_offsetInput->setString(fmt::format("{:.1f}", offset));
    m_updatingControls = false;

    auto settings = Save::getOverlayLabels();
    if (settings.m_offset.x == offset && settings.m_offset.y == offset) return;
    settings.m_offset = ccp(offset, offset);
    Save::setOverlayLabels(settings);
}

void LabelEditLayer::onSizeSliderChanged(CCObject*) {
    if (m_updatingControls) return;

    auto size = OverlayLabelSettings::MIN_SIZE + (OverlayLabelSettings::MAX_SIZE - OverlayLabelSettings::MIN_SIZE) * m_sizeSlider->getValue();
    setCurrentLabelSize(size);
}

void LabelEditLayer::onOpacitySliderChanged(CCObject*) {
    if (m_updatingControls) return;

    auto opacity = static_cast<int>(
        OverlayLabelSettings::MIN_OPACITY +
        (OverlayLabelSettings::MAX_OPACITY - OverlayLabelSettings::MIN_OPACITY) * m_opacitySlider->getValue() +
        0.5f
    );
    setCurrentLabelOpacity(opacity);
}

void LabelEditLayer::setCurrentLabelSize(float size, bool updateInput) {
    size = std::clamp(size, OverlayLabelSettings::MIN_SIZE, OverlayLabelSettings::MAX_SIZE);

    m_updatingControls = true;

    if (updateInput) m_sizeInput->setString(fmt::format("{:.2f}", size));

    m_sizeSlider->setValue(
        (size - OverlayLabelSettings::MIN_SIZE) /
        (OverlayLabelSettings::MAX_SIZE - OverlayLabelSettings::MIN_SIZE)
    );
    m_updatingControls = false;

    auto settings = Save::getOverlayLabels();

    if (settings.m_size == size) return;
    settings.m_size = size;

    Save::setOverlayLabels(settings);
}

void LabelEditLayer::setCurrentLabelOpacity(int opacity, bool updateInput) {
    opacity = std::clamp(opacity, OverlayLabelSettings::MIN_OPACITY, OverlayLabelSettings::MAX_OPACITY);

    m_updatingControls = true;
    if (updateInput) m_opacityInput->setString(fmt::format("{}", opacity));
    m_opacitySlider->setValue(
        static_cast<float>(opacity - OverlayLabelSettings::MIN_OPACITY) /
        (OverlayLabelSettings::MAX_OPACITY - OverlayLabelSettings::MIN_OPACITY)
    );
    m_updatingControls = false;

    auto settings = Save::getOverlayLabels();
    if (settings.m_opacity == opacity) return;
    settings.m_opacity = opacity;
    Save::setOverlayLabels(settings);
}

CCMenuItemSpriteExtra* LabelEditLayer::createAnchorButton(Anchor anchor, std::string const& text) {

    auto bg = CCScale9Sprite::create("geode.loader/white-square.png");

    auto btnText = geode::Label::create(text.c_str(), "bigFont.fnt");
    btnText->setScale(.35f);

    auto invertedAnchor = invertAnchor(anchor);

    bg->addChildAtPosition(btnText, invertedAnchor, anchorDirection(invertedAnchor) * 3.5f);
    btnText->setAnchorPoint(anchorPoint(invertedAnchor));

    auto btn = CCMenuItemSpriteExtra::create(
        bg,
        this,
        menu_selector(LabelEditLayer::onAnchorClicked)
    );

    m_previewAreaMenu->addChildAtPosition(btn, anchor);

    m_anchorButtons.insert({btn, anchor});

    return btn;
}

void LabelEditLayer::onAnchorClicked(CCObject* sender) {
    if (m_updatingControls || !m_currentEdit) return;

    auto anchorIt = m_anchorButtons.find(sender);
    if (anchorIt == m_anchorButtons.end()) return;

    auto label = m_currentEdit->getData();
    auto anchorPos = anchorPoint(anchorIt->second);

    m_updatingControls = true;
    m_xSlider->setValue(anchorPos.x);
    m_ySlider->setValue(anchorPos.y);
    m_updatingControls = false;

    label.m_anchor = anchorIt->second;
    label.m_posX = anchorPos.x;
    label.m_posY = anchorPos.y;
    m_currentEdit->setData(label);
    updateAnchorButtons();

    updateSavedLabels();
}

void LabelEditLayer::onRichTextBtn(CCObject*) {
    LabelEditSpecialKeyEditorPopup::create([this](const std::string& key) {
        if (!m_textInput || !m_currentEdit)
            return;

        std::string text;
        if (m_prevCursorPos.has_value()) {
            text = m_textInput->getString();
            log::info("Inserting key at cursor position: {}", m_prevCursorPos.value());
            log::info("Current text: {}", m_textInput->getString());
            text.insert(m_prevCursorPos.value(), key);
        } else {
            text = m_textInput->getString() + key;
        }

        m_textInput->setString(text);
        if (m_prevCursorPos.has_value())
            m_textInput->setCursorPos(m_prevCursorPos.value() + key.length());
        auto label = m_currentEdit->getData();
        label.m_text = text;
        m_currentEdit->setData(label);
        updateSavedLabels();
    })->show();
}

void LabelEditLayer::updateSavedLabels() {
    std::vector<OverlayLabel> labelsData{};

    for (auto label : m_labels) {
        labelsData.push_back(label->getData());
    }

    auto labelSettings = Save::getOverlayLabels();

    labelSettings.m_labels = labelsData;
    Save::setOverlayLabels(labelSettings);
}

void LabelEditLayer::RefreshPreviewLabels() {
    for (const auto& plabel : m_previewLabels)
    {
        plabel->removeMeAndCleanup();
    }

    m_previewLabels.clear();
    m_currentPreviewLabel = nullptr;

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto previewSize = m_previewArea->getContentSize();
    auto previewScale = std::min(
        previewSize.width / winSize.width,
        previewSize.height / winSize.height
    );

    auto labelSettings = Save::getOverlayLabels();
    auto dtLayer = DTLayer::get();
    auto currentLevel = dtLayer ? dtLayer->m_Level : nullptr;
    std::optional<size_t> selectedIndex;
    if (m_currentEdit) {
        auto it = std::find(m_labels.begin(), m_labels.end(), m_currentEdit);
        if (it != m_labels.end())
            selectedIndex = std::distance(m_labels.begin(), it);
    }

    std::vector<std::pair<size_t, OverlayLabel>> orderedLabels;
    orderedLabels.reserve(labelSettings.m_labels.size());
    for (size_t i = 0; i < labelSettings.m_labels.size(); i++)
        orderedLabels.emplace_back(i, labelSettings.m_labels[i]);

    std::stable_sort(orderedLabels.begin(), orderedLabels.end(), [](const auto& a, const auto& b) {
        return a.second.m_order > b.second.m_order;
    });

    std::map<Anchor, float> anchorOffsets;
    
    for (const auto& [index, label] : orderedLabels) {
        if (!label.m_enabled || !label.isOverlayLabelVisibleOnLevel(currentLevel)) continue;

        auto previewLabel = RichTextArea::create(expandDeaths(label.m_text), labelSettings.m_font, labelSettings.m_size * previewScale);
        m_previewLabels.push_back(previewLabel);

        auto isDoner = [](){ return AuthManager::isDoner(); };

        previewLabel->registerRichTextKey(RichTextKeys::makeGradientKey(isDoner));
        previewLabel->registerRichTextKey(RichTextKeys::makeWaveKey(isDoner));
        previewLabel->registerRichTextKey(RichTextKeys::makeCPlusKey());

        float widest = 0;

        for (const auto& line : previewLabel->getLines())
        {
            widest = std::max(widest, line->getScaledContentWidth());
        }
        
        previewLabel->setWidth(widest);

        if (selectedIndex && selectedIndex.value() == index) {
            m_currentPreviewLabel = previewLabel;
            auto background = CCScale9Sprite::create("geode.loader/white-square.png");
            background->setColor({0, 255, 0});
            background->setOpacity(65);
            background->setContentSize(previewLabel->getContentSize() + ccp(8, 6));
            background->setAnchorPoint({0, 0});
            background->setPosition({-4, -3});
            background->setScale(.2f);
            background->setContentSize(background->getContentSize() / background->getScale());
            previewLabel->addChild(background, -1);
        }

        auto anchor = label.m_freeMode ? ccp(label.m_posX, label.m_posY) : anchorPoint(label.m_anchor);
        previewLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

        if (!label.m_freeMode) {
            auto& offset = anchorOffsets[label.m_anchor];
            auto stackDirection = anchorDirection(label.m_anchor).y;
            if (stackDirection == 0) stackDirection = -1;
            m_previewArea->addChildAtPosition(previewLabel, label.m_anchor, ccp(0, stackDirection * offset) + labelSettings.m_offset * anchorDirection(anchor));
            previewLabel->setAnchorPoint(anchor);
            offset += previewLabel->getScaledContentHeight();
        }
        else {
            m_previewArea->addChild(previewLabel);

            previewLabel->setAnchorPoint(anchor);
            previewLabel->setPosition(m_previewArea->getContentSize() * anchor + labelSettings.m_offset * anchorDirection(anchor));
        }

        auto previewColor = previewLabel->getColor();
        previewColor.a = labelSettings.m_opacity;
        previewLabel->setColor(previewColor);
    }
}

Anchor LabelEditLayer::invertAnchor(Anchor anchor) {
    switch (anchor) {
        default:
        case Anchor::Center: return Anchor::Center;
        case Anchor::TopLeft: return Anchor::BottomRight;
        case Anchor::Top: return Anchor::Bottom;
        case Anchor::TopRight: return Anchor::BottomLeft;
        case Anchor::Right: return Anchor::Left;
        case Anchor::BottomRight: return Anchor::TopLeft;
        case Anchor::Bottom: return Anchor::Top;
        case Anchor::BottomLeft: return Anchor::TopRight;
        case Anchor::Left: return Anchor::Right;
    }
}

CCPoint LabelEditLayer::anchorPoint(Anchor anchor) {
    switch (anchor) {
        default:
        case Anchor::Center: return ccp(0.5f, 0.5f);
        case Anchor::TopLeft: return ccp(0, 1);
        case Anchor::Top: return ccp(.5f, 1);
        case Anchor::TopRight: return ccp(1, 1);
        case Anchor::Right: return ccp(1, .5f);
        case Anchor::BottomRight: return ccp(1, 0);
        case Anchor::Bottom: return ccp(.5f, 0);
        case Anchor::BottomLeft: return ccp(0, 0);
        case Anchor::Left: return ccp(0, .5f);
    }
}

CCPoint LabelEditLayer::anchorDirection(Anchor anchor) {
    return anchorDirection(anchorPoint(anchor));
}

CCPoint LabelEditLayer::anchorDirection(CCPoint anchor) {
    return ccp(
        1.f - 2.f * anchor.x,
        1.f - 2.f * anchor.y
    );
}

Result<GeneralData> LabelEditLayer::getDeathsData(char scope) {
    auto layer = DTLayer::get();

    if (!layer) {
        return Err("No DTLayer");
    }

    if (scope == 'g') {
        if (layer->m_MyLevelStats.isErr()) {
            return Err(layer->m_MyLevelStats.unwrapErr().m_error);
        }

        auto levelStats = layer->m_MyLevelStats.unwrap();
        if (levelStats.m_from0.isErr()) {
            return Err(levelStats.m_from0.unwrapErr().m_error);
        }

        auto data = levelStats.m_from0.unwrap();
        for (const auto& linkedLevel : layer->m_linkedLevelsData) {
            if (linkedLevel.m_from0.isOk() && linkedLevel.m_levelKey != levelStats.m_levelKey)
                data += linkedLevel.m_from0.unwrap();
        }
        return Ok(data);
    }

    auto selectedSession = layer->loadSessionFromSave();
    if (selectedSession.isErr()) {
        return Err(selectedSession.unwrapErr().m_error);
    }

    if (scope == 's')
        return Ok(selectedSession.unwrap().m_data);

    auto session = selectedSession.unwrap();
    auto sessionTime = session.m_sessionStartDate ? session.m_sessionStartDate : session.m_groupID;
    auto time = static_cast<std::time_t>(sessionTime);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    localTime.tm_sec = 0;
    localTime.tm_min = 0;
    localTime.tm_hour = 0;
    auto dayStart = static_cast<long long>(std::mktime(&localTime));

    auto day = layer->m_daySGroup.m_grouping.find(dayStart);
    if (day == layer->m_daySGroup.m_grouping.end()) {
        return Err("No saved sessions for this day");
    }

    GeneralData data{};
    for (const auto& [sessionDate, levelKeys] : day->second.m_group) {
        for (const auto& levelKey : levelKeys) {
            auto savedSession = StatsManager::getSession(levelKey, sessionDate);

            if (savedSession.isErr()) {
                return Err(savedSession.unwrapErr().m_error);
            }

            data += savedSession.unwrap().m_data;
        }
    }
    return Ok(data);
}

std::string LabelEditLayer::formatDeaths(const GeneralData& data, const std::array<std::optional<std::string>, 9>& groups, std::string_view expression) {
    std::map<int, uint64_t> deathsByPercent;

    auto sumDeaths = [&](const ComplexDeathMap& deathMap) {
        for (const auto& [key, count] : deathMap.toLegacyDeaths()) {
            auto runResult = StatsManager::splitRunKey(key);

            if (runResult.isErr())
                continue;

            auto run = runResult.unwrap();
            auto deathCount = static_cast<uint64_t>(count);

            deathsByPercent[static_cast<int>(run.m_end)] += deathCount;
        }
    };

    sumDeaths(data.m_deaths);

    if (groups[6]) {
        std::vector<std::string> output;

        auto countRes = utils::numFromString<int>(*groups[6]);
        if (countRes.isErr())
            return "Invalid count";
        auto count = countRes.unwrap();
        
        std::optional<ComplexPercent> startPercent;
        if (groups[7] && !groups[7]->empty()) {
            auto startPercentRes = utils::numFromString<double>(*groups[7]);
            if (startPercentRes.isErr())
                return "Invalid start percent";
            auto parsedStartPercent = ComplexPercent::fromPercent(startPercentRes.unwrap());
            if (parsedStartPercent.isErr())
                return "Invalid start percent";
            startPercent = parsedStartPercent.unwrap();
        }
        
        std::vector<std::pair<Run, uint64_t>> bestRuns;
        std::map<int64_t, uint64_t> bestRunsByWholePercent;
        auto showDecimals = groups[8].has_value();
        auto filteredRuns = data.m_runs.filterCheatedRuns();
        const auto& runData = startPercent ? filteredRuns.m_records : data.m_deaths.m_records;
        for (const auto& entry : runData) {
            auto& death = entry.second;
            bool matchesStart = startPercent ? death.m_runStartPercent.has_value() && death.m_runStartPercent->m_units == startPercent->m_units : !death.m_runStartPercent.has_value();
            if (matchesStart) {
                Run run{
                    death.m_runStartPercent ? std::optional<double>(death.m_runStartPercent->asPercent()) : std::nullopt,
                    death.m_runEndPercent.asPercent()
                };
                if (showDecimals) {
                    bestRuns.emplace_back(run, death.m_deaths);
                }
                else {
                    auto wholePercent = static_cast<int64_t>(std::floor(run.m_end));
                    auto& count = bestRunsByWholePercent[wholePercent];
                    if (std::numeric_limits<uint64_t>::max() - count < death.m_deaths)
                        count = std::numeric_limits<uint64_t>::max();
                    else
                        count += death.m_deaths;
                }
            }
        }

        if (!showDecimals) {
            auto wholeStart = startPercent
                ? std::optional<double>(std::floor(startPercent->asPercent()))
                : std::nullopt;
            for (const auto& [wholePercent, count] : bestRunsByWholePercent) {
                bestRuns.emplace_back(Run{wholeStart, static_cast<double>(wholePercent)}, count);
            }
        }

        std::stable_sort(bestRuns.begin(), bestRuns.end(), [](const auto& a, const auto& b) {
            return a.first.m_end > b.first.m_end;
        });

        for (int i = 0; i < count && i < static_cast<int>(bestRuns.size()); i++) {
            auto decimalPrecision = groups[8] ? Settings::getDecimalPrecision() : 0;

            std::string entry;

            if (bestRuns[i].first.m_start.has_value()) {
                entry = fmt::format("{:.{}f}-{:.{}f}% X{}", bestRuns[i].first.m_start.value(), decimalPrecision, bestRuns[i].first.m_end, decimalPrecision, bestRuns[i].second);
            }
            else {
                entry = fmt::format("{:.{}f}% X{}", bestRuns[i].first.m_end, decimalPrecision, bestRuns[i].second);
            }

            output.push_back(entry);
        }

        if (output.empty())
            return "[None]";

        return fmt::format("{}", fmt::join(output, ", "));
    }

    auto minPercent = std::numeric_limits<int>::min();
    auto maxPercent = std::numeric_limits<int>::max();
    bool exclusiveMin = false;
    bool exclusiveMax = false;

    if (groups[2]) {
        auto minPercentRes = utils::numFromString<int>(*groups[2]);
        if (minPercentRes.isErr())
            return "Invalid min percent";
        minPercent = minPercentRes.unwrap();

        exclusiveMin = expression.find(">=") == std::string_view::npos;
    }
    else if (groups[3]) {
        auto maxPercentRes = utils::numFromString<int>(*groups[3]);
        if (maxPercentRes.isErr())
            return "Invalid max percent";
        maxPercent = maxPercentRes.unwrap();
        exclusiveMax = true;
    }
    else {
        auto minPercentRes = utils::numFromString<int>(*groups[4]);
        if (minPercentRes.isErr())
            return "Invalid min percent";
        minPercent = minPercentRes.unwrap();

        auto maxPercentRes = utils::numFromString<int>(*groups[5]);
        if (maxPercentRes.isErr())
            return "Invalid max percent";
        maxPercent = maxPercentRes.unwrap();
    }

    uint64_t totalDeaths = 0;
    for (const auto& [percent, count] : deathsByPercent) {
        if (percent < minPercent || percent > maxPercent || (exclusiveMin && percent == minPercent) || (exclusiveMax && percent == maxPercent))
            continue;

        totalDeaths += count;
    }
    return std::to_string(totalDeaths);
}

std::string LabelEditLayer::expandDeaths(std::string_view textView) {
    return expandDeaths(textView, [](char scope) {
        return getDeathsData(scope);
    });
}

std::string LabelEditLayer::expandDeaths(std::string_view textView, const std::function<Result<GeneralData>(char)>& dataProvider) {
    auto text = std::string(textView);
    std::map<char, std::optional<GeneralData>> dataByScope;
    auto getData = [&](char scope) -> const std::optional<GeneralData>& {
        auto dataIt = dataByScope.find(scope);
        if (dataIt == dataByScope.end()) {
            auto result = dataProvider(scope);
            dataIt = dataByScope.emplace(
                scope,
                result.isOk() ? std::optional<GeneralData>(result.unwrap()) : std::nullopt
            ).first;
        }
        return dataIt->second;
    };

    std::regex currentDeathCountPattern(R"(\{\(([sgd])\)cd(d)?\})");
    std::string expandedCurrentDeathCount;
    auto textStart = text.cbegin();

    for (std::sregex_iterator it(text.begin(), text.end(), currentDeathCountPattern), end; it != end; ++it) {
        const auto& match = *it;
        expandedCurrentDeathCount.append(textStart, match[0].first);

        auto scope = match[1].str()[0];
        const auto& data = getData(scope);
        std::optional<long long> latestTimestamp;
        const ComplexDeath* latestDeath = nullptr;

        if (data) {
            for (const auto& [key, death] : data->m_deaths.m_records) {
                for (const auto& occurrence : death.m_occurrences) {
                    if (!occurrence.m_runEndTimestamp)
                        continue;

                    auto timestamp = occurrence.m_runEndTimestamp.value();
                    if (!latestTimestamp || timestamp >= latestTimestamp.value()) {
                        latestTimestamp = timestamp;
                        latestDeath = &death;
                    }
                }
            }
        }

        if (latestDeath) {
            if (match[2].matched) {
                expandedCurrentDeathCount += fmt::format(
                    "{:.{}f}% X{}",
                    latestDeath->m_runEndPercent.asPercent(),
                    Settings::getDecimalPrecision(),
                    latestDeath->m_deaths
                );
            }
            else {
                auto wholePercent = static_cast<int64_t>(std::floor(latestDeath->m_runEndPercent.asPercent()));
                auto legacyDeaths = data->m_deaths.toLegacyDeaths();
                auto count = legacyDeaths.find(std::to_string(wholePercent));
                expandedCurrentDeathCount += fmt::format(
                    "{}% X{}",
                    wholePercent,
                    count == legacyDeaths.end() ? latestDeath->m_deaths : count->second
                );
            }
        }
        else {
            expandedCurrentDeathCount += "[None]";
        }
        textStart = match[0].second;
    }

    expandedCurrentDeathCount.append(textStart, text.cend());

    std::regex queryPattern(R"(\{\(([sgd])\)(?:>=?(\d+)|<(\d+)|(\d+)-(\d+)|(\d+)b(\d+(?:\.\d+)?)?(d)?)\})");

    std::string expanded;
    textStart = expandedCurrentDeathCount.cbegin();

    for (std::sregex_iterator it(expandedCurrentDeathCount.begin(), expandedCurrentDeathCount.end(), queryPattern), end; it != end; ++it) {
        const auto& match = *it;
        expanded.append(textStart, match[0].first);

        auto scope = match[1].str()[0];
        const auto& data = getData(scope);
        if (data) {
            std::array<std::optional<std::string>, 9> groups;

            for (int i = 0; i < groups.size(); i++) {
                if (!match[i].matched) continue;
                
                groups[i] = match[i].str();
            }

            expanded += formatDeaths(data.value(), groups, groups[0].value());
        }
        else {
            expanded += "[None]";
        }
        
        textStart = match[0].second;
    }

    expanded.append(textStart, expandedCurrentDeathCount.cend());
    return expanded;
}

bool LabelEditLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    auto label = m_currentEdit ? m_currentEdit->getData() : OverlayLabel{};

    if (m_currentEdit && label.m_freeMode && m_currentPreviewLabel) {
        auto touchPosition = m_previewArea->convertToNodeSpace(pTouch->getLocation());

        if (m_currentPreviewLabel->boundingBox().containsPoint(touchPosition)) {
            m_isDraggingPreviewLabel = true;
            m_dragTouchLocalPoint = m_currentPreviewLabel->convertToNodeSpace(pTouch->getLocation());
            return true;
        }
    }

    return Popup::ccTouchBegan(pTouch, pEvent);
}

void LabelEditLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
    if (!m_isDraggingPreviewLabel) {
        Popup::ccTouchMoved(pTouch, pEvent);
        return;
    }

    auto touchPosition = m_previewArea->convertToNodeSpace(pTouch->getLocation());
    auto previewSize = m_previewArea->getContentSize();
    auto settings = Save::getOverlayLabels();
    auto label = m_currentEdit->getData();
    auto labelSize = m_currentPreviewLabel->getContentSize() * ccp(
        m_currentPreviewLabel->getScaleX(),
        m_currentPreviewLabel->getScaleY()
    );

    auto dragAxis = [](float touch, float grabbedPoint, float labelSize, float previewSize, float offset, float scale, float previousPosition) {
        auto desiredMin = touch - grabbedPoint * scale;
        if (labelSize <= previewSize)
            desiredMin = std::clamp(desiredMin, 0.f, previewSize - labelSize);
        else
            desiredMin = (previewSize - labelSize) / 2;

        auto denominator = previewSize - 2 * offset - labelSize;
        if (std::abs(denominator) < 0.001f)
            return previousPosition;
        
        return (desiredMin - offset) / denominator;
    };

    label.m_posX = dragAxis(
        touchPosition.x, m_dragTouchLocalPoint.x, labelSize.width,
        previewSize.width, settings.m_offset.x, m_currentPreviewLabel->getScaleX(), label.m_posX
    );
    label.m_posY = dragAxis(
        touchPosition.y, m_dragTouchLocalPoint.y, labelSize.height,
        previewSize.height, settings.m_offset.y, m_currentPreviewLabel->getScaleY(), label.m_posY
    );
    m_currentEdit->setData(label);

    m_updatingControls = true;
    m_xSlider->setValue(label.m_posX);
    m_ySlider->setValue(label.m_posY);
    m_updatingControls = false;

    auto anchor = ccp(label.m_posX, label.m_posY);
    m_currentPreviewLabel->setAnchorPoint(anchor);
    m_currentPreviewLabel->setPosition(previewSize * anchor + settings.m_offset * anchorDirection(anchor));
}

void LabelEditLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
    if (!m_isDraggingPreviewLabel) {
        Popup::ccTouchEnded(pTouch, pEvent);
        return;
    }

    m_isDraggingPreviewLabel = false;
    updateSavedLabels();
}

void LabelEditLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) {
    if (!m_isDraggingPreviewLabel) {
        Popup::ccTouchCancelled(pTouch, pEvent);
        return;
    }

    m_isDraggingPreviewLabel = false;
    updateSavedLabels();
}

void LabelEditLayer::update(float dt) {
    if (m_textInput && m_prevCursorPos.has_value() && m_doUpdateCursorPos) {
        m_prevCursorPos = m_textInput->getCursorPos();
    }
}