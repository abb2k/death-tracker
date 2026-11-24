#include "DTGraphLayer.hpp"

#include <utils/Save.hpp>
#include <nodes/Tutorial/TutorialButton.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>

#if defined(GEODE_IS_MOBILE)
#include <nodes/general/SwitcherButton.hpp>
#endif

#include <nodes/graphs/DTGraphNode.hpp>
#include <nodes/general/OptionSwitcher.hpp>
#include <nodes/graphs/GraphPointDisplay.hpp>
#include <nodes/graphs/GraphCell.hpp>
#include <nodes/graphs/GraphHolder.hpp>
#include <nodes/general/AdvancedScrollLayer.hpp>
#include <nodes/general/SessionSelector.hpp>

bool GraphPointDisplayPtrCompare::operator()(GraphPointDisplay const* a, GraphPointDisplay const* b) const {
    if (a == b) return false;
    if (!a) return true;
    if (!b) return false;
    if (a->getPositionReal() > b->getPositionReal()) return true;
    if (a->getPositionReal() < b->getPositionReal()) return false;
    return a < b;
}

DTGraphLayer* DTGraphLayer::create() {
    auto ret = new DTGraphLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DTGraphLayer::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!Popup::init(winSize.width - 120, winSize.height - 30, "square01_001.png", {0.f, 0.f, 94.f, 94.f}))
        return false;

    m_noGraphLabel = geode::Label::create("No Graphs\nEnabled!", "bigFont.fnt");
    m_noGraphLabel->setZOrder(1);
    m_noGraphLabel->setVisible(false);
    m_noGraphLabel->setPosition({46, 3});
    m_mainLayer->addChild(m_noGraphLabel);

    m_graph = GraphHolder::create(ccp(m_size.width - 140, m_size.height - 50));
    m_graph->setPosition({m_size.width - m_graph->getContentWidth() / 2 - 20, m_size.height / 2 + 5});
    m_graph->m_delegate = this;
    this->m_mainLayer->addChild(m_graph);

    auto graphTutorial = TutorialButton::create(.6f, "grapg-overall", [&](DTTutorialLayer* tl) {
        tl->appendDialogue("Welcome to the graph view!", TutorialCharacterFace::TCFNormal)
            ->appendDialogue("This is the main graphs view", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_graph->m_scrollLayer)
            ->joinTransform(TutorialBoxPlacement::TBPBottomLeft, .5f)
#if !defined(GEODE_IS_MOBILE)
            ->appendDialogue("You can hold <cg>control</c> and <cp>scroll</c> to zoom in! and <cc>shift</c> and <cp>scroll</c> to move side to side.", TutorialCharacterFace::TCFNormal)
#else
            ->appendDialogue("You can <cg>pinch</c> to zoom in! and <cc>swipe</c> to move side to side.", TutorialCharacterFace::TCFNormal)
#endif
            ->joinHighlight(m_graph->m_scrollLayer)
            ->appendDialogue("You can also hover on any of the points in the graph to see their details!", TutorialCharacterFace::TCFNormal)
            ->joinCallback([&, tl]() {
                for (const auto& graphNode : m_graph->getAllGraphNodes())
                {
                    if (!graphNode->isVisible()) continue;

                    for (const auto& child : CCArrayExt<CCNode*>(graphNode->m_pointHolder->getChildren())) {
                        tl->insertHighlight(3, child)
                            ->insertJoinTextToHighlight(3, "V", .25f);
                    }
                }
            }, true)
            ->appendDialogue("Feel free to check out how to customize graphs on the left menu!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter);
    });
    graphTutorial->setPosition(m_graph->getPosition() + m_graph->getContentSize() / 2 + ccp(10, 10));
    this->m_buttonMenu->addChild(graphTutorial);

    m_graphsPage = CCNode::create();
    m_graphsPage->setPositionX(55);
    m_graphsPage->setPositionY(m_size.height - 20);
    m_graphsPage->ignoreAnchorPointForPosition(false);
    m_graphsPage->setContentSize({0,0});
    m_mainLayer->addChild(m_graphsPage);

    m_graphOptionsPage = CCMenu::create();
    m_graphOptionsPage->setPositionX(55);
    m_graphOptionsPage->setPositionY(m_size.height - 20);
    m_graphOptionsPage->ignoreAnchorPointForPosition(false);
    m_graphOptionsPage->setContentSize({0,0});
    m_graphOptionsPage->setScaleY(0);
    m_mainLayer->addChild(m_graphOptionsPage);

    m_graphsScroll = ScrollLayer::create({70, m_size.height - 40});
    m_graphsScroll->setPosition({-35, -(m_size.height - 40)});
    m_graphsScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(m_graphsScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    m_graphsScroll->ignoreAnchorPointForPosition(true);
    m_graphsPage->addChild(m_graphsScroll);

    m_graphsScrollbar = Scrollbar::create(m_graphsScroll);
    m_graphsScrollbar->setPosition(m_graphsScroll->getPosition() + ccp(
        -4,
        m_graphsScroll->getScaledContentHeight() / 2
    ));
    m_graphsScrollbar->ignoreAnchorPointForPosition(false);
    m_graphsPage->addChild(m_graphsScrollbar);

    auto scrollBG = CCScale9Sprite::create("square02_001.png");
    scrollBG->setContentSize(m_graphsScroll->getContentSize() + ccp(5, 5));
    scrollBG->setAnchorPoint({0.5f, 0.5f});
    scrollBG->setPosition(m_graphsScroll->getPosition() + m_graphsScroll->getContentSize() / 2);
    scrollBG->setOpacity(100);
    scrollBG->setZOrder(-1);
    m_graphsPage->addChild(scrollBG);

    auto keysScrollFixer = CCNode::create();
    keysScrollFixer->setContentHeight(3.5f);
    keysScrollFixer->setZOrder(100000);
    m_graphsScroll->m_contentLayer->addChild(keysScrollFixer);

    auto addGraphMenu = CCMenu::create();
    addGraphMenu->setPosition({0, 0});
    m_graphsPage->addChild(addGraphMenu);

    auto addGraphBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    auto addGraphBtn = CCMenuItemSpriteExtra::create(
        addGraphBtnSpr,
        this,
        menu_selector(DTGraphLayer::onAddGraph)
    );
    addGraphBtn->setPositionX(m_graphsScroll->getContentWidth() / 2 - 7.5f);
    addGraphMenu->addChild(addGraphBtn);

#if defined(GEODE_IS_MOBILE)
    auto shiftSwitcher = SwitcherButton::create({scrollBG->getContentWidth(), 20}, "GJ_button_02.png", {"One Graph", "Multiple Graphs"});
    shiftSwitcher->setOptionChangedCallback([&](auto opt) {
        m_holdingShift = opt == 1;
    });
    shiftSwitcher->setPosition({
        0,
        -m_size.height + shiftSwitcher->getContentHeight() + 10
    });
    addGraphMenu->addChild(shiftSwitcher);
#endif

    auto graphsInfo = Save::getGraphs();

    for (const auto& currGraph : graphsInfo)
    {
        addGraph(currGraph);
    }

    auto sideTutorial = TutorialButton::create(.6f, "graphs-side", [
        &
#if defined(GEODE_IS_MOBILE)
        ,shiftSwitcher
#endif
    ](DTTutorialLayer* tl) {
        tl->appendDialogue("This is where you can manage your graphs!", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("These are the graphs you have available", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)
            ->joinHighlight(m_graphsScroll)
            ->appendDialogue("You can <cy>show/hide</c> graphs by clicking the <cp>circle</c> colored button right there", TutorialCharacterFace::TCFNormalTilted);
        CCNode* firstBtn = nullptr;
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!cell) continue;
            if (firstBtn == nullptr)
                firstBtn = cell->m_mainMenu->getChildByID("toggle-btn");
            tl->joinHighlight(cell->m_mainMenu->getChildByID("toggle-btn"));
        }
        tl->appendDialogue("If you toggle a graph <cy>ON</c>, all others will be <cr>disabled</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(firstBtn)
            ->joinTextToHighlight(">", .5f, TutorialTextPlacement::TTLeft);
#if !defined(GEODE_IS_MOBILE)
        tl->appendDialogue("Unless you hold <cy>SHIFT</c> while toggling to enable <cy>multiple graphs!</c>", TutorialCharacterFace::TCFHappy);
        int index = 0;
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!cell) continue;
            tl->joinHighlight(cell->m_mainMenu->getChildByID("toggle-btn"), index * .3f);
            tl->joinTextToHighlight(std::to_string(index + 1), .5f, TutorialTextPlacement::TTLeft);
            index++;
        }
#else
        tl->appendDialogue("Unless you hold click the <cf>bottom button</c> to switch to <cy>multiple graphs mode!</c>", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(shiftSwitcher);
#endif
        tl->appendDialogue("You can edit each graphs <cc>settings</c>", TutorialCharacterFace::TCFNormal);
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!cell) continue;
            tl->joinHighlight(cell->m_mainMenu->getChildByID("settings-btn"));
        }
        tl->appendDialogue("And you can also change their <cg>layering order</c> using the <cf>arrows</c> on the right!", TutorialCharacterFace::TCFHappy);
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!cell) continue;
            tl->joinHighlight(cell->m_mainMenu->getChildByID("up-arrow"));
            tl->joinHighlight(cell->m_mainMenu->getChildByID("down-arrow"));
        }
        tl->appendDialogue("You also have quick access to change the <cy>graphs type</c>, more on that in the graph settings page!", TutorialCharacterFace::TCFNormalTilted);
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!cell) continue;
            tl->joinHighlight(cell->m_mainMenu->getChildByID("type-switcher"));
            tl->joinHighlight(cell->m_mainMenu->getChildByID("type-label"));
        }
        tl->appendDialogue("You can choose which <co>session</c> the session graphs show using their <co>session switcher</c>", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottomRight, .75f);
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!cell) continue;
            tl->joinHighlight(cell->m_sessionSelectorLabel);
            tl->joinHighlight(cell->m_sessionSelector);
        }
        tl->appendDialogue("And choose which <cf>run</c> the run graphs would display using the <cf>run input!</c>", TutorialCharacterFace::TCFHappy);
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!cell) continue;
            tl->joinHighlight(cell->m_runLabel);
            tl->joinHighlight(cell->m_runInput);
            tl->joinHighlight(cell->m_runSelectHelperBtn);
        }
        tl->appendDialogue("The <cf>percent</c> you input would be the <cy>starting percent</c> of the run you wanna see!", TutorialCharacterFace::TCFNormal)
            ->joinPreviousHighlight()
            ->appendDialogue("Thats about it! Enjoy using graphs!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    sideTutorial->setPositionX(-m_graphsScroll->getContentWidth() / 2 + 2.5f);
    addGraphMenu->addChild(sideTutorial);


    auto nameLabel = geode::Label::create("Name", "bigFont.fnt");
    nameLabel->setScale(.3f);
    m_graphOptionsPage->addChild(nameLabel);

    m_nameInput = TextInput::create(90, "Name", "bigFont.fnt");
    m_nameInput->setAnchorPoint({.5f, 1});
    m_nameInput->setScale(.75f);
    m_nameInput->setPositionY(-nameLabel->getScaledContentHeight() / 2);
    m_nameInput->setCallback([&](const std::string& newStr) {
        if (!m_editedGraph.has_value()) return;

        m_editedGraph.value()->setName(newStr);
    });
    m_graphOptionsPage->addChild(m_nameInput);

    auto coverageLabel = geode::Label::create("Coverage", "bigFont.fnt");
    coverageLabel->setScale(.3f);
    coverageLabel->setAnchorPoint({.5f, 1});
    coverageLabel->setPositionY(m_nameInput->getPositionY() - m_nameInput->getScaledContentHeight() - 5);
    m_graphOptionsPage->addChild(coverageLabel);

    m_coverageSwitcher = OptionSwitcher<DTGraphCoverage>::create(90, {
        {DTGraphCoverage::GeneralCover, "General"},
        {DTGraphCoverage::GeneralRunsCover, "General Runs"},
        {DTGraphCoverage::SessionCover, "Session"},
        {DTGraphCoverage::SessionRunsCover, "Session Runs"},
        {DTGraphCoverage::SectionContentCover, "Section Content"},
        {DTGraphCoverage::SectionGeneralCover, "Section General"},
        {DTGraphCoverage::SectionSessionCover, "Section Session"},
    });
    m_coverageSwitcher->setScale(.75f);
    m_coverageSwitcher->setAnchorPoint({.5f, 1});
    m_coverageSwitcher->setPositionY(coverageLabel->getPositionY() - coverageLabel->getScaledContentHeight());
    m_coverageSwitcher->setCallback([&](auto value) {
        if (!m_editedGraph.has_value()) return;

        m_editedGraph.value()->setCoverage(value);
    });
    m_graphOptionsPage->addChild(m_coverageSwitcher);

    auto typeLabel = geode::Label::create("Type", "bigFont.fnt");
    typeLabel->setScale(.3f);
    typeLabel->setAnchorPoint({.5f, 1});
    typeLabel->setPositionY(m_coverageSwitcher->getPositionY() - m_coverageSwitcher->getScaledContentHeight() - 5);
    m_graphOptionsPage->addChild(typeLabel);

    m_typeSwitcher = OptionSwitcher<DTGraphType>::create(90, {
        {DTGraphType::Passrate, "Passrate"},
        {DTGraphType::Reachrate, "Reachrate"}
    });
    m_typeSwitcher->setScale(.75f);
    m_typeSwitcher->setAnchorPoint({.5f, 1});
    m_typeSwitcher->setPositionY(typeLabel->getPositionY() - typeLabel->getScaledContentHeight());
    m_typeSwitcher->setCallback([&](auto value) {
        if (!m_editedGraph.has_value()) return;

        m_editedGraph.value()->setType(value);
    });
    m_graphOptionsPage->addChild(m_typeSwitcher);

    auto thicknessLabel = geode::Label::create("Thickness", "bigFont.fnt");
    thicknessLabel->setScale(.3f);
    thicknessLabel->setAnchorPoint({.5f, 1});
    thicknessLabel->setPositionY(m_typeSwitcher->getPositionY() - m_typeSwitcher->getScaledContentHeight() - 5);
    m_graphOptionsPage->addChild(thicknessLabel);

    auto thicknessBaseLabel = geode::Label::create("Base", "bigFont.fnt");
    thicknessBaseLabel->setScale(.25f);
    thicknessBaseLabel->setAnchorPoint({1, 1});
    thicknessBaseLabel->setPositionY(thicknessLabel->getPositionY() - thicknessLabel->getScaledContentHeight());
    thicknessBaseLabel->setPositionX(-10);
    m_graphOptionsPage->addChild(thicknessBaseLabel);

    auto thicknessOutlineLabel = geode::Label::create("Outline", "bigFont.fnt");
    thicknessOutlineLabel->setScale(.25f);
    thicknessOutlineLabel->setAnchorPoint({0, 1});
    thicknessOutlineLabel->setPositionY(thicknessLabel->getPositionY() - thicknessLabel->getScaledContentHeight());
    thicknessOutlineLabel->setPositionX(0);
    m_graphOptionsPage->addChild(thicknessOutlineLabel);

    m_thicknessBaseInput = TextInput::create(40, "X", "bigFont.fnt");
    m_thicknessBaseInput->setScale(.75f);
    m_thicknessBaseInput->setAnchorPoint({.5f, 1});
    m_thicknessBaseInput->setPositionY(thicknessBaseLabel->getPositionY() - thicknessBaseLabel->getScaledContentHeight());
    m_thicknessBaseInput->setPositionX(thicknessBaseLabel->getPositionX() - thicknessBaseLabel->getScaledContentWidth() / 2);
    m_thicknessBaseInput->setCommonFilter(CommonFilter::Float);
    m_thicknessBaseInput->setCallback([&](auto value) {
        if (!m_editedGraph.has_value()) return;

        auto numRes = utils::numFromString<float>(value);
        if (numRes.isErr()) return;

        float num = numRes.unwrap();

        if (num < 0) num = 0;

        m_editedGraph.value()->setThickness(num);
    });
    m_graphOptionsPage->addChild(m_thicknessBaseInput);

    m_thicknessOutlineInput = TextInput::create(40, "X", "bigFont.fnt");
    m_thicknessOutlineInput->setScale(.75f);
    m_thicknessOutlineInput->setAnchorPoint({.5f, 1});
    m_thicknessOutlineInput->setPositionY(thicknessOutlineLabel->getPositionY() - thicknessOutlineLabel->getScaledContentHeight());
    m_thicknessOutlineInput->setPositionX(thicknessOutlineLabel->getPositionX() + thicknessOutlineLabel->getScaledContentWidth() / 2);
    m_thicknessOutlineInput->setCommonFilter(CommonFilter::Float);
    m_thicknessOutlineInput->setCallback([&](auto value) {
        if (!m_editedGraph.has_value()) return;

        auto numRes = utils::numFromString<float>(value);
        if (numRes.isErr()) return;

        float num = numRes.unwrap();

        if (num < 0) num = 0;

        m_editedGraph.value()->setOutlineThickness(num);
    });
    m_graphOptionsPage->addChild(m_thicknessOutlineInput);

    auto colorLabel = geode::Label::create("Color", "bigFont.fnt");
    colorLabel->setScale(.3f);
    colorLabel->setAnchorPoint({.5f, 1});
    colorLabel->setPositionY(m_thicknessOutlineInput->getPositionY() - m_thicknessOutlineInput->getScaledContentHeight() - 5);
    m_graphOptionsPage->addChild(colorLabel);

    auto colorBaseLabel = geode::Label::create("Base", "bigFont.fnt");
    colorBaseLabel->setScale(.25f);
    colorBaseLabel->setAnchorPoint({1, 1});
    colorBaseLabel->setPositionY(colorLabel->getPositionY() - colorLabel->getScaledContentHeight());
    colorBaseLabel->setPositionX(-10);
    m_graphOptionsPage->addChild(colorBaseLabel);

    auto colorOutlineLabel = geode::Label::create("Outline", "bigFont.fnt");
    colorOutlineLabel->setScale(.25f);
    colorOutlineLabel->setAnchorPoint({0, 1});
    colorOutlineLabel->setPositionY(colorLabel->getPositionY() - colorLabel->getScaledContentHeight());
    colorOutlineLabel->setPositionX(0);
    m_graphOptionsPage->addChild(colorOutlineLabel);

    m_colorBaseBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    m_colorBaseBtnSpr->setScale(.6f);
    auto colorBaseBtn = CCMenuItemSpriteExtra::create(
        m_colorBaseBtnSpr,
        this,
        menu_selector(DTGraphLayer::onColor)
    );
    colorBaseBtn->setPositionY(colorBaseLabel->getPositionY() - colorBaseLabel->getScaledContentHeight() - colorBaseBtn->getContentHeight() / 2);
    colorBaseBtn->setPositionX(colorBaseLabel->getPositionX() - colorBaseLabel->getScaledContentWidth() / 2);
    colorBaseBtn->setID("line");
    colorBaseBtn->setTag(1);
    m_graphOptionsPage->addChild(colorBaseBtn);

    m_colorOutlineBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    m_colorOutlineBtnSpr->setScale(.6f);
    auto colorOutlineBtn = CCMenuItemSpriteExtra::create(
        m_colorOutlineBtnSpr,
        this,
        menu_selector(DTGraphLayer::onColor)
    );
    colorOutlineBtn->setPositionY(colorOutlineLabel->getPositionY() - colorOutlineLabel->getScaledContentHeight() - colorOutlineBtn->getContentHeight() / 2);
    colorOutlineBtn->setPositionX(colorOutlineLabel->getPositionX() + colorOutlineLabel->getScaledContentWidth() / 2);
    colorOutlineBtn->setID("line");
    colorOutlineBtn->setTag(2);
    m_graphOptionsPage->addChild(colorOutlineBtn);

    auto pointLabel = geode::Label::create("Point", "bigFont.fnt");
    pointLabel->setScale(.3f);
    pointLabel->setAnchorPoint({.5f, 1});
    pointLabel->setPositionY(colorOutlineBtn->getPositionY() - colorOutlineBtn->getScaledContentHeight() / 2 - 5);
    m_graphOptionsPage->addChild(pointLabel);

    auto pointColorLabel = geode::Label::create("Color", "bigFont.fnt");
    pointColorLabel->setScale(.25f);
    pointColorLabel->setAnchorPoint({1, 1});
    pointColorLabel->setPositionY(pointLabel->getPositionY() - pointLabel->getScaledContentHeight());
    pointColorLabel->setPositionX(-10);
    m_graphOptionsPage->addChild(pointColorLabel);

    auto pointScaleLabel = geode::Label::create("Scale", "bigFont.fnt");
    pointScaleLabel->setScale(.25f);
    pointScaleLabel->setAnchorPoint({0, 1});
    pointScaleLabel->setPositionY(pointLabel->getPositionY() - pointLabel->getScaledContentHeight());
    pointScaleLabel->setPositionX(0);
    m_graphOptionsPage->addChild(pointScaleLabel);

    m_pointScaleInput = TextInput::create(40, "X", "bigFont.fnt");
    m_pointScaleInput->setScale(.75f);
    m_pointScaleInput->setAnchorPoint({.5f, 1});
    m_pointScaleInput->setPositionY(pointScaleLabel->getPositionY() - pointScaleLabel->getScaledContentHeight());
    m_pointScaleInput->setPositionX(pointScaleLabel->getPositionX() + pointScaleLabel->getScaledContentWidth() / 2);
    m_pointScaleInput->setCommonFilter(CommonFilter::Float);
    m_pointScaleInput->setCallback([&](auto value) {
        if (!m_editedGraph.has_value()) return;

        auto numRes = utils::numFromString<float>(value);
        if (numRes.isErr()) return;

        float num = numRes.unwrap();

        if (num < 0) num = 0;

        m_editedGraph.value()->setPointSize(num);
    });
    m_graphOptionsPage->addChild(m_pointScaleInput);

    m_pointColorBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    m_pointColorBtnSpr->setScale(.6f);
    auto pointColorBtn = CCMenuItemSpriteExtra::create(
        m_pointColorBtnSpr,
        this,
        menu_selector(DTGraphLayer::onColor)
    );
    pointColorBtn->setPositionY(pointColorLabel->getPositionY() - pointColorLabel->getScaledContentHeight() - pointColorBtn->getContentHeight() / 2);
    pointColorBtn->setPositionX(pointColorLabel->getPositionX() - pointColorLabel->getScaledContentWidth() / 2);
    pointColorBtn->setID("point");
    m_graphOptionsPage->addChild(pointColorBtn);

    auto okBtnSpr = ButtonSprite::create("OK");
    okBtnSpr->setScale(.6f);
    auto okBtn = CCMenuItemSpriteExtra::create(
        okBtnSpr,
        this,
        menu_selector(DTGraphLayer::onOk)
    );
    okBtn->setPositionY(pointColorBtn->getPositionY() - pointColorBtn->getScaledContentHeight() / 2 - okBtn->getContentHeight() / 2 - 5);
    okBtn->setPositionX(pointColorBtn->getPositionX());
    m_graphOptionsPage->addChild(okBtn);

    auto DeleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    DeleteBtnSpr->setScale(.6f);
    auto DeleteBtn = CCMenuItemSpriteExtra::create(
        DeleteBtnSpr,
        this,
        menu_selector(DTGraphLayer::onDelete)
    );
    DeleteBtn->setPositionY(m_pointScaleInput->getPositionY() - m_pointScaleInput->getScaledContentHeight() - DeleteBtn->getContentHeight() / 4 - 10);
    DeleteBtn->setPositionX(m_pointScaleInput->getPositionX());
    m_graphOptionsPage->addChild(DeleteBtn);

    m_graphOptionsPage->setEnabled(false);
    m_nameInput->setEnabled(false);
    m_thicknessBaseInput->setEnabled(false);
    m_thicknessOutlineInput->setEnabled(false);
    m_pointScaleInput->setEnabled(false);
    
    scheduleUpdate();

    return true;
}

