#include "GraphCell.hpp"

#include <managers/StatsManager.hpp>
#include <nodes/DTLayer.hpp>

#include <nodes/general/SimpleToggler.hpp>
#include <nodes/general/OptionSwitcher.hpp>
#include <nodes/general/SessionSelector.hpp>
#include <nodes/general/FloatingList.hpp>

GraphCell* GraphCell::create(float width, const DTGraphInfo& graphInfo) {
    auto ret = new GraphCell();
    
    if (ret && ret->init(width, graphInfo)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool GraphCell::init(float width, const DTGraphInfo& graphInfo) {
    if (!CCNode::init()) return false;

    setID("graph-cell");
    this->setContentSize({width, 45});

    this->m_graphInfo = graphInfo;

    m_everythingParent = CCNode::create();
    m_everythingParent->setAnchorPoint({0, 0});
    m_everythingParent->setPositionX(0);
    m_everythingParent->ignoreAnchorPointForPosition(false);
    this->addChild(m_everythingParent);

    m_bg = CCScale9Sprite::create("square01_001.png");
    m_bg->setAnchorPoint({0, 1});
    m_bg->setScale(.2f);
    m_bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / m_bg->getScale());
    m_bg->setPositionY(m_bg->getScaledContentHeight());
    m_everythingParent->addChild(m_bg);

    m_everythingParent->setContentSize(m_bg->getScaledContentSize());

    m_mainMenu = CCMenu::create();
    m_mainMenu->setPosition({0, 0});
    m_mainMenu->setZOrder(1);
    m_everythingParent->addChild(m_mainMenu);

    m_label = SimpleTextArea::create(graphInfo.m_name, "bigFont.fnt");
    m_label->setScale(.25f);
    m_label->setAnchorPoint({0, 1});
    m_label->setPosition(ccp(0 + 3, this->getContentHeight() - 5));
    m_label->setWidth(width - 10);
    m_label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_everythingParent->addChild(m_label);

    m_outerColor = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    m_outerColor->setColor({graphInfo.m_outlineColor.r, graphInfo.m_outlineColor.g, graphInfo.m_outlineColor.b});
    m_outerColor->setOpacity(graphInfo.m_outlineColor.a);

    m_innerColor = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    m_innerColor->setScale(.75f);
    m_innerColor->setPosition(m_outerColor->getContentSize() / 2);
    m_innerColor->setColor({graphInfo.m_color.r, graphInfo.m_color.g, graphInfo.m_color.b});
    m_innerColor->setOpacity(graphInfo.m_color.a);
    m_outerColor->addChild(m_innerColor);

    auto disabledCircle = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    disabledCircle->setColor({ 84, 43, 43 });

    auto disabledCircleX = CCSprite::createWithSpriteFrameName("edit_delBtnSmall_001.png");
    disabledCircleX->setPosition(disabledCircle->getContentSize() / 2);
    disabledCircleX->setScale(1.65f);
    disabledCircle->addChild(disabledCircleX);

    m_enableToggleBtn = SimpleToggler::create(
        disabledCircle,
        m_outerColor,
        .35f,
        graphInfo.m_isEnabled
    );
    m_enableToggleBtn->setPosition(m_label->getPosition() + ccp(
        m_enableToggleBtn->getContentWidth() / 2 + 1,
        -m_label->getScaledContentHeight() - m_enableToggleBtn->getContentHeight() / 2 - 2
    ));
    m_enableToggleBtn->setZOrder(1);
    m_enableToggleBtn->setID("toggle-btn");
    m_enableToggleBtn->setCallback([this](auto state) {
        setEnabledInfo(state, false, true);
    });
    m_mainMenu->addChild(m_enableToggleBtn);

    auto bgCircle = CCSprite::createWithSpriteFrameName("menuCircleWhite.png");
    bgCircle->setColor({0,0,0});
    bgCircle->setPosition(m_enableToggleBtn->getPosition());
    bgCircle->setScale(.4f);
    m_everythingParent->addChild(bgCircle);

    this->setZOrder(graphInfo.m_orderPos);

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
    m_mainMenu->addChild(arrowDown);

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
    m_mainMenu->addChild(arrowUp);

    auto settingBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingBtnSpr->setScale(.25f);
    auto settingBtn = CCMenuItemSpriteExtra::create(
        settingBtnSpr,
        this,
        menu_selector(GraphCell::onOptions)
    );
    settingBtn->setID("settings-btn");
    settingBtn->setScale(12 / settingBtn->getContentHeight());
    settingBtn->setPosition({m_enableToggleBtn->getPositionX(), m_enableToggleBtn->getPositionY() - m_enableToggleBtn->getContentHeight() / 2 - settingBtn->getContentHeight() / 2});
    m_mainMenu->addChild(settingBtn);

    m_typeSwitcher = OptionSwitcher<DTGraphType>::create(90, {
        {DTGraphType::Passrate, "Passrate"},
        {DTGraphType::Reachrate, "Reachrate"}
    });
    m_typeSwitcher->setID("type-switcher");
    m_typeSwitcher->setScale(.4f);
    m_typeSwitcher->setAnchorPoint({.5f, 0});
    m_typeSwitcher->setPositionX(width / 2 + 2);
    m_typeSwitcher->setPositionY(6.5f);
    m_typeSwitcher->setCallback([this](auto value) {
        setType(value);
    });
    m_typeSwitcher->setValue(graphInfo.m_type, false);
    m_mainMenu->addChild(m_typeSwitcher);
    
    auto typeSwitcherLabel = geode::Label::create("Type:", "bigFont.fnt");
    typeSwitcherLabel->setScale(.25f);
    typeSwitcherLabel->setPosition(m_typeSwitcher->getPosition() + ccp(
        0,
        m_typeSwitcher->getScaledContentHeight() + typeSwitcherLabel->getScaledContentHeight() / 2
    ));
    typeSwitcherLabel->setID("type-label");
    m_everythingParent->addChild(typeSwitcherLabel);

    m_extrasContainer = CCNode::create();
    m_extrasContainer->setContentSize({width / 1.2f, 80});
    m_extrasContainer->setPosition({0, 0});
    m_extrasContainer->setZOrder(2);
    m_extrasContainer->setLayout(SimpleAxisLayout::create(Axis::Column)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisScaling(AxisScaling::Scale)
    );
    m_extrasContainer->setAnchorPoint({.5f, 1});
    m_extrasContainer->setPositionX(width / 2);
    m_everythingParent->addChild(m_extrasContainer);

    m_sessionContainer = CCNode::create();
    m_sessionContainer->setVisible(false);
    m_extrasContainer->addChild(m_sessionContainer);
    
    m_sessionSelectorLabel = geode::Label::create("Session", "bigFont.fnt");
    m_sessionSelectorLabel->setScale(.4f);
    m_sessionSelectorLabel->setAnchorPoint({.5f, 1});
    m_sessionContainer->addChild(m_sessionSelectorLabel);

    m_sessionSelector = SessionSelector::create(DTLayer::get()->m_sessionSelector->getMaximumCount());
    m_sessionSelector->setEnabled(false);
    m_sessionSelector->setScale(.45f);
    m_sessionSelector->setAnchorPoint({.5f, 0});
    m_sessionSelector->setCallback([&](auto sessionIndex) {
        sendNewSession(sessionIndex);
    });
    m_sessionContainer->addChild(m_sessionSelector);

    m_sessionContainer->setContentSize({width, m_sessionSelector->getScaledContentHeight() + m_sessionSelectorLabel->getScaledContentHeight() + 5});
    m_sessionSelectorLabel->setPosition({
        m_sessionContainer->getContentWidth() / 2,
        m_sessionContainer->getContentHeight()
    });
    m_sessionSelector->setPosition({
        m_sessionContainer->getContentWidth() / 2,
        0
    });

    m_sectionContainer = CCMenu::create();
    m_sectionContainer->setVisible(false);
    m_extrasContainer->addChild(m_sectionContainer);

    m_sectionSelectorLabel = geode::Label::create("Section:", "bigFont.fnt");
    m_sectionSelectorLabel->setScale(.4f);
    m_sectionSelectorLabel->setAnchorPoint({.5f, 1});
    m_sectionContainer->addChild(m_sectionSelectorLabel);

    if (DTLayer::get() != nullptr && DTLayer::get()->m_MyLevelStats.isOk()) {
        m_sections = DTLayer::get()->m_MyLevelStats.unwrap().m_metadata.m_sections;
    }
    std::erase_if(m_sections, [](const Section& section) {
        return !section.isValid() || section.m_startPercent == section.m_endPercent;
    });
    std::sort(m_sections.begin(), m_sections.end(), [](const Section& a, const Section& b) {
        if (a.m_startPercent != b.m_startPercent) return a.m_startPercent < b.m_startPercent;
        return a.m_endPercent < b.m_endPercent;
    });

    std::vector<FloatingListItem> sectionItems;
    for (size_t i = 0; i < m_sections.size(); ++i) {
        sectionItems.push_back(FloatingListItem{
            .m_id = static_cast<int>(i),
            .m_text = m_sections[i].m_name
        });
    }

    auto sectionButtonSpr = ButtonSprite::create(
        m_sections.empty() ? "No Sections" : m_sections.front().m_name.c_str()
    );
    sectionButtonSpr->setScale(std::min(.45f, (width - 10) / sectionButtonSpr->getContentWidth()));
    m_sectionSelectorBtn = CCMenuItemSpriteExtra::create(
        sectionButtonSpr,
        this,
        menu_selector(GraphCell::onSectionSelectHelper)
    );
    m_sectionSelectorBtn->setEnabled(!m_sections.empty());
    m_sectionContainer->addChild(m_sectionSelectorBtn);

    m_sectionsFloatList = FloatingList::create({width, 100});
    m_sectionsFloatList->setAnchorPoint({.5f, 0});
    m_sectionsFloatList->addItems(sectionItems);
    m_sectionsFloatList->setCallback([&](const int& id) {
        if (id < 0 || id >= static_cast<int>(m_sections.size())) return;
        m_selectedSectionIndex = static_cast<size_t>(id);
        sendNewSection();
        m_sectionsFloatList->close();
    });
    m_sectionContainer->addChild(m_sectionsFloatList);

    m_sectionContainer->setContentSize({width, m_sectionSelectorBtn->getScaledContentHeight() + m_sectionSelectorLabel->getScaledContentHeight() + 5});
    m_sectionSelectorLabel->setPosition({
        m_sectionContainer->getContentWidth() / 2,
        m_sectionContainer->getContentHeight()
    });
    m_sectionSelectorBtn->setPosition({
        m_sectionContainer->getContentWidth() / 2,
        m_sectionSelectorBtn->getScaledContentHeight() / 2
    });
    m_sectionsFloatList->setPosition(
        m_sectionSelectorBtn->getPosition() +
        ccp(0, m_sectionSelectorBtn->getScaledContentHeight() / 2 + 5)
    );

    updateSectionSelector();

    m_runContainer = CCMenu::create();
    m_runContainer->setVisible(false);
    m_extrasContainer->addChild(m_runContainer);
    
    m_runLabel = geode::Label::create("Run Start %", "bigFont.fnt");
    m_runLabel->setScale(.3f);
    m_runLabel->setAnchorPoint({.5f, 1});
    m_runContainer->addChild(m_runLabel);

    m_runInput = TextInput::create(width / 1.1f, "run%");
    m_runInput->setEnabled(false);
    m_runInput->setScale(.7f);
    m_runInput->setAnchorPoint({.5f, 0});
    m_runInput->setCommonFilter(CommonFilter::Uint);
    m_runInput->setCallback([&](auto str) {
        auto numRes = utils::numFromString<int>(str);
        int num = 0;
        if (numRes.isOk())
            num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            m_runInput->setString("100");
        }
        
        if (m_onNewRun != NULL)
            m_onNewRun(this, num);
    });
    m_runContainer->addChild(m_runInput);

    m_runContainer->setContentSize({width, m_runInput->getScaledContentHeight() + m_runLabel->getScaledContentHeight() + 5});
    m_runLabel->setPosition({
        m_runContainer->getContentWidth() / 2,
        m_runContainer->getContentHeight()
    });
    m_runInput->setPosition({
        m_runContainer->getContentWidth() / 2,
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
        true,
        1
    );
    runSelectHelperSpr->setScale(0.4f);
    m_runSelectHelperBtn = CCMenuItemSpriteExtra::create(
        runSelectHelperSpr,
        this,
        menu_selector(GraphCell::onRunSelectHelper)
    );
    m_runSelectHelperBtn->setPosition(m_runInput->getPosition() + ccp(m_runInput->getScaledContentWidth() / 2 + 1, m_runInput->getScaledContentHeight() / 2));
    m_runContainer->addChild(m_runSelectHelperBtn);

    m_runInput->setPositionX(m_runInput->getPositionX() - m_runSelectHelperBtn->getContentWidth() / 2 - 1);

    m_runsFloatList = FloatingList::create({m_runInput->getScaledContentWidth(), 100});
    m_runsFloatList->setPosition(m_runInput->getPosition() + ccp(0, m_runInput->getScaledContentHeight() + 5));
    m_runsFloatList->setAnchorPoint({.5f, 0});

    std::vector<FloatingListItem> runItems{};
    if (DTLayer::get()->m_MyLevelStats.isOk()) {
        auto& levelStats = DTLayer::get()->m_MyLevelStats.unwrap();
        for (const auto& run : levelStats.m_metadata.m_runsToShow)
        {
            runItems.push_back(FloatingListItem{
                .m_id = run.first,
                .m_text = fmt::format("{}%", run.first)
            });
        }
    }

    m_runsFloatList->addItems(runItems);
    m_runsFloatList->setCallback([&](const int& id) {
        m_runInput->setString(std::to_string(id), true);

        m_runsFloatList->close();
    });
    m_runContainer->addChild(m_runsFloatList);

    m_extrasContainer->updateLayout();

    checkForOptVisibilityChange();

    return true;
}

void GraphCell::onArrowUp(CCObject*) {
    if (m_onArrowCallback != NULL) m_onArrowCallback(this, true);
}
void GraphCell::onArrowDown(CCObject*) {
    if (m_onArrowCallback != NULL) m_onArrowCallback(this, false);
}
void GraphCell::onOptions(CCObject*) {
    if (m_onOptionsCallback != NULL) m_onOptionsCallback(this);
}

void GraphCell::setOrderPos(int pos) {
    m_graphInfo.m_orderPos = pos;
    this->setZOrder(pos);
    onInfoChanged(true);
}

bool GraphCell::setName(const std::string& name) {
    if (!m_canChangeNameTo(name, this)) return false;

    m_oldName = m_graphInfo.m_name;

    m_graphInfo.m_name = name;
    m_label->setText(name);
    onInfoChanged(true);

    return true;
}
void GraphCell::setCoverage(DTGraphCoverage coverage) {
    m_graphInfo.m_coverage = coverage;
    checkForOptVisibilityChange();
    onInfoChanged(true);
}
void GraphCell::setType(DTGraphType type) {
    m_graphInfo.m_type = type;
    m_typeSwitcher->setValue(type, false);
    onInfoChanged(true);
}

void GraphCell::setThickness(float thickness) {
    m_graphInfo.m_thickness = thickness;
    onInfoChanged(true);
}
void GraphCell::setOutlineThickness(float outlineThickness) {
    m_graphInfo.m_outlineThickness = outlineThickness;
    onInfoChanged(true);
}
void GraphCell::setPointSize(float pointSize) {
    m_graphInfo.m_pointScale = pointSize;
    onInfoChanged(true);
}
void GraphCell::setColor(ccColor4B color) {
    m_graphInfo.m_color = color;
    m_innerColor->setColor({m_graphInfo.m_color.r, m_graphInfo.m_color.g, m_graphInfo.m_color.b});
    m_innerColor->setOpacity(m_graphInfo.m_color.a);
    onInfoChanged(true);
}
void GraphCell::setOutlineColor(ccColor4B color) {
    m_graphInfo.m_outlineColor = color;
    m_outerColor->setColor({m_graphInfo.m_outlineColor.r, m_graphInfo.m_outlineColor.g, m_graphInfo.m_outlineColor.b});
    m_outerColor->setOpacity(m_graphInfo.m_outlineColor.a);
    onInfoChanged(true);
}
void GraphCell::setPointColor(ccColor4B color) {
    m_graphInfo.m_pointColor = color;
    onInfoChanged(true);
}

void GraphCell::onInfoChanged(bool updateGraph) {
    if (updateGraph && m_onInfoChangedCallback != NULL)
        m_onInfoChangedCallback(this);
}

void GraphCell::setEnabledInfo(bool b, bool changeToggler, bool callback) {
    this->m_graphInfo.m_isEnabled = b;
    if (changeToggler)
        m_enableToggleBtn->toggle(b);
    if (callback)
        m_onEnabledChanged(this);

    onInfoChanged(true);
}

void GraphCell::deleteMe() {
    m_beforeDeletion(m_graphInfo, this);
    this->removeMeAndCleanup();
    
    m_onDeleted(m_graphInfo);
}

void GraphCell::setSessionOptionsVisible(bool visible) {
    int sizeScaler = 0;

    if (visible && !m_sessionContainer->isVisible()) {
        m_sessionContainer->setVisible(true);

        sizeScaler = 1;
    }
    else if (!visible && m_sessionContainer->isVisible()) {
        m_sessionContainer->setVisible(false);

        sizeScaler = -1;
    }

    m_extrasContainer->updateLayout();

    if (sizeScaler != 0)
        this->setContentHeight(
            this->getContentHeight() +
            m_sessionContainer->getScaledContentHeight() * sizeScaler
        );

    m_everythingParent->setPositionY(this->getContentHeight() - m_everythingParent->getScaledContentHeight());
    m_bg->setContentSize((this->getContentSize() + ccp(0, 2.5f)) / m_bg->getScale());
}
void GraphCell::setRunOptionsVisible(bool visible) {
    int sizeScaler = 0;

    if (visible && !m_runContainer->isVisible()) {
        m_runContainer->setVisible(true);

        sizeScaler = 1;

    }
    else if (!visible && m_runContainer->isVisible()) {
        m_runContainer->setVisible(false);
        
        sizeScaler = -1;
    }

    m_extrasContainer->updateLayout();

    if (sizeScaler != 0)
        this->setContentHeight(
            this->getContentHeight() +
            m_runContainer->getScaledContentHeight() * sizeScaler
        );

    m_everythingParent->setPositionY(this->getContentHeight() - m_everythingParent->getScaledContentHeight());
    m_bg->setContentSize((this->getContentSize() + ccp(0, 2.5f)) / m_bg->getScale());
}

void GraphCell::setSectionOptionsVisible(bool visible) {
    int sizeScaler = 0;

    if (visible && !m_sectionContainer->isVisible()) {
        m_sectionContainer->setVisible(true);
        sizeScaler = 1;
    }
    else if (!visible && m_sectionContainer->isVisible()) {
        m_sectionContainer->setVisible(false);
        sizeScaler = -1;
    }

    m_extrasContainer->updateLayout();

    if (sizeScaler != 0)
        this->setContentHeight(
            this->getContentHeight() +
            m_sectionContainer->getScaledContentHeight() * sizeScaler
        );

    m_everythingParent->setPositionY(this->getContentHeight() - m_everythingParent->getScaledContentHeight());
    m_bg->setContentSize((this->getContentSize() + ccp(0, 2.5f)) / m_bg->getScale());
}

void GraphCell::checkForOptVisibilityChange() {
    bool makeSessionVisible = true;
    bool makeRunVisible = true;
    bool makeSectionVisible = false;

    if (m_graphInfo.m_coverage == DTGraphCoverage::GeneralCover) {
        makeSessionVisible = false;
        makeRunVisible = false;
    }
    else if (m_graphInfo.m_coverage == DTGraphCoverage::GeneralRunsCover) {
        makeSessionVisible = false;
    }
    else if (m_graphInfo.m_coverage == DTGraphCoverage::SessionCover) {
        makeRunVisible = false;
    }
    else if (m_graphInfo.m_coverage == DTGraphCoverage::SectionContentCover) {
        makeSessionVisible = false;
        makeRunVisible = false;
        makeSectionVisible = !m_sections.empty();
    }
    else if (m_graphInfo.m_coverage == DTGraphCoverage::SectionGeneralCover) {
        makeSessionVisible = false;
        makeRunVisible = false;
    }
    else if (m_graphInfo.m_coverage == DTGraphCoverage::SectionSessionCover) {
        makeRunVisible = false;
    }
    setSessionOptionsVisible(makeSessionVisible);
    setRunOptionsVisible(makeRunVisible);
    setSectionOptionsVisible(makeSectionVisible);
}

void GraphCell::sendNewSession(int index) {
    if (!(m_graphInfo.m_coverage == DTGraphCoverage::SessionCover ||
        m_graphInfo.m_coverage == DTGraphCoverage::SessionRunsCover ||
        m_graphInfo.m_coverage == DTGraphCoverage::SectionSessionCover)) return;

    auto session = DTLayer::get()->loadSessionFromSave(index);
    std::optional<Session> sessionOpt = session.isOk() ? std::make_optional(session.unwrap()) : std::nullopt;
    
    if (m_onNewSession != NULL)
        m_onNewSession(this, std::move(sessionOpt));
}

void GraphCell::resendSession() {
    sendNewSession(m_sessionSelector->getCurrentCount());
}

void GraphCell::sendNewSection() {
    if (m_graphInfo.m_coverage != DTGraphCoverage::SectionContentCover || m_sections.empty() || !m_onNewSection) return;

    if (m_selectedSectionIndex >= m_sections.size()) return;

    updateSectionSelector();
    m_onNewSection(this, m_sections[m_selectedSectionIndex]);
}

void GraphCell::updateSectionSelector() {
    if (m_sections.empty() || m_selectedSectionIndex >= m_sections.size()) return;

    auto buttonSpr = ButtonSprite::create(m_sections[m_selectedSectionIndex].m_name.c_str());
    buttonSpr->setScale(std::min(.45f, (m_sectionContainer->getContentWidth() - 10) / buttonSpr->getContentWidth()));
    m_sectionSelectorBtn->setSprite(buttonSpr);
}

void GraphCell::resendSection() {
    sendNewSection();
}

void GraphCell::setEnabled(bool value) {
    m_runInput->setEnabled(value);
    m_sessionSelector->setEnabled(value);
    m_sectionSelectorBtn->setEnabled(value && !m_sections.empty());
}

void GraphCell::onRunSelectHelper(CCObject*) {
    if (m_runsFloatList->isOpened()) {
        m_runsFloatList->close();
    }
    else {
        m_runsFloatList->open();
    }
}

void GraphCell::onSectionSelectHelper(CCObject*) {
    if (m_sectionsFloatList->isOpened()) {
        m_sectionsFloatList->close();
    }
    else {
        m_sectionsFloatList->open();
    }
}

void GraphCell::setOnArrowCallback(Function<void(GraphCell* cell, bool up)> callback) {
    m_onArrowCallback = std::move(callback);
}

void GraphCell::setOnOptionsCallback(Function<void(GraphCell* cell)> callback) {
    m_onOptionsCallback = std::move(callback);
}

void GraphCell::setOnEnabledChanged(Function<void(GraphCell* cell)> callback) {
    m_onEnabledChanged = std::move(callback);
}

void GraphCell::setOnInfoChangedCallback(Function<void(GraphCell* cell)> callback) {
    m_onInfoChangedCallback = std::move(callback);
}

void GraphCell::setCanChangeNameTo(Function<bool(const std::string&, GraphCell*)> callback) {
    m_canChangeNameTo = std::move(callback);
}

void GraphCell::setBeforeDeletion(Function<void(DTGraphInfo, GraphCell*)> callback) {
    m_beforeDeletion = std::move(callback);
}

void GraphCell::setOnDeleted(Function<void(DTGraphInfo)> callback) {
    m_onDeleted = std::move(callback);
}

void GraphCell::setOnNewSession(Function<void(GraphCell* cell, std::optional<Session>)> callback) {
    m_onNewSession = std::move(callback);
}

void GraphCell::setOnNewRun(Function<void(GraphCell* cell, int)> callback) {
    m_onNewRun = std::move(callback);
}

void GraphCell::setOnNewSection(Function<void(GraphCell* cell, Section)> callback) {
    m_onNewSection = std::move(callback);
}