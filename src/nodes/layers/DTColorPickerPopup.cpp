#include "DTColorPickerPopup.hpp"
#include <utils/CCGradientAction.hpp>

#include <managers/AuthManager.hpp>
#include <nodes/DTLabel.hpp>

DTColorPickerPopup* DTColorPickerPopup::create(CustomizationData const& data, bool isRGBA) {
    auto ret = new DTColorPickerPopup();
    if (ret->init(data, isRGBA)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

DTColorPickerPopup* DTColorPickerPopup::create(bool isRGBA) {
    CustomizationData data{};
    data.color = cocos2d::ccColor4B{255, 255, 255, 255};
    return DTColorPickerPopup::create(data, isRGBA);
}

DTColorPickerPopup* DTColorPickerPopup::create(cocos2d::ccColor3B const& color) {
    CustomizationData data{};
    data.color = cocos2d::ccColor4B{color.r, color.g, color.b, 255};
    return DTColorPickerPopup::create(data, true);
}

DTColorPickerPopup* DTColorPickerPopup::create(cocos2d::ccColor4B const& color) {
    CustomizationData data{};
    data.color = color;
    return DTColorPickerPopup::create(data, true);
}

DTColorPickerPopup* DTColorPickerPopup::create(CustomizationData const& data) {
    return DTColorPickerPopup::create(data, true);
}

bool DTColorPickerPopup::init(CustomizationData const& data, bool isRGBA) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!ColorPickPopup::init(data.color, isRGBA))
        return false;
    
    m_normalContent = m_mainLayer->getChildByID("opacity-column");
    if (!m_normalContent)
        return true;

    m_normalContent->setTag(0);

    m_currentPage = static_cast<int>(Pages::Color);
    m_gradientEnabled = data.gradientEnabled;
    m_waveEnabled = data.waveEnabled;
    createGradientPage(isRGBA, data.gradient);
    createWavePage(data.wave);

    auto switcherBtnSpr = ButtonSprite::create("Color");
    switcherBtnSpr->setScale(.5f);
    auto switcherBtn = CCMenuItemSpriteExtra::create(
        switcherBtnSpr,
        nullptr,
        this,
        menu_selector(DTColorPickerPopup::onSwitcherBtnClick)
    );

    m_floatingList = FloatingList::create({75, 120});
    m_floatingList->setID("switcher-list");
    m_floatingList->setAnchorPoint(ccp(.5f, 1));
    m_floatingList->setListOpenAnchor(ccp(.5f, 1));
    m_floatingList->setPositionY(-switcherBtn->getScaledContentHeight() / 2);

    m_floatingList->addItem(FloatingListItem{.id = static_cast<int>(Pages::Color), .text = "Color"});
    m_floatingList->addItem(FloatingListItem{.id = static_cast<int>(Pages::Gradient), .text = "Gradient"});
    m_floatingList->addItem(FloatingListItem{.id = static_cast<int>(Pages::Wave), .text = "Wave"});

    this->setTitle("Modify Color");

    updateGradientEnabledPosition();

    m_floatingList->setCallback([&, switcherBtn](const int& id) {
        auto item = m_floatingList->getItemForID(id);

        std::string newText = item.has_value() ? item.value().text : "Unknown Page";

        auto switcherBtnSpr = ButtonSprite::create(newText.c_str());
        switcherBtnSpr->setScale(.5f);
        switcherBtn->setSprite(switcherBtnSpr);
        
        m_floatingList->close();

        auto pageNode = m_mainLayer->getChildByTag(m_currentPage);
        if (pageNode){
            pageNode->setVisible(false);
            setAllChildrenEnabled(pageNode, false);
        }
        
        m_currentPage = id;

        pageNode = m_mainLayer->getChildByTag(m_currentPage);
        if (pageNode){
            pageNode->setVisible(true);
            setAllChildrenEnabled(pageNode, true);

            switch (static_cast<Pages>(m_currentPage))
            {
                case Pages::Gradient:
                    onGradientPageOpen(pageNode);
                    break;

                case Pages::Wave:
                    onWavePageOpen(pageNode);
                    break;
                
                default:
                    break;
            }
        }

        this->setTitle(item.has_value() ? fmt::format("Modify {}", item.value().text) : "Unknown Page");
        m_gradientEnabledMenu->setVisible(id == static_cast<int>(Pages::Gradient));
        m_waveEnabledMenu->setVisible(id == static_cast<int>(Pages::Wave));
        updateGradientEnabledPosition();
        updateWaveEnabledPosition();
    });

    auto switcherArea = CCNode::create();
    switcherArea->setID("switcher-menu");
    m_mainLayer->addChildAtPosition(switcherArea, Anchor::TopRight, switcherBtn->getScaledContentSize() / 2 * -1 - ccp(15, 15));

    auto switcherMenu = CCMenu::create();
    switcherMenu->setID("switcher-menu");
    switcherMenu->setContentSize(switcherArea->getScaledContentSize());
    switcherMenu->setPosition(ccp(0, 0));
    switcherArea->addChild(switcherMenu);

    switcherMenu->addChild(switcherBtn);
    switcherArea->addChild(m_floatingList);

    return true;
}

void DTColorPickerPopup::onSwitcherBtnClick(cocos2d::CCObject* sender) {
    if (m_floatingList->isOpened())
        m_floatingList->close();
    else
        m_floatingList->open();
}

void DTColorPickerPopup::updateGradientEnabledPosition() {
    if (!m_gradientEnabledMenu || !m_title || !m_gradientPage) return;

    auto positionInMainLayer = ccp(
        m_title->getPositionX() - m_title->getScaledContentWidth() / 2 - 15.0f - m_gradientEnabledMenu->getScaledContentWidth(),
        m_title->getPositionY() - m_gradientEnabledMenu->getScaledContentHeight() / 2
    );
    auto positionInWorld = m_mainLayer->convertToWorldSpace(positionInMainLayer);
    m_gradientEnabledMenu->setPosition(m_gradientPage->convertToNodeSpace(positionInWorld));
}