void DTGraphLayer::keyDown(enumKeyCodes key, double d) {
    if (key == enumKeyCodes::KEY_LeftShift) {
        m_holdingShift = true;
    }

    m_graph->sendKeyStuff(false, key);
}

void DTGraphLayer::keyUp(enumKeyCodes key, double d) {
    if (key == enumKeyCodes::KEY_LeftShift) {
        m_holdingShift = false;
    }

    m_graph->sendKeyStuff(true, key);
}

void DTGraphLayer::OnPointSelected(GraphPoint* point) {
    if (m_displaysForPoints.contains(point)) return;

    auto display = GraphPointDisplay::create();

    CCPoint position;

    if (!m_displaysForPoints.size()) {
        auto pointPosConverted = m_mainLayer->convertToNodeSpace(point->convertToWorldSpace({point->getContentWidth() / 2, 0}));

        float heightOffset = display->getContentHeight();
#if defined(GEODE_IS_MOBILE)
        heightOffset += 15;
#endif

        position = pointPosConverted - ccp(display->getContentWidth() / 2, heightOffset);

    }
    else {
        GraphPointDisplay* lowestOther = *m_displays.rbegin();
        position = ccp(lowestOther->getPositionX(), lowestOther->getPositionReal()) + ccp(0, -lowestOther->getScaledContentHeight() / 2 - display->getScaledContentHeight() / 2);
    }

    auto innerColor = point->getRelatedGraph()->getInfo().value().m_color;
    auto outerColor = point->getRelatedGraph()->getInfo().value().m_outlineColor;

    display->setPositionReal(position.y);
    display->setPosition(position);
    display->setContent(
        point->getRun(), 
        point->getRate(), 
        point->getRelatedGraph()->getInfo().value().m_type,
        {innerColor.r, innerColor.g, innerColor.b},
        {outerColor.r, outerColor.g, outerColor.b}
    );
    m_mainLayer->addChild(display);

    m_displaysForPoints.insert({point, display});
    m_displays.insert(display);

    auto lowestDisplay = *m_displays.rbegin();

    auto gloablPos = lowestDisplay->getParent()->convertToWorldSpace({lowestDisplay->getPositionX(), lowestDisplay->getPositionReal()});
    if (gloablPos.y < 0) {
        float overallHeight = lowestDisplay->getScaledContentHeight();
        for (const auto& otherDisplay : m_displays) {
            otherDisplay->setPositionY(otherDisplay->getPositionReal() + otherDisplay->getScaledContentHeight() + overallHeight);
            overallHeight += otherDisplay->getScaledContentHeight() * 2;
        }
    }
}

