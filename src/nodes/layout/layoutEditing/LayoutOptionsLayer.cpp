#include "LayoutOptionsLayer.hpp"

#include <nodes/DTLayer.hpp>
#include <nodes/layout/layoutEditing/SpecialKeyCell.hpp>

#include <nodes/layout/usage/DTLabel.hpp>
#include <nodes/layout/usage/LayoutColumn.hpp>
#include <nodes/layout/layoutEditing/FontSelectionCell.hpp>
#include <nodes/general/SimpleToggler.hpp>

/*
settings needed:

- for labels:
    - labels name (string) :D
    - text color (ccColor3B) :D
    - font size (float) :XD
    - label color (ccColor3B) :D
    - text (string) :D
    - alignment (CCTextAlignment) :D
    - special key selection (other UI)
    - font
    - movement and expanding options maybe

- for columns:
    - column width (float)
    - column position (float)
    - color (ccColor3B)

- for both:
    - delete (button)
*/

LayoutOptionsLayer* LayoutOptionsLayer::create(CCSize size) {
    auto ret = new LayoutOptionsLayer();

    if (ret && ret->init(std::move(size))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool LayoutOptionsLayer::init(CCSize size) {
    if (!CCLayer::init()) return false;

    this->m_size = std::move(size);

    this->setContentSize(m_size);

    auto bg = CCScale9Sprite::create("geode.loader/GE_square01.png");
    bg->setContentSize(m_size);
    bg->setPosition(m_size / 2);
    bg->setID("background");
    this->addChild(bg);

    auto superBlocker = CCMenu::create();
    superBlocker->setPosition({0, 0});
    superBlocker->setContentSize(m_size);
    superBlocker->setID("super-blocker");
    this->addChild(superBlocker);

    auto blockerItem = CCMenuItem::create();
    blockerItem->setContentSize(m_size);
    blockerItem->setID("blocker-item");
    blockerItem->setAnchorPoint({0, 0});
    superBlocker->addChild(blockerItem);

    auto generalBtnsMenu = CCMenu::create();
    generalBtnsMenu->setPosition({m_size.width / 2, m_size.height - 10});
    generalBtnsMenu->setContentSize({0, 0});
    generalBtnsMenu->setID("general-btns-menu");
    generalBtnsMenu->setZOrder(10);
    this->addChild(generalBtnsMenu);

    m_labelSettingsNode = CCMenu::create();
    m_labelSettingsNode->setPosition({m_size.width / 2, m_size.height - 10});
    m_labelSettingsNode->setContentSize({0, 0});
    m_labelSettingsNode->setID("label-settings-node");
    this->addChild(m_labelSettingsNode);

    m_columnSettingsNode = CCMenu::create();
    m_columnSettingsNode->setPosition({m_size.width / 2, m_size.height - 10});
    m_columnSettingsNode->setContentSize({0, 0});
    m_columnSettingsNode->setID("column-settings-node");
    this->addChild(m_columnSettingsNode);

    m_fontSelectionNode = CCMenu::create();
    m_fontSelectionNode->setPosition({m_size.width / 2, m_size.height - 10});
    m_fontSelectionNode->setContentSize({0, 0});
    m_fontSelectionNode->setID("font-selection-node");
    this->addChild(m_fontSelectionNode);

    m_specialKeysNode = CCMenu::create();
    m_specialKeysNode->setPosition({m_size.width / 2, m_size.height - 10});
    m_specialKeysNode->setContentSize({0, 0});
    m_specialKeysNode->setID("special-keys-node");
    this->addChild(m_specialKeysNode);

    m_labelNameInput = TextInput::create((m_size.width - 10) / .75f, "label name");
    m_labelNameInput->setID("label-name-input");
    m_labelNameInput->setScale(.75f);
    m_labelNameInput->setCallback([&](const std::string& newStr) {
        if (!m_editedLabel.has_value()) return;

        m_editedLabel.value()->setLabelName(newStr);
    });
    m_labelNameInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ <>=,.");
    m_labelNameInput->setPositionY(-22);
    m_labelSettingsNode->addChild(m_labelNameInput);

    auto labelNameInputLabel = geode::Label::create("Label Name", "bigFont.fnt");
    labelNameInputLabel->setScale(.4f);
    labelNameInputLabel->setPosition(m_labelNameInput->getPosition() + ccp(0, m_labelNameInput->getScaledContentHeight() / 2 + labelNameInputLabel->getScaledContentHeight() / 2));
    labelNameInputLabel->setID("name-input-label");
    m_labelSettingsNode->addChild(labelNameInputLabel);

    m_labelTextInput = TextInput::createMultiline({(m_size.width - 10) / .75f, 60}, "label text");
    m_labelTextInput->setScale(.75f);
    m_labelTextInput->setCommonFilter(CommonFilter::Any);
    m_labelTextInput->setID("label-text-input");
    m_labelTextInput->setPositionY(-70);
    m_labelTextInput->setCallback([&](const std::string& newStr) {
        if (!m_editedLabel.has_value()) return;

        m_editedLabel.value()->setLabelText(newStr);

        m_labelTextSpecialKeysInput->setString(newStr);
    });
    m_labelSettingsNode->addChild(m_labelTextInput);

    auto labelTextInputLabel = geode::Label::create("Text", "bigFont.fnt");
    labelTextInputLabel->setScale(.4f);
    labelTextInputLabel->setPosition(m_labelTextInput->getPosition() + ccp(0, m_labelTextInput->getScaledContentHeight() / 2 + labelTextInputLabel->getScaledContentHeight() / 2));
    labelTextInputLabel->setID("text-label");
    m_labelSettingsNode->addChild(labelTextInputLabel);

    auto keysBtnSpr = ButtonSprite::create("Keys");
    keysBtnSpr->setScale(.35f);
    auto keysBtn = CCMenuItemSpriteExtra::create(
        keysBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onSpecialKeysClicked)
    );
    keysBtn->setID("special-keys-page-btn");
    keysBtn->setPosition(labelTextInputLabel->getPosition() + ccp(
        labelTextInputLabel->getScaledContentWidth() / 2 + keysBtn->getScaledContentWidth() / 2,
        0
    ));
    m_labelSettingsNode->addChild(keysBtn);

    m_labelColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    m_labelColorBtnSprite->setID("color-spr");
    m_labelColorBtnSprite->setScale(.6f);
    auto labelColorBtn = CCMenuItemSpriteExtra::create(
        m_labelColorBtnSprite,
        this,
        menu_selector(LayoutOptionsLayer::onLabelColorBtnClicked)
    );
    labelColorBtn->setID("label-color-btn");
    labelColorBtn->setPosition({-15, -120});
    m_labelSettingsNode->addChild(labelColorBtn);

    auto labelColorBtnLabel = geode::Label::create("label:", "bigFont.fnt");
    labelColorBtnLabel->setScale(.4f);
    labelColorBtnLabel->setPosition(labelColorBtn->getPosition() + ccp(-labelColorBtn->getScaledContentWidth() / 2 - labelColorBtnLabel->getScaledContentWidth() / 2 - 2, 0));
    labelColorBtnLabel->setID("label-color-label");
    m_labelSettingsNode->addChild(labelColorBtnLabel);

    m_textColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    m_textColorBtnSprite->setID("color-spr");
    m_textColorBtnSprite->setScale(.6f);
    auto textColorBtn = CCMenuItemSpriteExtra::create(
        m_textColorBtnSprite,
        this,
        menu_selector(LayoutOptionsLayer::onTextColorBtnClicked)
    );
    textColorBtn->setID("text-color-btn");
    textColorBtn->setPosition({55, -120});
    m_labelSettingsNode->addChild(textColorBtn);

    auto textColorBtnLabel = geode::Label::create("text:", "bigFont.fnt");
    textColorBtnLabel->setScale(.4f);
    textColorBtnLabel->setPosition(textColorBtn->getPosition() + ccp(-textColorBtn->getScaledContentWidth() / 2 - textColorBtnLabel->getScaledContentWidth() / 2 - 2, 0));
    textColorBtnLabel->setID("text-color-label");
    m_labelSettingsNode->addChild(textColorBtnLabel);

    auto colorLabel = geode::Label::create("color", "bigFont.fnt");
    colorLabel->setScale(.4f);
    colorLabel->setPosition({0, -100});
    colorLabel->setID("color-label");
    m_labelSettingsNode->addChild(colorLabel);

    m_fontSizeInput = TextInput::create((m_size.width - 100) / .75f, "size");
    m_fontSizeInput->setScale(.75f);
    m_fontSizeInput->setPositionY(-195);
    m_fontSizeInput->setPositionX(-32);
    m_fontSizeInput->setCommonFilter(CommonFilter::Float);
    m_fontSizeInput->setCallback([&](const std::string& newStr) {
        if (!m_editedLabel.has_value()) return;
        
        auto newSizeRes = geode::utils::numFromString<float>(newStr);
        float newSize = 0.0f;

        if (!newSizeRes.isErr()) newSize = newSizeRes.unwrap();

        m_scaleSlider->setValue((newSize - DTLabelInfo::MIN_MAX_SCALE.x) / (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x));
        m_editedLabel.value()->setFontSize(newSize);
    });
    m_fontSizeInput->setID("font-size-input");
    m_labelSettingsNode->addChild(m_fontSizeInput);

    auto fontSizeInputLabel = geode::Label::create("Font Size", "bigFont.fnt");
    fontSizeInputLabel->setScale(.35f);
    fontSizeInputLabel->setPosition(m_fontSizeInput->getPosition() + ccp(0, m_fontSizeInput->getScaledContentHeight() / 2 + fontSizeInputLabel->getScaledContentHeight() / 2));
    fontSizeInputLabel->setID("font-size-label");
    m_labelSettingsNode->addChild(fontSizeInputLabel);

    m_scaleSlider = Slider::create(this, menu_selector(LayoutOptionsLayer::scaleSliderChanged), .5f);
    m_scaleSlider->setContentSize({0, 0});
    m_scaleSlider->setPositionY(-215);
    //scaleSlider->setPositionX(fontSizeInput->getPositionX() + 30);
    m_scaleSlider->setID("font-size-slider");
    m_scaleSlider->m_delegate = this;
    m_labelSettingsNode->addChild(m_scaleSlider);

    m_alignmentMenu = CCMenu::create();
    m_alignmentMenu->setPosition({0, -160});
    m_alignmentMenu->setContentSize({133, 28});
    m_alignmentMenu->setID("alignment-menu");
    m_alignmentMenu->setScale(.8f);
    m_labelSettingsNode->addChild(m_alignmentMenu);
    m_alignmentMenu->ignoreAnchorPointForPosition(false);
    m_alignmentMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(5)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Center)
    );

    auto alignmentLabel = geode::Label::create("Alignment", "bigFont.fnt");
    alignmentLabel->setID("alignment-label");
    alignmentLabel->setScale(.4f);
    alignmentLabel->setPosition(m_alignmentMenu->getPosition() + ccp(0, 5 + m_alignmentMenu->getScaledContentHeight() / 2 + alignmentLabel->getScaledContentHeight() / 2));
    m_labelSettingsNode->addChild(alignmentLabel);

    auto leftAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignLeft.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    leftAlignBtn->setID("left-align-btn");
    m_alignmentMenu->addChild(leftAlignBtn);

    auto centerAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignCenter.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    centerAlignBtn->setID("center-align-btn");
    m_alignmentMenu->addChild(centerAlignBtn);

    auto rightAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignRight.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    rightAlignBtn->setID("right-align-btn");
    m_alignmentMenu->addChild(rightAlignBtn);

    m_alignmentMenu->updateLayout();

    auto wrapModeCutoff = ButtonSprite::create("CUTOFF");
    wrapModeCutoff->setID("cutoff");
    wrapModeCutoff->setScale(.5f);
    auto wrapModeSpace = ButtonSprite::create("SPACE");
    wrapModeSpace->setID("space");
    wrapModeSpace->setVisible(false);
    auto wrapModeWord = ButtonSprite::create("WORD");
    wrapModeWord->setID("word");
    wrapModeWord->setVisible(false);

    m_wrappingModeBtn = CCMenuItemSpriteExtra::create(
        wrapModeCutoff,
        this,
        menu_selector(LayoutOptionsLayer::onWrappingBtn)
    );
    m_wrappingModeBtn->setID("wrapping-mode-btn");
    
    wrapModeSpace->setPosition(wrapModeCutoff->getPosition());
    wrapModeSpace->setScale(wrapModeCutoff->getScale());
    m_wrappingModeBtn->addChild(wrapModeSpace);
    wrapModeWord->setPosition(wrapModeCutoff->getPosition());
    wrapModeWord->setScale(wrapModeCutoff->getScale());
    m_wrappingModeBtn->setPositionY(m_fontSizeInput->getPositionY());
    m_wrappingModeBtn->setPositionX(32);
    m_wrappingModeBtn->addChild(wrapModeWord);

    auto wrappingModeLabel = geode::Label::create("Wrapping", "bigFont.fnt");
    wrappingModeLabel->setScale(.35f);
    wrappingModeLabel->setPosition({m_wrappingModeBtn->getPositionX(), fontSizeInputLabel->getPositionY()});
    wrappingModeLabel->setID("wrapping-label");
    m_labelSettingsNode->addChild(wrappingModeLabel);

    m_labelSettingsNode->addChild(m_wrappingModeBtn);

    auto fontSelectionBtnSpr = ButtonSprite::create("Choose");
    fontSelectionBtnSpr->setScale(.45f);
    auto fontSelectionBtn = CCMenuItemSpriteExtra::create(
        fontSelectionBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onFontSelection)
    );
    fontSelectionBtn->setPositionY(-260);
    fontSelectionBtn->setPositionX(m_size.width / 2 - 5 - fontSelectionBtn->getContentWidth() / 2);
    fontSelectionBtn->setID("font-selection-btn");
    m_labelSettingsNode->addChild(fontSelectionBtn);

    updateFontSelectFont();

    m_hideHeaderToggler = SimpleToggler::createWithDefaults(
        .65f
    );
    m_hideHeaderToggler->setPositionY(fontSelectionBtn->getPositionY() + fontSelectionBtn->getContentHeight() / 2 + m_hideHeaderToggler->getContentHeight() / 2 + 7.5f);
    m_hideHeaderToggler->setPositionX(m_size.width / 2 - 20 - m_hideHeaderToggler->getContentWidth() / 2);
    m_hideHeaderToggler->setID("hide-header-toggler");
    m_hideHeaderToggler->setCallback([&](auto state) {
        if (!m_editedLabel.has_value()) return;

        m_editedLabel.value()->setHideHeader(state);
    });
    m_labelSettingsNode->addChild(m_hideHeaderToggler);

    auto hideHeaderTogglerLabel = geode::Label::create("Hide Header", "bigFont.fnt");
    hideHeaderTogglerLabel->setScale(.4f);
    hideHeaderTogglerLabel->setPosition(m_hideHeaderToggler->getPosition() + ccp(-m_hideHeaderToggler->getContentWidth() / 2 - 10, 0));
    hideHeaderTogglerLabel->setID("hide-header-label");
    hideHeaderTogglerLabel->setAnchorPoint({1, .5f});
    m_labelSettingsNode->addChild(hideHeaderTogglerLabel);

    m_fontSelectionNode->setScaleY(0);
    m_labelSettingsNode->setScaleY(0);
    m_columnSettingsNode->setScaleY(0);
    m_specialKeysNode->setScaleY(0);

    auto backBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    backBtnSpr->setScale(.65f);
    auto backBtn = CCMenuItemSpriteExtra::create(
        backBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onBack)
    );
    backBtn->setID("back-btn");
    backBtn->setPosition({-75, 6});
    generalBtnsMenu->addChild(backBtn);
    
    auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    deleteBtnSpr->setScale(.65f);
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onDelete)
    );
    deleteBtn->setID("delete-btn");
    deleteBtn->setPosition({62, 6});
    generalBtnsMenu->addChild(deleteBtn);

    m_fontsScroll = ScrollLayer::create(m_size - ccp(8, 30));
    m_fontsScroll->setPosition({-m_size.width / 2, -m_size.height + 20});
    m_fontsScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(m_fontsScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    m_fontsScroll->ignoreAnchorPointForPosition(true);
    m_fontSelectionNode->addChild(m_fontsScroll);

    auto fontsScrollbar = Scrollbar::create(m_fontsScroll);
    fontsScrollbar->setPosition({65, m_fontsScroll->getPositionY()});
    fontsScrollbar->ignoreAnchorPointForPosition(true);
    m_fontSelectionNode->addChild(fontsScrollbar);

    for (const auto& font : StatsManager::getAllFonts())
    {
        auto cell = FontSelectionCell::create(font, [&](auto cell) {onFontSelected(cell);});
        m_fontsScroll->m_contentLayer->addChild(cell);

        m_allFontCells.insert({font, cell});
    }

    m_fontsScroll->m_contentLayer->updateLayout();
    m_fontsScroll->moveToTop();

    m_fontsScroll->setMouseEnabled(false);


    m_labelTextSpecialKeysInput = TextInput::createMultiline({(m_size.width - 10) / .75f, 80} , "label text");
    m_labelTextSpecialKeysInput->setScale(.75f);
    m_labelTextSpecialKeysInput->setCommonFilter(CommonFilter::Any);
    m_labelTextSpecialKeysInput->setID("label-text-input");
    m_labelTextSpecialKeysInput->setPositionY(-42);
    m_labelTextSpecialKeysInput->setCallback([&](const std::string& newStr) {
        if (!m_editedLabel.has_value()) return;

        m_editedLabel.value()->setLabelText(newStr);

        m_labelTextInput->setString(newStr);
    });
    m_specialKeysNode->addChild(m_labelTextSpecialKeysInput);

    auto labelTextSpecialKeysInputLabel = geode::Label::create("text", "bigFont.fnt");
    labelTextSpecialKeysInputLabel->setScale(.4f);
    labelTextSpecialKeysInputLabel->setPosition(m_labelTextSpecialKeysInput->getPosition() + ccp(0, m_labelTextSpecialKeysInput->getScaledContentHeight() / 2 + labelTextSpecialKeysInputLabel->getScaledContentHeight() / 2));
    labelTextSpecialKeysInputLabel->setID("text-Label");
    m_specialKeysNode->addChild(labelTextSpecialKeysInputLabel);

    m_specialKeysScroll = ScrollLayer::create(m_size - ccp(8, 100));
    m_specialKeysScroll->setPosition({-m_size.width / 2, -m_size.height + 20});
    m_specialKeysScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(m_specialKeysScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    m_specialKeysScroll->ignoreAnchorPointForPosition(true);
    m_specialKeysNode->addChild(m_specialKeysScroll);

    auto specialKeysScrollbar = Scrollbar::create(m_specialKeysScroll);
    specialKeysScrollbar->setPosition({65, m_specialKeysScroll->getPositionY()});
    specialKeysScrollbar->ignoreAnchorPointForPosition(true);
    m_specialKeysNode->addChild(specialKeysScrollbar);

    for (const auto& [key, keyObj] : DTLayer::get()->m_specialStrings)
    {
        if (!keyObj->shouldShowInAddList()) continue;

        auto cell = SpecialKeyCell::create(keyObj, [&](auto str) {LayoutOptionsLayer::onSpecialKeyAdded(str);});
        m_specialKeysScroll->m_contentLayer->addChild(cell);
    }

    auto keysScrollFixer = CCNode::create();
    keysScrollFixer->setContentHeight(3.5f);
    m_specialKeysScroll->m_contentLayer->addChild(keysScrollFixer);

    m_specialKeysScroll->m_contentLayer->updateLayout();
    m_specialKeysScroll->moveToTop();
    m_specialKeysScroll->setMouseEnabled(false);

    m_columnWidthInput = TextInput::create((m_size.width - 10) / .75f, "Width");
    m_columnWidthInput->setScale(.75f);
    m_columnWidthInput->setCommonFilter(CommonFilter::Float);
    m_columnWidthInput->setID("column-width-input");
    m_columnWidthInput->setPositionY(-30);
    m_columnWidthInput->setCallback([&](const std::string& newStr) {
        if (!m_editedColumn.has_value()) return;

        float endValue = DTColumnInfo::minWidth;

        auto toNumRes = geode::utils::numFromString<float>(newStr);
        if (toNumRes.isOk()) endValue = toNumRes.unwrap();

        m_editedColumn.value()->setContentWidth(std::max(DTColumnInfo::minWidth, endValue));

        m_editedColumn.value()->updateSizesByContent();

        m_ignoreNextOrganization = true;
        DTLayer::get()->organizeLayout();
    });
    m_columnSettingsNode->addChild(m_columnWidthInput);

    auto columnWidthInputLabel = geode::Label::create("column width", "bigFont.fnt");
    columnWidthInputLabel->setScale(.4f);
    columnWidthInputLabel->setPosition(m_columnWidthInput->getPosition() + ccp(0, m_columnWidthInput->getScaledContentHeight() / 2 + columnWidthInputLabel->getScaledContentHeight() / 2));
    columnWidthInputLabel->setID("width-Label");
    m_columnSettingsNode->addChild(columnWidthInputLabel);

    auto addColumnMenu = CCMenu::create();
    addColumnMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::Fit)
        ->setGap(5)
        ->setMinRelativeScale(std::nullopt)
    );
    m_columnSettingsNode->addChild(addColumnMenu);

    auto addColumnToLeftBtnSpr = ButtonSprite::create("Add Column\nto Left");
    auto addColumnToLeftBtn = CCMenuItemSpriteExtra::create(
        addColumnToLeftBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onAddColumnToLeft)
    );
    addColumnMenu->addChild(addColumnToLeftBtn);

    auto addColumnToRightBtnSpr = ButtonSprite::create("Add Column\nto Right");
    auto addColumnToRightBtn = CCMenuItemSpriteExtra::create(
        addColumnToRightBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onAddColumnToRight)
    );
    addColumnMenu->addChild(addColumnToRightBtn);

    addColumnMenu->setContentWidth(m_size.width - 10);
    addColumnMenu->updateLayout();
    addColumnMenu->setPositionY(-m_size.height + addColumnMenu->getContentHeight() + 5);
    addColumnMenu->setPositionX(0);

    DTLayer::get()->subscribeToOrganizationEvent(this, [&](auto _) {
        if (!m_editedColumn.has_value()) return;
        if (m_ignoreNextOrganization) {
            m_ignoreNextOrganization = false;
            return;
        }

        auto widthText = std::to_string(m_editedColumn.value()->getInfo().m_currentWidth);
        for (int i = widthText.length() - 1; i >= 0; i--)
        {
            if (widthText[i] != '0') break;

            widthText.pop_back();
        }
        
        if (widthText.length() != 0 && widthText[widthText.length() - 1] == '.') widthText.pop_back();

        m_columnWidthInput->setString(widthText);
    });

    setAllInputs(m_labelSettingsNode, false);
    setAllInputs(m_columnSettingsNode, false);
    setAllInputs(m_fontSelectionNode, false);
    setAllInputs(m_specialKeysNode, false);

    return true;
}