void DTColorPickerPopup::updateWaveEnabledPosition() {
    if (!m_waveEnabledMenu || !m_title || !m_wavePage) return;

    auto positionInMainLayer = ccp(
        m_title->getPositionX() - m_title->getScaledContentWidth() / 2 - 15.0f - m_waveEnabledMenu->getScaledContentWidth(),
        m_title->getPositionY() - m_waveEnabledMenu->getScaledContentHeight() / 2
    );
    auto positionInWorld = m_mainLayer->convertToWorldSpace(positionInMainLayer);
    m_waveEnabledMenu->setPosition(m_wavePage->convertToNodeSpace(positionInWorld));
}

void DTColorPickerPopup::createGradientPage(bool isRGBA, GradientData const& data) {
    auto popupBg = m_mainLayer->getChildByID("popup-bg");
    if (!popupBg)
        return;

    m_gradientColorA = data.colorA;
    m_gradientColorB = data.colorB;
    m_stagger = data.stagger;
    m_duration = data.duration;

    m_originalGradientColorA = m_gradientColorA;
    m_originalGradientColorB = m_gradientColorB;

    m_gradientPage = CCNode::create();
    m_gradientPage->setID("gradient-page");
    m_mainLayer->addChild(m_gradientPage, 0, static_cast<int>(Pages::Gradient));
    m_gradientPage->setVisible(false);
    setAllChildrenEnabled(m_gradientPage, false);

    m_gradientPage->setContentSize(popupBg->getScaledContentSize());
    m_gradientPage->setPosition(popupBg->getPosition());
    m_gradientPage->setAnchorPoint({0.5f, 0.5f});

    m_gradientEnabledMenu = CCMenu::create();
    m_gradientEnabledMenu->setID("gradient-enabled-menu");

    m_gradientEnabledToggle = SimpleToggler::createWithDefaults(.45f, m_gradientEnabled);
    m_gradientEnabledToggle->setID("gradient-enabled-toggle");
    m_gradientEnabledToggle->setCallback([this](bool enabled) {
        if (!AuthManager::isDoner()) {
            m_gradientEnabledToggle->toggle(enabled);

            return;
        }
        m_gradientEnabled = enabled;
        updateWavePreview();

        if (m_gradientEnabledChangedCallback) {
            m_gradientEnabledChangedCallback(enabled);
        }

        onGradientPageOpen(m_gradientPage);
    });
    if (!AuthManager::isDoner()) {
        m_gradientEnabledToggle->setEnabled(false);
        m_gradientEnabledToggle->setOpacity(128);
    }
    auto enabledText = CCLabelBMFont::create("enabled", "bigFont.fnt");
    enabledText->setScale(.45f);

    auto toggleWidth = m_gradientEnabledToggle->getScaledContentWidth();
    auto textWidth = enabledText->getScaledContentWidth();
    auto toggleGap = 3.f;
    auto menuWidth = toggleWidth + toggleGap + textWidth;
    auto menuHeight = std::max(
        m_gradientEnabledToggle->getScaledContentHeight(),
        enabledText->getScaledContentHeight()
    );

    enabledText->setPosition({textWidth / 2, menuHeight / 2});
    m_gradientEnabledToggle->setPosition({textWidth + toggleGap + toggleWidth / 2, menuHeight / 2});

    m_gradientEnabledMenu->setContentSize({menuWidth, menuHeight});
    m_gradientEnabledMenu->addChild(m_gradientEnabledToggle);
    m_gradientEnabledMenu->addChild(enabledText);
    m_gradientEnabledMenu->setVisible(false);
    m_gradientPage->addChild(m_gradientEnabledMenu, 2);

    m_gradientDisabledFG = NineSlice::create(
        "square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
    );
    m_gradientDisabledFG->setID("popup-bg");
    m_gradientDisabledFG->setScale(.5f);
    m_gradientDisabledFG->setColor({ 0, 0, 0 });
    m_gradientDisabledFG->setOpacity(85);
    m_gradientDisabledFG->setContentSize({ m_size.width * 2 - 40.f, m_size.height * 2 - 140.f });
    m_gradientDisabledFG->setZOrder(5);
    m_gradientPage->addChildAtPosition(m_gradientDisabledFG, Anchor::Center, ccp(0, 0));

    m_donorLabel = RichTextArea::create("<gradient=ffff00,ffffff>Donor Feature", "bigFont.fnt");
    m_donorLabel->setID("donor-label");
    m_donorLabel->setPosition(m_gradientDisabledFG->getPosition());
    bool isGradient = true;
    m_donorLabel->registerRichTextKey(DTLabel::makeGradientKey([](){ return true; }));
    m_donorLabel->setZOrder(10);
    m_donorLabel->setText(m_donorLabel->getRawText());
    m_donorLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_gradientPage->addChild(m_donorLabel);

    auto colorMenu = CCMenu::create();
    colorMenu->setLayout(
        ColumnLayout::create()
            ->setGap(5.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setCrossAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    colorMenu->setID("color-menu");
    colorMenu->setAnchorPoint({0, 1});

    m_gradientPage->addChildAtPosition(colorMenu, Anchor::TopLeft, ccp(15.f, -15.f));

    //
    auto colorDiffAMenu = CCMenu::create();
    colorDiffAMenu->setAnchorPoint({0, .5f});
    colorDiffAMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisScaling(AxisScaling::Scale)
        ->setCrossAxisScaling(AxisScaling::Scale)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
    );
    colorDiffAMenu->setID("color-diff-a-menu");

    auto oldColorASpr = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
    oldColorASpr->setColor(to3B(m_gradientColorA));
    oldColorASpr->setID("gradient-old-color-A-spr");
    oldColorASpr->setColor(to3B(m_gradientColorA));
    oldColorASpr->setOpacity(m_gradientColorA.a);
    colorDiffAMenu->addChild(oldColorASpr);

    m_gradientNewColorASpr = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
    m_gradientNewColorASpr->setColor(to3B(m_gradientColorA));
    m_gradientNewColorASpr->setID("gradient-new-color-A-spr");
    colorDiffAMenu->addChild(m_gradientNewColorASpr);

    auto resetBtnASpr = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("geode.loader/reset-gold.png"), 32, true, 0.f,
        "GJ_button_01.png", 1.25f
    );
    resetBtnASpr->setScale(.6f);
    m_gradientResetBtnA = CCMenuItemSpriteExtra::create(
        resetBtnASpr, this, menu_selector(DTColorPickerPopup::onGradientResetA)
    );
    m_gradientResetBtnA->setID("gradient-reset-A-btn");
    m_gradientResetBtnA->setEnabled(false);
    m_gradientResetBtnA->setOpacity(0);
    colorDiffAMenu->addChild(m_gradientResetBtnA);

    colorDiffAMenu->setContentSize({
        oldColorASpr->getScaledContentWidth() * 2 + m_gradientResetBtnA->getScaledContentWidth(),
        oldColorASpr->getScaledContentHeight()
    });
    colorDiffAMenu->updateLayout();
    colorMenu->addChild(colorDiffAMenu);

    //
    auto colorDiffBMenu = CCMenu::create();
    colorDiffBMenu->setAnchorPoint({0, .5f});
    colorDiffBMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisScaling(AxisScaling::Scale)
        ->setCrossAxisScaling(AxisScaling::Scale)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
    );
    colorDiffBMenu->setID("color-diff-b-menu");

    auto oldColorBSpr = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
    oldColorBSpr->setColor(to3B(m_gradientColorB));
    oldColorBSpr->setID("gradient-old-color-B-spr");
    oldColorBSpr->setColor(to3B(m_gradientColorB));
    oldColorBSpr->setOpacity(m_gradientColorB.a);
    colorDiffBMenu->addChild(oldColorBSpr);

    m_gradientNewColorBSpr = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
    m_gradientNewColorBSpr->setColor(to3B(m_gradientColorB));
    m_gradientNewColorBSpr->setID("gradient-new-color-B-spr");
    colorDiffBMenu->addChild(m_gradientNewColorBSpr);

    auto resetBtnBSpr = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("geode.loader/reset-gold.png"), 0x20, true, 0.f,
        "GJ_button_01.png", 1.25f
    );
    resetBtnBSpr->setScale(.6f);
    m_gradientResetBtnB = CCMenuItemSpriteExtra::create(
        resetBtnBSpr, this, menu_selector(DTColorPickerPopup::onGradientResetB)
    );
    m_gradientResetBtnB->setID("gradient-reset-B-btn");
    m_gradientResetBtnB->setEnabled(false);
    m_gradientResetBtnB->setOpacity(0);
    colorDiffBMenu->addChild(m_gradientResetBtnB);

    colorDiffBMenu->setContentSize({
        oldColorBSpr->getScaledContentWidth() * 2 + m_gradientResetBtnB->getScaledContentWidth(),
        oldColorBSpr->getScaledContentHeight()
    });
    colorDiffBMenu->updateLayout();
    colorMenu->addChild(colorDiffBMenu);

    //
    auto seperator = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
    seperator->setOpacity(0);
    seperator->setScale(.25f);

    colorMenu->addChild(seperator);

    //
    auto colorGradiantDiffMenu = CCMenu::create();
    colorGradiantDiffMenu->setAnchorPoint({0, .5f});
    colorGradiantDiffMenu->setID("gradient-diff-menu");

    auto sampleSpr = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
    auto fullSize = sampleSpr->getContentSize();
    float squareWidth = fullSize.width / m_gradientSquareCount;

    auto addSquares = [&, fullSize, squareWidth](std::vector<CCSprite*>& out, float startX, char const* idPrefix) {
        for (int i = 0; i < m_gradientSquareCount; i++) {
            auto spr = CCSprite::createWithSpriteFrameName("whiteSquare60_001.png");
            spr->setAnchorPoint({0, .5f});
            spr->setScaleX(1.0f / m_gradientSquareCount);
            spr->setPosition({startX + squareWidth * i, fullSize.height / 2});
            spr->setID(fmt::format("{}-{}", idPrefix, i));

            colorGradiantDiffMenu->addChild(spr);

            out.push_back(spr);
        }
    };
    addSquares(m_gradientOldSquares, 0.f, "gradient-old-spr");
    addSquares(m_gradientNewSquares, fullSize.width, "gradient-new-spr");

    colorGradiantDiffMenu->setContentSize(fullSize * ccp(2, 1));
    colorMenu->addChild(colorGradiantDiffMenu);

    setGradient(m_gradientOldSquares, m_gradientColorA, m_gradientColorB);
    setGradient(m_gradientNewSquares, m_gradientColorA, m_gradientColorB);

    colorMenu->updateLayout();

    auto resetButtonMenu = CCMenu::create();
    resetButtonMenu->setAnchorPoint({0, 0});
    resetButtonMenu->setPosition({0, 0});
    m_gradientPage->addChild(resetButtonMenu, colorMenu->getZOrder() + 1);

    auto moveResetButton = [resetButtonMenu](CCMenuItemSpriteExtra* button) {
        auto worldPosition = button->getParent()->convertToWorldSpace(button->getPosition());
        button->retain();
        button->removeFromParentAndCleanup(false);
        resetButtonMenu->addChild(button);
        button->release();
        button->setPosition(resetButtonMenu->convertToNodeSpace(worldPosition) + ccp(7.5f, 0));
    };
    moveResetButton(m_gradientResetBtnA);
    moveResetButton(m_gradientResetBtnB);

    colorDiffAMenu->setContentWidth(colorDiffAMenu->getContentWidth() - m_gradientResetBtnA->getScaledContentWidth());
    colorDiffBMenu->setContentWidth(colorDiffBMenu->getContentWidth() - m_gradientResetBtnB->getScaledContentWidth());
    colorDiffAMenu->updateLayout();
    colorDiffBMenu->updateLayout();
    colorMenu->updateLayout();

    // picker

    m_gradientPickerA = CCControlColourPicker::colourPicker();
    m_gradientPickerADelegate.popup = this;
    m_gradientPickerADelegate.updatesColorA = true;
    m_gradientPickerA->setDelegate(&m_gradientPickerADelegate);
    m_gradientPickerA->setColorValue(to3B(m_gradientColorA));
    m_gradientPickerA->setScale(.6f);
    m_gradientPickerA->setAnchorPoint({0, 0});
    m_gradientPickerA->setID("gradient-color-picker-A");

    m_gradientPage->addChildAtPosition(m_gradientPickerA, Anchor::TopRight, -m_gradientPickerA->getScaledContentSize() / 2 - ccp(15, 5));

    m_gradientPickerB = CCControlColourPicker::colourPicker();
    m_gradientPickerBDelegate.popup = this;
    m_gradientPickerBDelegate.updatesColorA = false;
    m_gradientPickerB->setDelegate(&m_gradientPickerBDelegate);
    m_gradientPickerB->setColorValue(to3B(m_gradientColorB));
    m_gradientPickerB->setScale(.6f);
    m_gradientPickerB->setAnchorPoint({0, 0});
    m_gradientPickerB->setID("gradient-color-picker-B");

    m_gradientPickerB->setPosition(m_gradientPickerA->getPosition() - ccp(m_gradientPickerA->getScaledContentWidth() / 2 + m_gradientPickerB->getScaledContentWidth() / 2 + 25, 0));
    m_gradientPage->addChild(m_gradientPickerB);

    auto rgbaNodeA = makeRGBA(m_gradientColorA, isRGBA, TAG_GRADIENT_A_COLOR_R_INPUT);
    rgbaNodeA->setPosition(m_gradientPickerA->getPosition() - ccp(0, m_gradientPickerA->getScaledContentHeight() / 2 + rgbaNodeA->getScaledContentHeight() / 2));
    m_gradientPage->addChild(rgbaNodeA);

    auto rgbaNodeB = makeRGBA(m_gradientColorB, isRGBA, TAG_GRADIENT_B_COLOR_R_INPUT);
    rgbaNodeB->setPosition(m_gradientPickerB->getPosition() - ccp(0, m_gradientPickerB->getScaledContentHeight() / 2 + rgbaNodeB->getScaledContentHeight() / 2));
    m_gradientPage->addChild(rgbaNodeB);

    auto durationColumn = CCNode::create();
    durationColumn->setLayout(
        ColumnLayout::create()
            ->setGap(3.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    durationColumn->setID("duration-column");
    m_gradientPage->addChild(durationColumn);

    durationColumn->setPosition({15, 5});

    auto durationText = CCLabelBMFont::create("Duration", "goldFont.fnt");
    durationText->setScale(.55f);
    durationText->setID("duration-text");
    durationColumn->addChild(durationText);

    auto durationInput = TextInput::create(120.f, "duration");
    durationInput->setScale(.7f);
    durationInput->setCallback([&](std::string const& value) {

        auto numRes = geode::utils::numFromString<float>(value);

        if (!numRes.isOk())
            return;

        auto num = numRes.unwrap();

        m_duration = std::clamp(num, DURATION_LIMITS.x, DURATION_LIMITS.y);

        setGradient(m_gradientNewSquares, m_gradientColorA, m_gradientColorB);
        updateWavePreview();

        if (m_gradientChangedCallback) {
            GradientData data;
            data.colorA = m_gradientColorA;
            data.colorB = m_gradientColorB;
            data.duration = m_duration;
            data.stagger = m_stagger;

            m_gradientChangedCallback(data);
        }
    });
    durationInput->setString(std::to_string(m_duration));
    durationInput->setCommonFilter(CommonFilter::Float);
    durationInput->setID("duration-input");
    durationColumn->addChild(durationInput);

    durationColumn->updateLayout();

    auto staggerColumn = CCNode::create();
    staggerColumn->setLayout(
        ColumnLayout::create()
            ->setGap(3.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    staggerColumn->setID("stagger-column");
    m_gradientPage->addChild(staggerColumn);

    staggerColumn->setPosition(durationColumn->getPosition() + ccp(0, durationColumn->getScaledContentHeight() + 2));

    auto staggerText = CCLabelBMFont::create("Stagger", "goldFont.fnt");
    staggerText->setScale(.55f);
    staggerText->setID("stagger-text");
    staggerColumn->addChild(staggerText);

    auto staggerInput = TextInput::create(120.f, "stagger");
    staggerInput->setScale(.7f);
    staggerInput->setCallback([this](std::string const& value) {
        auto numRes = geode::utils::numFromString<float>(value);
        if (!numRes.isOk())
            return;

        auto num = numRes.unwrap();
        m_stagger = std::clamp(num, STAGGER_LIMITS.x, STAGGER_LIMITS.y);

        setGradient(m_gradientNewSquares, m_gradientColorA, m_gradientColorB);
        updateWavePreview();

        if (m_gradientChangedCallback) {
            GradientData data;
            data.colorA = m_gradientColorA;
            data.colorB = m_gradientColorB;
            data.duration = m_duration;
            data.stagger = m_stagger;

            m_gradientChangedCallback(data);
        }
    });
    staggerInput->setString(std::to_string(m_stagger));
    staggerInput->setCommonFilter(CommonFilter::Float);
    staggerInput->setID("stagger-input");
    staggerColumn->addChild(staggerInput);

    staggerColumn->updateLayout();

    updateGradientColorA(to3B(m_gradientColorA), true);
    updateGradientColorB(to3B(m_gradientColorB), true);
}

void DTColorPickerPopup::setGradient(std::vector<CCSprite*> const& squares, ccColor4B const& a, ccColor4B const& b) {
    for (int i = 0; i < squares.size(); i++) {
        squares[i]->stopActionByTag(12);

        auto animation = CCRepeatForever::create(CCGradientAction::create(
            m_duration * 3,
            a,
            b,
            -m_stagger * static_cast<float>(i)
        ));
        animation->setTag(12);
        squares[i]->runAction(animation);
    }
}

void DTColorPickerPopup::updateGradientColorA(cocos2d::ccColor3B color, bool doUpdateHexInput) {
    m_gradientColorA.r = color.r;
    m_gradientColorA.g = color.g;
    m_gradientColorA.b = color.b;

    bool changed = m_gradientColorA.r != m_originalGradientColorA.r || m_gradientColorA.g != m_originalGradientColorA.g || m_gradientColorA.b != m_originalGradientColorA.b || m_gradientColorA.a != m_originalGradientColorA.a;
    if (m_gradientResetBtnA) {
        m_gradientResetBtnA->setVisible(changed);
    }

    if (m_gradientNewColorASpr) {
        m_gradientNewColorASpr->setColor(color);
        m_gradientNewColorASpr->setOpacity(m_gradientColorA.a);
    }
    setGradient(m_gradientNewSquares, m_gradientColorA, m_gradientColorB);

    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_R_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_R_INPUT])->setString(std::to_string(m_gradientColorA.r));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_G_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_G_INPUT])->setString(std::to_string(m_gradientColorA.g));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_B_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_B_INPUT])->setString(std::to_string(m_gradientColorA.b));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_A_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_A_INPUT])->setString(std::to_string(m_gradientColorA.a));
    }
    if (m_gradientOpacitySliders.count(TAG_GRADIENT_A_COLOR_A_INPUT)) {
        m_gradientOpacitySliders[TAG_GRADIENT_A_COLOR_A_INPUT]->setValue(m_gradientColorA.a / 255.f);
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_HEX_INPUT) && doUpdateHexInput) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_HEX_INPUT])->setString(cc3bToHexString(to3B(m_gradientColorA)));
    }

    if (m_gradientChangedCallback) {
        GradientData data;
        data.colorA = m_gradientColorA;
        data.colorB = m_gradientColorB;
        data.duration = m_duration;
        data.stagger = m_stagger;

        m_gradientChangedCallback(data);
    }

    updateWavePreview();
}

