#include "RunOptions.hpp"

#include <nodes/DTLayer.hpp>
#include <utils/Dev.hpp>
#include <nodes/Tutorial/TutorialButton.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>
#include <nodes/general/SimpleToggler.hpp>
#include <nodes/options/pages/run/PercentCell.hpp>
#include <managers/StatsManager.hpp>

RunOptions* RunOptions::create(CCSize size) {
    auto ret = new RunOptions();

    if (ret && ret->initWithSetup(std::move(size))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool RunOptions::setup() {
    auto TARLabel = geode::Label::create("Show any run", "gjFont17.fnt");
    TARLabel->setAlignment(geode::Label::Alignment::Left);
    TARLabel->setAnchorPoint({.5f, .5f});
    TARLabel->setScale(.6f);
    TARLabel->setPosition({m_size.width / 4, m_size.height - TARLabel->getScaledContentHeight() / 2});
    this->addChild(TARLabel);

    auto dtlayer = DTLayer::get();

    auto TARToggler = SimpleToggler::createWithDefaults(
        .75f,
        dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().m_metadata.m_showAnyRun : false)
    );
    TARToggler->setCallback([&](bool isToggled) {
        auto dtlayer = DTLayer::get();
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.m_metadata.m_showAnyRun = isToggled;
        (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);

        dtlayer->foreachLinkedLevel([&isToggled](auto& lvlData) {
            lvlData.m_metadata.m_showAnyRun = isToggled;
            (void)StatsManager::setMetadata(lvlData.m_metadata, lvlData.m_levelKey);
        });

        DTLayer::get()->m_specialStrings["runs"]->updateContent();
        DTLayer::get()->m_specialStrings["sruns"]->updateContent();
    });
    float offset = TARToggler->getContentWidth() / 4 + 5;
    TARToggler->setPosition(TARLabel->getPosition() - ccp(TARLabel->getScaledContentWidth() / 2 + offset, 0));
    TARLabel->setPositionX(TARLabel->getPositionX() + offset);
    this->addChild(TARToggler);

    m_runAdditionInput = TextInput::create(m_size.width / 4.f, "Start %");
    m_runAdditionInput->setPosition({m_size.width / 4, TARLabel->getPositionY() - m_runAdditionInput->getContentHeight()});
    m_runAdditionInput->setCommonFilter(CommonFilter::Uint);
    m_runAdditionInput->setCallback([&](const std::string& newText) {
        auto numRes = geode::utils::numFromString<int>(newText);

        if (numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        m_runAdditionInput->setString(std::to_string(num));
    });
    this->addChild(m_runAdditionInput);

    auto startPosBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto spSpr = CCSprite::createWithSpriteFrameName("edit_eStartPosBtn_001.png");
    spSpr->setScale(.75f);
    startPosBtnSpr->setCascadeOpacityEnabled(true);
    startPosBtnSpr->setScale(.6f);
    startPosBtnSpr->addChild(spSpr);
    spSpr->setPosition(startPosBtnSpr->getContentSize() / 2);
    auto startPosBtn = CCMenuItemSpriteExtra::create(
        startPosBtnSpr,
        this,
        menu_selector(RunOptions::onStartPoses)
    );
    startPosBtn->setPosition(m_runAdditionInput->getPosition() - ccp(m_runAdditionInput->getContentWidth() / 2 + startPosBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(startPosBtn);

    auto plusBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    auto plusBtn = CCMenuItemSpriteExtra::create(
        plusBtnSpr,
        this,
        menu_selector(RunOptions::onAddNewRun)
    );
    plusBtn->setPosition(m_runAdditionInput->getPosition() + ccp(m_runAdditionInput->getScaledContentWidth() / 2 + plusBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(plusBtn);

    float offFromBottom = 45;

    auto runsMenuBG = CCScale9Sprite::create("GJ_square06.png");
    runsMenuBG->setContentSize({m_size.width / 2 / 1.1f, m_runAdditionInput->getPositionY() - m_runAdditionInput->getContentHeight() / 2 - offFromBottom});
    runsMenuBG->setPosition({abs(runsMenuBG->getContentWidth() / 2 - m_runAdditionInput->getPositionX()), offFromBottom});
    runsMenuBG->setAnchorPoint({0, 0});
    runsMenuBG->setColor({75, 75, 75});
    runsMenuBG->setOpacity(100);
    this->addChild(runsMenuBG);

    m_runsScrollLayer = ScrollLayer::create(runsMenuBG->getContentSize() - ccp(10, 10));
    m_runsScrollLayer->setAnchorPoint({0, 0});
    m_runsScrollLayer->setPosition(runsMenuBG->getPosition());
    m_runsScrollLayer->setPosition(m_runsScrollLayer->getPosition() + ccp(5, 5));
    m_runsScrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(m_runsScrollLayer->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
    );
    this->addChild(m_runsScrollLayer);

    if (dtlayer != nullptr && dtlayer->m_MyLevelStats.isOk()) {
        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        for (const auto& [startPercent, maxToShow] : stats.m_metadata.m_runsToShow)
        {
            createRunCell(startPercent, maxToShow);
        }
    }

    m_runsScrollBar = Scrollbar::create(m_runsScrollLayer);
    m_runsScrollBar->setPosition(m_runsScrollLayer->getPosition() + ccp(m_runsScrollLayer->getScaledContentWidth() + m_runsScrollBar->getContentWidth() / 2, 0));
    m_runsScrollBar->ignoreAnchorPointForPosition(true);
    this->addChild(m_runsScrollBar);

    m_sharedHideScroll = Slider::create(this, menu_selector(RunOptions::onSharedHideScroll), .75f);
    m_sharedHideScroll->setPosition({
        m_runsScrollLayer->getPositionX() + m_runsScrollLayer->getScaledContentWidth() / 2 - 22,
        m_sharedHideScroll->m_sliderBar->getScaledContentHeight() / 2 + 5
    });
    m_sharedHideScroll->setValue(dtlayer == nullptr ? 0 : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().m_metadata.m_sharedRunToShow / 100.0f : 0));
    m_sharedHideScroll->setContentSize({0, 0});
    m_sharedHideScroll->m_delegate = this;
    this->addChild(m_sharedHideScroll);

    auto sharedHideLabel = geode::Label::create("Shared Max To Hide", "gjFont17.fnt");
    sharedHideLabel->setLimitLabelWidth(m_size.width / 2 - m_sharedHideScroll->getContentWidth() - 10, .5f);
    sharedHideLabel->setPosition(m_sharedHideScroll->getPosition() + ccp(0, m_sharedHideScroll->m_sliderBar->getScaledContentHeight() / 2 + 5));
    sharedHideLabel->setAnchorPoint({.5f, 0});
    this->addChild(sharedHideLabel);

    m_sharedRunsToShowInput = TextInput::create(45, "%");
    m_sharedRunsToShowInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().m_metadata.m_sharedRunToShow) : ""));
    m_sharedRunsToShowInput->setCommonFilter(CommonFilter::Uint);
    m_sharedRunsToShowInput->setPosition(m_sharedHideScroll->getPosition() + ccp(110, 10));
    m_sharedRunsToShowInput->setCallback([&](const std::string& newText) {
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        m_sharedRunsToShowInput->setString(std::to_string(num));

        m_sharedHideScroll->setValue(num / 100.0f);
        for (const auto& child : m_runsScrollLayer->m_contentLayer->getChildrenExt<PercentCell*>())
        {
            child->setMaxToHide(m_sharedHideScroll->getValue());
        }
        sliderEnded(m_sharedHideScroll);

        dtlayer->m_specialStrings["runs"]->updateContent();
        dtlayer->m_specialStrings["sruns"]->updateContent();
    });
    this->addChild(m_sharedRunsToShowInput);


    auto seperator = CCScale9Sprite::create("square.png");
    seperator->setScale(.5f);
    seperator->setPosition(m_size / 2);
    seperator->setContentSize(ccp(1, m_size.height / 1.15f) / seperator->getScale());
    this->addChild(seperator);

    float rightOffset = 5;

    m_HidUpToInput = TextInput::create(45, "%");
    m_HidUpToInput->setPosition({m_size.width - m_HidUpToInput->getContentWidth() / 2 - rightOffset, m_size.height / 8 * 6});
    m_HidUpToInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().m_metadata.m_hideUpto) : ""));
    m_HidUpToInput->setCommonFilter(CommonFilter::Uint);
    m_HidUpToInput->setCallback([&](const std::string& newText) {
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);
        
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;
        
        int num = numRes.unwrap();
        num = std::min(num, 100);
        m_HidUpToInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.m_metadata.m_hideUpto = num;
        (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);

        dtlayer->m_specialStrings["general"]->updateContent();
    });
    this->addChild(m_HidUpToInput);

    auto HideUpToLabel = geode::Label::create("Hide Up To", "gjFont17.fnt");
    HideUpToLabel->setLimitLabelWidth(m_size.width / 2 - m_HidUpToInput->getContentWidth() - rightOffset, .65f);
    HideUpToLabel->setPosition(m_HidUpToInput->getPosition() - ccp(m_HidUpToInput->getContentWidth() / 2 + rightOffset, 0));
    HideUpToLabel->setAnchorPoint({1, .5f});
    this->addChild(HideUpToLabel);

    m_RealEndPerInput = TextInput::create(45, "E %");
    m_RealEndPerInput->setPosition({m_size.width - m_RealEndPerInput->getContentWidth() / 2 - rightOffset, m_size.height / 8 * 4});
    m_RealEndPerInput->setString(dtlayer == nullptr ? "" : (dtlayer->m_MyLevelStats.isOk() ? std::to_string(dtlayer->m_MyLevelStats.unwrap().m_metadata.m_realEndPercent) : ""));
    m_RealEndPerInput->setCommonFilter(CommonFilter::Uint);
    m_RealEndPerInput->setCallback([&](const std::string& newText) {
        auto dtlayer = DTLayer::get();
        auto numRes = geode::utils::numFromString<int>(newText);

        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

        int num = numRes.unwrap();
        num = std::min(num, 100);
        m_RealEndPerInput->setString(std::to_string(num));

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.m_metadata.m_realEndPercent = num;
        (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);

        DTLayer::get()->m_specialStrings["runs"]->updateContent();
        DTLayer::get()->m_specialStrings["sruns"]->updateContent();
        DTLayer::get()->m_specialStrings["general"]->updateContent();
        DTLayer::get()->m_specialStrings["s0"]->updateContent();
    });
    this->addChild(m_RealEndPerInput);

    auto RealEndPerLabel = geode::Label::create("Real End %", "gjFont17.fnt");
    RealEndPerLabel->setLimitLabelWidth(m_size.width / 2 - m_RealEndPerInput->getContentWidth() - rightOffset, .65f);
    RealEndPerLabel->setPosition(m_RealEndPerInput->getPosition() - ccp(m_RealEndPerInput->getContentWidth() / 2 + rightOffset, 0));
    RealEndPerLabel->setAnchorPoint({1, .5f});
    this->addChild(RealEndPerLabel);

    auto ResetAsDeathToggler = SimpleToggler::createWithDefaults(
        .75f,
        dtlayer == nullptr ? false : (dtlayer->m_MyLevelStats.isOk() ? dtlayer->m_MyLevelStats.unwrap().m_metadata.m_resetAsDeath : false)
    );
    ResetAsDeathToggler->setCallback([&](bool isToggled) {
        auto dtlayer = DTLayer::get();
        if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        stats.m_metadata.m_resetAsDeath = isToggled;
        (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);
    });
    ResetAsDeathToggler->setPosition({m_size.width - ResetAsDeathToggler->getContentWidth() / 2 - rightOffset, m_size.height / 8 * 2});
    this->addChild(ResetAsDeathToggler);

    auto ResetAsDeathLabel = geode::Label::create("Reset as Death", "gjFont17.fnt");
    ResetAsDeathLabel->setLimitLabelWidth(m_size.width / 2 - ResetAsDeathToggler->getContentWidth() - rightOffset, .65f);
    ResetAsDeathLabel->setPosition(ResetAsDeathToggler->getPosition() - ccp(ResetAsDeathToggler->getContentWidth() / 2 + rightOffset, 0));
    ResetAsDeathLabel->setAnchorPoint({1, .5f});
    this->addChild(ResetAsDeathLabel);

    auto runsHidingTutorial = TutorialButton::create(.75f, "run-hiding", [&, sharedHideLabel, TARToggler, TARLabel, plusBtn](DTTutorialLayer* tutorial) {
        int randomPer = 7 + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (95.f - 7.f);
        int randomMaxToHide = (randomPer + 2) + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (93.f - (randomPer + 2.f));

        auto emptyPer = PercentCell::create(
            m_runsScrollLayer->m_contentLayer->getContentWidth(),
            randomPer, 
            randomMaxToHide, 
            CCSprite::createWithSpriteFrameName("minus_button.png"_spr),
            [](PercentCell* _) {}
        );
        emptyPer->setPositionX(this->getContentWidth() / 2);
        emptyPer->setAnchorPoint({.5f, 0});
        emptyPer->hideInstant();
        emptyPer->setScale(1.5f);
        this->addChild(emptyPer);

        tutorial->appendDialogue("In here you can manage which runs you want to see! to clear up clutter!", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("Because sometimes it becomes hard to find the death you want", TutorialCharacterFace::TCFBored)
            ->appendDialogue("First you have <cy>\"Show any run\"</c>, this will tell death tracker to <cg>display all the runs</c> you have done", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)
            ->joinHighlight(TARToggler)
            ->joinHighlight(TARLabel)
            ->appendDialogue("if this is enabled, the runs you pick below <cd>will not effect anything</c>.", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(TARToggler)
            ->joinHighlight(TARLabel)
            ->appendDialogue("Now for addding runs you have this <cc>text input</c> and <cg>button</c>", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPBottomRight, .75f)
            ->joinHighlight(m_runAdditionInput)
            ->joinHighlight(plusBtn)
            ->appendDialogue("You write the <cy>starting percentage</c> of the run you wanna see into the <cc>text input</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_runAdditionInput)
            ->appendDialogue("And click the <cg>plus button</c> to confirm and add that percent to your list of showen percentages!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(plusBtn)
            ->appendDialogue("If you have <cy>\"Show any run\"</c> <cr>disabled</c>, you will <cj>only see the runs that start from any percentage on this list</c>!", TutorialCharacterFace::TCFNormalTilted)
            ->joinTransform(TutorialBoxPlacement::TBPTopRight, .75f)
            ->joinHighlight(m_runsScrollLayer)
            ->appendDialogue("In this list you have a few options per percent", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_runsScrollLayer)
            ->appendDialogue("You can <cr>remove</c> the percent from the list", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPTop, .75f)
            ->joinHighlight(emptyPer->getChildByID("btn"))
            ->joinCallback([emptyPer]() {emptyPer->show();}, true)
            ->appendDialogue("And you can edit the <cy>\"Max to Hide\"</c> percent.", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(emptyPer->getSilderBG())
            ->joinHighlight(emptyPer->getSilderHandle())
            ->joinHighlight(emptyPer->getChildByID("mth-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"))
            ->appendDialogue("The number you input here will <cy>hide any run</c> thats between <cp>that number</c> and the <cd>original percentage</c>.", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(emptyPer->getSilderBG())
            ->joinHighlight(emptyPer->getSilderHandle())
            ->joinHighlight(emptyPer->getChildByID("mth-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"))
            ->appendDialogue(fmt::format("For example, in this case it will hide all runs <cd>starting from {}</c> and <cp>ending below {}</c>", randomPer, randomMaxToHide), TutorialCharacterFace::TCFNormal)
            ->joinHighlight(emptyPer->getChildByID("percent-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"), 1.5f)
            ->appendDialogue(fmt::format("So <cd>{}-{}</c> wont be seen but <cy>{}-{}</c> will be!", randomPer, randomMaxToHide - 2, randomPer, randomMaxToHide), TutorialCharacterFace::TCFHappy)
            ->joinHighlight(emptyPer->getChildByID("percent-label"))
            ->joinHighlight(emptyPer->getChildByID("mth-input"))
            ->appendDialogue("You also have the <cy>\"Shared Max To Hide\"</c>, to more easily edit all the <cy>\"Max to Hide\"</c>s and more", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinHighlight(m_sharedHideScroll->m_sliderBar)
            ->joinHighlight(m_sharedHideScroll->m_groove)
            ->joinHighlight(sharedHideLabel)
            ->joinHighlight(m_sharedRunsToShowInput)
            ->joinCallback([emptyPer]() {emptyPer->hide();}, true)
            ->joinCallback([emptyPer]() {emptyPer->removeMeAndCleanup();}, false)
            ->appendDialogue("If you edit this, it will edit all <cy>\"Max To Hide\"</c>s with it relative to the slider location!", TutorialCharacterFace::TCFHappy)
            ->joinPreviousHighlight()
            ->appendDialogue("And if you dont have any specific runs to filter, it will act as a global <cy>\"Max To Hide\"</c> of sorts", TutorialCharacterFace::TCFNormalTilted)
            ->joinPreviousHighlight()
            ->appendDialogue("Allowing you to filter out runs by length!", TutorialCharacterFace::TCFNormal)
            ->joinPreviousHighlight()
            ->appendDialogue("Hope this feature helps you <cy>improve clutter</c> in your death tracker! :D", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    runsHidingTutorial->setPosition(TARLabel->getPosition() + ccp(15 + TARLabel->getScaledContentWidth() / 2, 0));
    this->addChild(runsHidingTutorial);

    auto runsOtherTutorial = TutorialButton::create(.75f, "run-overall", [&, HideUpToLabel, ResetAsDeathToggler, ResetAsDeathLabel, RealEndPerLabel](DTTutorialLayer* tutorial) {
        tutorial->appendDialogue("Here we have some extra settings for run managment", TutorialCharacterFace::TCFNormalTilted)
            ->appendDialogue("We have <cy>\"Hide Up To\"</c>, which like <cy>\"Max to Hide\"</c> on runs, will hide all runs up to that percent but from 0", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .70f)
            ->joinHighlight(m_HidUpToInput)
            ->joinHighlight(HideUpToLabel)
            ->appendDialogue("There is <cy>\"Real End Percent\"</c>, which allows you to make near ending deaths count as 100% runs", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(m_RealEndPerInput)
            ->joinHighlight(RealEndPerLabel)
            ->appendDialogue("This is useful in the case of a long end screen and such", TutorialCharacterFace::TCFHappy)
            ->joinPreviousHighlight()
            ->appendDialogue("And we also have <cy>\"Reset as death\"</c>! which makes resetting count as if you died, tracking it!", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(ResetAsDeathToggler)
            ->joinHighlight(ResetAsDeathLabel);
    });
    runsOtherTutorial->setPosition(m_size + ccp(-15, -15));
    this->addChild(runsOtherTutorial);

    this->setOpacity(0);

    this->addEventListener(
        KeybindSettingPressedEvent(
            Mod::get(),
            "enter-new-run-per"
        ),
        [&](const Keybind& keybind, bool down, bool repeat, double) {
            if (down && m_runAdditionInput->isFocused()) {
                onAddNewRun(nullptr);
            }
        }
    );

    static_cast<CCScale9Sprite*>(m_runsScrollBar->getChildren()->objectAtIndex(0))->setOpacity(0);
    static_cast<CCScale9Sprite*>(m_runsScrollBar->getChildren()->objectAtIndex(1))->setOpacity(0);
    m_runsScrollBar->setTouchEnabled(false);

    return true;
}

void RunOptions::onOpened() {
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));

    m_runAdditionInput->getContentLabel()->setOpacity(0);
    Dev::fadeTextInput(m_runAdditionInput, true, fadeTime);
    m_RealEndPerInput->getContentLabel()->setOpacity(0);
    Dev::fadeTextInput(m_RealEndPerInput, true, fadeTime);
    m_HidUpToInput->getContentLabel()->setOpacity(0);
    Dev::fadeTextInput(m_HidUpToInput, true, fadeTime);
    Dev::fadeTextInput(m_sharedRunsToShowInput, true, fadeTime);
    Dev::fadeSlider(m_sharedHideScroll, true, fadeTime);

    for (const auto& child : CCArrayExt<PercentCell*>(m_runsScrollLayer->m_contentLayer->getChildren())) {
        child->show();
    }

    static_cast<CCScale9Sprite*>(m_runsScrollBar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 150));
    static_cast<CCScale9Sprite*>(m_runsScrollBar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 255));
    m_runsScrollBar->setTouchEnabled(true);

    this->setEnabled(true);
}
void RunOptions::onClosed() {
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));

    Dev::fadeTextInput(m_runAdditionInput, false, fadeTime);
    Dev::fadeTextInput(m_RealEndPerInput, false, fadeTime);
    Dev::fadeTextInput(m_HidUpToInput, false, fadeTime);
    Dev::fadeTextInput(m_sharedRunsToShowInput, false, fadeTime);
    Dev::fadeSlider(m_sharedHideScroll, false, fadeTime);

    for (const auto& child : CCArrayExt<PercentCell*>(m_runsScrollLayer->m_contentLayer->getChildren())) {
        child->hide();
    }

    static_cast<CCScale9Sprite*>(m_runsScrollBar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 0));
    static_cast<CCScale9Sprite*>(m_runsScrollBar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 0));
    m_runsScrollBar->setTouchEnabled(false);

    this->setEnabled(false);
}

void RunOptions::onAddNewRun(CCObject*) {
    auto dtlayer = DTLayer::get();
    auto numRes = geode::utils::numFromString<int>(m_runAdditionInput->getString());

    if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr() || numRes.isErr()) return;

    int num = numRes.unwrap();

    addNewRun(num);
}

bool RunOptions::addNewRun(int percent) {
    auto dtlayer = DTLayer::get();
    if (dtlayer == nullptr) return false;

    auto& stats = dtlayer->m_MyLevelStats.unwrap();
    if (stats.m_metadata.m_runsToShow.contains(percent)) return false;
    
    stats.m_metadata.m_runsToShow.insert({percent, percent});

    (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);

    DTLayer::get()->m_specialStrings["runs"]->updateContent();
    DTLayer::get()->m_specialStrings["sruns"]->updateContent();

    dtlayer->foreachLinkedLevel([&](auto& lvlData) {
        if (lvlData.m_metadata.m_runsToShow.contains(percent)) return;

        lvlData.m_metadata.m_runsToShow.insert({percent, percent});
        (void)StatsManager::setMetadata(lvlData.m_metadata, lvlData.m_levelKey);
    });

    createRunCell(percent, percent);

    return true;
}

void RunOptions::PercentCellClicked(PercentCell* cell) {
    auto dtlayer = DTLayer::get();

    if (dtlayer != nullptr) {
        int percent = cell->getPercent();

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        if (stats.m_metadata.m_runsToShow.contains(percent)) {
            stats.m_metadata.m_runsToShow.erase(percent);
            (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);

            DTLayer::get()->m_specialStrings["runs"]->updateContent();
            DTLayer::get()->m_specialStrings["sruns"]->updateContent();
        }

        dtlayer->foreachLinkedLevel([&](auto& lvlData) {
            if (!lvlData.m_metadata.m_runsToShow.contains(percent)) return;

            lvlData.m_metadata.m_runsToShow.erase(percent);
            (void)StatsManager::setMetadata(lvlData.m_metadata, lvlData.m_levelKey);
        });
    }

    cell->removeMeAndCleanup();
    m_runsScrollLayer->m_contentLayer->updateLayout();
}

void RunOptions::PercentMaxHideValChanged(PercentCell* cell) {
    auto dtlayer = DTLayer::get();

    if (dtlayer != nullptr) {
        int percent = cell->getPercent();

        auto& stats = dtlayer->m_MyLevelStats.unwrap();
        if (stats.m_metadata.m_runsToShow.contains(percent)) {
            stats.m_metadata.m_runsToShow[percent] = cell->getMaxToHide();
            (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);

            DTLayer::get()->m_specialStrings["runs"]->updateContent();
            DTLayer::get()->m_specialStrings["sruns"]->updateContent();
        }

        dtlayer->foreachLinkedLevel([&](auto& lvlData) {
            lvlData.m_metadata.m_runsToShow[percent] = cell->getMaxToHide();
            (void)StatsManager::setMetadata(lvlData.m_metadata, lvlData.m_levelKey);
        });
    }
}

void RunOptions::createRunCell(int percent, int maxToHide) {
    auto percentCell = PercentCell::create(
        m_runsScrollLayer->m_contentLayer->getContentWidth(),
        percent,
        maxToHide,
        CCSprite::createWithSpriteFrameName("minus_button.png"_spr),
        [&](PercentCell* cell) { RunOptions::PercentCellClicked(cell); }
    );
    percentCell->setOnMaxToHideChangedCallback([&](PercentCell* cell) { RunOptions::PercentMaxHideValChanged(cell); });
    m_runsScrollLayer->m_contentLayer->addChild(percentCell);

    m_runsScrollLayer->m_contentLayer->updateLayout();
}

void RunOptions::onStartPoses(CCObject*) {
    auto currLvl = PlayLayer::get();
    if (currLvl == nullptr) {
        FLAlertLayer::create("Cant add StartPos runs", "You must enter the level itself to automatically add StartPos runs!", "OK")->show();
        return;
    }

    int anythingAdded = false;
    bool foundStartPoses = false;
    
    for (auto child : currLvl->m_objects->asExt<GameObject*>()) {
        if (auto sp = typeinfo_cast<StartPosObject*>(child)) {
            float LLength = PlayLayer::get()->m_levelLength;
            float startPosX = sp->getPositionX();

            if (currLvl->m_level->m_timestamp > 0) {
                float startPosTime = PlayLayer::get()->timeForPos({startPosX, 0}, 0, 0, true, 0);
                float LTime = PlayLayer::get()->timeForPos({LLength, 0}, 0, 0, true, 0);
                
                if (addNewRun(startPosTime / LTime * 100))
                    anythingAdded = true;
            } else {
                if (addNewRun(startPosX / LLength * 100))
                    anythingAdded = true;
            }

            foundStartPoses = true;
        }
    }

    if (!foundStartPoses) {
        Notification::create("No StartPoses detected!", NotificationIcon::Info)->show();
    }
    else if (anythingAdded)
        Notification::create("Added StartPos runs!", NotificationIcon::Success)->show();
    else
        Notification::create("StartPos runs were already added", NotificationIcon::Info)->show();

    m_runsScrollLayer->m_contentLayer->updateLayout();
}


void RunOptions::onSharedHideScroll(CCObject*) {
    for (const auto& child : m_runsScrollLayer->m_contentLayer->getChildrenExt<PercentCell*>())
    {
        child->setMaxToHide(m_sharedHideScroll->getValue());
    }

    m_sharedRunsToShowInput->setString(std::to_string(static_cast<int>(m_sharedHideScroll->getValue() * 100)));
}

void RunOptions::sliderEnded(Slider* slider) {
    for (const auto& child : m_runsScrollLayer->m_contentLayer->getChildrenExt<PercentCell*>())
    {
        child->sliderEnded(slider);
    }

    auto dtlayer = DTLayer::get();

    if (dtlayer == nullptr || dtlayer->m_MyLevelStats.isErr()) return;

    auto& stats = dtlayer->m_MyLevelStats.unwrap();
    stats.m_metadata.m_sharedRunToShow = static_cast<int>(m_sharedHideScroll->getValue() * 100);
    (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);

    dtlayer->m_specialStrings["runs"]->updateContent();
    dtlayer->m_specialStrings["sruns"]->updateContent();
}