void DTGraphLayer::OnPointDeselected(GraphPoint* point) {
    if (!m_displaysForPoints.contains(point)) return;

    auto display = m_displaysForPoints[point];

    m_displays.erase(display);

    for (const auto& otherDisplay : m_displays)
    {
        if (otherDisplay->getPositionReal() < display->getPositionReal()) {
            otherDisplay->setPositionReal(otherDisplay->getPositionReal() + display->getScaledContentHeight());
        }
    }
    
    display->removeMeAndCleanup();
    m_displaysForPoints.erase(point);

    if (!m_displays.size()) return;

    auto lowestDisplay = *m_displays.rbegin();

    auto gloablPos = lowestDisplay->getParent()->convertToWorldSpace({lowestDisplay->getPositionX(), lowestDisplay->getPositionReal()});
    if (gloablPos.y < 0) {
        float overallHeight = lowestDisplay->getScaledContentHeight();
        for (const auto& otherDisplay : m_displays) {
            otherDisplay->setPositionY(otherDisplay->getPositionReal() + otherDisplay->getScaledContentHeight() + overallHeight);
            overallHeight += otherDisplay->getScaledContentHeight() * 2;
        }
    }
    else {
        for (const auto& displayToSetYOf : m_displays) {
            displayToSetYOf->setPositionY(displayToSetYOf->getPositionReal());
        }
    }
}

