#include "LabelEditLayer.hpp"

#include <types/DTTypes.hpp>

#include <utils/Save.hpp>
#include <utils/Settings.hpp>

#include <nodes/DTLabel.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/TutorialButton.hpp>

#include <managers/AuthManager.hpp>

#include <nodes/layers/LabelEditSpecialKeyEditorPopup.hpp>

#include <cmath>

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
    // @geode-ignore(unknown-resource)
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
        auto label = CCLabelBMFont::create(title, "bigFont.fnt");
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
        if (offset < OFFSET_SLIDER_MIN || offset > OFFSET_SLIDER_MAX) return;
        m_updatingControls = true;
        m_offsetSlider->setValue((offset - OFFSET_SLIDER_MIN) / (OFFSET_SLIDER_MAX - OFFSET_SLIDER_MIN));
        m_updatingControls = false;

        auto settings = Save::getOverlayLabels();
        if (settings.offset.x == offset && settings.offset.y == offset) return;
        settings.offset = ccp(offset, offset);
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
        (initialSettings.offset.x + initialSettings.offset.y) / 2.f,
        OFFSET_SLIDER_MIN,
        OFFSET_SLIDER_MAX
    );
    m_updatingControls = true;
    m_offsetInput->setString(fmt::format("{:.1f}", initialOffset));
    m_sizeInput->setString(fmt::format("{:.2f}", initialSettings.size));
    m_opacityInput->setString(fmt::format("{}", initialSettings.opacity));
    m_offsetSlider->setValue(
        (initialOffset - OFFSET_SLIDER_MIN) / (OFFSET_SLIDER_MAX - OFFSET_SLIDER_MIN)
    );
    m_sizeSlider->setValue(
        (initialSettings.size - OverlayLabelSettings::MIN_SIZE) /
        (OverlayLabelSettings::MAX_SIZE - OverlayLabelSettings::MIN_SIZE)
    );
    m_opacitySlider->setValue(
        static_cast<float>(initialSettings.opacity - OverlayLabelSettings::MIN_OPACITY) /
        (OverlayLabelSettings::MAX_OPACITY - OverlayLabelSettings::MIN_OPACITY)
    );
    m_updatingControls = false;

    if (initialSettings.offset.x != initialOffset || initialSettings.offset.y != initialOffset) {
        initialSettings.offset = ccp(initialOffset, initialOffset);
        Save::setOverlayLabels(initialSettings);
    }

    m_noLabelsText = CCLabelBMFont::create("No Labels\nCreated!", "bigFont.fnt");
    m_noLabelsText->setScale(.5f);
    m_noLabelsText->setPosition(scrollBG->getPosition());
    m_noLabelsText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
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
        label.freeMode = state;
        m_currentEdit->setData(label);

        m_updatingControls = true;
        m_disabledAnchorSlidersFG->setVisible(!label.freeMode);
        if (label.freeMode) {
            m_xSlider->setValue(label.posX);
            m_ySlider->setValue(label.posY);
            m_xSlider->enableSlider();
            m_ySlider->enableSlider();
        }
        else {
            m_xSlider->disableSlider();
            m_ySlider->disableSlider();
        }
        m_updatingControls = false;

        updateAnchorButtons();
        updateSavedLabels();
    });
    m_editMenu->addChildAtPosition(m_freeModeToggle, Anchor::TopLeft, {m_editMenu->getContentWidth() / 4.5f, -m_freeModeToggle->getScaledContentHeight() / 2 - 5});

    auto freeModeToggleLabel = CCLabelBMFont::create("Free Mode", "bigFont.fnt");
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

    auto xLabel = CCLabelBMFont::create("X:", "bigFont.fnt");
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

    auto yLabel = CCLabelBMFont::create("Y:", "bigFont.fnt");
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

    m_xSlider->disableSlider();
    m_ySlider->disableSlider();

    auto nameLabel = CCLabelBMFont::create("Name:", "bigFont.fnt");
    nameLabel->setScale(.5f);
    m_editMenu->addChildAtPosition(nameLabel, Anchor::TopRight, {-m_editMenu->getContentWidth() / 6, -nameLabel->getScaledContentHeight() / 2 - 5});

    m_nameInput = TextInput::create(80, "Name");
    m_nameInput->setScale(.75f);
    m_nameInput->setCommonFilter(CommonFilter::Name);
    m_nameInput->setMaxCharCount(12);
    m_nameInput->setCallback([this](const std::string& name) {
        if (m_updatingControls || !m_currentEdit) return;

        auto label = m_currentEdit->getData();
        label.labelName = name;
        m_currentEdit->setData(label);
        updateSavedLabels();
    });
    m_editMenu->addChildAtPosition(m_nameInput, Anchor::TopRight, {
        -m_editMenu->getContentWidth() / 6,
        -m_nameInput->getScaledContentHeight() / 2 - nameLabel->getScaledContentHeight() - 10
    });

    auto textLabel = CCLabelBMFont::create("Text:", "bigFont.fnt");
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
        label.text = text;
        m_currentEdit->setData(label);
        updateSavedLabels();
    });
    m_textInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_=<>. {}()%:");
    m_editMenu->addChild(m_textInput);

    m_richTextBtn->setPosition(m_textInput->getPosition() + ccp(
        m_textInput->getScaledContentWidth() + m_richTextBtn->getScaledContentWidth() / 2 + 5,
        0
    ));

    m_noLabelEditedOverlay = CCScale9Sprite::create("square02_small.png");
    m_noLabelEditedOverlay->setContentSize(m_editMenu->getContentSize());
    m_noLabelEditedOverlay->setAnchorPoint(m_editMenu->getAnchorPoint());
    m_noLabelEditedOverlay->ignoreAnchorPointForPosition(false);
    m_noLabelEditedOverlay->setPosition(m_editMenu->getPosition());
    m_noLabelEditedOverlay->setOpacity(180);
    m_mainLayer->addChild(m_noLabelEditedOverlay);

    m_noLabelEditedText = CCLabelBMFont::create("No label edited", "bigFont.fnt");
    m_noLabelEditedText->setScale(.6f);
    m_noLabelEditedText->setAnchorPoint({.5f, .5f});
    m_noLabelEditedText->setPosition({
        m_editMenu->getContentWidth() / 2,
        m_editMenu->getContentHeight() / 2
    });
    m_noLabelEditedOverlay->addChild(m_noLabelEditedText);
    setLabelEditEnabled(false);

    auto labelSettings = Save::getOverlayLabels();
    for (auto const& label : labelSettings.labels) {
        auto labelCell = OverlayLabelCell::create(m_scroll->getContentWidth(), label);
        labelCell->onEditCallback([this](auto cell) { onCellEdit(cell); });
        labelCell->onDeleteCallback([this](auto cell) { onCellDelete(cell); });
        labelCell->onChangedCallback([this](auto) { updateSavedLabels(); });
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

    return true;
}

