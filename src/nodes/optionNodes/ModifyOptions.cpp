#include <nodes/optionNodes/ModifyOptions.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <utils/Dev.hpp>
#include <hooks/DTCCTextInputNode.hpp>

ModifyOptions* ModifyOptions::create(const CCSize& size) {
    auto ret = new ModifyOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ModifyOptions::setup() {

    this->setEnabled(false);
    this->setOpacity(0);

    float topeToggleHightPadding = 5;

    auto dtLayer = DTLayer::get();

    float previewBGMargin = 10;

    auto previewBGHolder = CCNode::create();
    this->addChild(previewBGHolder);

    previewBG = CCScale9Sprite::create("square02_001.png");
    previewBG->setContentSize({
        size.width / 2.5f - previewBGMargin,
        size.height - previewBGMargin
    });
    previewBG->setPosition({
        size.width - previewBG->getScaledContentWidth() / 2 - previewBGMargin / 2,
        0 + previewBGMargin / 2
    });
    previewBG->setAnchorPoint({0.5f, 0});
    previewBG->setOpacity(0);
    previewBGHolder->addChild(previewBG);

    previewScroll = ScrollLayer::create(previewBG->getContentSize() - ccp(5, 5));
    previewScroll->setPosition(previewBG->getPosition() + ccp(0, 2.5f));
    previewScroll->ignoreAnchorPointForPosition(false);
    previewScroll->setAnchorPoint({.5f, 0});
    previewScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(previewScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    previewScroll->setMouseEnabled(false);
    this->addChild(previewScroll);

    scrollbar = Scrollbar::create(previewScroll);
    scrollbar->ignoreAnchorPointForPosition(false);
    scrollbar->setPosition(previewScroll->getPosition() + previewScroll->getContentSize() / 2);
    this->addChild(scrollbar);

    DTLabelInfo info;
    info.scale = .5f;
    info.wrapping = WrappingMode::SPACE_WRAP;
    info.isExpanded = true;

    myLabel = DTLabel::create(info);
    myLabel->ignoreExtraSettings = true;
    myLabel->setExpandable(false);
    myLabel->setContentWidth(previewScroll->getContentWidth());
    previewScroll->m_contentLayer->addChild(myLabel);

    previewScroll->m_contentLayer->updateLayout();
    previewScroll->moveToTop();

    selectionBG = CCScale9Sprite::create("square02_small.png");
    selectionBG->setContentWidth(size.width - previewScroll->getScaledContentWidth() - previewBGMargin - 10);
    selectionBG->setContentHeight(35);
    selectionBG->setAnchorPoint({0, 1});
    selectionBG->setCascadeColorEnabled(false);
    selectionBG->setPosition({
        previewBGMargin / 2,
        size.height - previewBGMargin / 2
    });
    this->addChild(selectionBG);

    sessLvlSwitcher = SwitcherButton::create({
        65,
        selectionBG->getContentHeight() - 15
    }, "GJ_button_01.png", {
        "Level",
        "Session"
    });
    sessLvlSwitcher->setPosition(selectionBG->getPosition() + ccp(
        sessLvlSwitcher->getScaledContentWidth() / 2 + 5,
        -sessLvlSwitcher->getScaledContentHeight() / 2 - 15 / 2
    ));
    this->addChild(sessLvlSwitcher);

    runF0Toggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("from-zero.png"_spr),
        CCSprite::createWithSpriteFrameName("run.png"_spr),
        .75f
    );
    runF0Toggler->setPosition(selectionBG->getPosition() + selectionBG->getScaledContentSize() + ccp(
        -runF0Toggler->getScaledContentWidth() / 2 - 5,
        -selectionBG->getScaledContentHeight() * 1.5
    ));
    this->addChild(runF0Toggler);

    auto currentRunModeLabel = CCLabelBMFont::create("From 0", "bigFont.fnt");
    currentRunModeLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    currentRunModeLabel->setAnchorPoint({1, .5f});
    currentRunModeLabel->setPosition(runF0Toggler->getPosition() + ccp(
        -5 - runF0Toggler->getScaledContentWidth() / 2,
        0
    ));
    currentRunModeLabel->setScale(.5f);
    this->addChild(currentRunModeLabel);

    runF0Toggler->setCallback([&, currentRunModeLabel](auto state) {
        currentRunModeLabel->setString(
            state ?
                "Runs" :
                "From 0"
        );

        updatePreviewName(sessLvlSwitcher->getCurrentOptionIndex() == 1, !state);
    });

    sessLvlSwitcher->setOptionChangedCallback([&](auto option) {
        updatePreviewName(option == 1, !runF0Toggler->isToggled());
    });

    int sessionAmount = 1;
    if  (dtLayer != nullptr) {
        sessionAmount = dtLayer->getCurrentGrouping().grouping.size();
    }

    sessionSelector = SessionSelector::create(sessionAmount);
    sessionSelector->setCallback([&](int newSession) {
        DTLayer::get()->onSessionSelected(newSession, true);
        if (!runF0Toggler->isToggled())
            updatePreviewName(sessLvlSwitcher->getCurrentOptionIndex() == 1, !runF0Toggler->isToggled());
    });
    sessionSelector->setScale(.5f);
    sessionSelector->ignoreAnchorPointForPosition(false);
    sessionSelector->setCurrentCount(DTLayer::get()->getCurrentSelectedSession(), true);
    sessionSelector->setPosition(currentRunModeLabel->getPosition() + ccp(
        -currentRunModeLabel->getScaledContentWidth() - 5 - sessionSelector->getScaledContentWidth() / 2,
        0
    ));
    this->addChild(sessionSelector);

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
    removeInvalidRunsBtn->setPosition(selectionBG->getPosition() + ccp(
        0,
        -selectionBG->getScaledContentHeight() - removeInvalidRunsBtn->getScaledContentHeight() / 2 - 10
    ));
    this->addChild(removeInvalidRunsBtn);

    float diffBetweenSquares = 5;
    float squaresHeight = 135;

    runsBG = CCScale9Sprite::create("square02_001.png");
    runsBG->setContentWidth(selectionBG->getScaledContentWidth() / 3 - diffBetweenSquares);
    runsBG->setContentHeight(squaresHeight);
    runsBG->setCascadeColorEnabled(false);
    runsBG->setAnchorPoint({0, 0});
    runsBG->setPosition({
        previewBGMargin / 2,
        previewBGMargin / 2
    });
    this->addChild(runsBG);

    runAmountInput = TextInput::create(runsBG->getContentWidth() / 1.2f, "amount", "gjFont17.fnt");
    runAmountInput->setPosition({
        runsBG->getContentWidth() / 2,
        runAmountInput->getContentHeight() / 2 + 2
    });
    runAmountInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            runAmountInput->setString("100");
        }

        currRunsAmount = num;
    });
    runAmountInput->setString(std::to_string(currRunsAmount));
    runAmountInput->setCommonFilter(CommonFilter::Uint);
    runsBG->addChild(runAmountInput);

    runAmountLabel = CCLabelBMFont::create("Amount", "bigFont.fnt");
    runAmountLabel->setScale(.5f);
    runAmountLabel->setAnchorPoint({0.5f, 1});
    runAmountLabel->setPosition(runAmountInput->getPosition() + ccp(
        0,
        runAmountLabel->getScaledContentHeight() + runAmountInput->getScaledContentHeight() / 2
    ));
    runsBG->addChild(runAmountLabel);

    auto runPMAlignNode = CCNode::create();
    runPMAlignNode->setContentSize({
        runsBG->getContentWidth() / 10.f,
        25
    });
    runPMAlignNode->setAnchorPoint({.5f, .5f});
    runPMAlignNode->setPosition(runAmountLabel->getPosition() + ccp(
        0,
        runPMAlignNode->getScaledContentHeight() / 2
    ));
    runsBG->addChild(runPMAlignNode);

    addPlusMinusBtns(runPMAlignNode, false, [&](bool isPlus) {
        if (currentStartRunToAdd > currentEndRunToAdd) {
            auto tint = CCTintTo::create(.2f, 255, 0, 0);
            auto seq = CCSequence::create(
                CCTintTo::create(.2f, 255, 0, 0),
                CCTintTo::create(.2f, 255, 255, 255),
                nullptr
            );
            seq->setTag(8);
            addRunStartInput->getInputNode()->m_textLabel->stopActionByTag(8);
            addRunStartInput->getInputNode()->m_textLabel->runAction(seq);
            return;
        }

        std::optional<int> sessionNum = std::nullopt;
        if (sessLvlSwitcher->getCurrentOptionIndex() == 1)
            sessionNum = sessionSelector->getCurrentCount();

        DTLayer::get()->modifyRun(currentStartRunToAdd, currentEndRunToAdd, currRunsAmount * (isPlus ? 1 : -1), sessionNum);
        
        if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["runs"]->updateContent();
        else DTLayer::get()->specialStrings["sruns"]->updateContent();
    }, .9f);

    addRunSeperator = CCLabelBMFont::create("-", "bigFont.fnt");
    addRunSeperator->setPosition(runPMAlignNode->getPosition() + ccp(
        0,
        runPMAlignNode->getScaledContentHeight() / 2 + addRunSeperator->getScaledContentHeight() / 2
    ));
    addRunSeperator->setScale(.35f);
    runsBG->addChild(addRunSeperator);

    addRunStartInput = TextInput::create(30, "S%", "gjFont17.fnt");
    addRunStartInput->setPosition(addRunSeperator->getPosition() + ccp(-addRunSeperator->getScaledContentWidth() / 2 - addRunStartInput->getContentWidth() / 2 - 5, 0));
    addRunStartInput->setCommonFilter(CommonFilter::Uint);
    runsBG->addChild(addRunStartInput);

    addRunEndInput = TextInput::create(30, "E%", "gjFont17.fnt");
    addRunEndInput->setPosition(addRunSeperator->getPosition() + ccp(addRunSeperator->getScaledContentWidth() / 2 + addRunEndInput->getContentWidth() / 2 + 5, 0));
    addRunEndInput->setCommonFilter(CommonFilter::Uint);
    runsBG->addChild(addRunEndInput);

    static_cast<DTCCTextInputNode*>(addRunStartInput->getInputNode())->setCallback([&](const std::string& newText) {
        if (newText == "-" && addRunStartInput->getInputNode()->m_selected) {
            addRunStartInput->defocus();
            addRunEndInput->focus();
            addRunEndInput->setString("");
            addRunSeperator->setColor({ 0, 255, 0 });
            addRunSeperator->runAction(CCTintTo::create(.5f, 255, 255, 255));
        }
    });

    static_cast<DTCCTextInputNode*>(addRunEndInput->getInputNode())->setCallback([&](const std::string& newText) {
        if (newText == "-" && addRunEndInput->getInputNode()->m_selected) {
            addRunEndInput->defocus();
            addRunStartInput->focus();
            addRunStartInput->setString("");
            addRunSeperator->setColor({ 0, 255, 0 });
            addRunSeperator->runAction(CCTintTo::create(.5f, 255, 255, 255));
        }
    });

    addRunStartInput->setCallback([&](auto newstr) {
        auto numStartRes = geode::utils::numFromString<int>(newstr);
        if (numStartRes.isErr()) return;
        int numStart = numStartRes.unwrap();
        auto numEndRes = geode::utils::numFromString<int>(addRunEndInput->getString());
        int numEnd;
        if (numEndRes.isErr()) numEnd = 0;
        else numEnd = numEndRes.unwrap();

        if (numStart > 100) {
            numStart = 100;
            addRunStartInput->setString("100");
        }

        currentStartRunToAdd = numStart;
    });

    addRunEndInput->setCallback([&](auto newstr) {
        auto numStartRes = geode::utils::numFromString<int>(addRunStartInput->getString());
        int numStart;
        if (numStartRes.isErr()) numStart = 0;
        else numStart = numStartRes.unwrap();
        auto numEndRes = geode::utils::numFromString<int>(newstr);
        if (numEndRes.isErr()) return;
        int numEnd = numEndRes.unwrap();

        if (numEnd > 100) {
            numEnd = 100;
            addRunEndInput->setString("100");
        }

        currentEndRunToAdd = numEnd;
    });

    runLabel = CCLabelBMFont::create("Runs", "bigFont.fnt");
    runLabel->setScale(.5f);
    runLabel->setAnchorPoint({0, 1});
    runLabel->setPosition({
        5,
        runsBG->getContentHeight() - 5
    });
    runsBG->addChild(runLabel);

    f0BG = CCScale9Sprite::create("square02_001.png");
    f0BG->setContentWidth(runsBG->getContentWidth());
    f0BG->setContentHeight(squaresHeight);
    f0BG->setAnchorPoint({0, 0});
    f0BG->setCascadeColorEnabled(false);
    f0BG->setPosition(runsBG->getPosition() + ccp(
        runsBG->getContentWidth() + diffBetweenSquares,
        0
    ));
    this->addChild(f0BG);

    f0AmountInput = TextInput::create(f0BG->getContentWidth() / 1.2f, "amount", "gjFont17.fnt");
    f0AmountInput->setPosition({
        f0BG->getContentWidth() / 2,
        f0AmountInput->getContentHeight() / 2 + 2
    });
    f0AmountInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            f0AmountInput->setString("100");
        }

        currF0Amount = num;
    });
    f0AmountInput->setString(std::to_string(currF0Amount));
    f0AmountInput->setCommonFilter(CommonFilter::Uint);
    f0BG->addChild(f0AmountInput);

    f0AmountLabel = CCLabelBMFont::create("Amount", "bigFont.fnt");
    f0AmountLabel->setScale(.5f);
    f0AmountLabel->setAnchorPoint({0.5f, 1});
    f0AmountLabel->setPosition(f0AmountInput->getPosition() + ccp(
        0,
        f0AmountLabel->getScaledContentHeight() + f0AmountInput->getScaledContentHeight() / 2
    ));
    f0BG->addChild(f0AmountLabel);

    auto f0PMAlignNode = CCNode::create();
    f0PMAlignNode->setContentSize({
        f0BG->getContentWidth() / 10.f,
        25
    });
    f0PMAlignNode->setAnchorPoint({.5f, .5f});
    f0PMAlignNode->setPosition(f0AmountLabel->getPosition() + ccp(
        0,
        f0PMAlignNode->getScaledContentHeight() / 2
    ));
    f0BG->addChild(f0PMAlignNode);

    addPlusMinusBtns(f0PMAlignNode, false, [&](bool isPlus) {
        std::optional<int> sessionNum = std::nullopt;
        if (sessLvlSwitcher->getCurrentOptionIndex() == 1) 
            sessionNum = sessionSelector->getCurrentCount();

        DTLayer::get()->modifyRun(currentF0ToAdd, currF0Amount * (isPlus ? 1 : -1), sessionNum);
        
        if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["general"]->updateContent();
        else DTLayer::get()->specialStrings["s0"]->updateContent();
    }, .9f);

    addPercentInput = TextInput::create(75, "percent", "gjFont17.fnt");
    addPercentInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            addPercentInput->setString("100");
        }

        currentF0ToAdd = num;
    });
    addPercentInput->setPosition(f0PMAlignNode->getPosition() + ccp(
        0,
        f0PMAlignNode->getScaledContentHeight() / 2 + addPercentInput->getScaledContentHeight() / 2
    ));
    addPercentInput->setCommonFilter(CommonFilter::Uint);
    f0BG->addChild(addPercentInput);

    f0Label = CCLabelBMFont::create("From 0", "bigFont.fnt");
    f0Label->setScale(.5f);
    f0Label->setAnchorPoint({0, 1});
    f0Label->setPosition({
        5,
        f0BG->getContentHeight() - 5
    });
    f0BG->addChild(f0Label);

    nbBG = CCScale9Sprite::create("square02_001.png");
    nbBG->setContentWidth(f0BG->getContentWidth());
    nbBG->setContentHeight(squaresHeight);
    nbBG->setAnchorPoint({0, 0});
    nbBG->setCascadeColorEnabled(false);
    nbBG->setPosition(f0BG->getPosition() + ccp(
        f0BG->getContentWidth() + diffBetweenSquares,
        0
    ));
    this->addChild(nbBG);

    auto removeBtnSpr = ButtonSprite::create(
        "Remove",
        nbBG->getScaledContentWidth() / 1.4f,
        nbBG->getScaledContentWidth() / 1.4f,
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
    removeBtn->setPosition(nbBG->getPosition() + ccp(
        nbBG->getScaledContentWidth() / 2,
        removeBtn->getScaledContentHeight() / 2 + 5
    ));
    this->addChild(removeBtn);
    
    auto addBtnSpr = ButtonSprite::create(
        "  Add  ",
        nbBG->getScaledContentWidth() / 1.4f,
        nbBG->getScaledContentWidth() / 1.4f,
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

    addNewBestInput = TextInput::create(75, "New B %", "gjFont17.fnt");
    addNewBestInput->setPosition(addBtn->getPosition() + ccp(
        0,
        addNewBestInput->getScaledContentHeight() / 2 + addBtn->getScaledContentHeight() / 2 + 5
    ));
    addNewBestInput->setCallback([&](auto newstr) {
        auto numRes = geode::utils::numFromString<int>(newstr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num > 100) {
            num = 100;
            addNewBestInput->setString("100");
        }

        currentNBToAdd = num;
    });
    addNewBestInput->setCommonFilter(CommonFilter::Uint);
    this->addChild(addNewBestInput);

    NBLabel = CCLabelBMFont::create("New Bests", "bigFont.fnt");
    NBLabel->setScale(.4f);
    NBLabel->setAnchorPoint({0, 1});
    NBLabel->setPosition({
        5,
        nbBG->getContentHeight() - 5
    });
    nbBG->addChild(NBLabel);

    scheduleUpdate();

    auto modiftyInfo = TutorialButton::create(.75f, "modify-overall", [&, addBtn, removeInvalidRunsBtn, removeBtn, currentRunModeLabel](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("Here you are able to <cy>modify</c> your save file!", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("Here you have a <cf>preview</c> of how your save file <cy>currently</c> looks", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinHighlight(previewBG)

            ->appendDialogue("You can see <cs>what data</c> it currently displays in the <cl>labels title</c>", TutorialCharacterFace::TCFNormalTilted)
            ->joinTransform(TutorialBoxPlacement::TBPTopLeft, .75f)
            ->joinHighlight(myLabel->labelTitleBG)

            ->appendDialogue("At the top you can <cy>edit what data you are going to edit!</c>", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinHighlight(selectionBG)

            ->appendDialogue("You can choose between <cy>general</c> level stats or <cy>session</c> stats", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(sessLvlSwitcher)

            ->appendDialogue("You can choose between editing <cc>runs</c> or <cc>from 0</c> stats", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(runF0Toggler)
            ->joinHighlight(currentRunModeLabel)

            ->appendDialogue("And you can switch between <co>sessions</c> using the session selector", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(sessionSelector)
            ->joinTextToHighlight("Session Selector", .35f)

            ->appendDialogue("The <cf>preview</c> will change according to your selection!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinHighlight(previewBG)

            ->appendDialogue("At the bottom you have fields to <cg>add</c>/<cr>remove</c> data from your save!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPTopLeft, .75f)
            ->joinHighlight(runsBG)
            ->joinHighlight(f0BG)
            ->joinHighlight(nbBG)

            ->appendDialogue("One for <cy>runs</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(runsBG)

            ->appendDialogue("One for <cy>from 0</c> progress", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(f0BG)

            ->appendDialogue("And one for <cy>new bests</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(nbBG)

            ->appendDialogue("They all have <cc>input fields</c> to input percentages", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(addRunStartInput)
            ->joinHighlight(addRunEndInput)
            ->joinHighlight(addPercentInput)
            ->joinHighlight(addNewBestInput)
            
            ->appendDialogue("And buttons for <cg>adding</c>/<cr>removing</c> the inputted data from the save file!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(addBtn)
            ->joinHighlight(removeBtn);

        for (const auto& [btn, _] : plusMinusCallbacks)
        {
            tutorialLayer->joinHighlight(btn);
        }
        
        tutorialLayer->appendDialogue("Some have an <cy>amount field</c> for inputting <cy>how any of x runs</c> to add at a time", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(f0AmountInput)
            ->joinHighlight(runAmountInput)
            
            ->appendDialogue("Theres is also the <cr>\"Remove Invalid Runs\"</c> button", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(removeInvalidRunsBtn)
            
            ->appendDialogue("Which removes runs that go <co>below 0%</c> or <co>abover 100%</c> you might have saved", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(removeInvalidRunsBtn)

            ->appendDialogue("Enjoy editing your save file!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    modiftyInfo->setPosition(size - modiftyInfo->getScaledContentSize() / 2 + ccp(2, 2));
    this->addChild(modiftyInfo);

    this->setOpacity(0);

    Dev::fadeTextInput(addNewBestInput, false, 0);
    Dev::fadeTextInput(addPercentInput, false, 0);
    Dev::fadeTextInput(f0AmountInput, false, 0);
    Dev::fadeTextInput(addRunEndInput, false, 0);
    Dev::fadeTextInput(addRunStartInput, false, 0);
    Dev::fadeTextInput(runAmountInput, false, 0);

    myLabel->setOpacity(0);
    myLabel->setTextColor({255, 255, 255, 0});
    myLabel->fadeTitleColorTo({255, 255, 255, 0}, 0);
    Dev::fadeTextInput(sessionSelector->getTextInput(), false, 0);

    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(0))->setOpacity(0);
    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(1))->setOpacity(0);
    scrollbar->setTouchEnabled(false);

    sessionSelector->setEnabled(false);

    this->addEventListener(
        KeybindSettingPressedEvent(
            Mod::get(),
            "add-deaths"
        ),
        [&, f0PMAlignNode, runPMAlignNode](const Keybind& keybind, bool down, bool repeat, double) {
            if (down) {
                if (addNewBestInput->getInputNode()->m_selected) {
                    addNB(nullptr);
                }
                else if (addRunStartInput->getInputNode()->m_selected || addRunEndInput->getInputNode()->m_selected) {
                    (*plusMinusCallbacks[holdersOfPlusMinusBtns[runPMAlignNode].first])(true);
                }
                else if (addPercentInput->getInputNode()->m_selected) {
                    (*plusMinusCallbacks[holdersOfPlusMinusBtns[f0PMAlignNode].first])(true);
                }
            }
        }
    );
    this->addEventListener(
        KeybindSettingPressedEvent(
            Mod::get(),
            "remove-deaths"
        ),
        [&, f0PMAlignNode, runPMAlignNode](const Keybind& keybind, bool down, bool repeat, double) {
            if (down) {
                if (addNewBestInput->getInputNode()->m_selected) {
                    removeNB(nullptr);
                }
                else if (addRunStartInput->getInputNode()->m_selected || addRunEndInput->getInputNode()->m_selected) {
                    (*plusMinusCallbacks[holdersOfPlusMinusBtns[runPMAlignNode].second])(false);
                }
                else if (addPercentInput->getInputNode()->m_selected) {
                    (*plusMinusCallbacks[holdersOfPlusMinusBtns[f0PMAlignNode].second])(false);
                }
            }
        }
    );

    updatePreviewName(false, true);

    selectionBG->setOpacity(0);
    runsBG->setOpacity(0);
    f0BG->setOpacity(0);
    nbBG->setOpacity(0);

    runLabel->setOpacity(0);
    addRunSeperator->setOpacity(0);
    runAmountLabel->setOpacity(0);
    NBLabel->setOpacity(0);
    f0AmountLabel->setOpacity(0);
    f0Label->setOpacity(0);

    return true;
}

void ModifyOptions::onOpened() {
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));
    previewBG->runAction(CCFadeTo::create(fadeTime, 150));
    selectionBG->runAction(CCFadeTo::create(fadeTime, 150));
    runsBG->runAction(CCFadeTo::create(fadeTime, 150));
    f0BG->runAction(CCFadeTo::create(fadeTime, 150));
    nbBG->runAction(CCFadeTo::create(fadeTime, 150));

    runLabel->runAction(CCFadeTo::create(fadeTime, 255));
    addRunSeperator->runAction(CCFadeTo::create(fadeTime, 255));
    runAmountLabel->runAction(CCFadeTo::create(fadeTime, 255));
    NBLabel->runAction(CCFadeTo::create(fadeTime, 255));
    f0AmountLabel->runAction(CCFadeTo::create(fadeTime, 255));
    f0Label->runAction(CCFadeTo::create(fadeTime, 255));

    Dev::fadeTextInput(addNewBestInput, true, fadeTime);
    Dev::fadeTextInput(addPercentInput, true, fadeTime);
    Dev::fadeTextInput(f0AmountInput, true, fadeTime);
    Dev::fadeTextInput(addRunEndInput, true, fadeTime);
    Dev::fadeTextInput(addRunStartInput, true, fadeTime);
    Dev::fadeTextInput(runAmountInput, true, fadeTime);

    sessionSelector->setMaximumCount(DTLayer::get()->sessionSelector->getMaximumCount(), false);

    myLabel->runAction(CCFadeIn::create(fadeTime));
    myLabel->fadeTextColorTo({255, 255, 255, 255}, fadeTime);
    myLabel->fadeTitleColorTo({255, 255, 255, 255}, fadeTime);
    Dev::fadeTextInput(sessionSelector->getTextInput(), true, fadeTime);
    sessionSelector->setEnabled(true);

    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 150));
    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 255));
    scrollbar->setTouchEnabled(true);

    this->setEnabled(true);
    previewScroll->setMouseEnabled(true);  
}
void ModifyOptions::onClosed() {
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));
    previewBG->runAction(CCFadeTo::create(fadeTime, 0));
    selectionBG->runAction(CCFadeTo::create(fadeTime, 0));
    runsBG->runAction(CCFadeTo::create(fadeTime, 0));
    f0BG->runAction(CCFadeTo::create(fadeTime, 0));
    nbBG->runAction(CCFadeTo::create(fadeTime, 0));

    runLabel->runAction(CCFadeTo::create(fadeTime, 0));
    addRunSeperator->runAction(CCFadeTo::create(fadeTime, 0));
    runAmountLabel->runAction(CCFadeTo::create(fadeTime, 0));
    NBLabel->runAction(CCFadeTo::create(fadeTime, 0));
    f0AmountLabel->runAction(CCFadeTo::create(fadeTime, 0));
    f0Label->runAction(CCFadeTo::create(fadeTime, 0));

    Dev::fadeTextInput(addNewBestInput, false, fadeTime);
    Dev::fadeTextInput(addPercentInput, false, fadeTime);
    Dev::fadeTextInput(f0AmountInput, false, fadeTime);
    Dev::fadeTextInput(addRunEndInput, false, fadeTime);
    Dev::fadeTextInput(addRunStartInput, false, fadeTime);
    Dev::fadeTextInput(runAmountInput, false, fadeTime);

    myLabel->runAction(CCFadeOut::create(fadeTime));
    myLabel->fadeTextColorTo({255, 255, 255, 0}, fadeTime);
    myLabel->fadeTitleColorTo({255, 255, 255, 0}, fadeTime);
    Dev::fadeTextInput(sessionSelector->getTextInput(), false, fadeTime);
    sessionSelector->setEnabled(false);

    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(0))->runAction(CCFadeTo::create(fadeTime, 0));
    static_cast<CCScale9Sprite*>(scrollbar->getChildren()->objectAtIndex(1))->runAction(CCFadeTo::create(fadeTime, 0));
    scrollbar->setTouchEnabled(false);

    this->setEnabled(false);
    previewScroll->setMouseEnabled(false);
}

void ModifyOptions::update(float dt) {
    float prevHeight = previewScroll->m_contentLayer->getContentHeight();
    previewScroll->m_contentLayer->updateLayout();
    if (previewScroll->m_contentLayer->getContentHeight() != prevHeight)
        previewScroll->moveToTop();
}

void ModifyOptions::updatePreviewName(bool categotyIsSession, bool f0State) {
    std::string category = !categotyIsSession ?
        "level" :
        fmt::format("session {}", DTLayer::get()->getCurrentSelectedSession());
    std::string type = f0State ?
        "from 0" :
        "runs";

    myLabel->setLabelName(fmt::format("preview ({}, {})", category, type));

    std::string text;

    if (!categotyIsSession) {
        if (f0State) text = "general";
        else text = "runs";
    }
    else {
        if (f0State) text = "s0";
        else text = "sruns";
    }

    myLabel->setLabelText(fmt::format("{{{}}}", text));
}

void ModifyOptions::addPlusMinusBtns(CCNode* around, bool flip, geode::Function<void(bool isPlus)> callback, float scale) {
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

    auto sharedCb = std::make_shared<geode::Function<void(bool)>>(std::move(callback));

    plusMinusCallbacks.emplace(plusBtn, sharedCb);
    plusMinusCallbacks.emplace(minusBtn, sharedCb);

    holdersOfPlusMinusBtns.insert({around, std::make_pair(plusBtn, minusBtn)});
}
void ModifyOptions::onPlusMinusBtn(CCObject* sender) {
    if (auto btnSender = typeinfo_cast<CCMenuItemSpriteExtra*>(sender)) {
        if (!plusMinusCallbacks.contains(btnSender)) return;

        (*plusMinusCallbacks[btnSender])(btnSender->getTag() == 1);
    }
}

void ModifyOptions::removeNB(CCObject*) {
    std::optional<int> sessionNum = std::nullopt;
    if (sessLvlSwitcher->getCurrentOptionIndex() == 1)
        sessionNum = sessionSelector->getCurrentCount();

    DTLayer::get()->modifyNewBest(currentNBToAdd, false, sessionNum);

    if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["general"]->updateContent();
    else DTLayer::get()->specialStrings["s0"]->updateContent();
}
void ModifyOptions::addNB(CCObject*) {
    std::optional<int> sessionNum = std::nullopt;
    if (sessLvlSwitcher->getCurrentOptionIndex() == 1)
        sessionNum = sessionSelector->getCurrentCount();

    DTLayer::get()->modifyNewBest(currentNBToAdd, true, sessionNum);

    if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["general"]->updateContent();
    else DTLayer::get()->specialStrings["s0"]->updateContent();
}

void ModifyOptions::removeInvalidRuns(CCObject*) {
    std::optional<int> sessionNum = std::nullopt;
    if (sessLvlSwitcher->getCurrentOptionIndex() == 1)
        sessionNum = sessionSelector->getCurrentCount();

    auto togglerState = runF0Toggler->isToggled();

    auto task = arc::spawn(DTLayer::get()->getTFor<Deaths>([togglerState](GeneralData const& data) {
        return togglerState ? data.runs : data.deaths;
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

        if (split.start == std::nullopt) from0Maybe = true;


        if (split.start.has_value() && ((split.start.value() < 0 || split.start.value() > 100) || (split.end < 0 || split.end > 100))) {
            runs.push_back({split, count});
            continue;
        }
        else if (!split.start.has_value() && split.end < 0 || split.end > 100) {
            runs.push_back({split, count});
            continue;
        }
    }

    for (const auto& [run, deathCount] : runs)
    {
        if (from0Maybe) {
            DTLayer::get()->modifyRun(run.end, -deathCount, sessionNum);
        }
        else {
            DTLayer::get()->modifyRun(run.start.value_or(-1), run.end, -deathCount, sessionNum);
        }
    }
    
    if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["runs"]->updateContent();
    else DTLayer::get()->specialStrings["sruns"]->updateContent();
    if (sessionNum == std::nullopt) DTLayer::get()->specialStrings["general"]->updateContent();
    else DTLayer::get()->specialStrings["s0"]->updateContent();
}