void DTGraphLayer::onClose(cocos2d::CCObject* sender) {
    if (m_editedGraph.has_value()) {
        onOk(nullptr);
        return;
    }
    m_displaysForPoints.clear();
    Popup::onClose(sender);
}

void DTGraphLayer::update(float dt) {
    auto mousePos = getMousePos();

    if (m_graphsScroll->boundingBox().containsPoint(m_graphsScroll->getParent()->convertToNodeSpace(mousePos))) {
        m_graphsScroll->setMouseEnabled(true);
    }
    else {
        m_graphsScroll->setMouseEnabled(false);
    }
}

void DTGraphLayer::onAddGraph(CCObject*) {
    addGraph();
}

void DTGraphLayer::addGraph() {
    std::string name = "New-Graph";
    int highestnewGraphNum = 0;

    int highestZ = -1;
    for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer))
    {
        if (!cell) continue;

        if (highestZ < cell->getZOrder()) highestZ = cell->getZOrder();

        auto splitName = string::split(cell->getinfo().m_name, " ");
        if (!splitName.size()) continue;

        if (splitName[0] == name) {
            if (splitName.size() == 1 && highestnewGraphNum == 0) {
                highestnewGraphNum = 1;
            }
            else if (splitName.size() == 2) {
                auto numRes = utils::numFromString<int>(splitName[1]);
                if (numRes.isErr()) continue;
                auto num = numRes.unwrap();

                if (highestnewGraphNum <= num) highestnewGraphNum = num + 1;
            }
        }
    }

    if (highestnewGraphNum != 0)
        name += " " + std::to_string(highestnewGraphNum);

    DTGraphInfo GInfo;
    GInfo.m_name = name;
    GInfo.m_orderPos = highestZ + 1;

    addGraph(GInfo);
    
    m_graphsScroll->m_contentLayer->setPositionY(0);
}