void LayoutOptionsLayer::setEditedNodeTo(DTLabel* label) {
    if (m_editedLabel.has_value() && m_editedLabel.value() == label) return;

    if (m_editedLabel.has_value()) m_editedLabel.value()->onBeingEditedEnded();

    m_editedLabel = label;
    m_editedColumn = std::nullopt;

    label->onBeingEdited();

    if (!label->getInfo().m_isExpanded)
        label->toggleExpand(nullptr);

    onAlignmentChanged(m_alignmentMenu->getChildByID(fmt::format(
        "{}-align-btn",
        label->getInfo().m_horizontalAlignment == CCTextAlignment::kCCTextAlignmentLeft ? "left" : (
            label->getInfo().m_horizontalAlignment == CCTextAlignment::kCCTextAlignmentCenter ? "center" : "right"
        )
    )));

    m_labelColorBtnSprite->setColor({label->getInfo().m_labelColor.r, label->getInfo().m_labelColor.g, label->getInfo().m_labelColor.b});
    m_textColorBtnSprite->setColor({label->getInfo().m_textColor.r, label->getInfo().m_textColor.g, label->getInfo().m_textColor.b});

    m_labelNameInput->setString(label->getInfo().m_labelName);
    m_labelTextInput->setString(label->getInfo().m_text);
    m_labelTextSpecialKeysInput->setString(label->getInfo().m_text);

    m_scaleSlider->setValue(
        (label->getInfo().m_scale - DTLabelInfo::MIN_MAX_SCALE.x) / (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x)
    );
    scaleSliderChanged(nullptr);

    updateFontSelectFont();
    if (m_currentlySelectedFontCell != nullptr)
        m_currentlySelectedFontCell->deselect();
    m_currentlySelectedFontCell = m_allFontCells[label->getInfo().m_font];
    m_currentlySelectedFontCell->select();

    m_hideHeaderToggler->toggle(label->getInfo().m_hideHeader);

    switchToMenu(1);

    updateWrapModeBtnVisuals();

    geode::Label::Alignment textAlignment;

    switch (label->getInfo().m_horizontalAlignment)
    {
    default:
    case kCCTextAlignmentLeft:
        textAlignment = geode::Label::Alignment::Left;
        break;
    
    case kCCTextAlignmentCenter:
        textAlignment = geode::Label::Alignment::Center;
        break;

    case kCCTextAlignmentRight:
        textAlignment = geode::Label::Alignment::Right;
        break;
    }

    m_labelTextInput->setAlignment(textAlignment);
    m_labelTextSpecialKeysInput->setAlignment(textAlignment);

    if (m_onEnter != NULL)
        m_onEnter();
}