void DTColorPickerPopup::updateGradientColorB(cocos2d::ccColor3B color, bool doUpdateHexInput) {
    m_gradientColorB.r = color.r;
    m_gradientColorB.g = color.g;
    m_gradientColorB.b = color.b;

    bool changed = m_gradientColorB.r != m_originalGradientColorB.r || m_gradientColorB.g != m_originalGradientColorB.g || m_gradientColorB.b != m_originalGradientColorB.b || m_gradientColorB.a != m_originalGradientColorB.a;
    if (m_gradientResetBtnB) {
        m_gradientResetBtnB->setVisible(changed);
    }

    if (m_gradientNewColorBSpr) {
        m_gradientNewColorBSpr->setColor(color);
        m_gradientNewColorBSpr->setOpacity(m_gradientColorB.a);
    }
    setGradient(m_gradientNewSquares, m_gradientColorA, m_gradientColorB);

    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_R_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_R_INPUT])->setString(std::to_string(m_gradientColorB.r));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_G_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_G_INPUT])->setString(std::to_string(m_gradientColorB.g));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_B_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_B_INPUT])->setString(std::to_string(m_gradientColorB.b));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_A_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_A_INPUT])->setString(std::to_string(m_gradientColorB.a));
    }
    if (m_gradientOpacitySliders.count(TAG_GRADIENT_B_COLOR_A_INPUT)) {
        m_gradientOpacitySliders[TAG_GRADIENT_B_COLOR_A_INPUT]->setValue(m_gradientColorB.a / 255.f);
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_HEX_INPUT) && doUpdateHexInput) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_HEX_INPUT])->setString(cc3bToHexString(to3B(m_gradientColorB)));
    }

    if (m_gradientChangedCallback) {
        GradientData data;
        data.colorA = m_gradientColorA;
        data.colorB = m_gradientColorB;
        data.duration = m_duration;
        data.stagger = m_stagger;

        m_gradientChangedCallback(data);
    }

    updateWavePreview();
}