void DTGraphLayer::addGraph(const DTGraphInfo& info) {
    auto graphCell = GraphCell::create(m_graphsScroll->getContentWidth(), info);

    graphCell->m_onArrowCallback = [&](GraphCell* cell, bool isUp) {
        if (!isUp) {
            int highestZ = 0;
            GraphCell* existingCellWithZ = nullptr;

            for (const auto& otherCell : getGraphCells(m_graphsScroll->m_contentLayer))
            {
                if (!otherCell) continue;
                if (highestZ < otherCell->getZOrder()) highestZ = otherCell->getZOrder();

                if (otherCell->getZOrder() == cell->getZOrder() + 1) existingCellWithZ = otherCell;
            }

            if (cell->getZOrder() == highestZ) return;

            cell->setOrderPos(cell->getZOrder() + 1);
            if (existingCellWithZ != nullptr) existingCellWithZ->setOrderPos(existingCellWithZ->getZOrder() - 1);
        }
        else {
            if (cell->getZOrder() == 0) return;

            GraphCell* existingCellWithZ = nullptr;

            for (const auto& otherCell : getGraphCells(m_graphsScroll->m_contentLayer))
            {
                if (!otherCell) continue;
                if (otherCell->getZOrder() == cell->getZOrder() - 1) existingCellWithZ = otherCell;
            }

            cell->setOrderPos(cell->getZOrder() - 1);
            if (existingCellWithZ != nullptr) existingCellWithZ->setOrderPos(existingCellWithZ->getZOrder() + 1);
        }

        updateGRapgCellLayout();

        saveAllGraphs();
    };
    graphCell->m_onOptionsCallback = [&](GraphCell* cell) {
        openOptionsFor(cell);
    };
    graphCell->m_onEnabledChanged = [&](GraphCell* cell) {
        m_graph->getGraphNode(cell->getinfo().m_name)->setInfo(cell->getinfo());

        if (!m_holdingShift && cell->getinfo().m_isEnabled) {
            for (const auto& graphCell : getGraphCells(m_graphsScroll->m_contentLayer)) {
                if (!graphCell) continue;
                if (graphCell == cell) continue;
                graphCell->setEnabledInfo(false, true, false);
            }
        }

        saveAllGraphs();
    };
    graphCell->m_onInfoChangedCallback = [&](GraphCell* cell) {
        auto graphNode = m_graph->getGraphNode(cell->getinfo().m_name);
        bool changeName = false;

        if (graphNode == nullptr) {
            graphNode = m_graph->getGraphNode(cell->m_oldName);
            changeName = true;
        }

        if (graphNode == nullptr) return;

        if (changeName)
            m_graph->changeGraphName(cell->m_oldName, cell->getinfo().m_name);

        if (graphNode->getInfo().has_value() && cell->getinfo().m_coverage != graphNode->getInfo().value().m_coverage)
            updateGRapgCellLayout();

        graphNode->setInfo(cell->getinfo());
        cell->resendSession();
        cell->resendSection();

        saveAllGraphs();
    };

    graphCell->m_canChangeNameTo = [&](const std::string& to, GraphCell* cell) -> bool {
        GraphCell* cellWithName = nullptr;

        for (const auto& child : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!child) continue;
            if (child->getinfo().m_name == to) {
                cellWithName = child;
                break;
            }
        }
        
        return cellWithName == nullptr || cellWithName == cell;
    };

    graphCell->m_beforeDeletion = [&](DTGraphInfo info, GraphCell* deletingCell) {
        m_graph->removeGraph(info.m_name);

        int indexZ = 0;
        for (const auto& child : getGraphCells(m_graphsScroll->m_contentLayer))
        {
            if (!child) continue;
            if (child == deletingCell) continue;

            child->setOrderPos(indexZ);
            indexZ++;
        }
    };

    graphCell->m_onDeleted = [&](DTGraphInfo info) {
        updateGRapgCellLayout();
        saveAllGraphs();
    };

    graphCell->m_onNewSession = [&](GraphCell* cell, auto newSession) {
        auto graphNode = m_graph->getGraphNode(cell->getinfo().m_name);

        graphNode->m_sessionToShow = std::move(newSession);

        graphNode->updateDeaths();
    };

    graphCell->m_onNewRun = [&](GraphCell* cell, auto newRun) {
        auto graphNode = m_graph->getGraphNode(cell->getinfo().m_name);

        graphNode->m_runPercent = newRun;

        graphNode->updateDeaths();
    };

    graphCell->m_onNewSection = [&](GraphCell* cell, const Section& section) {
        auto graphNode = m_graph->getGraphNode(cell->getinfo().m_name);
        graphNode->setSectionToShow(section);
    };

    if (graphCell->getinfo().m_isEnabled && !m_holdingShift) {
        for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer)) {
            if (cell == graphCell) continue;
            cell->setEnabledInfo(false, true, false);
        }
    }

    m_graphsScroll->m_contentLayer->addChild(graphCell);

    m_graph->addGraph(info);
    graphCell->resendSession();
    graphCell->resendSection();
    graphCell->setEnabled(true);
    graphCell->setAnchorPoint({0,0});
    
    updateGRapgCellLayout();

    saveAllGraphs();
}

