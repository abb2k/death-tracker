#include "GraphCell.hpp"

#include <managers/StatsManager.hpp>
#include <nodes/layers/DTLayer.hpp>

GraphCell* GraphCell::create(float width, const DTGraphInfo& graphInfo){
    auto ret = new GraphCell();
    if (ret && ret->init(width, graphInfo)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool GraphCell::init(float width, const DTGraphInfo& graphInfo){
    if (!CCNode::init()) return false;

    this->setContentSize({width, 45});

    this->graphInfo = graphInfo;

    everythingParent = CCNode::create();
    everythingParent->setAnchorPoint({0, 0});
    everythingParent->setPositionX(0);
    everythingParent->ignoreAnchorPointForPosition(false);
    this->addChild(everythingParent);

    bg = CCScale9Sprite::create("square01_001.png");
    bg->setAnchorPoint({0, 1});
    bg->setScale(.2f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    bg->setPositionY(bg->getScaledContentHeight());
    everythingParent->addChild(bg);

    everythingParent->setContentSize(bg->getScaledContentSize());

    mainMenu = CCMenu::create();
    mainMenu->setPosition({0, 0});
    mainMenu->setZOrder(1);
    everythingParent->addChild(mainMenu);

    label = SimpleTextArea::create(graphInfo.name, "bigFont.fnt");
    label->setScale(.25f);
    label->setAnchorPoint({0, 1});
    label->setPosition(ccp(0 + 3, this->getContentHeight() - 5));
    label->setWidth(width - 10);
    label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    everythingParent->addChild(label);

    outerColor = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    outerColor->setColor({graphInfo.outlineColor.r, graphInfo.outlineColor.g, graphInfo.outlineColor.b});
    outerColor->setOpacity(graphInfo.outlineColor.a);

    innerColor = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    innerColor->setScale(.75f);
    innerColor->setPosition(outerColor->getContentSize() / 2);
    innerColor->setColor({graphInfo.color.r, graphInfo.color.g, graphInfo.color.b});
    innerColor->setOpacity(graphInfo.color.a);
    outerColor->addChild(innerColor);

    auto disabledCircle = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    disabledCircle->setColor({ 84, 43, 43 });

    auto disabledCircleX = CCSprite::createWithSpriteFrameName("edit_delBtnSmall_001.png");
    disabledCircleX->setPosition(disabledCircle->getContentSize() / 2);
    disabledCircleX->setScale(1.65f);
    disabledCircle->addChild(disabledCircleX);

    enableToggleBtn = SimpleToggler::create(
        disabledCircle,
        outerColor,
        .35f,
        graphInfo.isEnabled
    );
    enableToggleBtn->setPosition(label->getPosition() + ccp(
        enableToggleBtn->getContentWidth() / 2 + 1,
        -label->getScaledContentHeight() - enableToggleBtn->getContentHeight() / 2 - 2
    ));
    enableToggleBtn->setZOrder(1);
    enableToggleBtn->setID("toggle-btn");
    enableToggleBtn->setCallback([this](auto state){
        setEnabledInfo(state, false, true);
    });
    mainMenu->addChild(enableToggleBtn);

    auto bgCircle = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    bgCircle->setColor({0,0,0});
    bgCircle->setPosition(enableToggleBtn->getPosition());
    bgCircle->setScale(.4f);
    everythingParent->addChild(bgCircle);

    this->setZOrder(graphInfo.orderPos);

    auto arrowDownSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    arrowDownSpr->setScale(.2f);
    arrowDownSpr->setRotation(-90);
    auto arrowDown = CCMenuItemSpriteExtra::create(
        arrowDownSpr,
        this,
        menu_selector(GraphCell::onArrowDown)
    );
    arrowDown->setID("down-arrow");
    arrowDown->setPosition({
        width - arrowDown->getContentWidth() / 2 - 4,
        10
    });
    mainMenu->addChild(arrowDown);

    auto arrowUpSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    arrowUpSpr->setScale(.2f);
    arrowUpSpr->setRotation(90);
    auto arrowUp = CCMenuItemSpriteExtra::create(
        arrowUpSpr,
        this,
        menu_selector(GraphCell::onArrowUp)
    );
    arrowUp->setID("up-arrow");
    arrowUp->setPosition({
        arrowDown->getPositionX(),
        arrowDown->getPositionY() + arrowUp->getContentHeight() / 2 + arrowDown->getContentHeight() / 2 + 1
    });
    mainMenu->addChild(arrowUp);

    auto settingBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingBtnSpr->setScale(.25f);
    auto settingBtn = CCMenuItemSpriteExtra::create(
        settingBtnSpr,
        this,
        menu_selector(GraphCell::onOptions)
    );
    settingBtn->setID("settings-btn");
    settingBtn->setScale(12 / settingBtn->getContentHeight());
    settingBtn->setPosition({enableToggleBtn->getPositionX(), enableToggleBtn->getPositionY() - enableToggleBtn->getContentHeight() / 2 - settingBtn->getContentHeight() / 2});
    mainMenu->addChild(settingBtn);

    typeSwitcher = OptionSwitcher<DTGraphType>::create(90, {
        {DTGraphType::Passrate, "Passrate"},
        {DTGraphType::Reachrate, "Reachrate"}
    });
    typeSwitcher->setID("type-switcher");
    typeSwitcher->setScale(.4f);
    typeSwitcher->setAnchorPoint({.5f, 0});
    typeSwitcher->setPositionX(width / 2 + 2);
    typeSwitcher->setPositionY(6.5f);
    typeSwitcher->setCallback([this](auto value){
        setType(value);
    });
    typeSwitcher->setValue(graphInfo.type, false);
    mainMenu->addChild(typeSwitcher);
    
    auto typeSwitcherLabel = CCLabelBMFont::create("Type:", "bigFont.fnt");
    typeSwitcherLabel->setScale(.25f);
    typeSwitcherLabel->setPosition(typeSwitcher->getPosition() + ccp(
        0,
        typeSwitcher->getScaledContentHeight() + typeSwitcherLabel->getScaledContentHeight() / 2
    ));
    typeSwitcherLabel->setID("type-label");
    everythingParent->addChild(typeSwitcherLabel);

    extrasContainer = CCNode::create();
    extrasContainer->setContentSize({width / 1.2f, 80});
    extrasContainer->setPosition({0, 0});
    extrasContainer->setZOrder(2);
    extrasContainer->setLayout(SimpleAxisLayout::create(Axis::Column)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisScaling(AxisScaling::Scale)
    );
    extrasContainer->setAnchorPoint({.5f, 1});
    extrasContainer->setPositionX(width / 2);
    everythingParent->addChild(extrasContainer);

    sessionContainer = CCNode::create();
    sessionContainer->setVisible(false);
    extrasContainer->addChild(sessionContainer);
    
    sessionSelectorLabel = CCLabelBMFont::create("Session", "bigFont.fnt");
    sessionSelectorLabel->setScale(.4f);
    sessionSelectorLabel->setAnchorPoint({.5f, 1});
    sessionContainer->addChild(sessionSelectorLabel);

    sessionSelector = SessionSelector::create(DTLayer::get()->sessionSelector->getMaximumCount());
    sessionSelector->setEnabled(false);
    sessionSelector->setScale(.45f);
    sessionSelector->setAnchorPoint({.5f, 0});
    sessionSelector->setCallback([&](auto sessionIndex){
        sendNewSession(sessionIndex);
    });
    sessionContainer->addChild(sessionSelector);

    sessionContainer->setContentSize({width, sessionSelector->getScaledContentHeight() + sessionSelectorLabel->getScaledContentHeight() + 5});
    sessionSelectorLabel->setPosition({
        sessionContainer->getContentWidth() / 2,
        sessionContainer->getContentHeight()
    });
    sessionSelector->setPosition({
        sessionContainer->getContentWidth() / 2,
        0
    });

    runContainer = CCMenu::create();
    runContainer->setVisible(false);
    extrasContainer->addChild(runContainer);
    
    runLabel = CCLabelBMFont::create("Run Start %", "bigFont.fnt");
    runLabel->setScale(.3f);
    runLabel->setAnchorPoint({.5f, 1});
    runContainer->addChild(runLabel);

    runInput = TextInput::create(width / 1.1f, "run%");
    runInput->setEnabled(false);
    runInput->setScale(.7f);
    runInput->setAnchorPoint({.5f, 0});
    runInput->setCommonFilter(CommonFilter::Uint);
    runInput->setCallback([&](auto str){
        auto numRes = utils::numFromString<int>(str);
        int num = 0;
        if (numRes.isOk())
            num = numRes.unwrap();

        if (num > 100){
            num = 100;
            runInput->setString("100");
        }
        
        if (onNewRun != NULL)
            onNewRun(this, num);
    });
    runContainer->addChild(runInput);

    runContainer->setContentSize({width, runInput->getScaledContentHeight() + runLabel->getScaledContentHeight() + 5});
    runLabel->setPosition({
        runContainer->getContentWidth() / 2,
        runContainer->getContentHeight()
    });
    runInput->setPosition({
        runContainer->getContentWidth() / 2,
        0
    });

    auto runSelectHelperTopSpr = CCSprite::createWithSpriteFrameName("edit_findBtn_001.png");
    auto runSelectHelperSpr = ButtonSprite::create(
        runSelectHelperTopSpr,
        runSelectHelperTopSpr->getContentHeight(),
        runSelectHelperTopSpr->getContentHeight(),
        runSelectHelperTopSpr->getContentHeight(),
        1.25f,
        false,
        "GJ_button_04.png",
        true
    );
    runSelectHelperSpr->setScale(0.4f);
    runSelectHelperBtn = CCMenuItemSpriteExtra::create(
        runSelectHelperSpr,
        this,
        menu_selector(GraphCell::onRunSelectHelper)
    );
    runSelectHelperBtn->setPosition(runInput->getPosition() + ccp(runInput->getScaledContentWidth() / 2 + 1, runInput->getScaledContentHeight() / 2));
    runContainer->addChild(runSelectHelperBtn);

    runInput->setPositionX(runInput->getPositionX() - runSelectHelperBtn->getContentWidth() / 2 - 1);

    runsFloatList = FloatingList::create({runInput->getScaledContentWidth(), 100});
    runsFloatList->setPosition(runInput->getPosition() + ccp(0, runInput->getScaledContentHeight() + 5));
    runsFloatList->setAnchorPoint({.5f, 0});

    std::vector<FloatingListItem> runItems{};
    if (DTLayer::get()->m_MyLevelStats.isOk()){
        auto& levelStats = DTLayer::get()->m_MyLevelStats.unwrap();
        for (const auto& run : levelStats.metadata.runsToShow)
        {
            runItems.push_back(FloatingListItem{
                .id = run.first,
                .text = fmt::format("{}%", run.first)
            });
        }
    }

    runsFloatList->addItems(runItems);
    runsFloatList->setCallback([&](const int& id){
        runInput->setString(std::to_string(id), true);

        runsFloatList->close();
    });
    runContainer->addChild(runsFloatList);

    extrasContainer->updateLayout();

    checkForOptVisibilityChange();

    return true;
}

void GraphCell::onArrowUp(CCObject*){
    if (onArrowCallback != NULL) onArrowCallback(this, true);
}
void GraphCell::onArrowDown(CCObject*){
    if (onArrowCallback != NULL) onArrowCallback(this, false);
}
void GraphCell::onOptions(CCObject*){
    if (onOptionsCallback != NULL) onOptionsCallback(this);
}

void GraphCell::setOrderPos(int pos){
    graphInfo.orderPos = pos;
    this->setZOrder(pos);
    onInfoChanged(true);
}

bool GraphCell::setName(const std::string& name){
    if (!canChangeNameTo(name, this)) return false;

    oldName = graphInfo.name;

    graphInfo.name = name;
    label->setText(name);
    onInfoChanged(true);

    return true;
}
void GraphCell::setCoverage(DTGraphCoverage coverage){
    graphInfo.coverage = coverage;
    checkForOptVisibilityChange();
    onInfoChanged(true);
}
void GraphCell::setType(DTGraphType type){
    graphInfo.type = type;
    typeSwitcher->setValue(type, false);
    onInfoChanged(true);
}

void GraphCell::setThickness(float thickness){
    graphInfo.thickness = thickness;
    onInfoChanged(true);
}
void GraphCell::setOutlineThickness(float outlineThickness){
    graphInfo.outlineThickness = outlineThickness;
    onInfoChanged(true);
}
void GraphCell::setPointSize(float pointSize){
    graphInfo.pointScale = pointSize;
    onInfoChanged(true);
}
void GraphCell::setColor(ccColor4B color){
    graphInfo.color = color;
    innerColor->setColor({graphInfo.color.r, graphInfo.color.g, graphInfo.color.b});
    innerColor->setOpacity(graphInfo.color.a);
    onInfoChanged(true);
}
void GraphCell::setOutlineColor(ccColor4B color){
    graphInfo.outlineColor = color;
    outerColor->setColor({graphInfo.outlineColor.r, graphInfo.outlineColor.g, graphInfo.outlineColor.b});
    outerColor->setOpacity(graphInfo.outlineColor.a);
    onInfoChanged(true);
}
void GraphCell::setPointColor(ccColor4B color){
    graphInfo.pointColor = color;
    onInfoChanged(true);
}

void GraphCell::onInfoChanged(bool updateGraph){
    if (updateGraph && onInfoChangedCallback != NULL)
        onInfoChangedCallback(this);
}

void GraphCell::setEnabledInfo(bool b, bool changeToggler, bool callback){
    this->graphInfo.isEnabled = b;
    if (changeToggler)
        enableToggleBtn->toggle(b);
    if (callback)
        onEnabledChanged(this);

    onInfoChanged(true);
}

void GraphCell::deleteMe(){
    beforeDeletion(graphInfo, this);
    this->removeMeAndCleanup();
    
    onDeleted(graphInfo);
}

void GraphCell::setSessionOptionsVisible(bool visible){
    int sizeScaler = 0;

    if (visible && !sessionContainer->isVisible()){
        sessionContainer->setVisible(true);

        sizeScaler = 1;
    }
    else if (!visible && sessionContainer->isVisible()){
        sessionContainer->setVisible(false);

        sizeScaler = -1;
    }

    extrasContainer->updateLayout();

    if (sizeScaler != 0)
        this->setContentHeight(
            this->getContentHeight() +
            sessionContainer->getScaledContentHeight() * sizeScaler
        );

    everythingParent->setPositionY(this->getContentHeight() - everythingParent->getScaledContentHeight());
    bg->setContentSize((this->getContentSize() + ccp(0, 2.5f)) / bg->getScale());
}
void GraphCell::setRunOptionsVisible(bool visible){
    int sizeScaler = 0;

    if (visible && !runContainer->isVisible()){
        runContainer->setVisible(true);

        sizeScaler = 1;

    }
    else if (!visible && runContainer->isVisible()){
        runContainer->setVisible(false);
        
        sizeScaler = -1;
    }

    extrasContainer->updateLayout();

    if (sizeScaler != 0)
        this->setContentHeight(
            this->getContentHeight() +
            runContainer->getScaledContentHeight() * sizeScaler
        );

    everythingParent->setPositionY(this->getContentHeight() - everythingParent->getScaledContentHeight());
    bg->setContentSize((this->getContentSize() + ccp(0, 2.5f)) / bg->getScale());
}

void GraphCell::checkForOptVisibilityChange(){
    bool makeSessionVisible = true;
    bool makeRunVisible = true;

    if (graphInfo.coverage == DTGraphCoverage::GeneralCover){
        makeSessionVisible = false;
        makeRunVisible = false;
    }
    else if (graphInfo.coverage == DTGraphCoverage::GeneralRunsCover){
        makeSessionVisible = false;
    }
    else if (graphInfo.coverage == DTGraphCoverage::SessionCover){
        makeRunVisible = false;
    }

    setSessionOptionsVisible(makeSessionVisible);
    setRunOptionsVisible(makeRunVisible);
}

void GraphCell::sendNewSession(int index){
    if (!(graphInfo.coverage == DTGraphCoverage::SessionCover || graphInfo.coverage == DTGraphCoverage::SessionRunsCover)) return;

    auto session = DTLayer::get()->loadSessionFromSave(index);
    std::optional<Session> sessionOpt = session.isOk() ? std::make_optional(session.unwrap()) : std::nullopt;
    
    if (onNewSession != NULL)
        onNewSession(this, std::move(sessionOpt));
}

void GraphCell::resendSession(){
    sendNewSession(sessionSelector->getCurrentCount());
}

void GraphCell::setEnabled(bool value){
    //CCMenu::setEnabled(value);

    runInput->setEnabled(value);
    sessionSelector->setEnabled(value);
}

void GraphCell::onRunSelectHelper(CCObject*){
    if (runsFloatList->isOpened()){
        runsFloatList->close();
    }
    else{
        runsFloatList->open();
    }
}