void DTColorPickerPopup::onGradientResetA(cocos2d::CCObject*) {
    m_gradientColorA.a = m_originalGradientColorA.a;
    updateGradientColorA(to3B(m_originalGradientColorA));
    if (m_gradientPickerA) {
        m_gradientPickerA->setColorValue(to3B(m_originalGradientColorA));
    }

    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_R_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_R_INPUT])->setString(std::to_string(m_originalGradientColorA.r));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_G_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_G_INPUT])->setString(std::to_string(m_originalGradientColorA.g));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_B_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_B_INPUT])->setString(std::to_string(m_originalGradientColorA.b));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_A_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_A_INPUT])->setString(std::to_string(m_originalGradientColorA.a));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_A_COLOR_HEX_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_A_COLOR_HEX_INPUT])->setString(cc3bToHexString(to3B(m_originalGradientColorA)));
    }
}

void DTColorPickerPopup::onGradientResetB(cocos2d::CCObject*) {
    m_gradientColorB.a = m_originalGradientColorB.a;
    updateGradientColorB(to3B(m_originalGradientColorB));
    if (m_gradientPickerB) {
        m_gradientPickerB->setColorValue(to3B(m_originalGradientColorB));
    }

    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_R_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_R_INPUT])->setString(std::to_string(m_originalGradientColorB.r));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_G_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_G_INPUT])->setString(std::to_string(m_originalGradientColorB.g));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_B_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_B_INPUT])->setString(std::to_string(m_originalGradientColorB.b));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_A_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_A_INPUT])->setString(std::to_string(m_originalGradientColorB.a));
    }
    if (m_gradientInputs.count(TAG_GRADIENT_B_COLOR_HEX_INPUT)) {
        std::get<0>(m_gradientInputs[TAG_GRADIENT_B_COLOR_HEX_INPUT])->setString(cc3bToHexString(to3B(m_originalGradientColorB)));
    }
}