void LayoutOptionsLayer::setEditedNodeTo(LayoutColumn* column) {
    if (m_editedColumn.has_value() && m_editedColumn.value() == column) return;

    if (m_editedLabel.has_value()) m_editedLabel.value()->onBeingEditedEnded();

    m_editedColumn = column;
    m_editedLabel = std::nullopt;

    auto widthText = std::to_string(column->getInfo().m_currentWidth);
    for (int i = widthText.length() - 1; i >= 0; i--)
    {
        if (widthText[i] != '0') break;

        widthText.pop_back();
    }
    
    if (widthText.length() != 0 && widthText[widthText.length() - 1] == '.') widthText.pop_back();

    m_columnWidthInput->setString(widthText);

    switchToMenu(2);

    if (m_onEnter != NULL)
        m_onEnter();
}

void LayoutOptionsLayer::close() {
    //close the options layer
    switchToMenu(0);

    if (m_editedLabel.has_value()) m_editedLabel.value()->onBeingEditedEnded();
    
    m_editedLabel = std::nullopt;
    m_editedColumn = std::nullopt;
    m_colorChangeFunc = NULL;
}

void LayoutOptionsLayer::onTextColorBtnClicked(CCObject*) {
    if (!m_editedLabel.has_value()) return;

    auto popup = geode::ColorPickPopup::create(m_editedLabel.value()->getInfo().m_textColor);
    popup->setCallback([&](auto color) { updateColor(color); });
    popup->setColorTarget(m_textColorBtnSprite);
    m_colorChangeFunc = [&](cocos2d::ccColor4B const& color) {
        m_editedLabel.value()->setTextColor(color);
    };
    popup->show();
}
void LayoutOptionsLayer::onLabelColorBtnClicked(CCObject*) {
    if (!m_editedLabel.has_value()) return;

    auto popup = geode::ColorPickPopup::create(m_editedLabel.value()->getInfo().m_labelColor);
    popup->setCallback([&](auto color) { updateColor(color); });
    popup->setColorTarget(m_labelColorBtnSprite);
    m_colorChangeFunc = [&](cocos2d::ccColor4B const& color) {
        m_editedLabel.value()->setLabelColor(color);
    };
    popup->show();
}