void DTGraphLayer::removeGraph(std::string_view graphName) {
    for (const auto& cell : getGraphCells(m_graphsScroll->m_contentLayer)) {
        if (!cell) continue;
        if (cell->getinfo().m_name == graphName) {
            cell->removeMeAndCleanup();
            m_graph->removeGraph(std::string(graphName));
            updateGRapgCellLayout();
            break;
        }
    }
}

void DTGraphLayer::openOptionsFor(GraphCell* cell) {
    auto exitEasing = [](bool open, float finalSize = 1) -> CCEaseExponentialOut* {
        return CCEaseExponentialOut::create(CCScaleTo::create(.2f, finalSize, open ? finalSize : 0));
    };

    m_graphsPage->stopAllActions();
    m_graphsPage->runAction(CCSequence::create(
        exitEasing(false),
        nullptr
    ));

    m_graphOptionsPage->stopAllActions();
    m_graphOptionsPage->runAction(CCSequence::create(
        exitEasing(false),
        CCDelayTime::create(.1f),
        exitEasing(true, .97f),
        nullptr
    ));

    m_closeBtn->setEnabled(false);
    m_closeBtn->runAction(CCFadeTo::create(.2f, 0));

    auto cellInfo = cell->getinfo();

    m_nameInput->setString(cellInfo.m_name);
    m_coverageSwitcher->setValue(cellInfo.m_coverage, true);
    m_typeSwitcher->setValue(cellInfo.m_type, true);
    m_thicknessBaseInput->setString(fmt::format("{:.2f}", cellInfo.m_thickness));
    m_thicknessOutlineInput->setString(fmt::format("{:.2f}", cellInfo.m_outlineThickness));
    m_colorBaseBtnSpr->setColor({cellInfo.m_color.r, cellInfo.m_color.g, cellInfo.m_color.b});
    m_colorBaseBtnSpr->setOpacity(cellInfo.m_color.a);
    m_colorOutlineBtnSpr->setColor({cellInfo.m_outlineColor.r, cellInfo.m_outlineColor.g, cellInfo.m_outlineColor.b});
    m_colorOutlineBtnSpr->setOpacity(cellInfo.m_outlineColor.a);
    m_pointScaleInput->setString(fmt::format("{:.2f}", cellInfo.m_pointScale));
    m_pointColorBtnSpr->setColor({cellInfo.m_pointColor.r, cellInfo.m_pointColor.g, cellInfo.m_pointColor.b});
    m_pointColorBtnSpr->setOpacity(cellInfo.m_pointColor.a);

    m_editedGraph = cell;
    m_graphsScroll->setTouchEnabled(false);
    m_graphsScrollbar->setTouchEnabled(false);
    m_graphsScroll->setMouseEnabled(false);
    // sessionSelector->setEnabled(false);
    // runSelectInput->setEnabled(false);
    for (const auto& child : getGraphCells(m_graphsScroll->m_contentLayer))
    {
        child->setEnabled(false);
    }
    
    m_graphOptionsPage->setEnabled(true);
    m_nameInput->setEnabled(true);
    m_thicknessBaseInput->setEnabled(true);
    m_thicknessOutlineInput->setEnabled(true);
    m_pointScaleInput->setEnabled(true);
}