void DTColorPickerPopup::createWavePage(WaveData const& data) {
    auto popupBg = m_mainLayer->getChildByID("popup-bg");
    if (!popupBg)
        return;

    m_waveData = data;
    m_waveData.speed = std::clamp(m_waveData.speed, WAVE_SPEED_LIMITS.x, WAVE_SPEED_LIMITS.y);
    m_waveData.distanceY = std::clamp(m_waveData.distanceY, WAVE_DISTANCE_LIMITS.x, WAVE_DISTANCE_LIMITS.y);
    m_waveData.distanceX = std::clamp(m_waveData.distanceX, WAVE_DISTANCE_LIMITS.x, WAVE_DISTANCE_LIMITS.y);
    m_waveData.offsetPerIndex = std::clamp(m_waveData.offsetPerIndex, WAVE_OFFSET_LIMITS.x, WAVE_OFFSET_LIMITS.y);

    m_wavePage = CCNode::create();
    m_wavePage->setID("wave-page");
    m_mainLayer->addChild(m_wavePage, 0, static_cast<int>(Pages::Wave));
    m_wavePage->setVisible(false);
    setAllChildrenEnabled(m_wavePage, false);
    m_wavePage->setContentSize(popupBg->getScaledContentSize());
    m_wavePage->setPosition(popupBg->getPosition());
    m_wavePage->setAnchorPoint({.5f, .5f});

    m_waveEnabledMenu = CCMenu::create();
    m_waveEnabledMenu->setID("wave-enabled-menu");

    m_waveEnabledToggle = SimpleToggler::createWithDefaults(.45f, m_waveEnabled);
    m_waveEnabledToggle->setID("wave-enabled-toggle");
    m_waveEnabledToggle->setCallback([this](bool enabled) {
        if (!AuthManager::isDoner()) {
            m_waveEnabledToggle->toggle(enabled);
            return;
        }

        m_waveEnabled = enabled;
        if (m_waveEnabledChangedCallback)
            m_waveEnabledChangedCallback(enabled);

        onWavePageOpen(m_wavePage);
    });
    if (!AuthManager::isDoner()) {
        m_waveEnabledToggle->setEnabled(false);
        m_waveEnabledToggle->setOpacity(128);
    }

    auto enabledText = CCLabelBMFont::create("enabled", "bigFont.fnt");
    enabledText->setScale(.45f);

    auto toggleWidth = m_waveEnabledToggle->getScaledContentWidth();
    auto textWidth = enabledText->getScaledContentWidth();
    auto toggleGap = 3.f;
    auto menuWidth = toggleWidth + toggleGap + textWidth;
    auto menuHeight = std::max(
        m_waveEnabledToggle->getScaledContentHeight(),
        enabledText->getScaledContentHeight()
    );
    enabledText->setPosition({textWidth / 2, menuHeight / 2});
    m_waveEnabledToggle->setPosition({textWidth + toggleGap + toggleWidth / 2, menuHeight / 2});
    m_waveEnabledMenu->setContentSize({menuWidth, menuHeight});
    m_waveEnabledMenu->addChild(m_waveEnabledToggle);
    m_waveEnabledMenu->addChild(enabledText);
    m_waveEnabledMenu->setVisible(false);
    m_wavePage->addChild(m_waveEnabledMenu, 2);

    m_waveDisabledFG = NineSlice::create(
        "square02b_001.png", { 0.f, 0.f, 80.f, 80.f }
    );
    m_waveDisabledFG->setID("popup-bg");
    m_waveDisabledFG->setScale(.5f);
    m_waveDisabledFG->setColor({0, 0, 0});
    m_waveDisabledFG->setOpacity(85);
    m_waveDisabledFG->setContentSize({m_size.width * 2 - 40.f, m_size.height * 2 - 140.f});
    m_waveDisabledFG->setZOrder(5);
    m_wavePage->addChildAtPosition(m_waveDisabledFG, Anchor::Center);

    m_waveDonorLabel = RichTextArea::create(
        "<wave>Donor Feature</wave>", "bigFont.fnt"
    );
    m_waveDonorLabel->setID("wave-donor-label");
    m_waveDonorLabel->registerRichTextKey(DTLabel::makeWaveKey([] { return true; }));
    m_waveDonorLabel->setText(m_waveDonorLabel->getRawText());
    m_waveDonorLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_waveDonorLabel->setPosition(m_waveDisabledFG->getPosition());
    m_waveDonorLabel->setZOrder(10);
    m_wavePage->addChild(m_waveDonorLabel);

    m_wavePreview = RichTextArea::create("", "bigFont.fnt", .75f);
    m_wavePreview->setID("wave-preview");
    m_wavePreview->registerRichTextKey(DTLabel::makeWaveKey([] { return true; }));
    m_wavePreview->registerRichTextKey(DTLabel::makeGradientKey([] { return true; }));
    m_wavePreview->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_wavePage->addChildAtPosition(m_wavePreview, Anchor::Bottom, {0, 25});

    updateWaveData();
}