void LayoutOptionsLayer::updateColor(cocos2d::ccColor4B const& color) {
    if (m_colorChangeFunc == NULL) return;

    m_colorChangeFunc(color);
}

void LayoutOptionsLayer::onAlignmentChanged(CCObject* sender) {
    if (!m_editedLabel.has_value()) return;

    auto menuItem = static_cast<CCMenuItemSpriteExtra*>(sender);

    for (const auto& alignmentButton : CCArrayExt<CCMenuItemSpriteExtra*>(m_alignmentMenu->getChildren()))
    {
        alignmentButton->setOpacity(255);
        alignmentButton->setEnabled(true);
    }

    CCTextAlignment alignment;


    if (menuItem->getID() == "left-align-btn") {
        alignment = kCCTextAlignmentLeft;
    } else if (menuItem->getID() == "center-align-btn") {
        alignment = kCCTextAlignmentCenter;
    } else if (menuItem->getID() == "right-align-btn") {
        alignment = kCCTextAlignmentRight;
    } else {
        return;
    }

    menuItem->setOpacity(100);
    menuItem->setEnabled(false);

    m_editedLabel.value()->setTextAlignment(alignment);

    geode::Label::Alignment textAlignment;

    switch (alignment)
    {
    default:
    case kCCTextAlignmentLeft:
        textAlignment = geode::Label::Alignment::Left;
        break;
    
    case kCCTextAlignmentCenter:
        textAlignment = geode::Label::Alignment::Center;
        break;

    case kCCTextAlignmentRight:
        textAlignment = geode::Label::Alignment::Right;
        break;
    }

    m_labelTextInput->setAlignment(textAlignment);
    m_labelTextSpecialKeysInput->setAlignment(textAlignment);
}