void DTGraphLayer::onColor(CCObject* sender) {
    auto nodeSender = static_cast<CCNode*>(sender);

    ColorPickPopup* popup = nullptr;

    if (nodeSender->getID() == "line") {
        ccColor4B color;
        

        if (nodeSender->getTag() == 1) {
            color = {
                m_colorBaseBtnSpr->getColor().r,
                m_colorBaseBtnSpr->getColor().g,
                m_colorBaseBtnSpr->getColor().b,
                m_colorBaseBtnSpr->getOpacity(),
            };
        }
        else {
            color = {
                m_colorOutlineBtnSpr->getColor().r,
                m_colorOutlineBtnSpr->getColor().g,
                m_colorOutlineBtnSpr->getColor().b,
                m_colorOutlineBtnSpr->getOpacity(),
            };
        }

        popup = ColorPickPopup::create(color);
        popup->setColorTarget(nodeSender->getTag() == 1 ? m_colorBaseBtnSpr : m_colorOutlineBtnSpr);

        if (nodeSender->getTag() == 1) {
            m_callbacksForColorPopups = [&](const auto& color) {
                m_colorBaseBtnSpr->setColor({color.r, color.g, color.b});
                m_colorBaseBtnSpr->setOpacity(color.a);

                if (!m_editedGraph.has_value()) return;

                m_editedGraph.value()->setColor(color);
            };
        }
        else {
            m_callbacksForColorPopups = [&](const auto& color) {
                m_colorOutlineBtnSpr->setColor({color.r, color.g, color.b});
                m_colorOutlineBtnSpr->setOpacity(color.a);

                if (!m_editedGraph.has_value()) return;

                m_editedGraph.value()->setOutlineColor(color);
            };
        }
    }
    else if (nodeSender->getID() == "point") {
        popup = ColorPickPopup::create({
            m_pointColorBtnSpr->getColor().r,
            m_pointColorBtnSpr->getColor().g,
            m_pointColorBtnSpr->getColor().b,
            m_pointColorBtnSpr->getOpacity(),
        });

        popup->setColorTarget(m_pointColorBtnSpr);

        m_callbacksForColorPopups = [&](const auto& color) {
            m_pointColorBtnSpr->setColor({color.r, color.g, color.b});
            m_pointColorBtnSpr->setOpacity(color.a);

            if (!m_editedGraph.has_value()) return;

            m_editedGraph.value()->setPointColor(color);
        };
    }

    if (popup != nullptr) {
        popup->setCallback([&](auto color) { DTGraphLayer::updateColor(color);});
        popup->show();
    }
}

