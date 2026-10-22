#include "LayoutOptionsLayer.hpp"
#include <nodes/layers/DTLayer.hpp>
#include <nodes/SpecialKeyCell.hpp>

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

LayoutOptionsLayer* LayoutOptionsLayer::create(const CCSize& size) {
    auto ret = new LayoutOptionsLayer();
    if (ret && ret->init(size)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool LayoutOptionsLayer::init(const CCSize& size) {
    if (!CCLayer::init()) return false;

    this->size = size;

    this->setContentSize(size);

    // @geode-ignore(unknown-resource)
    auto bg = CCScale9Sprite::create("geode.loader/GE_square01.png");
    bg->setContentSize(size);
    bg->setPosition(size / 2);
    bg->setID("background");
    this->addChild(bg);

    auto superBlocker = CCMenu::create();
    superBlocker->setPosition({0, 0});
    superBlocker->setContentSize(size);
    superBlocker->setID("super-blocker");
    this->addChild(superBlocker);

    auto blockerItem = CCMenuItem::create();
    blockerItem->setContentSize(size);
    blockerItem->setID("blocker-item");
    blockerItem->setAnchorPoint({0, 0});
    superBlocker->addChild(blockerItem);

    auto generalBtnsMenu = CCMenu::create();
    generalBtnsMenu->setPosition({size.width / 2, size.height - 10});
    generalBtnsMenu->setContentSize({0, 0});
    generalBtnsMenu->setID("general-btns-menu");
    generalBtnsMenu->setZOrder(10);
    this->addChild(generalBtnsMenu);

    labelSettingsNode = CCMenu::create();
    labelSettingsNode->setPosition({size.width / 2, size.height - 10});
    labelSettingsNode->setContentSize({0, 0});
    labelSettingsNode->setID("label-settings-node");
    this->addChild(labelSettingsNode);

    columnSettingsNode = CCMenu::create();
    columnSettingsNode->setPosition({size.width / 2, size.height - 10});
    columnSettingsNode->setContentSize({0, 0});
    columnSettingsNode->setID("column-settings-node");
    this->addChild(columnSettingsNode);

    fontSelectionNode = CCMenu::create();
    fontSelectionNode->setPosition({size.width / 2, size.height - 10});
    fontSelectionNode->setContentSize({0, 0});
    fontSelectionNode->setID("font-selection-node");
    this->addChild(fontSelectionNode);

    specialKeysNode = CCMenu::create();
    specialKeysNode->setPosition({size.width / 2, size.height - 10});
    specialKeysNode->setContentSize({0, 0});
    specialKeysNode->setID("special-keys-node");
    this->addChild(specialKeysNode);

    labelNameInput = TextInput::create((size.width - 10) / .75f, "label name");
    labelNameInput->setID("label-name-input");
    labelNameInput->setScale(.75f);
    labelNameInput->setCallback([&](const std::string& newStr) {
        if (!editedLabel.has_value()) return;

        editedLabel.value()->setLabelName(newStr);
    });
    labelNameInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ <>=,.");
    labelNameInput->setPositionY(-22);
    labelSettingsNode->addChild(labelNameInput);

    auto labelNameInputLabel = CCLabelBMFont::create("Label Name", "bigFont.fnt");
    labelNameInputLabel->setScale(.4f);
    labelNameInputLabel->setPosition(labelNameInput->getPosition() + ccp(0, labelNameInput->getScaledContentHeight() / 2 + labelNameInputLabel->getScaledContentHeight() / 2));
    labelNameInputLabel->setID("name-input-label");
    labelSettingsNode->addChild(labelNameInputLabel);

    labelTextInput = ScrollableTextInput::create((size.width - 10) / .75f, "label text");
    labelTextInput->setScale(.75f);
    labelTextInput->setCommonFilter(CommonFilter::Any);
    labelTextInput->setID("label-text-input");
    labelTextInput->setPositionY(-60);
    labelTextInput->setCallback([&](const std::string& newStr) {
        if (!editedLabel.has_value()) return;

        editedLabel.value()->setLabelText(newStr);

        labelTextSpecialKeysInput->setString(newStr);
    });
    labelSettingsNode->addChild(labelTextInput);

    auto labelTextInputLabel = CCLabelBMFont::create("Text", "bigFont.fnt");
    labelTextInputLabel->setScale(.4f);
    labelTextInputLabel->setPosition(labelTextInput->getPosition() + ccp(0, labelTextInput->getScaledContentHeight() / 2 + labelTextInputLabel->getScaledContentHeight() / 2));
    labelTextInputLabel->setID("text-label");
    labelSettingsNode->addChild(labelTextInputLabel);

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
    labelSettingsNode->addChild(keysBtn);

    labelColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    labelColorBtnSprite->setID("color-spr");
    labelColorBtnSprite->setScale(.7f);
    auto labelColorBtn = CCMenuItemSpriteExtra::create(
        labelColorBtnSprite,
        this,
        menu_selector(LayoutOptionsLayer::onLabelColorBtnClicked)
    );
    labelColorBtn->setID("label-color-btn");
    labelColorBtn->setPosition({-30, -110});
    labelSettingsNode->addChild(labelColorBtn);

    auto labelColorBtnLabel = CCLabelBMFont::create("label", "bigFont.fnt");
    labelColorBtnLabel->setScale(.4f);
    labelColorBtnLabel->setPosition(labelColorBtn->getPosition() + ccp(0, labelColorBtn->getScaledContentHeight() / 2 + labelColorBtnLabel->getScaledContentHeight() / 2));
    labelColorBtnLabel->setID("label-color-label");
    labelSettingsNode->addChild(labelColorBtnLabel);

    textColorBtnSprite = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    textColorBtnSprite->setID("color-spr");
    textColorBtnSprite->setScale(.7f);
    auto textColorBtn = CCMenuItemSpriteExtra::create(
        textColorBtnSprite,
        this,
        menu_selector(LayoutOptionsLayer::onTextColorBtnClicked)
    );
    textColorBtn->setID("text-color-btn");
    textColorBtn->setPosition({30, -110});
    labelSettingsNode->addChild(textColorBtn);

    auto textColorBtnLabel = CCLabelBMFont::create("text", "bigFont.fnt");
    textColorBtnLabel->setScale(.4f);
    textColorBtnLabel->setPosition(textColorBtn->getPosition() + ccp(0, textColorBtn->getScaledContentHeight() / 2 + textColorBtnLabel->getScaledContentHeight() / 2));
    textColorBtnLabel->setID("text-color-label");
    labelSettingsNode->addChild(textColorBtnLabel);

    auto colorLabel = CCLabelBMFont::create("color", "bigFont.fnt");
    colorLabel->setScale(.4f);
    colorLabel->setPosition({0, -80});
    colorLabel->setID("color-label");
    labelSettingsNode->addChild(colorLabel);

    fontSizeInput = TextInput::create((size.width - 100) / .75f, "size");
    fontSizeInput->setScale(.75f);
    fontSizeInput->setPositionY(-195);
    fontSizeInput->setPositionX(-32);
    fontSizeInput->setCommonFilter(CommonFilter::Float);
    fontSizeInput->setCallback([&](const std::string& newStr) {
        if (!editedLabel.has_value()) return;
        
        auto newSizeRes = geode::utils::numFromString<float>(newStr);
        float newSize = 0.0f;

        if (!newSizeRes.isErr()) newSize = newSizeRes.unwrap();

        scaleSlider->setValue((newSize - DTLabelInfo::MIN_MAX_SCALE.x) / (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x));
        editedLabel.value()->setFontSize(newSize);
    });
    fontSizeInput->setID("font-size-input");
    labelSettingsNode->addChild(fontSizeInput);

    auto fontSizeInputLabel = CCLabelBMFont::create("Font Size", "bigFont.fnt");
    fontSizeInputLabel->setScale(.35f);
    fontSizeInputLabel->setPosition(fontSizeInput->getPosition() + ccp(0, fontSizeInput->getScaledContentHeight() / 2 + fontSizeInputLabel->getScaledContentHeight() / 2));
    fontSizeInputLabel->setID("font-size-label");
    labelSettingsNode->addChild(fontSizeInputLabel);

    scaleSlider = Slider::create(this, menu_selector(LayoutOptionsLayer::scaleSliderChanged), .5f);
    scaleSlider->setContentSize({0, 0});
    scaleSlider->setPositionY(-215);
    //scaleSlider->setPositionX(fontSizeInput->getPositionX() + 30);
    scaleSlider->setID("font-size-slider");
    scaleSlider->m_delegate = this;
    labelSettingsNode->addChild(scaleSlider);

    alignmentMenu = CCMenu::create();
    alignmentMenu->setPosition({0, -155});
    alignmentMenu->setContentSize({133, 28});
    alignmentMenu->setID("alignment-menu");
    labelSettingsNode->addChild(alignmentMenu);
    alignmentMenu->ignoreAnchorPointForPosition(false);
    alignmentMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(5)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Center)
    );

    auto alignmentLabel = CCLabelBMFont::create("Alignment", "bigFont.fnt");
    alignmentLabel->setID("alignment-label");
    alignmentLabel->setScale(.4f);
    alignmentLabel->setPosition(alignmentMenu->getPosition() + ccp(0, 5 + alignmentMenu->getScaledContentHeight() / 2 + alignmentLabel->getScaledContentHeight() / 2));
    labelSettingsNode->addChild(alignmentLabel);

    auto leftAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignLeft.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    leftAlignBtn->setID("left-align-btn");
    alignmentMenu->addChild(leftAlignBtn);

    auto centerAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignCenter.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    centerAlignBtn->setID("center-align-btn");
    alignmentMenu->addChild(centerAlignBtn);

    auto rightAlignBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("alignRight.png"_spr),
        this,
        menu_selector(LayoutOptionsLayer::onAlignmentChanged)
    );
    rightAlignBtn->setID("right-align-btn");
    alignmentMenu->addChild(rightAlignBtn);

    alignmentMenu->updateLayout();

    auto wrapModeCutoff = ButtonSprite::create("CUTOFF");
    wrapModeCutoff->setID("cutoff");
    wrapModeCutoff->setScale(.5f);
    auto wrapModeSpace = ButtonSprite::create("SPACE");
    wrapModeSpace->setID("space");
    wrapModeSpace->setVisible(false);
    auto wrapModeWord = ButtonSprite::create("WORD");
    wrapModeWord->setID("word");
    wrapModeWord->setVisible(false);

    wrappingModeBtn = CCMenuItemSpriteExtra::create(
        wrapModeCutoff,
        this,
        menu_selector(LayoutOptionsLayer::onWrappingBtn)
    );
    wrappingModeBtn->setID("wrapping-mode-btn");
    
    wrapModeSpace->setPosition(wrapModeCutoff->getPosition());
    wrapModeSpace->setScale(wrapModeCutoff->getScale());
    wrappingModeBtn->addChild(wrapModeSpace);
    wrapModeWord->setPosition(wrapModeCutoff->getPosition());
    wrapModeWord->setScale(wrapModeCutoff->getScale());
    wrappingModeBtn->setPositionY(fontSizeInput->getPositionY());
    wrappingModeBtn->setPositionX(32);
    wrappingModeBtn->addChild(wrapModeWord);

    auto wrappingModeLabel = CCLabelBMFont::create("Wrapping", "bigFont.fnt");
    wrappingModeLabel->setScale(.35f);
    wrappingModeLabel->setPosition({wrappingModeBtn->getPositionX(), fontSizeInputLabel->getPositionY()});
    wrappingModeLabel->setID("wrapping-label");
    labelSettingsNode->addChild(wrappingModeLabel);

    labelSettingsNode->addChild(wrappingModeBtn);

    auto fontSelectionBtnSpr = ButtonSprite::create("Choose");
    fontSelectionBtnSpr->setScale(.45f);
    auto fontSelectionBtn = CCMenuItemSpriteExtra::create(
        fontSelectionBtnSpr,
        this,
        menu_selector(LayoutOptionsLayer::onFontSelection)
    );
    fontSelectionBtn->setPositionY(-260);
    fontSelectionBtn->setPositionX(size.width / 2 - 5 - fontSelectionBtn->getContentWidth() / 2);
    fontSelectionBtn->setID("font-selection-btn");
    labelSettingsNode->addChild(fontSelectionBtn);

    updateFontSelectFont();

    hideHeaderToggler = SimpleToggler::createWithDefaults(
        .65f
    );
    hideHeaderToggler->setPositionY(fontSelectionBtn->getPositionY() + fontSelectionBtn->getContentHeight() / 2 + hideHeaderToggler->getContentHeight() / 2 + 7.5f);
    hideHeaderToggler->setPositionX(size.width / 2 - 20 - hideHeaderToggler->getContentWidth() / 2);
    hideHeaderToggler->setID("hide-header-toggler");
    hideHeaderToggler->setCallback([&](auto state) {
        if (!editedLabel.has_value()) return;

        editedLabel.value()->setHideHeader(state);
    });
    labelSettingsNode->addChild(hideHeaderToggler);

    auto hideHeaderTogglerLabel = CCLabelBMFont::create("Hide Header", "bigFont.fnt");
    hideHeaderTogglerLabel->setScale(.4f);
    hideHeaderTogglerLabel->setPosition(hideHeaderToggler->getPosition() + ccp(-hideHeaderToggler->getContentWidth() / 2 - 10, 0));
    hideHeaderTogglerLabel->setID("hide-header-label");
    hideHeaderTogglerLabel->setAnchorPoint({1, .5f});
    labelSettingsNode->addChild(hideHeaderTogglerLabel);

    this->addEventListener(
        KeybindSettingPressedEvent(
            Mod::get(),
            "enter-new-line"
        ),
        [&](const Keybind& keybind, bool down, bool repeat, double) {
            if (down && editedLabel.has_value()) {
                ScrollableTextInput* toEdit = nullptr;
                bool doUpdateScrollableInput = false;

                if (labelTextInput->isSelected()) {
                    toEdit = labelTextInput;
                    doUpdateScrollableInput = true;
                }
                else if (labelTextSpecialKeysInput->isSelected()) toEdit = labelTextSpecialKeysInput;
                
                if (toEdit == nullptr) return;

                auto str = std::string(toEdit->getString());
                int pos = toEdit->getTextInput()->getInputNode()->m_textField->m_uCursorPos;

                if (pos == -1) str += "{nl}";
                else {
                    str = str.insert(pos, "{nl}");
                    toEdit->getTextInput()->getInputNode()->m_textField->m_uCursorPos += 4;
                }

                toEdit->setString(str, true);
            }
        }
    );

    fontSelectionNode->setScaleY(0);
    labelSettingsNode->setScaleY(0);
    columnSettingsNode->setScaleY(0);
    specialKeysNode->setScaleY(0);

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

    fontsScroll = ScrollLayer::create(size - ccp(8, 30));
    fontsScroll->setPosition({-size.width / 2, -size.height + 20});
    fontsScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(fontsScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    fontSelectionNode->addChild(fontsScroll);

    auto fontsScrollbar = Scrollbar::create(fontsScroll);
    fontsScrollbar->setPosition({65, fontsScroll->getPositionY()});
    fontsScrollbar->ignoreAnchorPointForPosition(true);
    fontSelectionNode->addChild(fontsScrollbar);

    for (const auto& font : StatsManager::getAllFonts())
    {
        auto cell = FontSelectionCell::create(font, [&](auto cell) {onFontSelected(cell);});
        fontsScroll->m_contentLayer->addChild(cell);

        allFontCells.insert({font, cell});
    }

    fontsScroll->m_contentLayer->updateLayout();
    fontsScroll->moveToTop();

    fontsScroll->setMouseEnabled(false);


    labelTextSpecialKeysInput = ScrollableTextInput::create((size.width - 10) / .75f, "label text");
    labelTextSpecialKeysInput->setScale(.75f);
    labelTextSpecialKeysInput->setCommonFilter(CommonFilter::Any);
    labelTextSpecialKeysInput->setID("label-text-input");
    labelTextSpecialKeysInput->setPositionY(-22);
    labelTextSpecialKeysInput->setCallback([&](const std::string& newStr) {
        if (!editedLabel.has_value()) return;

        editedLabel.value()->setLabelText(newStr);

        labelTextInput->setString(newStr);
    });
    specialKeysNode->addChild(labelTextSpecialKeysInput);

    auto labelTextSpecialKeysInputLabel = CCLabelBMFont::create("text", "bigFont.fnt");
    labelTextSpecialKeysInputLabel->setScale(.4f);
    labelTextSpecialKeysInputLabel->setPosition(labelTextSpecialKeysInput->getPosition() + ccp(0, labelTextSpecialKeysInput->getScaledContentHeight() / 2 + labelTextSpecialKeysInputLabel->getScaledContentHeight() / 2));
    labelTextSpecialKeysInputLabel->setID("text-Label");
    specialKeysNode->addChild(labelTextSpecialKeysInputLabel);

    specialKeysScroll = ScrollLayer::create(size - ccp(8, 60));
    specialKeysScroll->setPosition({-size.width / 2, -size.height + 20});
    specialKeysScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(specialKeysScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    specialKeysNode->addChild(specialKeysScroll);

    auto specialKeysScrollbar = Scrollbar::create(specialKeysScroll);
    specialKeysScrollbar->setPosition({65, specialKeysScroll->getPositionY()});
    specialKeysScrollbar->ignoreAnchorPointForPosition(true);
    specialKeysNode->addChild(specialKeysScrollbar);

    for (const auto& [key, keyObj] : DTLayer::get()->specialStrings)
    {
        auto cell = SpecialKeyCell::create(keyObj, [&](auto str) {LayoutOptionsLayer::onSpecialKeyAdded(str);});
        specialKeysScroll->m_contentLayer->addChild(cell);
    }

    specialKeysScroll->m_contentLayer->updateLayout();
    specialKeysScroll->moveToTop();
    specialKeysScroll->setMouseEnabled(false);

    columnWidthInput = TextInput::create((size.width - 10) / .75f, "Width");
    columnWidthInput->setScale(.75f);
    columnWidthInput->setCommonFilter(CommonFilter::Float);
    columnWidthInput->setID("column-width-input");
    columnWidthInput->setPositionY(-30);
    columnWidthInput->setCallback([&](const std::string& newStr) {
        if (!editedColumn.has_value()) return;

        float endValue = DTColumnInfo::minWidth;

        auto toNumRes = geode::utils::numFromString<float>(newStr);
        if (toNumRes.isOk()) endValue = toNumRes.unwrap();

        editedColumn.value()->setContentWidth(std::max(DTColumnInfo::minWidth, endValue));

        editedColumn.value()->updateSizesByContent();

        ignoreNextOrganization = true;
        DTLayer::get()->organizeLayout();
    });
    columnSettingsNode->addChild(columnWidthInput);

    auto columnWidthInputLabel = CCLabelBMFont::create("column width", "bigFont.fnt");
    columnWidthInputLabel->setScale(.4f);
    columnWidthInputLabel->setPosition(columnWidthInput->getPosition() + ccp(0, columnWidthInput->getScaledContentHeight() / 2 + columnWidthInputLabel->getScaledContentHeight() / 2));
    columnWidthInputLabel->setID("width-Label");
    columnSettingsNode->addChild(columnWidthInputLabel);

    DTLayer::get()->subscribeToOrganizationEvent(this, [&](auto _) {
        if (!editedColumn.has_value()) return;
        if (ignoreNextOrganization) {
            ignoreNextOrganization = false;
            return;
        }

        auto widthText = std::to_string(editedColumn.value()->info.currentWidth);
        for (int i = widthText.length() - 1; i >= 0; i--)
        {
            if (widthText[i] != '0') break;

            widthText.pop_back();
        }
        
        if (widthText.length() != 0 && widthText[widthText.length() - 1] == '.') widthText.pop_back();

        columnWidthInput->setString(widthText);
    });

    setAllInputs(labelSettingsNode, false);
    setAllInputs(columnSettingsNode, false);
    setAllInputs(fontSelectionNode, false);
    setAllInputs(specialKeysNode, false);

    return true;
}

void LayoutOptionsLayer::setEditedNodeTo(DTLabel* label) {
    if (editedLabel.has_value() && editedLabel.value() == label) return;

    if (editedLabel.has_value()) editedLabel.value()->onBeingEditedEnded();

    editedLabel = label;
    editedColumn = std::nullopt;

    label->onBeingEdited();

    if (!label->info.isExpanded)
        label->toggleExpand(nullptr);

    onAlignmentChanged(alignmentMenu->getChildByID(fmt::format(
        "{}-align-btn",
        label->info.horizontalAlignment == CCTextAlignment::kCCTextAlignmentLeft ? "left" : (
            label->info.horizontalAlignment == CCTextAlignment::kCCTextAlignmentCenter ? "center" : "right"
        )
    )));

    labelColorBtnSprite->setColor({label->info.labelColor.r, label->info.labelColor.g, label->info.labelColor.b});
    textColorBtnSprite->setColor({label->info.textColor.r, label->info.textColor.g, label->info.textColor.b});

    labelNameInput->setString(label->info.labelName);
    labelTextInput->setString(label->info.text);
    labelTextSpecialKeysInput->setString(label->info.text);

    scaleSlider->setValue(
        (label->info.scale - DTLabelInfo::MIN_MAX_SCALE.x) / (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x)
    );
    scaleSliderChanged(nullptr);

    updateFontSelectFont();
    if (currentlySelectedFontCell != nullptr)
        currentlySelectedFontCell->deselect();
    currentlySelectedFontCell = allFontCells[label->info.font];
    currentlySelectedFontCell->select();

    hideHeaderToggler->toggle(label->info.hideHeader);

    switchToMenu(1);

    updateWrapModeBtnVisuals();

    if (onEnter != NULL)
        onEnter();
}

void LayoutOptionsLayer::setEditedNodeTo(LayoutColumn* column) {
    if (editedColumn.has_value() && editedColumn.value() == column) return;

    if (editedLabel.has_value()) editedLabel.value()->onBeingEditedEnded();

    editedColumn = column;
    editedLabel = std::nullopt;

    auto widthText = std::to_string(column->info.currentWidth);
    for (int i = widthText.length() - 1; i >= 0; i--)
    {
        if (widthText[i] != '0') break;

        widthText.pop_back();
    }
    
    if (widthText.length() != 0 && widthText[widthText.length() - 1] == '.') widthText.pop_back();

    columnWidthInput->setString(widthText);

    switchToMenu(2);

    if (onEnter != NULL)
        onEnter();
}

void LayoutOptionsLayer::close() {
    //close the options layer
    switchToMenu(0);

    if (editedLabel.has_value()) editedLabel.value()->onBeingEditedEnded();
    
    editedLabel = std::nullopt;
    editedColumn = std::nullopt;
    colorChangeFunc = NULL;
}

void LayoutOptionsLayer::onTextColorBtnClicked(CCObject*) {
    if (!editedLabel.has_value()) return;

    auto popup = geode::ColorPickPopup::create(editedLabel.value()->info.textColor);
    popup->setCallback([&](auto color) { updateColor(color); });
    popup->setColorTarget(textColorBtnSprite);
    colorChangeFunc = [&](cocos2d::ccColor4B const& color) {
        editedLabel.value()->setTextColor(color);
    };
    popup->show();
}
void LayoutOptionsLayer::onLabelColorBtnClicked(CCObject*) {
    if (!editedLabel.has_value()) return;

    auto popup = geode::ColorPickPopup::create(editedLabel.value()->info.labelColor);
    popup->setCallback([&](auto color) { updateColor(color); });
    popup->setColorTarget(labelColorBtnSprite);
    colorChangeFunc = [&](cocos2d::ccColor4B const& color) {
        editedLabel.value()->setLabelColor(color);
    };
    popup->show();
}

void LayoutOptionsLayer::updateColor(cocos2d::ccColor4B const& color) {
    if (colorChangeFunc == NULL) return;

    colorChangeFunc(color);
}

void LayoutOptionsLayer::onAlignmentChanged(CCObject* sender) {
    if (!editedLabel.has_value()) return;

    auto menuItem = static_cast<CCMenuItemSpriteExtra*>(sender);

    for (const auto& alignmentButton : CCArrayExt<CCMenuItemSpriteExtra*>(alignmentMenu->getChildren()))
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

    editedLabel.value()->setTextAlignment(alignment);
}

void LayoutOptionsLayer::scaleSliderChanged(CCObject*) {
    #if !defined(GEODE_IS_MOBILE)
    sliderUpdate();
    #endif
}

void LayoutOptionsLayer::sliderUpdate() {
    auto numValue = DTLabelInfo::MIN_MAX_SCALE.x + (DTLabelInfo::MIN_MAX_SCALE.y - DTLabelInfo::MIN_MAX_SCALE.x) * scaleSlider->getValue();

    editedLabel.value()->setFontSize(numValue);

    auto scaleText = std::to_string(numValue);
    for (int i = scaleText.length() - 1; i >= 0; i--)
    {
        if (scaleText[i] != '0') break;

        scaleText.pop_back();
    }
    
    if (scaleText.length() != 0 && scaleText[scaleText.length() - 1] == '.') scaleText.pop_back();

    fontSizeInput->setString(scaleText);
}

void LayoutOptionsLayer::onFontSelection(CCObject*) {
    switchToMenu(3);
}

void LayoutOptionsLayer::switchToMenu(uint8_t menuID) {
    labelSettingsNode->stopAllActions();
    columnSettingsNode->stopAllActions();
    fontSelectionNode->stopAllActions();
    specialKeysNode->stopAllActions();

    currentPage = menuID;

    fontsScroll->setMouseEnabled(false);
    specialKeysScroll->setMouseEnabled(false);

    auto exitEasing = [](bool open) -> CCEaseExponentialOut* {
        return CCEaseExponentialOut::create(CCScaleTo::create(.2f, 1, open ? 1 : 0));
    };

    labelSettingsNode->runAction(exitEasing(false));
    columnSettingsNode->runAction(exitEasing(false));
    fontSelectionNode->runAction(exitEasing(false));
    specialKeysNode->runAction(exitEasing(false));
    setAllInputs(labelSettingsNode, false);
    setAllInputs(columnSettingsNode, false);
    setAllInputs(fontSelectionNode, false);
    setAllInputs(specialKeysNode, false);

    CCNode* selectedNode = nullptr;

    switch (menuID)
    {

    case 1:
        selectedNode = labelSettingsNode;
        break;

    case 2:
        selectedNode = columnSettingsNode;
        break;

    case 3:
        selectedNode = fontSelectionNode;
        fontsScroll->setMouseEnabled(true);
        fontsScroll->moveToTop();
        break;

    case 4:
        selectedNode = specialKeysNode;
        specialKeysScroll->setMouseEnabled(true);
        specialKeysScroll->moveToTop();
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
    if (currentPage == 3) {
        switchToMenu(editedLabel.has_value() ? 1 : 2);
        return;
    }
    else if (currentPage == 4) {
        switchToMenu(1);
        return;
    }

    if (onBackedOut != NULL)
        onBackedOut();
}

void LayoutOptionsLayer::onDelete(CCObject*) {
    if (editedLabel.has_value()) {
        editedLabel.value()->removeFromColumns();
        editedLabel.value()->removeFromParentAndCleanup(true);
        DTLayer::get()->organizeLayout();
        if (onBackedOut != NULL)
            onBackedOut();
    }
    else if (editedColumn.has_value()) {
        editedColumn.value()->destroyColumnAndCleanup();
        DTLayer::get()->organizeLayout();
        if (onBackedOut != NULL)
            onBackedOut();
    }
} 

void LayoutOptionsLayer::onFontSelected(FontSelectionCell* cell) {
    if (!isEditingNode()) return;

    if (currentlySelectedFontCell != nullptr) currentlySelectedFontCell->deselect();
    currentlySelectedFontCell = cell;
    currentlySelectedFontCell->select();

    if (editedLabel.has_value()) {
        editedLabel.value()->setFont(cell->font);
        DTLayer::get()->organizeLayout();
        updateFontSelectFont();
    }
}

void LayoutOptionsLayer::onWrappingBtn(CCObject*) {
    if (!editedLabel.has_value()) return;

    WrappingMode newMode = WrappingMode::CUTOFF_WRAP;

    if (wrappingModeBtn->getChildByID("cutoff")->isVisible()) {
        newMode = WrappingMode::SPACE_WRAP;
    }
    else if (wrappingModeBtn->getChildByID("space")->isVisible()) {
        newMode = WrappingMode::WORD_WRAP;
    }
    else if (wrappingModeBtn->getChildByID("word")->isVisible()) {
        newMode = WrappingMode::CUTOFF_WRAP;
    }

    editedLabel.value()->setTextWrapping(newMode);

    updateWrapModeBtnVisuals();
}

void LayoutOptionsLayer::updateWrapModeBtnVisuals() {
    if (!editedLabel.has_value()) return;

    for (const auto& child : CCArrayExt<CCNode*>(wrappingModeBtn->getChildren()))
    {
        child->setVisible(false);
    }

    switch (editedLabel.value()->info.wrapping)
    {
    case WrappingMode::CUTOFF_WRAP:
        wrappingModeBtn->getChildByID("cutoff")->setVisible(true);
        break;
    case WrappingMode::SPACE_WRAP:
        wrappingModeBtn->getChildByID("space")->setVisible(true);
        break;
    case WrappingMode::WORD_WRAP:
        wrappingModeBtn->getChildByID("word")->setVisible(true);
        break;
    
    default:
        wrappingModeBtn->getChildByID("cutoff")->setVisible(true);
        break;
    }
}

void LayoutOptionsLayer::onSpecialKeysClicked(CCObject*) {
    switchToMenu(4);
}

void LayoutOptionsLayer::onSpecialKeyAdded(const std::string& str) {
    if (!editedLabel.has_value()) return;

    auto toEditStr = std::string(labelTextSpecialKeysInput->getString());
    int pos = labelTextSpecialKeysInput->getTextInput()->getInputNode()->m_textField->m_uCursorPos;

    // log::info("is selected {}", labelTextSpecialKeysInput->getInputNode()->m_selected);

    auto modifiedStr = fmt::format("{{{}}}", str);

    if (pos == -1) toEditStr += modifiedStr;
    else {
        toEditStr = toEditStr.insert(pos, modifiedStr);
        labelTextSpecialKeysInput->getTextInput()->getInputNode()->m_textField->m_uCursorPos += modifiedStr.length();
    }

    labelTextSpecialKeysInput->setString(toEditStr, true);
    #if !defined(GEODE_IS_MOBILE)
    labelTextSpecialKeysInput->getTextInput()->focus();
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
        else if (auto scrollText = typeinfo_cast<ScrollableTextInput*>(child)) {
            scrollText->setEnabled(enabled);
        }
    }
}

void LayoutOptionsLayer::sliderEnded(Slider* slider) {
    #if defined(GEODE_IS_MOBILE)
    sliderUpdate();
    #endif
}

void LayoutOptionsLayer::updateFontSelectFont() {
    if (!editedLabel.has_value()) return;

    if (fontSelectedIndicatorLabel != nullptr)
        fontSelectedIndicatorLabel->removeMeAndCleanup();
    
    fontSelectedIndicatorLabel = TextInput::create(
        size.width / 2 + 10, 
        "",
        editedLabel.value()->info.font
    );
    fontSelectedIndicatorLabel->setID("font-select-indicator-label");
    fontSelectedIndicatorLabel->setString(fmt::format(
        "Font: \"{}\"", 
        editedLabel.value()->info.font.substr(0, editedLabel.value()->info.font.length() - 4)
    ));
    fontSelectedIndicatorLabel->getInputNode()->setTouchEnabled(false);
    fontSelectedIndicatorLabel->setAnchorPoint({0, .5f});
    fontSelectedIndicatorLabel->setPosition(ccp(-size.width / 2 + 5, -260));
    fontSelectedIndicatorLabel->getBGSprite()->setScaleY(.3f);
    labelSettingsNode->addChild(fontSelectedIndicatorLabel);
}