void LayoutOptionsLayer::scaleSliderChanged(CCObject*) {
    #if !defined(GEODE_IS_MOBILE)
    sliderUpdate();
    #endif
}

void LayoutOptionsLayer::sliderUpdate() {
    auto numValue = DTLabelInfo::MIN_MAX_SCALE.x + (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x) * m_scaleSlider->getValue();

    m_editedLabel.value()->setFontSize(numValue);

    auto scaleText = std::to_string(numValue);
    for (int i = scaleText.length() - 1; i >= 0; i--)
    {
        if (scaleText[i] != '0') break;

        scaleText.pop_back();
    }
    
    if (scaleText.length() != 0 && scaleText[scaleText.length() - 1] == '.') scaleText.pop_back();

    m_fontSizeInput->setString(scaleText);
}

void LayoutOptionsLayer::onFontSelection(CCObject*) {
    switchToMenu(3);
}

void LayoutOptionsLayer::switchToMenu(uint8_t menuID) {
    m_labelSettingsNode->stopAllActions();
    m_columnSettingsNode->stopAllActions();
    m_fontSelectionNode->stopAllActions();
    m_specialKeysNode->stopAllActions();

    m_currentPage = menuID;

    m_fontsScroll->setMouseEnabled(false);
    m_specialKeysScroll->setMouseEnabled(false);

    auto exitEasing = [](bool open) -> CCEaseExponentialOut* {
        return CCEaseExponentialOut::create(CCScaleTo::create(.2f, 1, open ? 1 : 0));
    };

    m_labelSettingsNode->runAction(exitEasing(false));
    m_columnSettingsNode->runAction(exitEasing(false));
    m_fontSelectionNode->runAction(exitEasing(false));
    m_specialKeysNode->runAction(exitEasing(false));
    setAllInputs(m_labelSettingsNode, false);
    setAllInputs(m_columnSettingsNode, false);
    setAllInputs(m_fontSelectionNode, false);
    setAllInputs(m_specialKeysNode, false);

    CCNode* selectedNode = nullptr;

    switch (menuID)
    {

    case 1:
        selectedNode = m_labelSettingsNode;
        break;

    case 2:
        selectedNode = m_columnSettingsNode;
        break;

    case 3:
        selectedNode = m_fontSelectionNode;
        m_fontsScroll->setMouseEnabled(true);
        m_fontsScroll->moveToTop();
        break;

    case 4:
        selectedNode = m_specialKeysNode;
        m_specialKeysScroll->setMouseEnabled(true);
        m_specialKeysScroll->moveToTop();
        break;
    
    default:
        break;
    }

    if (selectedNode != nullptr) {
        setAllInputs(selectedNode, true);

        selectedNode->stopAllActions();
        selectedNode->runAction(CCSequence::create(
            exitEasing(false),
            CCDelayTime::create(.1f),
            exitEasing(true),
            nullptr
        ));
    }
}