void DTColorPickerPopup::updateWaveData() {
    updateWavePreview();

    if (m_waveChangedCallback)
        m_waveChangedCallback(m_waveData);
}

void DTColorPickerPopup::updateWavePreview() {
    if (!m_wavePreview) return;

    auto sample = fmt::format(
        "<wave={},{},{},{}>Wave Preview</wave>",
        m_waveData.speed,
        m_waveData.distanceY,
        m_waveData.distanceX,
        m_waveData.offsetPerIndex
    );

    if (m_gradientEnabled && AuthManager::isDoner()) {
        sample = fmt::format(
            "<gradient={},{},{},{}>{}</gradient>",
            cc4bToHexString(m_gradientColorA),
            cc4bToHexString(m_gradientColorB),
            m_duration,
            m_stagger,
            sample
        );
    }

    m_wavePreview->setText(sample);
}

void DTColorPickerPopup::onWavePageOpen(CCNode* page) {
    auto allEnabled = m_waveEnabled && AuthManager::isDoner();
    setAllChildrenEnabled(page, allEnabled);
    m_waveDisabledFG->setVisible(!allEnabled);

    auto isDoner = AuthManager::isDoner();
    m_waveEnabledToggle->setEnabled(isDoner);
    m_waveEnabledMenu->setEnabled(isDoner);
    m_waveEnabledToggle->setOpacity(isDoner ? 255 : 128);
    m_waveDonorLabel->setVisible(!isDoner);
}

void DTColorPickerPopup::setAllChildrenEnabled(CCNode* node, bool enabled) {
    if (!node) return;

    auto processNode = [&](CCNode* n) -> bool {
        
        if (auto control = dynamic_cast<CCControl*>(n)) {
            control->setEnabled(enabled);
        }
        else if (auto input = dynamic_cast<TextInput*>(n)) {
            input->setEnabled(enabled);

            return false;
        }
        else if (auto menuItem = dynamic_cast<CCMenuItem*>(n)) {
            menuItem->setEnabled(enabled);
        }
        else if (auto menu = dynamic_cast<CCMenu*>(n)) {
            menu->setEnabled(enabled);
        }
        else if (auto slider = dynamic_cast<Slider*>(n)) {
            if (enabled)
                slider->enableSlider();
            else
                slider->disableSlider();
        }

        return true;
    };

    if (!processNode(node)) return;

    for (auto child : CCArrayExt<CCNode*>(node->getChildren())) {
        setAllChildrenEnabled(child, enabled);
    }

}

void DTColorPickerPopup::colorValueChanged(cocos2d::ccColor3B color) {
    geode::ColorPickPopup::colorValueChanged(color);
}

void DTColorPickerPopup::opacitySliderAChanged(cocos2d::CCObject*) {
    updateGradientOpacity(TAG_GRADIENT_A_COLOR_A_INPUT);
}

