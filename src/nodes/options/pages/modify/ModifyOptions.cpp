#include "ModifyOptions.hpp"

#include <nodes/DTLayer.hpp>
#include <utils/Dev.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>
#include <nodes/general/SimpleToggler.hpp>
#include <nodes/layout/usage/DTLabel.hpp>
#include <nodes/general/SessionSelector.hpp>
#include <nodes/general/SwitcherButton.hpp>
#include <nodes/Tutorial/TutorialButton.hpp>
#include <nodes/layout/usage/DecimalPopup.hpp>
#include <managers/StatsManager.hpp>

ModifyOptions* ModifyOptions::create(CCSize size) {
    auto ret = new ModifyOptions();

    if (ret && ret->initWithSetup(std::move(size))) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool ModifyOptions::setup() {
    this->setEnabled(false);
    this->setOpacity(0);

    auto dtLayer = DTLayer::get();

    float previewBGMargin = 10;

    auto previewBGHolder = CCNode::create();
    this->addChild(previewBGHolder);

    m_previewBG = CCScale9Sprite::create("square02_001.png");
    m_previewBG->setContentSize({
        m_size.width / 2.5f - previewBGMargin,
        m_size.height - previewBGMargin
    });
    m_previewBG->setPosition({
        m_size.width - m_previewBG->getScaledContentWidth() / 2 - previewBGMargin / 2,
        0 + previewBGMargin / 2
    });
    m_previewBG->setAnchorPoint({0.5f, 0});
    m_previewBG->setOpacity(0);
    previewBGHolder->addChild(m_previewBG);

    m_previewScroll = ScrollLayer::create(m_previewBG->getContentSize() - ccp(5, 5));
    m_previewScroll->setPosition(m_previewBG->getPosition() + ccp(0, 2.5f));
    m_previewScroll->ignoreAnchorPointForPosition(false);
    m_previewScroll->setAnchorPoint({.5f, 0});
    m_previewScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_previewScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    m_previewScroll->setMouseEnabled(false);
    this->addChild(m_previewScroll);

    m_scrollbar = Scrollbar::create(m_previewScroll);
    m_scrollbar->ignoreAnchorPointForPosition(false);
    m_scrollbar->setPosition(m_previewScroll->getPosition() + m_previewScroll->getContentSize() / 2);
    this->addChild(m_scrollbar);

    DTLabelInfo info;
    info.m_scale = .5f;
    info.m_wrapping = WrappingMode::SPACE_WRAP;
    info.m_isExpanded = true;

    m_myLabel = DTLabel::create(info);
    m_myLabel->setIgnoreExtraSettings(true);
    m_myLabel->setExpandable(false);
    m_myLabel->setContentWidth(m_previewScroll->getContentWidth());
    m_previewScroll->m_contentLayer->addChild(m_myLabel);

    m_previewScroll->m_contentLayer->updateLayout();
    m_previewScroll->moveToTop();

    m_selectionBG = CCScale9Sprite::create("square02_small.png");
    m_selectionBG->setContentWidth(m_size.width - m_previewScroll->getScaledContentWidth() - previewBGMargin - 10);
    m_selectionBG->setContentHeight(35);
    m_selectionBG->setAnchorPoint({0, 1});
    m_selectionBG->setCascadeColorEnabled(false);
    m_selectionBG->setPosition({
        previewBGMargin / 2,
        m_size.height - previewBGMargin / 2
    });
    this->addChild(m_selectionBG);

    m_sessLvlSwitcher = SwitcherButton::create({
        65,
        m_selectionBG->getContentHeight() - 15
    }, "GJ_button_01.png", {
        "Level",
        "Session"
    });
    m_sessLvlSwitcher->setPosition(m_selectionBG->getPosition() + ccp(
        m_sessLvlSwitcher->getScaledContentWidth() / 2 + 5,
        -m_sessLvlSwitcher->getScaledContentHeight() / 2 - 15 / 2
    ));
    this->addChild(m_sessLvlSwitcher);

    m_runF0Toggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("from-zero.png"_spr),
        CCSprite::createWithSpriteFrameName("run.png"_spr),
        .75f
    );
    m_runF0Toggler->setPosition(m_selectionBG->getPosition() + m_selectionBG->getScaledContentSize() + ccp(
        -m_runF0Toggler->getScaledContentWidth() / 2 - 5,
        -m_selectionBG->getScaledContentHeight() * 1.5
    ));
    this->addChild(m_runF0Toggler);

    auto currentRunModeLabel = geode::Label::create("From 0", "bigFont.fnt");
    currentRunModeLabel->setAlignment(geode::Label::Alignment::Right);
    currentRunModeLabel->setAnchorPoint({1, .5f});
    currentRunModeLabel->setPosition(m_runF0Toggler->getPosition() + ccp(
        -5 - m_runF0Toggler->getScaledContentWidth() / 2,
        0
    ));
    currentRunModeLabel->setScale(.5f);
    this->addChild(currentRunModeLabel);

    m_runF0Toggler->setCallback([&, currentRunModeLabel](auto state) {
        currentRunModeLabel->setString(
            state ?
                "Runs" :
                "From 0"
        );

        updatePreviewName(m_sessLvlSwitcher->getCurrentOptionIndex() == 1, !state);
    });

    m_sessLvlSwitcher->setOptionChangedCallback([&](auto option) {
        updatePreviewName(option == 1, !m_runF0Toggler->isToggled());
    });

    int sessionAmount = 1;
    if  (dtLayer != nullptr) {
        sessionAmount = dtLayer->getCurrentGrouping().m_grouping.size();
    }

    m_sessionSelector = SessionSelector::create(sessionAmount);
    m_sessionSelector->setCallback([&](int newSession) {
        DTLayer::get()->onSessionSelected(newSession, true);
        if (!m_runF0Toggler->isToggled())
            updatePreviewName(m_sessLvlSwitcher->getCurrentOptionIndex() == 1, !m_runF0Toggler->isToggled());
    });
    m_sessionSelector->setScale(.5f);
    m_sessionSelector->ignoreAnchorPointForPosition(false);
    m_sessionSelector->setCurrentCount(DTLayer::get()->getCurrentSelectedSession(), true);
    m_sessionSelector->setPosition(currentRunModeLabel->getPosition() + ccp(
        -currentRunModeLabel->getScaledContentWidth() - 5 - m_sessionSelector->getScaledContentWidth() / 2,
        0
    ));
    this->addChild(m_sessionSelector);

    auto removeInvalidRunsBtnSpr = ButtonSprite::create(
        "Remove Invalid Runs",
        "bigFont.fnt",
        "GJ_button_06.png"
    );
    removeInvalidRunsBtnSpr->setCascadeOpacityEnabled(true);
    removeInvalidRunsBtnSpr->setScale(.35f);
    auto removeInvalidRunsBtn = CCMenuItemSpriteExtra::create(
        removeInvalidRunsBtnSpr,
        this,
        menu_selector(ModifyOptions::removeInvalidRuns)
    );
    removeInvalidRunsBtn->setAnchorPoint({0, .5f});
    removeInvalidRunsBtn->setPosition(m_selectionBG->getPosition() + ccp(
        0,
        -m_selectionBG->getScaledContentHeight() - removeInvalidRunsBtn->getScaledContentHeight() / 2 - 10
    ));
    this->addChild(removeInvalidRunsBtn);

    float diffBetweenSquares = 5;
    float squaresHeight = 135;

    m_runsBG = CCScale9Sprite::create("square02_001.png");
    m_runsBG->setContentWidth(m_selectionBG->getScaledContentWidth() / 3 - diffBetweenSquares);
    m_runsBG->setContentHeight(squaresHeight);
    m_runsBG->setCascadeColorEnabled(false);
    m_runsBG->setAnchorPoint({0, 0});
    m_runsBG->setPosition({
        previewBGMargin / 2,
        previewBGMargin / 2
    });
    this->addChild(m_runsBG);

    m_runAmountInput = TextInput::create(m_runsBG->getContentWidth() / 1.2f, "amount", "gjFont17.fnt");
    m_runAmountInput->setPosition({
        m_runsBG->getContentWidth() / 2,
        m_runAmountInput->getContentHeight() / 2 + 2
    });
    m_runAmountInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            m_runAmountInput->setString("100");
        }

        m_currRunsAmount = num;
    });
    m_runAmountInput->setString(std::to_string(m_currRunsAmount));
    m_runAmountInput->setCommonFilter(CommonFilter::Uint);
    m_runsBG->addChild(m_runAmountInput);

    m_runAmountLabel = geode::Label::create("Amount", "bigFont.fnt");
    m_runAmountLabel->setScale(.5f);
    m_runAmountLabel->setAnchorPoint({0.5f, 1});
    m_runAmountLabel->setPosition(m_runAmountInput->getPosition() + ccp(
        0,
        m_runAmountLabel->getScaledContentHeight() + m_runAmountInput->getScaledContentHeight() / 2
    ));
    m_runsBG->addChild(m_runAmountLabel);

    auto runPMAlignNode = CCNode::create();
    runPMAlignNode->setContentSize({
        m_runsBG->getContentWidth() / 10.f,
        25
    });
    runPMAlignNode->setAnchorPoint({.5f, .5f});
    runPMAlignNode->setPosition(m_runAmountLabel->getPosition() + ccp(
        0,
        runPMAlignNode->getScaledContentHeight() / 2
    ));
    m_runsBG->addChild(runPMAlignNode);

    addPlusMinusBtns(runPMAlignNode, false, [&](bool isPlus) {
        if (m_currentStartRunToAdd > m_currentEndRunToAdd) {
            auto tint = CCTintTo::create(.2f, 255, 0, 0);
            auto seq = CCSequence::create(
                CCTintTo::create(.2f, 255, 0, 0),
                CCTintTo::create(.2f, 255, 255, 255),
                nullptr
            );
            seq->setTag(8);
            m_addRunStartInput->getContentLabel()->stopActionByTag(8);
            m_addRunStartInput->getContentLabel()->runAction(seq);
            return;
        }

        std::optional<int> sessionNum = std::nullopt;
        if (
            m_sessLvlSwitcher->getCurrentOptionIndex() == 1
        ) {
            sessionNum = m_sessionSelector->getCurrentCount();
        }

        DTLayer::get()->modifyRun(m_currentStartRunToAdd, m_currentEndRunToAdd, m_currRunsAmount * (isPlus ? 1 : -1), sessionNum);
        
        if (sessionNum == std::nullopt) DTLayer::get()->m_specialStrings["runs"]->updateContent();
        else DTLayer::get()->m_specialStrings["sruns"]->updateContent();
    }, .9f);

    m_addRunSeperator = geode::Label::create("-", "bigFont.fnt");
    m_addRunSeperator->setPosition(runPMAlignNode->getPosition() + ccp(
        0,
        runPMAlignNode->getScaledContentHeight() / 2 + m_addRunSeperator->getScaledContentHeight() / 2
    ));
    m_addRunSeperator->setScale(.35f);
    m_runsBG->addChild(m_addRunSeperator);

    m_addRunStartInput = TextInput::create(35, "S%", "gjFont17.fnt");
    m_addRunStartInput->setPosition(m_addRunSeperator->getPosition() + ccp(-m_addRunSeperator->getScaledContentWidth() / 2 - m_addRunStartInput->getContentWidth() / 2 - 5, 0));
    m_addRunStartInput->setCommonFilter(CommonFilter::Float);
    m_runsBG->addChild(m_addRunStartInput);

    m_addRunEndInput = TextInput::create(35, "E%", "gjFont17.fnt");
    m_addRunEndInput->setPosition(m_addRunSeperator->getPosition() + ccp(m_addRunSeperator->getScaledContentWidth() / 2 + m_addRunEndInput->getContentWidth() / 2 + 5, 0));
    m_addRunEndInput->setCommonFilter(CommonFilter::Float);
    m_runsBG->addChild(m_addRunEndInput);

    m_addRunStartInput->setCallback([&](std::string const& newstr) {
        if (newstr.contains("-") && m_addRunStartInput->isFocused()) {
            m_addRunStartInput->unfocus();
            m_addRunEndInput->focus();
            m_addRunEndInput->setString("");
            m_addRunSeperator->setColor({ 0, 255, 0 });
            m_addRunSeperator->runAction(CCTintTo::create(.5f, 255, 255, 255));
            m_addRunStartInput->setString(newstr.substr(0, newstr.find("-")));
            return;
        }

        auto numStartRes = geode::utils::numFromString<double>(newstr);
        if (numStartRes.isErr()) return;
        double numStart = numStartRes.unwrap();
        if (numStart < 0) return;

        if (numStart > 100) {
            numStart = 100;
            m_addRunStartInput->setString("100");
        }

        m_currentStartRunToAdd = numStart;
    });

    m_addRunEndInput->setCallback([&](auto newstr) {
        if (newstr.contains("-") && m_addRunEndInput->isFocused()) {
            m_addRunEndInput->unfocus();
            m_addRunStartInput->focus();
            m_addRunStartInput->setString("");
            m_addRunSeperator->setColor({ 0, 255, 0 });
            m_addRunSeperator->runAction(CCTintTo::create(.5f, 255, 255, 255));
            m_addRunEndInput->setString(newstr.substr(0, newstr.find("-")));
            return;
        }

        auto numEndRes = geode::utils::numFromString<double>(newstr);
        if (numEndRes.isErr()) return;
        double numEnd = numEndRes.unwrap();
        if (numEnd < 0) return;

        if (numEnd > 100) {
            numEnd = 100;
            m_addRunEndInput->setString("100");
        }

        m_currentEndRunToAdd = numEnd;
    });

    m_runLabel = geode::Label::create("Runs", "bigFont.fnt");
    m_runLabel->setScale(.5f);
    m_runLabel->setAnchorPoint({0, 1});
    m_runLabel->setPosition({
        5,
        m_runsBG->getContentHeight() - 5
    });
    m_runsBG->addChild(m_runLabel);

    m_f0BG = CCScale9Sprite::create("square02_001.png");
    m_f0BG->setContentWidth(m_runsBG->getContentWidth());
    m_f0BG->setContentHeight(squaresHeight);
    m_f0BG->setAnchorPoint({0, 0});
    m_f0BG->setCascadeColorEnabled(false);
    m_f0BG->setPosition(m_runsBG->getPosition() + ccp(
        m_runsBG->getContentWidth() + diffBetweenSquares,
        0
    ));
    this->addChild(m_f0BG);

    m_f0AmountInput = TextInput::create(m_f0BG->getContentWidth() / 1.2f, "amount", "gjFont17.fnt");
    m_f0AmountInput->setPosition({
        m_f0BG->getContentWidth() / 2,
        m_f0AmountInput->getContentHeight() / 2 + 2
    });
    m_f0AmountInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            m_f0AmountInput->setString("100");
        }

        m_currF0Amount = num;
    });
    m_f0AmountInput->setString(std::to_string(m_currF0Amount));
    m_f0AmountInput->setCommonFilter(CommonFilter::Uint);
    m_f0BG->addChild(m_f0AmountInput);

    m_f0AmountLabel = geode::Label::create("Amount", "bigFont.fnt");
    m_f0AmountLabel->setScale(.5f);
    m_f0AmountLabel->setAnchorPoint({0.5f, 1});
    m_f0AmountLabel->setPosition(m_f0AmountInput->getPosition() + ccp(
        0,
        m_f0AmountLabel->getScaledContentHeight() + m_f0AmountInput->getScaledContentHeight() / 2
    ));
    m_f0BG->addChild(m_f0AmountLabel);

    auto f0PMAlignNode = CCNode::create();
    f0PMAlignNode->setContentSize({
        m_f0BG->getContentWidth() / 10.f,
        25
    });
    f0PMAlignNode->setAnchorPoint({.5f, .5f});
    f0PMAlignNode->setPosition(m_f0AmountLabel->getPosition() + ccp(
        0,
        f0PMAlignNode->getScaledContentHeight() / 2
    ));
    m_f0BG->addChild(f0PMAlignNode);

    addPlusMinusBtns(f0PMAlignNode, false, [&](bool isPlus) {
        std::optional<int> sessionNum = std::nullopt;
        if (m_sessLvlSwitcher->getCurrentOptionIndex() == 1) {
            sessionNum = m_sessionSelector->getCurrentCount();
        }

        DTLayer::get()->modifyRun(m_currentF0ToAdd, m_currF0Amount * (isPlus ? 1 : -1), sessionNum);
        
        if (sessionNum == std::nullopt) DTLayer::get()->m_specialStrings["general"]->updateContent();
        else DTLayer::get()->m_specialStrings["s0"]->updateContent();
    }, .9f);

    m_addPercentInput = TextInput::create(75, "percent", "gjFont17.fnt");
    m_addPercentInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<double>(newstr);
        if (numRes.isErr()) return;
        double num = numRes.unwrap();
        if (num < 0) return;

        if (num > 100) {
            num = 100;
            m_addPercentInput->setString("100");
        }

        m_currentF0ToAdd = num;
    });
    m_addPercentInput->setPosition(f0PMAlignNode->getPosition() + ccp(
        0,
        f0PMAlignNode->getScaledContentHeight() / 2 + m_addPercentInput->getScaledContentHeight() / 2
    ));
    m_addPercentInput->setCommonFilter(CommonFilter::Float);
    m_f0BG->addChild(m_addPercentInput);

    m_f0Label = geode::Label::create("From 0", "bigFont.fnt");
    m_f0Label->setScale(.5f);
    m_f0Label->setAnchorPoint({0, 1});
    m_f0Label->setPosition({
        5,
        m_f0BG->getContentHeight() - 5
    });
    m_f0BG->addChild(m_f0Label);

    m_nbBG = CCScale9Sprite::create("square02_001.png");
    m_nbBG->setContentWidth(m_f0BG->getContentWidth());
    m_nbBG->setContentHeight(squaresHeight);
    m_nbBG->setAnchorPoint({0, 0});
    m_nbBG->setCascadeColorEnabled(false);
    m_nbBG->setPosition(m_f0BG->getPosition() + ccp(
        m_f0BG->getContentWidth() + diffBetweenSquares,
        0
    ));
    this->addChild(m_nbBG);

    auto removeBtnSpr = ButtonSprite::create(
        "Remove",
        m_nbBG->getScaledContentWidth() / 1.4f,
        m_nbBG->getScaledContentWidth() / 1.4f,
        1,
        false,
        "bigFont.fnt",
        "GJ_button_04.png",
        30
    );
    removeBtnSpr->setCascadeOpacityEnabled(true);
    auto removeBtn = CCMenuItemSpriteExtra::create(
        removeBtnSpr,
        this,
        menu_selector(ModifyOptions::removeNB)
    );
    removeBtn->setPosition(m_nbBG->getPosition() + ccp(
        m_nbBG->getScaledContentWidth() / 2,
        removeBtn->getScaledContentHeight() / 2 + 5
    ));
    this->addChild(removeBtn);
    
    auto addBtnSpr = ButtonSprite::create(
        "  Add  ",
        m_nbBG->getScaledContentWidth() / 1.4f,
        m_nbBG->getScaledContentWidth() / 1.4f,
        1,
        false,
        "bigFont.fnt",
        "GJ_button_03.png",
        30
    );
    addBtnSpr->setCascadeOpacityEnabled(true);
    auto addBtn = CCMenuItemSpriteExtra::create(
        addBtnSpr,
        this,
        menu_selector(ModifyOptions::addNB)
    );
    addBtn->setPosition(removeBtn->getPosition() + ccp(
        0,
        removeBtn->getScaledContentHeight() / 2 + addBtn->getScaledContentHeight() / 2 + 5
    ));
    this->addChild(addBtn);

    m_addNewBestInput = TextInput::create(75, "New B %", "gjFont17.fnt");
    m_addNewBestInput->setPosition(addBtn->getPosition() + ccp(
        0,
        m_addNewBestInput->getScaledContentHeight() / 2 + addBtn->getScaledContentHeight() / 2 + 5
    ));
    m_addNewBestInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<double>(newstr);
        if (numRes.isErr()) return;
        double num = numRes.unwrap();
        if (num < 0) return;

        if (num > 100) {
            num = 100;
            m_addNewBestInput->setString("100");
        }

        m_currentNBToAdd = num;
    });
    m_addNewBestInput->setCommonFilter(CommonFilter::Float);
    this->addChild(m_addNewBestInput);

    m_NBLabel = geode::Label::create("New Bests", "bigFont.fnt");
    m_NBLabel->setScale(.4f);
    m_NBLabel->setAnchorPoint({0, 1});
    m_NBLabel->setPosition({
        5,
        m_nbBG->getContentHeight() - 5
    });
    m_nbBG->addChild(m_NBLabel);

    scheduleUpdate();

    auto modiftyInfo = TutorialButton::create(.75f, "modify-overall", [&, addBtn, removeInvalidRunsBtn, removeBtn, currentRunModeLabel](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("Here you are able to <cy>modify</c> your save file!", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("Here you have a <cf>preview</c> of how your save file <cy>currently</c> looks", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinHighlight(m_previewBG)

            ->appendDialogue("You can see <cs>what data</c> it currently displays in the <cl>labels title</c>", TutorialCharacterFace::TCFNormalTilted)
            ->joinTransform(TutorialBoxPlacement::TBPTopLeft, .75f)
            ->joinHighlight(m_myLabel->getLabelTitleBG())

            ->appendDialogue("At the top you can <cy>edit what data you are going to edit!</c>", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinHighlight(m_selectionBG)

            ->appendDialogue("You can choose between <cy>general</c> level stats or <cy>session</c> stats", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_sessLvlSwitcher)

            ->appendDialogue("You can choose between editing <cc>runs</c> or <cc>from 0</c> stats", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(m_runF0Toggler)
            ->joinHighlight(currentRunModeLabel)

            ->appendDialogue("And you can switch between <co>sessions</c> using the session selector", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_sessionSelector)
            ->joinTextToHighlight("Session Selector", .35f)

            ->appendDialogue("The <cf>preview</c> will change according to your selection!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinHighlight(m_previewBG)

            ->appendDialogue("At the bottom you have fields to <cg>add</c>/<cr>remove</c> data from your save!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPTopLeft, .75f)
            ->joinHighlight(m_runsBG)
            ->joinHighlight(m_f0BG)
            ->joinHighlight(m_nbBG)

            ->appendDialogue("One for <cy>runs</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_runsBG)

            ->appendDialogue("One for <cy>from 0</c> progress", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_f0BG)

            ->appendDialogue("And one for <cy>new bests</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_nbBG)

            ->appendDialogue("They all have <cc>input fields</c> to input percentages", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_addRunStartInput)
            ->joinHighlight(m_addRunEndInput)
            ->joinHighlight(m_addPercentInput)
            ->joinHighlight(m_addNewBestInput)
            
            ->appendDialogue("And buttons for <cg>adding</c>/<cr>removing</c> the inputted data from the save file!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(addBtn)
            ->joinHighlight(removeBtn);

        for (const auto& [btn, _] : m_plusMinusCallbacks)
        {
            tutorialLayer->joinHighlight(btn);
        }
        
        tutorialLayer->appendDialogue("Some have an <cy>amount field</c> for inputting <cy>how any of x runs</c> to add at a time", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_f0AmountInput)
            ->joinHighlight(m_runAmountInput)
            
            ->appendDialogue("Theres is also the <cr>\"Remove Invalid Runs\"</c> button", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(removeInvalidRunsBtn)
            
            ->appendDialogue("Which removes runs that go <co>below 0%</c> or <co>abover 100%</c> you might have saved", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(removeInvalidRunsBtn)

            ->appendDialogue("Enjoy editing your save file!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    modiftyInfo->setPosition(m_size - modiftyInfo->getScaledContentSize() / 2 + ccp(2, 2));
    this->addChild(modiftyInfo);

    this->setOpacity(0);

    Dev::fadeTextInput(m_addNewBestInput, false, 0);
    Dev::fadeTextInput(m_addPercentInput, false, 0);
    Dev::fadeTextInput(m_f0AmountInput, false, 0);
    Dev::fadeTextInput(m_addRunEndInput, false, 0);
    Dev::fadeTextInput(m_addRunStartInput, false, 0);
    Dev::fadeTextInput(m_runAmountInput, false, 0);

    m_myLabel->setOpacity(0);
    m_myLabel->setTextColor({255, 255, 255, 0});
    m_myLabel->fadeTitleColorTo({255, 255, 255, 0}, 0);
    m_myLabel->setRunMoreButtonsVisible(false, 0);
    Dev::fadeTextInput(m_sessionSelector->getTextInput(), false, 0);

    static_cast<CCScale9Sprite*>(m_scrollbar->getChildren()->objectAtIndex(0))->setOpacity(0);
    static_cast<CCScale9Sprite*>(m_scrollbar->getChildren()->objectAtIndex(1))->setOpacity(0);
    m_scrollbar->setTouchEnabled(false);

    m_sessionSelector->setEnabled(false);

    this->addEventListener(
        KeybindSettingPressedEvent(
            Mod::get(),
            "add-deaths"
        ),
        [&, f0PMAlignNode, runPMAlignNode](const Keybind& keybind, bool down, bool repeat, double) {
            if (down) {
                if (m_addNewBestInput->isFocused()) {
                    addNB(nullptr);
                    return ListenerResult::Stop;
                }
                else if (m_addRunStartInput->isFocused() || m_addRunEndInput->isFocused()) {
                    (*m_plusMinusCallbacks[m_holdersOfPlusMinusBtns[runPMAlignNode].first])(true);
                    return ListenerResult::Stop;
                }
                else if (m_addPercentInput->isFocused()) {
                    (*m_plusMinusCallbacks[m_holdersOfPlusMinusBtns[f0PMAlignNode].first])(true);
                    return ListenerResult::Stop;
                }
            }
            return ListenerResult::Propagate;
        }
    );
    this->addEventListener(
        KeybindSettingPressedEvent(
            Mod::get(),
            "remove-deaths"
        ),
        [&, f0PMAlignNode, runPMAlignNode](const Keybind& keybind, bool down, bool repeat, double) {
            if (down) {
                if (m_addNewBestInput->isFocused()) {
                    removeNB(nullptr);
                    return ListenerResult::Stop;
                }
                else if (m_addRunStartInput->isFocused() || m_addRunEndInput->isFocused()) {
                    (*m_plusMinusCallbacks[m_holdersOfPlusMinusBtns[runPMAlignNode].second])(false);
                    return ListenerResult::Stop;
                }
                else if (m_addPercentInput->isFocused()) {
                    (*m_plusMinusCallbacks[m_holdersOfPlusMinusBtns[f0PMAlignNode].second])(false);
                    return ListenerResult::Stop;
                }
            }
            return ListenerResult::Propagate;
        }
    );

    updatePreviewName(false, true);

    m_selectionBG->setOpacity(0);
    m_runsBG->setOpacity(0);
    m_f0BG->setOpacity(0);
    m_nbBG->setOpacity(0);

    m_runLabel->setOpacity(0);
    m_addRunSeperator->setOpacity(0);
    m_runAmountLabel->setOpacity(0);
    m_NBLabel->setOpacity(0);
    m_f0AmountLabel->setOpacity(0);
    m_f0Label->setOpacity(0);

    m_decimalPopup = DecimalPopup::create({200, m_size.height / 2.f});
    m_myLabel->attachedDecimalPopup(m_decimalPopup);
    this->addChild(m_decimalPopup);

    return true;
}

void ModifyOptions::onOpened() {
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));
    m_previewBG->runAction(CCFadeTo::create(fadeTime, 150));
    m_selectionBG->runAction(CCFadeTo::create(fadeTime, 150));
    m_runsBG->runAction(CCFadeTo::create(fadeTime, 150));
    m_f0BG->runAction(CCFadeTo::create(fadeTime, 150));
    m_nbBG->runAction(CCFadeTo::create(fadeTime, 150));

    m_runLabel->runAction(CCFadeTo::create(fadeTime, 255));
    m_addRunSeperator->runAction(CCFadeTo::create(fadeTime, 255));
    m_runAmountLabel->runAction(CCFadeTo::create(fadeTime, 255));
    m_NBLabel->runAction(CCFadeTo::create(fadeTime, 255));
    m_f0AmountLabel->runAction(CCFadeTo::create(fadeTime, 255));
    m_f0Label->runAction(CCFadeTo::create(fadeTime, 255));

    Dev::fadeTextInput(m_addNewBestInput, true, fadeTime);
    Dev::fadeTextInput(m_addPercentInput, true, fadeTime);
    Dev::fadeTextInput(m_f0AmountInput, true, fadeTime);
    Dev::fadeTextInput(m_addRunEndInput, true, fadeTime);
    Dev::fadeTextInput(m_addRunStartInput, true, fadeTime);
    Dev::fadeTextInput(m_runAmountInput, true, fadeTime);

    m_sessionSelector->setMaximumCount(DTLayer::get()->m_sessionSelector->getMaximumCount(), false);

    m_myLabel->stopActionByTag(m_previewLabelFadeActionTag);
    auto labelFade = CCFadeIn::create(fadeTime);
    labelFade->setTag(m_previewLabelFadeActionTag);
    m_myLabel->runAction(labelFade);
    m_myLabel->fadeTextColorTo({255, 255, 255, 255}, fadeTime);
    m_myLabel->fadeTitleColorTo({255, 255, 255, 255}, fadeTime);
    m_myLabel->setRunMoreButtonsVisible(true, fadeTime);
    Dev::fadeTextInput(m_sessionSelector->getTextInput(), true, fadeTime);
    m_sessionSelector->setEnabled(true);

    static_cast<CCScale9Sprite*>(m_scrollbar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 150));
    static_cast<CCScale9Sprite*>(m_scrollbar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 255));
    m_scrollbar->setTouchEnabled(true);

    this->setEnabled(true);
    m_previewScroll->setMouseEnabled(true);
}
void ModifyOptions::onClosed() {
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));
    m_previewBG->runAction(CCFadeTo::create(fadeTime, 0));
    m_selectionBG->runAction(CCFadeTo::create(fadeTime, 0));
    m_runsBG->runAction(CCFadeTo::create(fadeTime, 0));
    m_f0BG->runAction(CCFadeTo::create(fadeTime, 0));
    m_nbBG->runAction(CCFadeTo::create(fadeTime, 0));

    m_runLabel->runAction(CCFadeTo::create(fadeTime, 0));
    m_addRunSeperator->runAction(CCFadeTo::create(fadeTime, 0));
    m_runAmountLabel->runAction(CCFadeTo::create(fadeTime, 0));
    m_NBLabel->runAction(CCFadeTo::create(fadeTime, 0));
    m_f0AmountLabel->runAction(CCFadeTo::create(fadeTime, 0));
    m_f0Label->runAction(CCFadeTo::create(fadeTime, 0));

    Dev::fadeTextInput(m_addNewBestInput, false, fadeTime);
    Dev::fadeTextInput(m_addPercentInput, false, fadeTime);
    Dev::fadeTextInput(m_f0AmountInput, false, fadeTime);
    Dev::fadeTextInput(m_addRunEndInput, false, fadeTime);
    Dev::fadeTextInput(m_addRunStartInput, false, fadeTime);
    Dev::fadeTextInput(m_runAmountInput, false, fadeTime);

    m_myLabel->stopActionByTag(m_previewLabelFadeActionTag);
    auto labelFade = CCFadeOut::create(fadeTime);
    labelFade->setTag(m_previewLabelFadeActionTag);
    m_myLabel->runAction(labelFade);
    m_myLabel->fadeTextColorTo({255, 255, 255, 0}, fadeTime);
    m_myLabel->fadeTitleColorTo({255, 255, 255, 0}, fadeTime);
    m_myLabel->setRunMoreButtonsVisible(false, fadeTime);
    Dev::fadeTextInput(m_sessionSelector->getTextInput(), false, fadeTime);
    m_sessionSelector->setEnabled(false);

    static_cast<CCScale9Sprite*>(m_scrollbar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 0));
    static_cast<CCScale9Sprite*>(m_scrollbar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 0));
    m_scrollbar->setTouchEnabled(false);

    this->setEnabled(false);
    m_previewScroll->setMouseEnabled(false);

    m_decimalPopup->close();
}

void ModifyOptions::update(float dt) {
    float prevHeight = m_previewScroll->m_contentLayer->getContentHeight();
    m_previewScroll->m_contentLayer->updateLayout();
    if (m_previewScroll->m_contentLayer->getContentHeight() != prevHeight)
        m_previewScroll->moveToTop();
}

void ModifyOptions::updatePreviewName(bool categotyIsSession, bool f0State) {
    std::string category = !categotyIsSession ?
        "level" :
        fmt::format("session {}", DTLayer::get()->getCurrentSelectedSession());
    std::string type = f0State ?
        "from 0" :
        "runs";

    m_myLabel->setLabelName(fmt::format("preview ({}, {})", category, type));

    std::string text;

    if (!categotyIsSession) {
        if (f0State) text = "general";
        else text = "runs";
    }
    else {
        if (f0State) text = "s0";
        else text = "sruns";
    }

    m_myLabel->setLabelText(fmt::format("{{{}}}", text));
}

void ModifyOptions::addPlusMinusBtns(CCNode* around, bool flip, Function<void(bool isPlus)> callback, float scale) {
    auto plusBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    plusBtnSpr->setScale(scale);
    auto plusBtn = CCMenuItemSpriteExtra::create(
        plusBtnSpr,
        this,
        menu_selector(ModifyOptions::onPlusMinusBtn)
    );
    plusBtn->setTag(1);
    this->addChild(plusBtn);
    
    auto minusBtnSpr = CCSprite::createWithSpriteFrameName("minus_button.png"_spr);
    minusBtnSpr->setScale(scale);
    auto minusBtn = CCMenuItemSpriteExtra::create(
        minusBtnSpr,
        this,
        menu_selector(ModifyOptions::onPlusMinusBtn)
    );
    minusBtn->setTag(2);
    this->addChild(minusBtn);

    auto c1 = this->convertToNodeSpace(around->convertToWorldSpace({0,0}));
    auto c2 = this->convertToNodeSpace(around->convertToWorldSpace(around->getContentSize()));
    auto pos = this->convertToNodeSpace(around->getParent()->convertToWorldSpace(around->getPosition()));

    plusBtn->setPosition(pos +
        ccp((flip ? -1 : 1) * (std::abs(c2.x - c1.x) * around->getAnchorPoint().x + plusBtn->getContentWidth() / 2 + 5), 0)
    );

    minusBtn->setPosition(pos +
        ccp((flip ? 1 : -1) * (std::abs(c2.x - c1.x) * around->getAnchorPoint().x + minusBtn->getContentWidth() / 2 + 5), 0)
    );

    auto sharedCb = std::make_shared<Function<void(bool)>>(std::move(callback));

    m_plusMinusCallbacks.emplace(plusBtn, sharedCb);
    m_plusMinusCallbacks.emplace(minusBtn, sharedCb);

    m_holdersOfPlusMinusBtns.insert({around, std::make_pair(plusBtn, minusBtn)});
}
void ModifyOptions::onPlusMinusBtn(CCObject* sender) {
    if (auto btnSender = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
        if (!m_plusMinusCallbacks.contains(btnSender)) return;

        (*m_plusMinusCallbacks[btnSender])(btnSender->getTag() == 1);
    }
}

void ModifyOptions::removeNB(CCObject*) {
    std::optional<int> sessionNum = std::nullopt;
    if (m_sessLvlSwitcher->getCurrentOptionIndex() == 1)
        sessionNum = m_sessionSelector->getCurrentCount();

    DTLayer::get()->modifyNewBest(m_currentNBToAdd, false, sessionNum);

    if (sessionNum == std::nullopt) DTLayer::get()->m_specialStrings["general"]->updateContent();
    else DTLayer::get()->m_specialStrings["s0"]->updateContent();
}
void ModifyOptions::addNB(CCObject*) {
    std::optional<int> sessionNum = std::nullopt;
    if (m_sessLvlSwitcher->getCurrentOptionIndex() == 1)
        sessionNum = m_sessionSelector->getCurrentCount();

    DTLayer::get()->modifyNewBest(m_currentNBToAdd, true, sessionNum);

    if (sessionNum == std::nullopt) DTLayer::get()->m_specialStrings["general"]->updateContent();
    else DTLayer::get()->m_specialStrings["s0"]->updateContent();
}

void ModifyOptions::removeInvalidRuns(CCObject*) {
    std::optional<int> sessionNum = std::nullopt;
    if (m_sessLvlSwitcher->getCurrentOptionIndex() == 1)
        sessionNum = m_sessionSelector->getCurrentCount();

    auto togglerState = m_runF0Toggler->isToggled();

    auto task = arc::spawn(DTLayer::get()->getTFor<Deaths>([togglerState](GeneralData const& data) {
        return togglerState ? data.m_runs.toLegacyDeaths() : data.m_deaths.toLegacyDeaths();
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, sessionNum.has_value(), true));

    auto deathsRes = task.blockOn();
    if (deathsRes.isErr()) return;

    Deaths toEdit = std::move(deathsRes).unwrap();

    std::vector<std::pair<Run, int>> runs{};

    bool from0Maybe = false;

    for (const auto& [deathStr, count] : toEdit) {
        auto splitRes = StatsManager::splitRunKey(deathStr);
        if (splitRes.isErr()) continue;
        auto split = splitRes.unwrap();

        if (split.m_start == std::nullopt) from0Maybe = true;


        if (split.m_start.has_value() && ((split.m_start.value() < 0 || split.m_start.value() > 100) || (split.m_end < 0 || split.m_end > 100))) {
            runs.push_back({split, count});
            continue;
        }
        else if (!split.m_start.has_value() && split.m_end < 0 || split.m_end > 100) {
            runs.push_back({split, count});
            continue;
        }
    }

    for (const auto& [run, deathCount] : runs)
    {
        if (from0Maybe) {
            DTLayer::get()->modifyRun(run.m_end, -deathCount, sessionNum);
        }
        else {
            DTLayer::get()->modifyRun(run.m_start.value_or(-1), run.m_end, -deathCount, sessionNum);
        }
    }
    
    if (sessionNum == std::nullopt) DTLayer::get()->m_specialStrings["runs"]->updateContent();
    else DTLayer::get()->m_specialStrings["sruns"]->updateContent();
    if (sessionNum == std::nullopt) DTLayer::get()->m_specialStrings["general"]->updateContent();
    else DTLayer::get()->m_specialStrings["s0"]->updateContent();
}