void LayoutOptionsLayer::onBack(CCObject*) {
    if (m_currentPage == 3) {
        switchToMenu(m_editedLabel.has_value() ? 1 : 2);
        return;
    }
    else if (m_currentPage == 4) {
        switchToMenu(1);
        return;
    }

    if (m_onBackedOut != NULL)
        m_onBackedOut();
}

void LayoutOptionsLayer::onDelete(CCObject*) {
    if (m_editedLabel.has_value()) {
        m_editedLabel.value()->removeFromColumns();
        m_editedLabel.value()->removeFromParentAndCleanup(true);
        DTLayer::get()->organizeLayout();
        if (m_onBackedOut != NULL)
            m_onBackedOut();
    }
    else if (m_editedColumn.has_value()) {
        m_editedColumn.value()->destroyColumnAndCleanup();
        DTLayer::get()->organizeLayout();
        if (m_onBackedOut != NULL)
            m_onBackedOut();
    }
} 

void LayoutOptionsLayer::onFontSelected(FontSelectionCell* cell) {
    if (!isEditingNode()) return;

    if (m_currentlySelectedFontCell != nullptr) m_currentlySelectedFontCell->deselect();
    m_currentlySelectedFontCell = cell;
    m_currentlySelectedFontCell->select();

    if (m_editedLabel.has_value()) {
        m_editedLabel.value()->setFont(cell->getFont());
        DTLayer::get()->organizeLayout();
        updateFontSelectFont();
    }
}