void DTGraphLayer::updateColor(cocos2d::ccColor4B const& color) {
    if (m_callbacksForColorPopups.has_value())
        m_callbacksForColorPopups.value()(color);
}

void DTGraphLayer::saveAllGraphs() {
    std::vector<DTGraphInfo> allInfos{};

    for (const auto& graphCell : getGraphCells(m_graphsScroll->m_contentLayer))
    {
        if (!graphCell) continue;
        allInfos.push_back(graphCell->getinfo());
    }
    
    Save::setGraphs(allInfos);
}

void DTGraphLayer::onOk(CCObject*) {
    if (!m_editedGraph.has_value()) return;

    if (!m_editedGraph.value()->setName(m_nameInput->getString())) {
        FLAlertLayer::create("Cant save graph!", "A graph with this name already exists! please change the name you selected!", "OK")->show();
        return;
    }

    closeOptionsTab();
}
void DTGraphLayer::onDelete(CCObject*) {
    if (!m_editedGraph.has_value()) return;
    m_deleteNotification = FLAlertLayer::create(this, "WARNING!", "This action will delete the selected graph! are you sure you want to do this?", "CANCEL", "DELETE");
    m_deleteNotification->show();
}

void DTGraphLayer::closeOptionsTab() {
    auto exitEasing = [](bool open, float finalSize = 1) -> CCEaseExponentialOut* {
        return CCEaseExponentialOut::create(CCScaleTo::create(.2f, finalSize, open ? finalSize : 0));
    };

    m_graphOptionsPage->stopAllActions();
    m_graphOptionsPage->runAction(CCSequence::create(
        exitEasing(false),
        nullptr
    ));

    m_graphsPage->stopAllActions();
    m_graphsPage->runAction(CCSequence::create(
        exitEasing(false),
        CCDelayTime::create(.1f),
        exitEasing(true, .97f),
        nullptr
    ));

    m_graphsScroll->setTouchEnabled(true);
    m_graphsScrollbar->setTouchEnabled(true);
    // sessionSelector->setEnabled(true);
    // runSelectInput->setEnabled(true);
    for (const auto& child : getGraphCells(m_graphsScroll->m_contentLayer))
    {
        if (!child) continue;
        child->setEnabled(true);
    }
    
    m_closeBtn->setEnabled(true);
    m_closeBtn->runAction(CCFadeTo::create(.2f, 255));
    m_editedGraph = std::nullopt;

    m_graphOptionsPage->setEnabled(false);
    m_nameInput->setEnabled(false);
    m_thicknessBaseInput->setEnabled(false);
    m_thicknessOutlineInput->setEnabled(false);
    m_pointScaleInput->setEnabled(false);
}

void DTGraphLayer::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (layer == m_deleteNotification && btn2) {
        m_editedGraph.value()->deleteMe();
        m_editedGraph = std::nullopt;

        closeOptionsTab();
    }
}

void DTGraphLayer::updateGRapgCellLayout() {
    std::set<CCNode*> invisible{};
    for (const auto& child : m_graphsScroll->m_contentLayer->getChildrenExt<CCNode*>())
    {
        if (!child) continue;
        if (!child->isVisible()) {
            invisible.insert(child);
            child->setVisible(true);
        }
    }
    
    m_graphsScroll->m_contentLayer->updateLayout();

    for (const auto& child : invisible)
    {
        child->setVisible(false);
    }
    
}

std::vector<GraphCell*> DTGraphLayer::getGraphCells(CCNode* contentLayer) {
    std::vector<GraphCell*> graphCells;
    for (const auto& child : CCArrayExt<CCNode*>(contentLayer->getChildren())) {
        if (child && child->getID() == "graph-cell")
            graphCells.push_back(static_cast<GraphCell*>(child));
    }
    return graphCells;
}