void LabelEditLayer::onLabelAdded(CCObject*) {
    auto newLabel = OverlayLabel{};
    newLabel.labelName = "New Label";

    auto newLabelCell = OverlayLabelCell::create(m_scroll->getContentWidth(), newLabel);
    newLabelCell->onEditCallback([&](auto cell){ onCellEdit(cell); });
    newLabelCell->onDeleteCallback([&](auto cell){ onCellDelete(cell); });
    newLabelCell->onChangedCallback([this](auto) { updateSavedLabels(); });

    m_scroll->m_contentLayer->addChild(newLabelCell);
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
        m_xSlider->disableSlider();
        m_ySlider->setValue(0);
        m_ySlider->disableSlider();
        m_updatingControls = false;
        m_disabledAnchorSlidersFG->setVisible(true);
        setLabelEditEnabled(false);
    }

    m_labels.erase(labelIt);

    cell->setVisible(false);
    cell->removeMeAndCleanup();

    m_scroll->m_contentLayer->updateLayout();

    updateNoLabelsText();

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
        m_xSlider->disableSlider();
        m_ySlider->setValue(0);
        m_ySlider->disableSlider();
        m_updatingControls = false;
        m_disabledAnchorSlidersFG->setVisible(true);
        setLabelEditEnabled(false);
        RefreshPreviewLabels();
        return;
    }

    if (m_currentEdit)
        m_currentEdit->setEditing(false);

    m_currentEdit = cell;
    m_currentEdit->setEditing(true);
    setLabelEditEnabled(true);

    auto const& label = cell->getData();
    m_updatingControls = true;
    m_nameInput->setString(label.labelName);
    m_textInput->setString(label.text);
    m_freeModeToggle->toggle(label.freeMode);
    m_xSlider->setValue(label.posX);
    m_ySlider->setValue(label.posY);
    m_updatingControls = false;
    m_disabledAnchorSlidersFG->setVisible(!label.freeMode);
    if (!label.freeMode) {
        m_xSlider->disableSlider();
        m_ySlider->disableSlider();
    }
    else {
        m_xSlider->enableSlider();
        m_ySlider->enableSlider();
    }

    RefreshPreviewLabels();
}