void DTColorPickerPopup::opacitySliderBChanged(cocos2d::CCObject*) {
    updateGradientOpacity(TAG_GRADIENT_B_COLOR_A_INPUT);
}

void DTColorPickerPopup::updateGradientOpacity(int tag) {
    auto slider = m_gradientOpacitySliders.find(tag);
    auto input = m_gradientInputs.find(tag);
    if (slider == m_gradientOpacitySliders.end() || input == m_gradientInputs.end()) return;

    auto color = std::get<1>(input->second);
    color->a = static_cast<GLubyte>(std::lround(slider->second->getValue() * 255.f));

    if (tag == TAG_GRADIENT_A_COLOR_A_INPUT) {
        updateGradientColorA(to3B(*color));
    } else if (tag == TAG_GRADIENT_B_COLOR_A_INPUT) {
        updateGradientColorB(to3B(*color));
    }
}

void DTColorPickerPopup::GradientColorPickerDelegate::colorValueChanged(cocos2d::ccColor3B color) {
    if (!popup) return;

    if (updatesColorA) {
        popup->updateGradientColorA(color);
    } else {
        popup->updateGradientColorB(color);
    }
}

CCNode* DTColorPickerPopup::makeRGBA(ccColor4B& color, bool isRGBA, int tag) {
    auto inputColumn = CCNode::create();
    inputColumn->setLayout(
        ColumnLayout::create()
            ->setGap(3.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    inputColumn->setID("input-column");

    auto rgbRow = CCNode::create();
    rgbRow->setLayout(
        RowLayout::create()
            ->setGap(5.f)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    rgbRow->setID("rgb-row");
    inputColumn->addChild(rgbRow);

    // r

    auto rColumn = CCNode::create();
    rColumn->setLayout(
        ColumnLayout::create()
            ->setGap(3.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    rColumn->setID("r-column");
    rgbRow->addChild(rColumn);

    auto rText = CCLabelBMFont::create("R", "goldFont.fnt");
    rText->setScale(.55f);
    rText->setID("r-text");
    rColumn->addChild(rText);

    auto gradientArInput = TextInput::create(50.f, "R");
    gradientArInput->setScale(.7f);
    gradientArInput->setDelegate(this, tag);
    gradientArInput->setID("r-input");
    rColumn->addChild(gradientArInput);

    m_gradientInputs.erase(tag);
    m_gradientInputs.emplace(
        tag,
        std::tuple<TextInput*, ccColor4B*, GLubyte*>(gradientArInput, &color, &color.r)
    );

    rColumn->updateLayout();
    auto rRect = calculateChildCoverage(rColumn);
    rColumn->setContentSize(rRect.size);

    // g

    auto gColumn = CCNode::create();
    gColumn->setLayout(
        ColumnLayout::create()
            ->setGap(3.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    gColumn->setID("g-column");
    rgbRow->addChild(gColumn);

    auto gText = CCLabelBMFont::create("G", "goldFont.fnt");
    gText->setScale(.55f);
    gText->setID("g-text");
    gColumn->addChild(gText);

    auto gradientAgInput = TextInput::create(50.f, "G");
    gradientAgInput->setScale(.7f);
    gradientAgInput->setDelegate(this, tag + 1);
    gradientAgInput->setID("g-input");
    gColumn->addChild(gradientAgInput);

    m_gradientInputs.erase(tag + 1);
    m_gradientInputs.emplace(
        tag + 1,
        std::tuple<TextInput*, ccColor4B*, GLubyte*>(gradientAgInput, &color, &color.g)
    );

    gColumn->updateLayout();
    auto gRect = calculateChildCoverage(gColumn);
    gColumn->setContentSize(gRect.size);

    // b

    auto bColumn = CCNode::create();
    bColumn->setLayout(
        ColumnLayout::create()
            ->setGap(3.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    bColumn->setID("b-column");
    rgbRow->addChild(bColumn);

    auto bText = CCLabelBMFont::create("B", "goldFont.fnt");
    bText->setScale(.55f);
    bText->setID("b-text");
    bColumn->addChild(bText);

    auto gradientAbInput = TextInput::create(50.f, "B");
    gradientAbInput->setScale(.7f);
    gradientAbInput->setDelegate(this, tag + 2);
    gradientAbInput->setID("b-input");
    bColumn->addChild(gradientAbInput);

    m_gradientInputs.erase(tag + 2);
    m_gradientInputs.emplace(
        tag + 2,
        std::tuple<TextInput*, ccColor4B*, GLubyte*>(gradientAbInput, &color, &color.b)
    );

    bColumn->updateLayout();
    auto bRect = calculateChildCoverage(bColumn);
    bColumn->setContentSize(bRect.size);

    // hex

    auto hexColumn = CCLayer::create();
    hexColumn->setLayout(
        ColumnLayout::create()
            ->setGap(3.f)
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    hexColumn->setID("hex-column");
    inputColumn->addChild(hexColumn);

    auto hexText = CCLabelBMFont::create("Hex", "goldFont.fnt");
    hexText->setScale(.55f);
    hexText->setID("hex-text");
    hexColumn->addChild(hexText);

    auto gradientAhexInput = TextInput::create(165.f, "Hex");
    gradientAhexInput->setScale(.7f);
    gradientAhexInput->setDelegate(this, tag + 3);
    gradientAhexInput->setID("hex-input");
    hexColumn->addChild(gradientAhexInput);

    m_gradientInputs.erase(tag + 3);
    m_gradientInputs.emplace(
        tag + 3,
        std::tuple<TextInput*, ccColor4B*, GLubyte*>(
            gradientAhexInput, &color, nullptr
        )
    );

    hexColumn->updateLayout();
    rgbRow->updateLayout();

    auto opacityColumn = CCLayer::create();
    opacityColumn->setLayout(
        ColumnLayout::create()
            ->setGap(5.f)
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::Start)
            ->setAutoGrowAxis(0)
    );
    opacityColumn->setID("opacity-column");
    inputColumn->addChild(opacityColumn);

    if (isRGBA) {
        auto opacitySection = CCLayer::create();
        opacitySection->setLayout(
            RowLayout::create()
                ->setGap(10.f)
                ->setAutoScale(false)
                ->setAxisAlignment(AxisAlignment::Start)
                ->setCrossAxisLineAlignment(AxisAlignment::Start)
                ->setAutoGrowAxis(0)
        );
        opacitySection->setID("opacity-section");
        opacityColumn->addChild(opacitySection);

        auto sliderColumn = CCLayer::create();
        sliderColumn->setLayout(
            ColumnLayout::create()
                ->setGap(7.f)
                ->setAxisReverse(true)
                ->setAutoScale(false)
                ->setAxisAlignment(AxisAlignment::Start)
                ->setAutoGrowAxis(0)
        );
        sliderColumn->setID("slider-column");
        opacitySection->addChild(sliderColumn);


        auto opacityText = CCLabelBMFont::create("Opacity", "goldFont.fnt");
        opacityText->setScale(.55f);
        opacityText->setID("opacity-text");
        sliderColumn->addChild(opacityText);

        auto opacitySlider = Slider::create(
            this,
            tag == TAG_GRADIENT_A_COLOR_R_INPUT
                ? menu_selector(DTColorPickerPopup::opacitySliderAChanged)
                : menu_selector(DTColorPickerPopup::opacitySliderBChanged),
            .3f
        );
        opacitySlider->setValue(color.a / 255.f);
        opacitySlider->setTag(tag + 4);
        opacitySlider->setID("opacity-slider");
        m_gradientOpacitySliders[tag + 4] = opacitySlider;

        auto sliderWrapper = CCNode::create();
        sliderWrapper->setContentSize(ccp(opacitySlider->m_width, opacitySlider->m_height) * .3f);
        sliderWrapper->setID("slider-wrapper");
        sliderWrapper->addChildAtPosition(opacitySlider, Anchor::Center, ccp(0, 0));
        sliderColumn->addChild(sliderWrapper);

        auto opacityInput = TextInput::create(60.f, "Opacity");
        opacityInput->setScale(.7f);
        opacityInput->setDelegate(this, tag + 4);
        opacityInput->setID("opacity-input");
        opacitySection->addChild(opacityInput);

        m_gradientInputs.erase(tag + 4);
        m_gradientInputs.emplace(
            tag + 4,
            std::tuple<TextInput*, ccColor4B*, GLubyte*>(
                opacityInput, &color, &color.a
            )
        );

        sliderColumn->updateLayout();
        opacitySection->updateLayout();
    }

    opacityColumn->updateLayout();

    inputColumn->updateLayout();

    inputColumn->setAnchorPoint({.5f, .5f});

    return inputColumn;
}

void DTColorPickerPopup::textChanged(CCTextInputNode* input) {
    if (input->getTag() >= 1 && input->getTag() <= 4) {
        ColorPickPopup::textChanged(input);
        return;
    }

    if (input->getString().size() && m_gradientInputs.count(input->getTag())) {
        auto& [textInput, colorPtr, colorElementPtr] = m_gradientInputs[input->getTag()];

        if (!colorPtr) return;

        if (!colorElementPtr) {
            if (auto color = cc3bFromHexString(input->getString(), true)) {
                colorPtr->r = color.unwrap().r;
                colorPtr->g = color.unwrap().g;
                colorPtr->b = color.unwrap().b;

                if (input->getTag() >= TAG_GRADIENT_A_COLOR_R_INPUT && input->getTag() <= TAG_GRADIENT_A_COLOR_A_INPUT) {
                    updateGradientColorA(to3B(*colorPtr), false);
                } else if (input->getTag() >= TAG_GRADIENT_B_COLOR_R_INPUT && input->getTag() <= TAG_GRADIENT_B_COLOR_A_INPUT) {
                    updateGradientColorB(to3B(*colorPtr), false);
                }
            }
            return;
        }

        if (&colorPtr->a == colorElementPtr) {
            auto res = numFromString<uint32_t>(input->getString().c_str());
            if (res) colorPtr->a = std::clamp(res.unwrap(), 0u, 255u);
        }
        else if (colorElementPtr == &colorPtr->r) {
            auto res = numFromString<uint32_t>(input->getString().c_str());
            if (res) colorPtr->r = std::clamp(res.unwrap(), 0u, 255u);
        }
        else if (colorElementPtr == &colorPtr->g) {
            auto res = numFromString<uint32_t>(input->getString().c_str());
            if (res) colorPtr->g = std::clamp(res.unwrap(), 0u, 255u);
        }
        else if (colorElementPtr == &colorPtr->b) {
            auto res = numFromString<uint32_t>(input->getString().c_str());
            if (res) colorPtr->b = std::clamp(res.unwrap(), 0u, 255u);
        }

        if (input->getTag() >= TAG_GRADIENT_A_COLOR_R_INPUT && input->getTag() <= TAG_GRADIENT_A_COLOR_A_INPUT) {
            updateGradientColorA(to3B(*colorPtr));
        } else if (input->getTag() >= TAG_GRADIENT_B_COLOR_R_INPUT && input->getTag() <= TAG_GRADIENT_B_COLOR_A_INPUT) {
            updateGradientColorB(to3B(*colorPtr));
        }
    }
    this->updateState(input);
}

void DTColorPickerPopup::setGradientChangedCallback(geode::Function<void(GradientData const& data)> callback) {
    m_gradientChangedCallback = std::move(callback);
}

void DTColorPickerPopup::setGradientEnabledChangedCallback(geode::Function<void(bool enabled)> callback) {
    m_gradientEnabledChangedCallback = std::move(callback);
}

void DTColorPickerPopup::setWaveChangedCallback(geode::Function<void(WaveData const& data)> callback) {
    m_waveChangedCallback = std::move(callback);
}

void DTColorPickerPopup::setWaveEnabledChangedCallback(geode::Function<void(bool enabled)> callback) {
    m_waveEnabledChangedCallback = std::move(callback);
}

void DTColorPickerPopup::onGradientPageOpen(CCNode* page) {
    auto allEnabled = m_gradientEnabled && AuthManager::isDoner();

    setAllChildrenEnabled(page, allEnabled);

    m_gradientDisabledFG->setVisible(!allEnabled);

    auto enableEnabled = AuthManager::isDoner();

    m_gradientEnabledToggle->setEnabled(enableEnabled);
    m_gradientEnabledMenu->setEnabled(enableEnabled);
    m_gradientEnabledToggle->setOpacity(enableEnabled ? 255 : 128);

    m_donorLabel->setVisible(!enableEnabled);
}