void LayoutOptionsLayer::onWrappingBtn(CCObject*) {
    if (!m_editedLabel.has_value()) return;

    WrappingMode newMode = WrappingMode::CUTOFF_WRAP;

    if (m_wrappingModeBtn->getChildByID("cutoff")->isVisible()) {
        newMode = WrappingMode::SPACE_WRAP;
    }
    else if (m_wrappingModeBtn->getChildByID("space")->isVisible()) {
        newMode = WrappingMode::WORD_WRAP;
    }
    else if (m_wrappingModeBtn->getChildByID("word")->isVisible()) {
        newMode = WrappingMode::CUTOFF_WRAP;
    }

    m_editedLabel.value()->setTextWrapping(newMode);

    updateWrapModeBtnVisuals();
}

void LayoutOptionsLayer::updateWrapModeBtnVisuals() {
    if (!m_editedLabel.has_value()) return;

    for (const auto& child : CCArrayExt<CCNode*>(m_wrappingModeBtn->getChildren()))
    {
        child->setVisible(false);
    }

    switch (m_editedLabel.value()->getInfo().m_wrapping)
    {
    case WrappingMode::CUTOFF_WRAP:
        m_wrappingModeBtn->getChildByID("cutoff")->setVisible(true);
        break;
    case WrappingMode::SPACE_WRAP:
        m_wrappingModeBtn->getChildByID("space")->setVisible(true);
        break;
    case WrappingMode::WORD_WRAP:
        m_wrappingModeBtn->getChildByID("word")->setVisible(true);
        break;
    
    default:
        m_wrappingModeBtn->getChildByID("cutoff")->setVisible(true);
        break;
    }
}