void LabelEditLayer::setLabelEditEnabled(bool enabled) {
    m_editMenu->setEnabled(enabled);
    m_nameInput->setEnabled(enabled);
    m_textInput->setEnabled(enabled);
    updateAnchorButtons();
    if (!enabled) {
        m_xSlider->disableSlider();
        m_ySlider->disableSlider();
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
        auto enabled = m_currentEdit && !label.freeMode && anchor != label.anchor;
        button->setEnabled(enabled);
        button->setVisible(enabled);
    }
}

void LabelEditLayer::onSliderX(CCObject*) {
    if (m_updatingControls || !m_currentEdit || !m_currentEdit->getData().freeMode) return;

    auto label = m_currentEdit->getData();
    label.posX = m_xSlider->getValue();
    m_currentEdit->setData(label);
    updateSavedLabels();
}

void LabelEditLayer::onSliderY(CCObject*) {
    if (m_updatingControls || !m_currentEdit || !m_currentEdit->getData().freeMode) return;

    auto label = m_currentEdit->getData();
    label.posY = m_ySlider->getValue();
    m_currentEdit->setData(label);
    updateSavedLabels();
}

void LabelEditLayer::onOffsetSliderChanged(CCObject*) {
    if (m_updatingControls) return;

    auto offset = OFFSET_SLIDER_MIN + (OFFSET_SLIDER_MAX - OFFSET_SLIDER_MIN) * m_offsetSlider->getValue();
    m_updatingControls = true;
    m_offsetInput->setString(fmt::format("{:.1f}", offset));
    m_updatingControls = false;

    auto settings = Save::getOverlayLabels();
    if (settings.offset.x == offset && settings.offset.y == offset) return;
    settings.offset = ccp(offset, offset);
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

    if (settings.size == size) return;
    settings.size = size;

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
    if (settings.opacity == opacity) return;
    settings.opacity = opacity;
    Save::setOverlayLabels(settings);
}

CCMenuItemSpriteExtra* LabelEditLayer::createAnchorButton(Anchor anchor, std::string const& text) {

    // @geode-ignore(unknown-resource)
    auto bg = CCScale9Sprite::create("geode.loader/white-square.png");

    auto btnText = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
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

    label.anchor = anchorIt->second;
    label.posX = anchorPos.x;
    label.posY = anchorPos.y;
    m_currentEdit->setData(label);
    updateAnchorButtons();

    updateSavedLabels();
}