void LayoutOptionsLayer::onSpecialKeysClicked(CCObject*) {
    switchToMenu(4);
}

void LayoutOptionsLayer::onSpecialKeyAdded(std::string_view str) {
    if (!m_editedLabel.has_value()) return;

    std::optional<int> prevCursorPos = std::nullopt;

    auto toEditStr = std::string(m_labelTextSpecialKeysInput->getString());
    auto modifiedStr = fmt::format("{{{}}}", str);
    if (!m_labelTextSpecialKeysInput->isFocused())
        toEditStr += modifiedStr;
    else {
        auto cursorPos = m_labelTextSpecialKeysInput->getCursorPos();
        prevCursorPos = cursorPos;
        toEditStr.insert(cursorPos, modifiedStr);
    }
    m_labelTextSpecialKeysInput->setString(toEditStr, true);
    if (prevCursorPos.has_value())
        m_labelTextSpecialKeysInput->setCursorPos(prevCursorPos.value() + modifiedStr.length());
    #if !defined(GEODE_IS_MOBILE)
    m_labelTextSpecialKeysInput->focus();
    #endif
}

void LayoutOptionsLayer::keyBackClicked() {
    onBack(nullptr);
}

void LayoutOptionsLayer::setAllInputs(CCNode* node, bool enabled) {
    for (const auto& child : CCArrayExt<CCNode*>(node->getChildren()))
    {
        if (auto text = typeinfo_cast<TextInput*>(child))
            text->setEnabled(enabled);
    }
}

void LayoutOptionsLayer::sliderEnded(Slider* slider) {
    #if defined(GEODE_IS_MOBILE)
    sliderUpdate();
    #endif
}

void LayoutOptionsLayer::updateFontSelectFont() {
    if (!m_editedLabel.has_value()) return;

    if (m_fontSelectedIndicatorLabel != nullptr)
        m_fontSelectedIndicatorLabel->removeMeAndCleanup();
    
    m_fontSelectedIndicatorLabel = TextInput::create(
        m_size.width / 2 + 10,
        "",
        m_editedLabel.value()->getInfo().m_font
    );
    m_fontSelectedIndicatorLabel->setID("font-select-indicator-label");
    m_fontSelectedIndicatorLabel->setString(fmt::format(
        "Font: \"{}\"", 
        m_editedLabel.value()->getInfo().m_font.substr(0, m_editedLabel.value()->getInfo().m_font.length() - 4)
    ));
    m_fontSelectedIndicatorLabel->setAnchorPoint({0, .5f});
    m_fontSelectedIndicatorLabel->setPosition(ccp(-m_size.width / 2 + 5, -260));
    m_fontSelectedIndicatorLabel->getBGSprite()->setScaleY(.3f);
    m_labelSettingsNode->addChild(m_fontSelectedIndicatorLabel);

    m_fontSelectedIndicatorLabel->setEnabled(false);
    CCTouchDispatcher::get()->removeDelegate(m_fontSelectedIndicatorLabel);
    m_fontSelectedIndicatorLabel->getBGSprite()->setOpacity(90);
    m_fontSelectedIndicatorLabel->getBGSprite()->setContentHeight(m_fontSizeInput->getBGSprite()->getContentHeight() + 15);
}

void LayoutOptionsLayer::onAddColumnToLeft(CCObject*) {
    if (!m_editedColumn.has_value()) return;

    DTColumnInfo info {
        .m_orderPos = m_editedColumn.value()->getInfo().m_orderPos
    };

    DTLayer::get()->addColumn(info);
}

void LayoutOptionsLayer::onAddColumnToRight(CCObject*) {
    if (!m_editedColumn.has_value()) return;

    DTColumnInfo info {
        .m_orderPos = m_editedColumn.value()->getInfo().m_orderPos + 1
    };

    DTLayer::get()->addColumn(info);
}

void LayoutOptionsLayer::setOnBackedOutCallback(Function<void()> func) {
    m_onBackedOut = std::move(func);
}

void LayoutOptionsLayer::setOnEnterCallback(Function<void()> func) {
    m_onEnter = std::move(func);
}