void LabelEditLayer::onRichTextBtn(CCObject*) {
    LabelEditSpecialKeyEditorPopup::create([this](const std::string& key) {
        if (!m_textInput || !m_currentEdit)
            return;

        auto text = m_textInput->getString() + key;
        m_textInput->setString(text);
        auto label = m_currentEdit->getData();
        label.text = text;
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

    labelSettings.labels = labelsData;
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
    orderedLabels.reserve(labelSettings.labels.size());
    for (size_t i = 0; i < labelSettings.labels.size(); i++)
        orderedLabels.emplace_back(i, labelSettings.labels[i]);

    std::stable_sort(orderedLabels.begin(), orderedLabels.end(), [](const auto& a, const auto& b) {
        return a.second.order > b.second.order;
    });

    std::map<Anchor, float> anchorOffsets;
    
    for (const auto& [index, label] : orderedLabels) {
        if (!label.enabled || !label.isOverlayLabelVisibleOnLevel(currentLevel, dtLayer ? (dtLayer->m_MyLevelStats.isOk() ? dtLayer->m_MyLevelStats.unwrap().metadata : LevelMetadata{}) : LevelMetadata{})) continue;

        auto previewLabel = RichTextArea::create(expandDeaths(label.text), labelSettings.font, labelSettings.size * previewScale);
        m_previewLabels.push_back(previewLabel);

        auto isDoner = [](){ return AuthManager::isDoner(); };

        previewLabel->registerRichTextKey(DTLabel::makeGradientKey(isDoner));
        previewLabel->registerRichTextKey(DTLabel::makeWaveKey(isDoner));
        previewLabel->registerRichTextKey(DTLabel::makeCPlusKey());

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

        auto anchor = label.freeMode ? ccp(label.posX, label.posY) : anchorPoint(label.anchor);
        previewLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

        if (!label.freeMode) {
            auto& offset = anchorOffsets[label.anchor];
            auto stackDirection = anchorDirection(label.anchor).y;
            if (stackDirection == 0) stackDirection = -1;
            m_previewArea->addChildAtPosition(previewLabel, label.anchor, ccp(0, stackDirection * offset) + labelSettings.offset * anchorDirection(anchor));
            previewLabel->setAnchorPoint(anchor);
            offset += previewLabel->getScaledContentHeight();
        }
        else {
            m_previewArea->addChild(previewLabel);

            previewLabel->setAnchorPoint(anchor);
            previewLabel->setPosition(m_previewArea->getContentSize() * anchor + labelSettings.offset * anchorDirection(anchor));
        }

        for (const auto& line : previewLabel->getLines())
            line->setOpacity(labelSettings.opacity);
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
            return Err(layer->m_MyLevelStats.unwrapErr().error);
        }

        auto levelStats = layer->m_MyLevelStats.unwrap();
        if (levelStats.from0.isErr()) {
            return Err(levelStats.from0.unwrapErr().error);
        }

        auto data = levelStats.from0.unwrap();
        for (const auto& linkedLevel : layer->linkedLevelsData) {
            if (linkedLevel.from0.isOk() && linkedLevel.levelKey != levelStats.levelKey)
                data += linkedLevel.from0.unwrap();
        }
        return Ok(data);
    }

    auto selectedSession = layer->loadSessionFromSave();
    if (selectedSession.isErr()) {
        return Err(selectedSession.unwrapErr().error);
    }

    if (scope == 's')
        return Ok(selectedSession.unwrap().data);

    auto session = selectedSession.unwrap();
    auto sessionTime = session.sessionStartDate ? session.sessionStartDate : session.groupID;
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

    auto day = layer->daySGroup.grouping.find(dayStart);
    if (day == layer->daySGroup.grouping.end()) {
        return Err("No saved sessions for this day");
    }

    GeneralData data{};
    for (const auto& [sessionDate, levelKeys] : day->second.group) {
        for (const auto& levelKey : levelKeys) {
            auto savedSession = StatsManager::getSession(levelKey, sessionDate);

            if (savedSession.isErr()) {
                return Err(savedSession.unwrapErr().error);
            }

            data += savedSession.unwrap().data;
        }
    }
    return Ok(data);
}

std::string LabelEditLayer::formatDeaths(const GeneralData& data, const std::smatch& match) {
    std::map<int, uint64_t> deathsByPercent;

    auto sumDeaths = [&](const ComplexDeathMap& deathMap) {
        for (const auto& [key, count] : deathMap.toLegacyDeaths()) {
            auto runResult = StatsManager::splitRunKey(key);

            if (runResult.isErr())
                continue;

            auto run = runResult.unwrap();
            auto deathCount = static_cast<uint64_t>(count);

            deathsByPercent[static_cast<int>(run.end)] += deathCount;
        }
    };

    sumDeaths(data.deaths);

    if (match[6].matched) {
        std::vector<std::string> output;

        auto countRes = utils::numFromString<int>(match[6].str());
        if (countRes.isErr())
            return "Invalid count";
        auto count = countRes.unwrap();
        
        std::optional<ComplexPercent> startPercent;
        if (match[7].matched && !match[7].str().empty()) {
            auto startPercentRes = utils::numFromString<double>(match[7].str());
            if (startPercentRes.isErr())
                return "Invalid start percent";
            auto parsedStartPercent = ComplexPercent::fromPercent(startPercentRes.unwrap());
            if (parsedStartPercent.isErr())
                return "Invalid start percent";
            startPercent = parsedStartPercent.unwrap();
        }
        
        std::vector<std::pair<Run, uint64_t>> bestRuns;
        std::map<int64_t, uint64_t> bestRunsByWholePercent;
        auto showDecimals = match[8].matched;
        auto filteredRuns = data.runs.filterCheatedRuns();
        const auto& runData = startPercent ? filteredRuns.records : data.deaths.records;
        for (const auto& entry : runData) {
            auto& death = entry.second;
            bool matchesStart = startPercent ? death.runStartPercent.has_value() && death.runStartPercent->units == startPercent->units : !death.runStartPercent.has_value();
            if (matchesStart) {
                Run run{
                    death.runStartPercent ? std::optional<double>(death.runStartPercent->asPercent()) : std::nullopt,
                    death.runEndPercent.asPercent()
                };
                if (showDecimals) {
                    bestRuns.emplace_back(run, death.deaths);
                }
                else {
                    auto wholePercent = static_cast<int64_t>(std::floor(run.end));
                    auto& count = bestRunsByWholePercent[wholePercent];
                    if (std::numeric_limits<uint64_t>::max() - count < death.deaths)
                        count = std::numeric_limits<uint64_t>::max();
                    else
                        count += death.deaths;
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
            return a.first.end > b.first.end;
        });

        for (int i = 0; i < count && i < static_cast<int>(bestRuns.size()); i++) {
            auto decimalPrecision = match[8].matched ? Settings::getDecimalPrecision() : 0;

            std::string entry;

            if (bestRuns[i].first.start.has_value()) {
                entry = fmt::format("{:.{}f}-{:.{}f}% X{}", bestRuns[i].first.start.value(), decimalPrecision, bestRuns[i].first.end, decimalPrecision, bestRuns[i].second);
            }
            else {
                entry = fmt::format("{:.{}f}% X{}", bestRuns[i].first.end, decimalPrecision, bestRuns[i].second);
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

    if (match[2].matched) {
        auto minPercentRes = utils::numFromString<int>(match[2].str());
        if (minPercentRes.isErr())
            return "Invalid min percent";
        minPercent = minPercentRes.unwrap();

        exclusiveMin = match[0].str().find(">=") == std::string::npos;
    }
    else if (match[3].matched) {
        auto maxPercentRes = utils::numFromString<int>(match[3].str());
        if (maxPercentRes.isErr())
            return "Invalid max percent";
        maxPercent = maxPercentRes.unwrap();
        exclusiveMax = true;
    }
    else {
        auto minPercentRes = utils::numFromString<int>(match[4].str());
        if (minPercentRes.isErr())
            return "Invalid min percent";
        minPercent = minPercentRes.unwrap();

        auto maxPercentRes = utils::numFromString<int>(match[5].str());
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

std::string LabelEditLayer::expandDeaths(const std::string& text) {
    return expandDeaths(text, [](char scope) {
        return getDeathsData(scope);
    });
}

std::string LabelEditLayer::expandDeaths(const std::string& text, const std::function<Result<GeneralData>(char)>& dataProvider) {
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
            for (const auto& [key, death] : data->deaths.records) {
                for (const auto& occurrence : death.occurrences) {
                    if (!occurrence.runEndTimestamp)
                        continue;

                    auto timestamp = occurrence.runEndTimestamp.value();
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
                    latestDeath->runEndPercent.asPercent(),
                    Settings::getDecimalPrecision(),
                    latestDeath->deaths
                );
            }
            else {
                auto wholePercent = static_cast<int64_t>(std::floor(latestDeath->runEndPercent.asPercent()));
                auto legacyDeaths = data->deaths.toLegacyDeaths();
                auto count = legacyDeaths.find(std::to_string(wholePercent));
                expandedCurrentDeathCount += fmt::format(
                    "{}% X{}",
                    wholePercent,
                    count == legacyDeaths.end() ? latestDeath->deaths : count->second
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
            expanded += formatDeaths(data.value(), match);
        } else {
            expanded += "[None]";
        }
        
        textStart = match[0].second;
    }

    expanded.append(textStart, expandedCurrentDeathCount.cend());
    return expanded;
}

bool LabelEditLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    auto label = m_currentEdit ? m_currentEdit->getData() : OverlayLabel{};

    if (m_currentEdit && label.freeMode && m_currentPreviewLabel) {
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

    label.posX = dragAxis(
        touchPosition.x, m_dragTouchLocalPoint.x, labelSize.width,
        previewSize.width, settings.offset.x, m_currentPreviewLabel->getScaleX(), label.posX
    );
    label.posY = dragAxis(
        touchPosition.y, m_dragTouchLocalPoint.y, labelSize.height,
        previewSize.height, settings.offset.y, m_currentPreviewLabel->getScaleY(), label.posY
    );
    m_currentEdit->setData(label);

    m_updatingControls = true;
    m_xSlider->setValue(label.posX);
    m_ySlider->setValue(label.posY);
    m_updatingControls = false;

    auto anchor = ccp(label.posX, label.posY);
    m_currentPreviewLabel->setAnchorPoint(anchor);
    m_currentPreviewLabel->setPosition(previewSize * anchor + settings.offset * anchorDirection(anchor));
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