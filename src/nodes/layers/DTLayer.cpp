#include <nodes/layers/DTLayer.hpp>

#include <nodes/layers/DTGraphLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>
#include <nodes/layers/CalculatorPopup.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <regex>
#include <utils/CCResizeWidthTo.hpp>

#include <arc/task/Yield.hpp>
#include <arc/time/Sleep.hpp>

#include <utils/DateFormatter.hpp>
#include <utils/Settings.hpp>
#include <nodes/layers/ChangelogPopup.hpp>
#include <nodes/layers/TextInputPopup.hpp>

float DTLayer::transitionTime = .35f;
CCLabelBMFont* DTLayer::verText = nullptr;

bool ColumnComperator::operator()(LayoutColumn* a, LayoutColumn* b) const {
    if (!a || !b) {
        return a < b;
    }
    return a->info.orderPos < b->info.orderPos;
}

DTLayer* DTLayer::instance = nullptr;

DTLayer* DTLayer::create(GJGameLevel* const& Level) {
    auto popup = new DTLayer;
    if (popup->init(Level)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool DTLayer::init(GJGameLevel* const& level) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(winSize.width - 150, winSize.height - 30, "geode.loader/GE_square01.png"))
        return false;

    m_Level = level;

    // ================================== //
    // loading data

    CleanGetStats();

    if (!std::filesystem::exists(StatsManager::getSavesFolderPath())){
        Notification::create("Error! Invalid save directory!", NotificationIcon::Error)->show();
    }

    // ================================== //

    /*
     * main page
    */

    instance = this;

    this->setID("dt-layer");

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString()){
        bool isNewPlayer = Save::getLastOpenedVersion() == "";
        Save::setLastOpenedVersion(Mod::get()->getVersion().toNonVString());
        Mod::get()->getVersion();
        if (!isNewPlayer){
            auto versionHandle = async::spawn(
                []() -> arc::Future<> {
                    co_await arc::yield();
                },
                [](){
                    auto changelog = ChangelogPopup::create();
                    if (changelog != nullptr)
                        changelog->show();
                }
            );
            versionHandle.setName("DT-version-popup-delay-task");
        }
    }

    
    if (verText == nullptr){
        verText = CCLabelBMFont::create(Mod::get()->getVersion().toVString(true).c_str(), "gjFont17.fnt");
        verText->setAnchorPoint({0, 0});
        verText->setOpacity(0);
        verText->setPosition({5, 5});
        verText->setScale(.5f);
        OverlayManager::get()->addChild(verText);
    }

    verText->stopAllActions();
    verText->runAction(CCFadeTo::create(.5f, 120));

    float height = 60;
    ogLimits = CCSize{m_size.width - 30 + 1, m_size.height - height  + 1};
    scrollLayer = AdvancedScrollLayer::create({m_size.width - 30, m_size.height - height}, ogLimits);
    //scrollLayer->drawGrid(50, .5f, ccColor4B{ 143, 143, 143, 255 });
    scrollLayer->setPosition(m_size / 2 - scrollLayer->getContentSize() / 2 + ccp(0, height / 4));
    scrollLayer->setZOrder(2);
    scrollLayer->maxZoom = 0.05f;
    scrollLayer->minZoom = 10;
    m_mainLayer->addChild(scrollLayer);

    std::vector<CCPoint> points{
        scrollLayer->getPosition(),
        scrollLayer->getPosition() + ccp(scrollLayer->getContentSize().width, 0),

        scrollLayer->getPosition() + ccp(scrollLayer->getContentSize().width, 0),
        scrollLayer->getPosition() + scrollLayer->getContentSize(),

        scrollLayer->getPosition() + scrollLayer->getContentSize(),
        scrollLayer->getPosition() + ccp(0, scrollLayer->getContentSize().height),

        scrollLayer->getPosition() + ccp(0, scrollLayer->getContentSize().height),
        scrollLayer->getPosition(),
    };

    auto outline = CCDrawNode::create();
    outline->m_bUseArea = false;
    outline->drawLines(&points[0], points.size(), .5f, ccc4FFromccc4B({ 143, 143, 143, 255 }));
    outline->setZOrder(2);
    m_mainLayer->addChild(outline);

    auto shadow = CCScale9Sprite::create("square.png");
    shadow->setContentSize(scrollLayer->getContentSize());
    shadow->setAnchorPoint({0, 0});
    shadow->setPosition(scrollLayer->getPosition());
    shadow->setColor({0, 0, 0});
    shadow->setOpacity(100);
    m_mainLayer->addChild(shadow);

    sessionSelector = SessionSelector::create(getCurrentGrouping().grouping.size());
    sessionSelector->setCallback([&](int newSession){ onSessionSelected(newSession, true); });
    sessionSelector->setScale(.75f);
    sessionSelector->setPosition({m_size.width / 2, 20});
    m_mainLayer->addChild(sessionSelector);

    bottomLeftMenu = CCMenu::create();
    bottomLeftMenu->setContentSize({m_size.width / 2 - sessionSelector->getScaledContentWidth() / 2 - 10, height / 2.4f});
    bottomLeftMenu->setAnchorPoint({0, .5f});
    bottomLeftMenu->setPosition(ccp(10, height / 2.5f  / 2) + ccp(0, 7.5f));
    bottomLeftMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(10)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Even)
        ->setMinRelativeScale(std::nullopt)
        ->setMaxRelativeScale(std::nullopt)
    );
    bottomLeftMenu->setID("bottom-left-menu");
    m_mainLayer->addChild(bottomLeftMenu);

    bottomRightMenu = CCMenu::create();
    bottomRightMenu->setContentSize({m_size.width / 2 - sessionSelector->getScaledContentWidth() / 2 - 10, height / 2.4f});
    bottomRightMenu->setAnchorPoint({1, .5f});
    bottomRightMenu->setPosition(ccp(m_size.width - 10, height / 2.5f  / 2) + ccp(0, 7.5f));
    bottomRightMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(10)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Even)
        ->setMinRelativeScale(std::nullopt)
        ->setMaxRelativeScale(std::nullopt)
    );
    bottomRightMenu->setID("bottom-right-menu");
    m_mainLayer->addChild(bottomRightMenu);

    auto levelSpecificOptionsSpr = CCSprite::createWithSpriteFrameName("DTLevelOptionsBtn.png"_spr);
    auto levelSpecificOptionsBtn = CCMenuItemSpriteExtra::create(
        levelSpecificOptionsSpr,
        this,
        menu_selector(DTLayer::onLSOClicked)
    );
    bottomLeftMenu->addChild(levelSpecificOptionsBtn);

    auto graphBtnSpr = CCSprite::createWithSpriteFrameName("graph_button.png"_spr);
    auto graphBtn = CCMenuItemSpriteExtra::create(
        graphBtnSpr,
        this,
        menu_selector(DTLayer::graphBtnClicked)
    );
    bottomLeftMenu->addChild(graphBtn);

    auto calculatorSpr = CCSprite::createWithSpriteFrameName("caluclator.png"_spr);
    auto calculatorBtnSpr = CCSprite::create("GJ_button_01.png");
    calculatorSpr->setPosition(calculatorBtnSpr->getContentSize() / 2);
    calculatorSpr->setScale(.9f);
    calculatorBtnSpr->addChild(calculatorSpr);
    auto calculatorBtn = CCMenuItemSpriteExtra::create(
        calculatorBtnSpr,
        this,
        menu_selector(DTLayer::onCalculator)
    );
    bottomLeftMenu->addChild(calculatorBtn);

    onSessionSelected(1, false);

    auto groupsBtnSpr = ButtonSprite::create(
        sessionsOrder.groupName.c_str(),
        100,
        100,
        1,
        false,
        "bigFont.fnt", 
        "GJ_button_04.png"
    );
    groupsBtnSpr->setCascadeOpacityEnabled(true);
    groupsBtnSpr->setScale(.45f);
    groupsBtn = CCMenuItemSpriteExtra::create(
        groupsBtnSpr,
        this,
        menu_selector(DTLayer::onGroups)
    );

    groupsHolder = CCMenu::create();
    groupsHolder->setContentSize(ccp(45, 0));
    groupsBtn->setPosition(groupsHolder->getContentSize() / 2);
    bottomRightMenu->addChild(groupsHolder);
    groupsHolder->addChild(groupsBtn);

    auto editLayoutBtnSprBG = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    editLayoutBtnSpr = CCSprite::createWithSpriteFrameName("layout_button.png"_spr);
    editLayoutBtnSpr->setPosition(editLayoutBtnSprBG->getContentSize() / 2);
    editLayoutBtnSprBG->addChild(editLayoutBtnSpr);
    editLayoutBtnSprBG->setScale(.75f);
    auto editLayoutBtn = CCMenuItemSpriteExtra::create(
        editLayoutBtnSprBG,
        this,
        menu_selector(DTLayer::onEditLayout)
    );
    editLayoutBtn->setPosition(scrollLayer->getPosition() + scrollLayer->getContentSize());
    bottomRightMenu->addChild(editLayoutBtn);

    auto settingsBtnSpr = CCSprite::createWithSpriteFrameName("DTSettingsBtn.png"_spr);
    settingsBtnSpr->setScale(.75f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSpr,
        this,
        menu_selector(DTLayer::onSettings)
    );
    settingsBtn->setPosition({m_size.width - 3.f, 3.f});
    bottomRightMenu->addChild(settingsBtn);

    columnHolder = CCMenu::create();
    columnHolder->setAnchorPoint({0, 1});
    columnHolder->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
        ->setCrossAxisAlignment(CrossAxisAlignment::Start)
    );
    columnHolder->setPosition({0, scrollLayer->content->getContentHeight()});
    scrollLayer->content->addChild(columnHolder);

    labelsHolder = CCNode::create();
    labelsHolder->setPosition(columnHolder->getPosition());
    scrollLayer->content->addChild(labelsHolder);

    setLayoutBy(Save::getLayout());

    auto addColumnButtonSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    addColumnButton = CCMenuItemSpriteExtra::create(
        addColumnButtonSpr,
        this,
        menu_selector(DTLayer::addColumnBtnClicked)
    );
    addColumnButton->setZOrder(1000);
    columnHolder->addChild(addColumnButton);
    addColumnButton->setOpacity(0);
    columnHolder->setEnabled(false);

    lc = LoadingCircle::create();
    lc->setContentSize({0, 0});
    lc->m_sprite->setPosition(m_size / 2);
    m_mainLayer->addChild(lc);

    CCTouchDispatcher::get()->removeDelegate(scrollLayer);

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    populateSpecialStrings();

    this->organizeLayout();

    this->scheduleUpdate();

    bottomRightMenu->updateLayout();
    bottomLeftMenu->updateLayout();

    groupsList = FloatingList::create({
        groupsBtn->getScaledContentWidth(),
        120
    });
    groupsList->setZOrder(2);
    groupsList->addItems({
        FloatingListItem{
            .id = -4,
            .text = sessionsOrder.groupName,
            .BGTexture = "GJ_button_04.png"
        },
        FloatingListItem{
            .id = -3,
            .text = daySGroup.groupName,
            .BGTexture = "GJ_button_05.png"
        },
        FloatingListItem{
            .id = -2,
            .text = weekSGroup.groupName,
            .BGTexture = "GJ_button_05.png"
        },
        FloatingListItem{
            .id = -1,
            .text = monthSGroup.groupName,
            .BGTexture = "GJ_button_05.png"
        },
    });
    groupsList->setCallback([&](auto id){
        this->onGroupSelected(id);
    });
    groupsList->setAnchorPoint({.5f, 0});
    m_mainLayer->addChild(groupsList);
    groupsList->setPosition(
        groupsList->getParent()->convertToNodeSpace(
            groupsBtn->convertToWorldSpace({groupsBtn->getContentWidth() / 2,groupsBtn->getContentHeight()})
        )
    );

    editLayoutMenu = CCMenu::create();
    editLayoutMenu->setEnabled(false);
    editLayoutMenu->setPosition({winSize.width / 2,160});
    editLayoutMenu->ignoreAnchorPointForPosition(false);
    editLayoutMenu->setAnchorPoint({0,0});
    m_mainLayer->addChild(editLayoutMenu);

    applyChangesButtonSpr = ButtonSprite::create("Apply Changes", "goldFont.fnt", "GJ_button_01.png");
    applyChangesButtonSpr->m_BGSprite->setOpacity(0);
    applyChangesButtonSpr->m_label->setOpacity(0);
    applyChangesButtonSpr->setScale(.5f);
    auto applyChangesButton = CCMenuItemSpriteExtra::create(
        applyChangesButtonSpr,
        this,
        menu_selector(DTLayer::onApplyLayoutChanges)
    );
    applyChangesButton->setPosition({41, -140});
    editLayoutMenu->addChild(applyChangesButton);

    discardChangesButtonSpr = ButtonSprite::create("Discard Changes", "goldFont.fnt", "GJ_button_06.png");
    discardChangesButtonSpr->m_BGSprite->setOpacity(0);
    discardChangesButtonSpr->m_label->setOpacity(0);
    discardChangesButtonSpr->setScale(.45f);
    auto discardChangesButton = CCMenuItemSpriteExtra::create(
        discardChangesButtonSpr,
        this,
        menu_selector(DTLayer::onDiscardLayoutChanges)
    );
    discardChangesButton->setPosition({-191, -140});
    editLayoutMenu->addChild(discardChangesButton);

    auto mainInfo = TutorialButton::create(
        1,
        "main-overall",
        [
            &,
            levelSpecificOptionsBtn,
            graphBtn,
            editLayoutBtn,
            settingsBtn,
            calculatorBtn
        ](DTTutorialLayer* tutorialLayer){
        tutorialLayer->appendDialogue("Welcome to the <cy>main death tracker page!</c>", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("This is the <cy>main view</c> where you can view <cg>all your data!</c>", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .75f)
            ->joinHighlight(scrollLayer)
            ->joinTextToHighlight("Main Scroll View", .5f, TutorialTextPlacement::TTTop)
#if !defined(GEODE_IS_MOBILE)
            ->appendDialogue("You can hold <cg>control</c> and <cp>scroll</c> to zoom in! and <cc>shift</c> and <cp>scroll</c> to move side to side.", TutorialCharacterFace::TCFNormalTilted)
#else
            ->appendDialogue("You can <cg>pinch</c> to zoom in! and <cc>swipe</c> to move side to side.", TutorialCharacterFace::TCFNormalTilted)
#endif
            ->joinHighlight(scrollLayer)

            ->appendDialogue("Of course you can also scroll normally :D", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter)

            ->appendDialogue("You also have many options <cy>at the bottom</c> here!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->joinHighlight(bottomRightMenu)
            ->joinHighlight(bottomLeftMenu)
            ->joinHighlight(sessionSelector)

            ->appendDialogue("There are the <cy>level options</c>", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(levelSpecificOptionsBtn)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f)
            ->joinTextToHighlight("level options", .3f, TutorialTextPlacement::TTTop)
            ->appendDialogue("Which allow you to change many things about how you <cr>track/display your data</c>", TutorialCharacterFace::TCFNormal)
            ->joinPreviousHighlight()

            ->appendDialogue("You have the <cg>graphs</c> which allow you to visually see your consistancy and other aspects of your data", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(graphBtn)
            ->joinTextToHighlight("graphs", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("Theres the <cy>calculator</c>, allowing you to see some extra data about your runs", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(calculatorBtn)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->joinTextToHighlight("calculator", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("You have the <co>session selector</c>, allowing you to choose which session to view", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(sessionSelector)
            ->joinTextToHighlight("Session Selector", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("The <cy>higher</c> the number, the <cr>older</c> the session! so <co>session 1</c> is the most recent and <co>the last</c> is the oldest!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(sessionSelector)

            ->appendDialogue("There are the session groups! which allow you to group session by specific categories         ", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(groupsBtn)
            ->joinTextToHighlight("Session Groups", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("You also have the option to <cy>edit how your data is layed out</c> using this button!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(editLayoutBtn)
            ->joinTextToHighlight("Layout Editor", .3f, TutorialTextPlacement::TTTop)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)

            ->appendDialogue("And lastly you have quick access to the mod settings for death tracker right here!", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(settingsBtn)
            ->joinTextToHighlight("Mod Options", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("You can also <cy>double click</c> an of the <cf>labels top parts</c> to copy their contents", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .75f);
        std::set<DTLabel*> allLabels{};

        int index = 0;
        for (const auto& column : columns)
        {
            for (const auto& [_, label] : column->labels)
            {
                if (allLabels.contains(label)) continue;

                allLabels.insert(label);
                tutorialLayer->joinHighlight(label->labelTitleBG, .1f * index);
                index++;
            }
        }

            tutorialLayer->appendDialogue("Have fun playing around with the features!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter);
    });
    mainInfo->setPosition(m_size);
    m_buttonMenu->addChild(mainInfo);

    resetLayoutMenu = CCMenu::create();
    resetLayoutMenu->ignoreAnchorPointForPosition(false);
    resetLayoutMenu->setPosition({0, 0});
    resetLayoutMenu->setContentSize({0, 0});
    resetLayoutMenu->setCascadeOpacityEnabled(true);
    resetLayoutMenu->setEnabled(false);
    resetLayoutMenu->setZOrder(3);
    m_mainLayer->addChild(resetLayoutMenu);
    
    auto resetLayoutBtnSpr = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
    resetLayoutBtnSpr->setScale(.5f);
    auto resetLayoutBtn = CCMenuItemSpriteExtra::create(
        resetLayoutBtnSpr,
        this,
        menu_selector(DTLayer::onResetLayout)
    );
    resetLayoutBtn->setPosition({0, 0});
    resetLayoutMenu->addChild(resetLayoutBtn);

    auto layoutPresetsBtnSpr = ButtonSprite::create("Presets", "bigFont.fnt", "GJ_button_05.png");
    layoutPresetsBtnSpr->setScale(.4f);
    layoutPresetsBtnSpr->setCascadeOpacityEnabled(true);
    auto layoutPresetsBtn = CCMenuItemSpriteExtra::create(
        layoutPresetsBtnSpr,
        this,
        menu_selector(DTLayer::onLayoutPresets)
    );
    layoutPresetsBtn->setPosition({m_size.width, 0});
    resetLayoutMenu->addChild(layoutPresetsBtn);

    presetList = FloatingList::create({
        layoutPresetsBtn->getContentWidth() + 10,
        90
    });
    presetList->setPosition(layoutPresetsBtn->getPosition() + ccp(0, layoutPresetsBtn->getContentHeight() / 2 + 2));
    presetList->setAnchorPoint({.5f, 0});
    presetList->setCallback([&](int id){
        auto optItem = presetList->getItemForID(id);
        if (!optItem.has_value()) return;

        auto presets = Save::getLayoutPresets();
        int index = 0;
        for (const auto& preset : presets)
        {
            if (optItem.value().text == preset.name){
                resetLayoutTo(preset);
                break;
            }

            index++;
        }
    });
    resetLayoutMenu->addChild(presetList);

    auto addPreset = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    addPreset->setScale(.45f);
    auto addPresetBtn = CCMenuItemSpriteExtra::create(
        addPreset,
        this,
        menu_selector(DTLayer::onAddPreset)
    );
    addPresetBtn->setPosition(layoutPresetsBtn->getPosition() + ccp(
        -layoutPresetsBtn->getContentWidth() / 2 - addPresetBtn->getContentWidth() / 2 - 1.5f,
        0
    ));
    resetLayoutMenu->addChild(addPresetBtn);

    auto importBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    // @geode-ignore(unknown-resource)
    auto importBtnSpr1 = CCSprite::createWithSpriteFrameName("geode.loader/install.png");
    importBtnSpr1->setPosition(importBtnSpr->getContentSize() / 2);
    importBtnSpr1->setScale(1.2f);
    importBtnSpr->addChild(importBtnSpr1);
    importBtnSpr->setCascadeOpacityEnabled(true);
    importBtnSpr->setScale(0.45f);
    auto importPresetBtn = CCMenuItemSpriteExtra::create(
        importBtnSpr,
        this,
        menu_selector(DTLayer::onImportPreset)
    );
    importPresetBtn->setPosition(addPresetBtn->getPosition() + ccp(
        -addPresetBtn->getContentWidth() / 2 - importPresetBtn->getContentWidth() / 2 - 1.5f,
        0
    ));
    resetLayoutMenu->addChild(importPresetBtn);

    auto exportPreset = CCSprite::createWithSpriteFrameName("GJ_shareBtn_001.png");
    exportPreset->setScale(.25f);
    auto exportPresetBtn = CCMenuItemSpriteExtra::create(
        exportPreset,
        this,
        menu_selector(DTLayer::onExportPreset)
    );
    exportPresetBtn->setPosition(importPresetBtn->getPosition() + ccp(
        -importPresetBtn->getContentWidth() / 2 - exportPresetBtn->getContentWidth() / 2 - 1.5f,
        0
    ));
    resetLayoutMenu->addChild(exportPresetBtn);

    auto presetsTutorial = TutorialButton::create(.75f, "layout-presets", [
        &,
        exportPresetBtn,
        importPresetBtn,
        addPresetBtn,
        layoutPresetsBtn,
        discardChangesButton
    ](DTTutorialLayer* tutorialLayer){
        tutorialLayer->appendDialogue("Here you are able to save <cy>presets</c> of your layout!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)
            ->joinHighlight(exportPresetBtn)
            ->joinHighlight(importPresetBtn)
            ->joinHighlight(addPresetBtn)
            ->joinHighlight(layoutPresetsBtn)

            ->appendDialogue("You can click the <cy>presets</c> button to look at a <cf>list of all the presets</c> you have", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(layoutPresetsBtn)

            ->appendDialogue("In that list, you can click the <cg>presets name</c> to <cy>switch</c> to it, and the <cr>trash</c> button to <cr>delete</c> it!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(layoutPresetsBtn)

            ->appendDialogue("Theres the <cg>create preset</c> button, that takes the <cy>current layout</c> and makes it into a preset", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(addPresetBtn)
            ->joinTextToHighlight("Create Preset", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("Theres the <cf>import preset</c> button, which lets you choose a '.dtl' file and add it as a preset", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(importPresetBtn)
            ->joinTextToHighlight("Import Preset", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("And lastly, you have the <cd>export preset</c> button, which adds your <cy>current layout</c> into a '.dtl' file!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(exportPresetBtn)
            ->joinTextToHighlight("Export Preset", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("Remember you can always <cr>discard your changes</c> if something isnt like you wanted", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->joinHighlight(discardChangesButton);
    });
    presetsTutorial->setPosition(layoutPresetsBtn->getPosition() + ccp(
        0,
        presetsTutorial->getContentHeight() / 2 + 1 + layoutPresetsBtn->getContentHeight() / 2
    ));
    resetLayoutMenu->addChild(presetsTutorial);


    resetLayoutMenu->setOpacity(0);

    colorChangeBG = CCScale9Sprite::create("GJ_square05.png");
    colorChangeBG->setContentSize({100, 120});
    colorChangeBG->setAnchorPoint({0, .5f});
    colorChangeBG->setPosition(m_mainLayer->convertToNodeSpace({5, winSize.height / 2}));
    colorChangeBG->setZOrder(101);
    colorChangeBG->setOpacity(0);
    colorChangeBG->setScale(.85f);
    colorChangeBG->setCascadeOpacityEnabled(true);
    m_mainLayer->addChild(colorChangeBG);

    colorMenu = CCMenu::create();
    colorMenu->setPosition({0, 0});
    colorMenu->ignoreAnchorPointForPosition(false);
    colorMenu->setContentSize({0, 0});
    colorMenu->setEnabled(false);
    colorMenu->setCascadeOpacityEnabled(true);
    colorChangeBG->addChild(colorMenu);

    newBestColorBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    newBestColorBtnSpr->setColor(Save::getNewBestColor());
    newBestColorBtnSpr->setScale(.75f);
    newBestColorBtnSpr->setCascadeOpacityEnabled(true);
    auto newBestColorBtn = CCMenuItemSpriteExtra::create(
        newBestColorBtnSpr,
        this,
        menu_selector(DTLayer::onNewBestColor)
    );
    newBestColorBtn->setPosition({
        colorChangeBG->getContentWidth() / 2,
        colorChangeBG->getContentHeight() - newBestColorBtn->getContentHeight() / 2 - 20
    });
    colorMenu->addChild(newBestColorBtn);

    auto newBestColorBtnLabel = CCLabelBMFont::create("New Best Color", "bigFont.fnt");
    newBestColorBtnLabel->setScale(.3f);
    newBestColorBtnLabel->setAnchorPoint({.5f, 0});
    newBestColorBtnLabel->setCascadeOpacityEnabled(true);
    newBestColorBtnLabel->setPosition(newBestColorBtn->getPosition() + ccp(
        0,
        newBestColorBtn->getContentHeight() / 2 + 1
    ));
    colorMenu->addChild(newBestColorBtnLabel);
    
    sessionBestColorBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    sessionBestColorBtnSpr->setColor(Save::getSessionBestColor());
    sessionBestColorBtnSpr->setScale(.75f);
    sessionBestColorBtnSpr->setCascadeOpacityEnabled(true);
    auto sessionBestColorBtn = CCMenuItemSpriteExtra::create(
        sessionBestColorBtnSpr,
        this,
        menu_selector(DTLayer::onSessionBestColor)
    );
    sessionBestColorBtn->setPosition({
        colorChangeBG->getContentWidth() / 2,
        sessionBestColorBtn->getContentHeight() / 2 + 20
    });
    colorMenu->addChild(sessionBestColorBtn);

    auto sessionBestColorBtnLabel = CCLabelBMFont::create("Session Best\nColor", "bigFont.fnt");
    sessionBestColorBtnLabel->setScale(.3f);
    sessionBestColorBtnLabel->setAnchorPoint({.5f, 0});
    sessionBestColorBtnLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    sessionBestColorBtnLabel->setCascadeOpacityEnabled(true);
    sessionBestColorBtnLabel->setPosition(sessionBestColorBtn->getPosition() + ccp(
        0,
        sessionBestColorBtn->getContentHeight() / 2 + 1
    ));
    colorMenu->addChild(sessionBestColorBtnLabel);
    colorMenu->setOpacity(0);

    scrollLayer->setVisible(false);

    layoutInfo = TutorialButton::create(1, "layout-overall", [&, applyChangesButton, discardChangesButton, resetLayoutBtn](DTTutorialLayer* tutorialLayer){
        tutorialLayer
            ->appendDialogue("This is where you can <cg>Edit how death tracker looks!</c>", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("Here you have different labels! which can display any text you want!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .65f);

        std::set<DTLabel*> allLabels{};

        int index = 0;
        for (const auto& column : columns)
        {
            for (const auto& [_, label] : column->labels)
            {
                if (allLabels.contains(label)) continue;

                allLabels.insert(label);
                tutorialLayer->joinHighlight(label->labelTitleBG, .1f * index);
                index++;
            }
        }

        tutorialLayer
            ->appendDialogue("You can <cy>click</c> on them to enter the label settings", TutorialCharacterFace::TCFNormal)
            ->joinPreviousHighlight()
            ->appendDialogue("<cy>Drag them around</c> to move them", TutorialCharacterFace::TCFNormalTilted)
            ->joinPreviousHighlight()
            ->appendDialogue("And also <cy>drag the edges</c> of the labels to <cg>expand</c> them", TutorialCharacterFace::TCFNormal);

        index = 0;
        for (const auto& label : allLabels){
            tutorialLayer->joinHighlight(label->leftExpandLine, .2f * index, false, false);
            tutorialLayer->joinHighlight(label->rightExpandLine, .2f * index, false, false);

            index++;
        }

        tutorialLayer
            ->appendDialogue("Every <cy>label</c> has to be attached to some <cp>column</c>", TutorialCharacterFace::TCFNormalTilted)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .65f);

        index = 0;
        for (const auto& column : columns)
        {
            tutorialLayer->joinHighlight(column->topSpr, .1f * index);
            index++;
        }

        tutorialLayer
            ->appendDialogue("Each <cp>column</c> can be <cy>clicked</c> to enter its settings", TutorialCharacterFace::TCFHappy)
            ->joinPreviousHighlight()
            ->appendDialogue("And can be <cg>expanded</c> when <cy>dragging its edges</c>", TutorialCharacterFace::TCFNormal);

        index = 0;
        for (const auto& column : columns)
        {
            tutorialLayer->joinHighlight(column->topBorder2, .2f * index);
            index++;
        }

        tutorialLayer
            ->appendDialogue("You can also edit the <cy>new best</c> and <co>session best</c> colors", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(colorChangeBG)
            ->appendDialogue("One you are done, you can click the <cg>Apply Changes</c> button to save your changes", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(applyChangesButton)
            ->appendDialogue("And if you <cr>regret your changes</c> you can either revert to the <cy>default layout</c> of the mod", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(resetLayoutBtn)
            ->joinTextToHighlight("Default Layout", .35f, TutorialTextPlacement::TTTop)
            ->appendDialogue("Or revert all the changes you have made using the <cr>Discard Changes</c> button!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(discardChangesButton)
            ->appendDialogue("Feel free to play around and explore the different settings!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1);
    });
    layoutInfo->setPosition(m_size);
    layoutInfo->setOpacity(0);
    layoutInfo->setEnabled(false);
    auto layoutInfoMenu = CCMenu::createWithItem(layoutInfo);
    layoutInfoMenu->setPosition({0, 0});
    m_mainLayer->addChild(layoutInfoMenu);

    return true;
}

void DTLayer::onEditLayout(CCObject*){
    if (layoutOptionsLayer == nullptr){
        layoutOptionsLayer = LayoutOptionsLayer::create({150, m_size.height});
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        layoutOptionsLayer->setPosition({winSize.width + 10, (winSize.height - layoutOptionsLayer->getContentHeight()) / 2});
        layoutOptionsLayer->onBackedOut = [&](){closeOptionsLayer();};
        this->addChild(layoutOptionsLayer);
    }

    isEditingLayout = true;

    std::set<DTLabel*> visitedLabels{};

    for (const auto& column : columns)
    {
        column->setVisibility(true);

        for (const auto& [_, label] : column->labels)
        {
            if (visitedLabels.contains(label)) continue;

            visitedLabels.insert(label);
            label->setEditable(true);
        }
    }

    bottomLeftMenu->setEnabled(false);
    bottomLeftMenu->stopAllActions();
    bottomLeftMenu->runAction(CCFadeTo::create(.15f, 0));

    bottomRightMenu->setEnabled(false);
    bottomRightMenu->stopAllActions();
    bottomRightMenu->runAction(CCFadeTo::create(.15f, 0));

    m_buttonMenu->setEnabled(false);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 0));
    editLayoutBtnSpr->stopAllActions();
    editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 0));

    groupsHolder->setEnabled(false);
    groupsList->close();
    
    colorChangeBG->stopActionByTag(2);
    auto fa = CCFadeTo::create(.15f, 255);
    fa->setTag(2);
    colorChangeBG->runAction(fa);
    colorMenu->stopAllActions();
    colorMenu->runAction(CCFadeTo::create(.15f, 255));
    colorMenu->setEnabled(true);
    resetLayoutMenu->stopAllActions();
    resetLayoutMenu->runAction(CCFadeTo::create(.15f, 255));
    resetLayoutMenu->setEnabled(true);

    editLayoutMenu->setEnabled(true);
    layoutInfo->stopAllActions();
    layoutInfo->runAction(CCFadeTo::create(.15f, 255));
    layoutInfo->setEnabled(true);
    applyChangesButtonSpr->m_BGSprite->stopAllActions();
    applyChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 255));
    applyChangesButtonSpr->m_label->stopAllActions();
    applyChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 255));
    discardChangesButtonSpr->m_BGSprite->stopAllActions();
    discardChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 255));
    discardChangesButtonSpr->m_label->stopAllActions();
    discardChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 255));

    addColumnButton->stopAllActions();
    addColumnButton->runAction(CCFadeTo::create(.15f, 255));
    columnHolder->setEnabled(true);

    originalNewBestColor = Save::getNewBestColor();
    originalSessionBestColor = Save::getSessionBestColor();
    newBestColorBtnSpr->setColor(originalNewBestColor);
    sessionBestColorBtnSpr->setColor(originalSessionBestColor);

    scrollLayer->moveBy(ccp(0, -LayoutColumn::topHeight));

    this->organizeLayout();
}

bool DTLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent){
    if (doMoveScroll)
        scrollLayer->ccTouchBegan(pTouch, pEvent);

    return true;
}

void DTLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent){
    if (doMoveScroll)
        scrollLayer->ccTouchMoved(pTouch, pEvent);
}

void DTLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent){
    scrollLayer->ccTouchEnded(pTouch, pEvent);
}

void DTLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent){
    scrollLayer->ccTouchCancelled(pTouch, pEvent);
}

void DTLayer::ccTouchesMoved(CCSet* touches, CCEvent* event){
    if (doMoveScroll)
        scrollLayer->ccTouchesMoved(touches, event);
}

void DTLayer::graphBtnClicked(CCObject*){
    DTGraphLayer::create()->show();
}

void DTLayer::addSpecialString(const std::shared_ptr<SpecialKey>& key){
    key->setUpdateStartedCallback([&](const std::shared_ptr<SpecialKey>& k){ this->specialKeyUpdateStarted(k); });
    key->setUpdateCompletedCallback([&](const std::shared_ptr<SpecialKey>& k) { this->specialKeyUpdateCompleted(k); });
    specialStrings.emplace(key->getKey(), key);
    key->updateContent();
}

void DTLayer::populateSpecialStrings(){
    auto nlKey = std::make_shared<SpecialKey>("nl", "Adds a new line");
    nlKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onNLKey));
    addSpecialString(nlKey);

    auto attemptsKey = std::make_shared<SpecialKey>("att", "Adds your geometry dash attempt count (shared with linked levels)");
    attemptsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onATTKey));
    addSpecialString(attemptsKey);

    auto levelNameKey = std::make_shared<SpecialKey>("lvln", "Adds the current levels name");
    levelNameKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onLVLNKey));
    addSpecialString(levelNameKey);

    auto generalKey = std::make_shared<SpecialKey>("general", "Adds all your runs from 0% (shared with linked levels)");
    generalKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onGeneralKey));
    addSpecialString(generalKey);

    auto runsKey = std::make_shared<SpecialKey>("runs", "Adds all your runs from practice mode/start positions (shared with linked levels)");
    runsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onRUNSKey));
    addSpecialString(runsKey);

    auto dtattKey = std::make_shared<SpecialKey>("dtatt", "Adds your death tracker attempt count (shared with linked levels)");
    dtattKey->refreshWith({
        runsKey->getKey(),
        generalKey->getKey()
    });
    dtattKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDTATTKey));
    addSpecialString(dtattKey);

    auto dtF0attKey = std::make_shared<SpecialKey>("dtf0att", "Adds your death tracker attempt count from 0 (shared with linked levels)");
    dtF0attKey->refreshWith(generalKey->getKey());
    dtF0attKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDTF0ATTKey));
    addSpecialString(dtF0attKey);

    auto dtrunsattKey = std::make_shared<SpecialKey>("dtrunsatt", "Adds your death tracker attempt count from runs (shared with linked levels)");
    dtrunsattKey->refreshWith(runsKey->getKey());
    dtrunsattKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDTRunsATTKey));
    addSpecialString(dtrunsattKey);

    auto sessionFrom0Key = std::make_shared<SpecialKey>("s0", "Adds all your runs on the selected session from 0");
    sessionFrom0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onS0Key));
    addSpecialString(sessionFrom0Key);

    auto sessionRunsKey = std::make_shared<SpecialKey>("sruns", "Adds all your runs on the selected session");
    sessionRunsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSRUNSKey));
    addSpecialString(sessionRunsKey);

    auto aptallKey = std::make_shared<SpecialKey>("aptgen", "Adds your total estimated calculated playtime (shared with linked levels)");
    aptallKey->refreshWith({
        generalKey->getKey(),
        runsKey->getKey()
    });
    aptallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTALLSKey));
    addSpecialString(aptallKey);

    auto aptf0Key = std::make_shared<SpecialKey>("aptf0", "Adds your total estimated calculated playtime from 0 (shared with linked levels)");
    aptf0Key->refreshWith(generalKey->getKey());
    aptf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTF0SKey));
    addSpecialString(aptf0Key);

    auto aptrunKey = std::make_shared<SpecialKey>("aptruns", "Adds your total estimated calculated playtime in runs (shared with linked levels)");
    aptrunKey->refreshWith(runsKey->getKey());
    aptrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTRUNSKey));
    addSpecialString(aptrunKey);

    auto aptsallKey = std::make_shared<SpecialKey>("aptsgen", "Adds your total estimated calculated session playtime");
    aptsallKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    aptsallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTSALLSKey));
    addSpecialString(aptsallKey);

    auto aptsf0Key = std::make_shared<SpecialKey>("aptsf0", "Adds your total estimated calculated session playtime from 0");
    aptsf0Key->refreshWith(sessionFrom0Key->getKey());
    aptsf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTSF0Key));
    addSpecialString(aptsf0Key);

    auto aptsrunKey = std::make_shared<SpecialKey>("aptsruns", "Adds your total estimated calculated session playtime in runs");
    aptsrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onAPTSRUNSKey));
    addSpecialString(aptsrunKey);

    auto runsTo100Key = std::make_shared<SpecialKey>("rt100", "Adds all your runs to 100");
    runsTo100Key->refreshWith(generalKey->getKey());
    runsTo100Key->refreshWith(runsKey->getKey());
    runsTo100Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onRunsTo100Key));
    addSpecialString(runsTo100Key);

    auto bRunsKey = std::make_shared<SpecialKey>("bruns", "Adds all your best runs from each percent");
    bRunsKey->refreshWith(generalKey->getKey());
    bRunsKey->refreshWith(runsKey->getKey());
    bRunsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onBestRunsKey));
    addSpecialString(bRunsKey);

    auto sessionRunsTo100Key = std::make_shared<SpecialKey>("srt100", "Adds all your runs to 100 this session");
    sessionRunsTo100Key->refreshWith(sessionFrom0Key->getKey());
    sessionRunsTo100Key->refreshWith(sessionRunsKey->getKey());
    sessionRunsTo100Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSessionRunsTo100Key));
    addSpecialString(sessionRunsTo100Key);

    auto sessionBRunsKey = std::make_shared<SpecialKey>("sbruns", "Adds all your best runs from each percent this session");
    sessionBRunsKey->refreshWith(sessionFrom0Key->getKey());
    sessionBRunsKey->refreshWith(sessionRunsKey->getKey());
    sessionBRunsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSessionBestRunsKey));
    addSpecialString(sessionBRunsKey);

    auto sAttKey = std::make_shared<SpecialKey>("satt", "Adds your attempt count for the selected session");
    sAttKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    sAttKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSAttKey));
    addSpecialString(sAttKey);

    auto sF0AttKey = std::make_shared<SpecialKey>("sf0att", "Adds your from 0 attempt count for the selected session");
    sF0AttKey->refreshWith(sessionFrom0Key->getKey());
    sF0AttKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSF0AttKey));
    addSpecialString(sF0AttKey);

    auto sRunsAttKey = std::make_shared<SpecialKey>("srunsatt", "Adds your runs attempt count for the selected session");
    sRunsAttKey->refreshWith(sessionRunsKey->getKey());
    sRunsAttKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSRunsAttKey));
    addSpecialString(sRunsAttKey);


    //playtime
    auto ptallKey = std::make_shared<SpecialKey>("ptgen", "Adds your total accurate calculated playtime (shared with linked levels)");
    ptallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTALLSKey));
    addSpecialString(ptallKey);

    auto ptf0Key = std::make_shared<SpecialKey>("ptf0", "Adds your total accurate calculated playtime from 0 (shared with linked levels)");
    ptf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTF0SKey));
    addSpecialString(ptf0Key);

    auto ptrunKey = std::make_shared<SpecialKey>("ptruns", "Adds your total accurate calculated playtime in runs (shared with linked levels)");
    ptrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTRUNSKey));
    addSpecialString(ptrunKey);

    auto ptsallKey = std::make_shared<SpecialKey>("ptsgen", "Adds your total accurate calculated session playtime");
    ptsallKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    ptsallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTSALLSKey));
    addSpecialString(ptsallKey);

    auto ptsf0Key = std::make_shared<SpecialKey>("ptsf0", "Adds your total accurate calculated session playtime from 0");
    ptsf0Key->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    ptsf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTSF0Key));
    addSpecialString(ptsf0Key);

    auto ptsrunKey = std::make_shared<SpecialKey>("ptsruns", "Adds your total accurate calculated session playtime in runs");
    ptsrunKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    ptsrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPTSRUNSKey));
    addSpecialString(ptsrunKey);
    
    //playtime deaths
    auto dptallKey = std::make_shared<SpecialKey>("dptgen", "Adds your total dead accurate calculated playtime (shared with linked levels)");
    dptallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDeadPTALLSKey));
    addSpecialString(dptallKey);

    auto dptf0Key = std::make_shared<SpecialKey>("dptf0", "Adds your total dead accurate calculated playtime from 0 (shared with linked levels)");
    dptf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDeadPTF0SKey));
    addSpecialString(dptf0Key);

    auto dptrunKey = std::make_shared<SpecialKey>("dptruns", "Adds your total dead accurate calculated playtime in runs (shared with linked levels)");
    dptrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDeadPTRUNSKey));
    addSpecialString(dptrunKey);

    auto dptsallKey = std::make_shared<SpecialKey>("dptsgen", "Adds your total dead accurate calculated session playtime");
    dptsallKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    dptsallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDeadPTSALLSKey));
    addSpecialString(dptsallKey);

    auto dptsf0Key = std::make_shared<SpecialKey>("dptsf0", "Adds your total dead accurate calculated session playtime from 0");
    dptsf0Key->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    dptsf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDeadPTSF0Key));
    addSpecialString(dptsf0Key);

    auto dptsrunKey = std::make_shared<SpecialKey>("dptsruns", "Adds your total dead accurate calculated session playtime in runs");
    dptsrunKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    dptsrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDeadPTSRUNSKey));
    addSpecialString(dptsrunKey);
    
    //playtime paused
    auto pptallKey = std::make_shared<SpecialKey>("pptgen", "Adds your total paused accurate calculated playtime (shared with linked levels)");
    pptallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPausedPTALLSKey));
    addSpecialString(pptallKey);

    auto pptf0Key = std::make_shared<SpecialKey>("pptf0", "Adds your total paused accurate calculated playtime from 0 (shared with linked levels)");
    pptf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPausedPTF0SKey));
    addSpecialString(pptf0Key);

    auto pptrunKey = std::make_shared<SpecialKey>("pptruns", "Adds your total paused accurate calculated playtime in runs (shared with linked levels)");
    pptrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPausedPTRUNSKey));
    addSpecialString(pptrunKey);

    auto pptsallKey = std::make_shared<SpecialKey>("pptsgen", "Adds your total paused accurate calculated session playtime");
    pptsallKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    pptsallKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPausedPTSALLSKey));
    addSpecialString(pptsallKey);

    auto pptsf0Key = std::make_shared<SpecialKey>("pptsf0", "Adds your total paused accurate calculated session playtime from 0");
    pptsf0Key->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    pptsf0Key->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPausedPTSF0Key));
    addSpecialString(pptsf0Key);

    auto pptsrunKey = std::make_shared<SpecialKey>("pptsruns", "Adds your total paused accurate calculated session playtime in runs");
    pptsrunKey->refreshWith({
        sessionFrom0Key->getKey(),
        sessionRunsKey->getKey()
    });
    pptsrunKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onPausedPTSRUNSKey));
    addSpecialString(pptsrunKey);


    auto sectionKey = std::make_shared<SpecialKey>("section", "Adds your section runs");
    sectionKey->refreshWith(runsKey->getKey());
    sectionKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSectionKey));
    addSpecialString(sectionKey);

    auto sdateKey = std::make_shared<SpecialKey>("sdate", "Adds the date of the current session grouping");
    sdateKey->refreshWith(sessionFrom0Key->getKey());
    sdateKey->refreshWith(sessionRunsKey->getKey());
    sdateKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onSessionDateKey));
    addSpecialString(sdateKey);

    auto lvlRunsKey = std::make_shared<SpecialKey>("lvlruns", "Gets how many times you completed the level in x runs");
    lvlRunsKey->refreshWith(generalKey->getKey());
    lvlRunsKey->refreshWith(runsKey->getKey());
    lvlRunsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onLevelRunsKey));
    addSpecialString(lvlRunsKey);
}

void DTLayer::UpdateSharedStats(){
    if (m_MyLevelStats.isErr()){
        linkedLevelsData.clear();
        return;
    }
    auto sharedStats = m_MyLevelStats.unwrap();

    linkedLevelsData.clear();
    sessionsOrder.grouping.clear();
    sessionsOrder.groupName = "None";

    std::set<std::string> linkedLevels{};
    std::map<std::string, LevelData> visitedLevels{};
    linkedLevels.insert(sharedStats.metadata.linkedLevels.begin(), sharedStats.metadata.linkedLevels.end());

    while (true){
        auto startSize = linkedLevels.size();
        for (const auto& linkedLevel : linkedLevels)
        {
            if (visitedLevels.contains(linkedLevel)) continue;

            auto currStatsRes = StatsManager::getLevelData(linkedLevel);
            if (currStatsRes.isErr()){
                Notification::create(fmt::format("failed to get data for linked level - {} | {}", linkedLevel, currStatsRes.unwrapErr().error))->show();
                continue;
            }
            auto currStats = currStatsRes.unwrap();

            linkedLevels.insert(currStats.metadata.linkedLevels.begin(), currStats.metadata.linkedLevels.end());
            visitedLevels.insert({currStats.levelKey, currStats});
        }

        if (startSize == linkedLevels.size()) break;
    }

    visitedLevels.insert({sharedStats.levelKey, sharedStats});

    for (const auto& [_, level] : visitedLevels){
        std::for_each(level.sessionNames.begin(), level.sessionNames.end(), [&](long long key) {
            if (this->sessionsOrder.grouping.contains(key)){
                if (!this->sessionsOrder.grouping[key].group[key].contains(level.levelKey))
                    this->sessionsOrder.grouping[key].group[key].insert(level.levelKey);
            }
            else{
                SessionGrouping grouping{};
                grouping.group.insert({key, {level.levelKey}});
                this->sessionsOrder.grouping.insert({key, grouping});
            }
        });

        linkedLevelsData.push_back(level);
    }

    if (sessionSelector != nullptr)
        sessionSelector->setMaximumCount(sessionsOrder.grouping.size(), false);

    updateStaticGroupings();

    UpdateDeathRelatedStrings();
}

void DTLayer::onSettings(CCObject*){
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::keyBackClicked(){
    if (isEditingLayout){
        if (layoutOptionsLayer != nullptr){
            if (!layoutOptionsLayer->isEditingNode())
                onApplyLayoutChanges(nullptr);
            else{
                layoutOptionsLayer->keyBackClicked();
            }
        }
    }
    else{
        DTLayer::onClose(nullptr);
    }
}

void DTLayer::onClose(CCObject* sender){
    if (m_MyLevelStats.isOk() && Settings::getAutoBackupEnabled() && Settings::getAutoBackupAtDTExit()){
        if (m_MyLevelStats.unwrap().metadata.autoBackup){
            StatsManager::addBackup(
                m_MyLevelStats.unwrap().levelKey,
                Settings::getAutoBackupGeneral(),
                Settings::getAutoBackupSessionAmount()
            );
        }
    }

    organizationListener.cancel();

    for (auto& [_, key] : specialStrings) {
        if (key)
        {
            key->setUpdateCompletedCallback(nullptr);
            key->setUpdateStartedCallback(nullptr);
            key->cancel();
        }
    }

    keyListeners.clear();

    instance = nullptr;

    verText->stopAllActions();
    verText->runAction(CCFadeTo::create(.2f, 0));

    Popup::onClose(sender);
}

void DTLayer::show(){
    Popup::show();
    this->setZOrder(100);

    m_mainLayer->stopAllActions();
    m_mainLayer->setScale(1);
}

void DTLayer::keyDown(enumKeyCodes key, double d){
    scrollLayer->keyDown(key, d);
}
void DTLayer::keyUp(enumKeyCodes key, double d){
    scrollLayer->keyUp(key, d);
}

void DTLayer::onLSOClicked(CCObject*){
    DTLevelSpecificSettingsLayer::create()->show();
}

DTLayer* DTLayer::get() { return instance; }

AdvancedScrollLayer* DTLayer::getScrollLayer(){
    return scrollLayer;
}

void DTLayer::UpdateDeathRelatedStrings(){
    if (m_MyLevelStats.isErr()) return;

    for (const auto& [_, key] : specialStrings)
    {
        if (key->getKey() == "general" || key->getKey() == "s0" || key->getKey() == "runs" || key->getKey() == "sruns")
            key->updateContent();
    }
}

void DTLayer::updateStaticGroupings(){
    daySGroup.groupName = "Day";
    daySGroup.grouping.clear();
    weekSGroup.groupName = "Week";
    weekSGroup.grouping.clear();
    monthSGroup.groupName = "Month";
    monthSGroup.grouping.clear();

    for (const auto& [date, group] : sessionsOrder.grouping)
    {
        for (const auto& [date, lvls] : group.group)
        {
            std::time_t time = static_cast<std::time_t>(date);
            std::tm lt{};
            #if defined(_WIN32)
                localtime_s(&lt, &time);
            #else
                localtime_r(&time, &lt);
            #endif

            lt.tm_sec = 0;
            lt.tm_min = 0;
            lt.tm_hour = 0;
            std::time_t startOfDay = std::mktime(&lt);

            lt.tm_mday = 1;
            std::time_t startOfMonth = std::mktime(&lt);
            
            #if defined(_WIN32)
                localtime_s(&lt, &time);
            #else
                localtime_r(&time, &lt);
            #endif
            
            lt.tm_sec = 0; lt.tm_min = 0; lt.tm_hour = 0;
            lt.tm_mday -= lt.tm_wday;
            std::time_t startOfWeek = std::mktime(&lt);
            
            if (!daySGroup.grouping.contains(startOfDay)){
                SessionGrouping grouping{};
                daySGroup.grouping.insert({startOfDay, grouping});
            }
            daySGroup.grouping[startOfDay].group.insert({date, lvls});

            if (!weekSGroup.grouping.contains(startOfWeek)){
                SessionGrouping grouping{};
                weekSGroup.grouping.insert({startOfWeek, grouping});
            }
            weekSGroup.grouping[startOfWeek].group.insert({date, lvls});

            if (!monthSGroup.grouping.contains(startOfMonth)){
                SessionGrouping grouping{};
                monthSGroup.grouping.insert({startOfMonth, grouping});
            }
            monthSGroup.grouping[startOfMonth].group.insert({date, lvls});
        }
    }  
}

bool DTLayer::createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests, const ccColor3B& newBestColoring, bool ignoreExtraSettings){
    out = "";
    if (m_MyLevelStats.isErr()) return false;
    auto& stats = m_MyLevelStats.unwrap();

    auto toReturn = createDeathsString(deaths, stats.metadata, custom, out, newBests, newBestColoring, ignoreExtraSettings);

    return toReturn;
}

bool DTLayer::createDeathsString(const Deaths& deaths, const LevelMetadeta& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests,  const ccColor3B& newBestColoring, bool ignoreExtraSettings) {
    out = "";

    std::vector<std::pair<std::string, int>> deathVec;

    if (!ignoreExtraSettings)
    {
        std::unordered_map<std::string, int> mergedDeaths;

        for (const auto& [originalRunStr, count] : deaths) 
        {
            auto runSplitRes = StatsManager::splitRunKey(originalRunStr);
            if (runSplitRes.isErr()) continue;
            auto runSplit = runSplitRes.unwrap();

            if (runSplit.end >= meta.realEndPercent)
                runSplit.end = 100;

            if (runSplit.start.has_value() && runSplit.start.value() >= meta.realEndPercent)
                runSplit.start = 100;

            auto res = StatsManager::createRunKey(runSplit);
            if (res.isErr()) continue;

            mergedDeaths[res.unwrap()] += count;
        }

        deathVec.assign(mergedDeaths.begin(), mergedDeaths.end());
    } 
    else 
    {
        for (const auto& [originalRunStr, count] : deaths) 
        {
            auto runSplitRes = StatsManager::splitRunKey(originalRunStr);
            if (runSplitRes.isErr()) continue;
            
            auto res = StatsManager::createRunKey(runSplitRes.unwrap());
            if (res.isErr()) continue;

            deathVec.emplace_back(res.unwrap(), count);
        }
    }
        

    std::sort(deathVec.begin(), deathVec.end(), [](const auto& a, const auto& b) {
        auto runARes = StatsManager::splitRunKey(a.first);
        auto runBRes = StatsManager::splitRunKey(b.first);

        if (runARes.isErr() || runBRes.isErr()) {
            log::error(
                "Failed to split run key! {} | {}",
                runARes.isErr() ? runARes.unwrapErr() : "",
                runBRes.isErr() ? runBRes.unwrapErr() : ""
            );

            return a.first < b.first;
        }

        auto& runA = runARes.unwrap();
        auto& runB = runBRes.unwrap();

        if (runA.start == runB.start)
            return runA.end < runB.end;

        return runA.start < runB.start;
    });

    int prevStart = -2;
    int hiddenRuns = 0;
    int runIndex = 0;

    for (const auto& [run, amount] : deathVec) {
        auto runSplitRes = StatsManager::splitRunKey(run);

        if (runSplitRes.isErr()) {
            log::error("Failed to split run key! {}", runSplitRes.unwrapErr());
            continue;
        }

        auto& runSplit = runSplitRes.unwrap();

        bool includeRunStart = runSplit.start.has_value();

        std::string nbDeColor;
        std::string nbColor;

        if (includeRunStart && !ignoreExtraSettings) {
            int startValue = runSplit.start.value();

            if (!meta.showAnyRun && !meta.runsToShow.empty()) {
                auto it = meta.runsToShow.find(startValue);

                if (it != meta.runsToShow.end()) {
                    if (it->second > runSplit.end) {
                        hiddenRuns += amount;
                        continue;
                    }
                }
                else {
                    hiddenRuns += amount;
                    continue;
                }
            }
            else {
                if (meta.sharedRunToShow > runSplit.end - startValue) {
                    hiddenRuns += amount;
                    continue;
                }
            }

            if (runIndex % 2 == 0) {
                nbDeColor = "</cplus>";
                nbColor = fmt::format("<cplus={}>", custom.alternateStrength);
            }
        }
        else if (!includeRunStart && !ignoreExtraSettings) {
            if (meta.hideUpto > runSplit.end && runSplit.end >= 0) {
                hiddenRuns += amount;
                continue;
            }

            if (newBests.has_value() && newBests.value().contains(runSplit.end)) {
                nbDeColor = "</color>";

                // constexpr int lowerAmount = 135;

                // int lowerR = std::min<int>(newBestColoring.r + lowerAmount, 255);
                // int lowerG = std::min<int>(newBestColoring.g + lowerAmount, 255);
                // int lowerB = std::min<int>(newBestColoring.b + lowerAmount, 255);

                nbColor = fmt::format(
                    //"<gradient={},{},.3,.15><wave>",
                    "<color={}>",
                    cc3bToHexString(newBestColoring)
                    // cc3bToHexString(newBestColoring),
                    // cc3bToHexString(ccColor3B{
                    //     static_cast<GLubyte>(lowerR),
                    //     static_cast<GLubyte>(lowerG),
                    //     static_cast<GLubyte>(lowerB)
                    // })
                );
            }
            else if (runIndex % 2 == 0) {
                nbDeColor = "</cplus>";
                nbColor = fmt::format("<cplus={}>", custom.alternateStrength);
            }
        }
        else{
            if (newBests.has_value() && newBests.value().contains(runSplit.end)) {
                nbDeColor = "</color>";

                // constexpr int lowerAmount = 135;

                // int lowerR = std::min<int>(newBestColoring.r + lowerAmount, 255);
                // int lowerG = std::min<int>(newBestColoring.g + lowerAmount, 255);
                // int lowerB = std::min<int>(newBestColoring.b + lowerAmount, 255);

                nbColor = fmt::format(
                    //"<gradient={},{},.3,.15><wave>",
                    "<color={}>",
                    cc3bToHexString(newBestColoring)
                    // cc3bToHexString(newBestColoring),
                    // cc3bToHexString(ccColor3B{
                    //     static_cast<GLubyte>(lowerR),
                    //     static_cast<GLubyte>(lowerG),
                    //     static_cast<GLubyte>(lowerB)
                    // })
                );
            }
        }

        int currentStart = runSplit.start.value_or(-1);

        if (prevStart != currentStart) {
            if (prevStart != -2) {
                out += "----------{nl}";
            }

            prevStart = currentStart;
        }

        auto format = custom.format;

        std::string toReplaceWith =
            includeRunStart
                ? fmt::format("{}-{}", runSplit.start.value(), runSplit.end)
                : fmt::format("{}", runSplit.end);

        format = std::regex_replace(
            format,
            std::regex("\\{per\\}"),
            toReplaceWith
        );

        format = std::regex_replace(
            format,
            std::regex("\\{d\\}"),
            std::to_string(amount)
        );

        out += fmt::format(
            "{}{}{}{}",
            nbColor,
            format,
            nbDeColor,
            custom.seperator
        );

        runIndex++;
    }

    if (out.empty() && hiddenRuns == 0) {
        out = "No Deaths Found!";
    }
    else {
        if (!custom.seperator.empty() && out.size() >= custom.seperator.size()) {
            out.erase(out.size() - custom.seperator.size());
        }
    }

    if (hiddenRuns != 0) {
        if (!out.empty()) {
            out += "{nl}";
        }

        out += fmt::format("{}x hidden", hiddenRuns);
    }

    return true;
}

int DTLayer::getCurrentSelectedSession(){
    return sessionSelector->getCurrentCount();
}

void DTLayer::onSessionSelected(int sessionNum, bool updateContent){
    auto it = getCurrentGrouping().grouping.begin();
    std::advance(it, sessionNum - 1);

    if (it == getCurrentGrouping().grouping.end()) return;

    if (sessionNum - 1 == currentSession) return;
    currentSession = sessionNum - 1;

    if (sessionSelector->getCurrentCount() != currentSession + 1) sessionSelector->setCurrentCount(currentSession + 1);

    if (updateContent){
        specialStrings["s0"]->updateContent();
        specialStrings["sruns"]->updateContent();
    }
}

//better info time calc

uint64_t DTLayer::timeInMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string DTLayer::decodeBase64Gzip(const std::string& input) {
    return ZipUtils::decompressString(input, false, 0);
}

inline bool objectIDIsSpeedPortal(int id) {
    return (id == 200 || id == 201 || id == 202 || id == 203 || id == 1334);
}

inline int speedToPortalId(int speed) {
    switch(speed) {
    default:
        return 201;
        break;
    case 1:
        return 200;
        break;
    case 2:
        return 202;
        break;
    case 3:
        return 203;
        break;
    case 4:
        return 1334;
        break;
    }
}

inline float travelForPortalId(int speed) {
    switch (speed)
    {
    case 200:
        return 251.16008f;
        break;
    default:
        return 311.58011f;
        break;
    case 202:
        return 387.42014f;
        break;
    case 203:
        return 468.00015f;
        break;
    case 1334:
        return 576.00018f;
        break;
    }
}

int DTLayer::stoi(std::string_view str) {
    int result = 0;
    std::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
float DTLayer::stof(std::string_view str) {
    return utils::numFromString<float>(str).unwrapOr(0);
}

float DTLayer::timeForLevelString(const std::string& levelString) {
    struct SpeedPortalObject {
        int id;
        float xPos;
        bool checked;
    };

    try {
        auto a = timeInMs();

        auto decompressString = decodeBase64Gzip(levelString);
        auto c = timeInMs();
        std::stringstream responseStream(decompressString);
        std::string currentObject;
        std::string currentKey;
        std::string keyID;
        std::vector<SpeedPortalObject> speedPortals;

        //std::stringstream objectStream;
        float prevPortalX = 0;
        int prevPortalId = 0;

        float timeFull = 0;

        float maxPos = 0;
        while(getline(responseStream, currentObject, ';')){
            size_t i = 0;
            int objID = 0;
            float xPos = 0;
            bool checked = false;

            /*objectStream.str("");
            objectStream.clear();
            objectStream << currentObject;
            objectStream.seekp(0);
            objectStream.seekg(0);*/
            std::stringstream objectStream(currentObject);
            while(getline(objectStream, currentKey, ',')) {
                if(i % 2 == 0) keyID = currentKey;
                else {
                    if(keyID == "1") objID = DTLayer::stoi(currentKey);
                    else if(keyID == "2") xPos = DTLayer::stof(currentKey);
                    else if(keyID == "13") checked = DTLayer::stoi(currentKey);
                    else if(keyID == "kA4") prevPortalId = speedToPortalId(DTLayer::stoi(currentKey));
                }
                i++;

                if(xPos != 0 && objID != 0 && checked == true) break;
            }

            if(maxPos < xPos) maxPos = xPos;
            if(!checked || !objectIDIsSpeedPortal(objID)) continue;

            speedPortals.push_back({objID, xPos, checked});
        }

        std::sort(speedPortals.begin(), speedPortals.end(), [](const SpeedPortalObject& a, const SpeedPortalObject& b) {
            return a.xPos < b.xPos;
        });

        for(const auto& portal : speedPortals) {
            //log::info("Object ID: {}, X Position: {}, Portal ID: {}", portal.id, portal.xPos, prevPortalId);
            timeFull += (portal.xPos - prevPortalX) / travelForPortalId(prevPortalId);
            prevPortalId = portal.id;
            prevPortalX = portal.xPos;
        }

        //log::info("Last portal ID: {}, Last X Position: {}", prevPortalId, prevPortalX);
        timeFull += (maxPos - prevPortalX) / travelForPortalId(prevPortalId);
        auto b = timeInMs() - a;
        //log::info("Time for levelString: {}ms, decompress: {}ms, parse: {}ms, maxPos {}", b, c - a, timeInMs() - c, maxPos);
        return timeFull;
    } catch(std::exception e) {
        log::error("An exception has occured while calculating time for levelString: {}", e.what());
        return 0;
    }
}

void DTLayer::update(float dt){
    columnHolder->updateLayout();

    float oldWidth = scrollLayer->content->getContentWidth();

    float width = std::max(ogLimits.width, columnHolder->getContentWidth() - (isEditingLayout ? 0 : addColumnButton->getContentWidth()));

    if (oldWidth != width){
        scrollLayer->setLimitsWidth(width);
        scrollLayer->zoomBy(0);
        float delta = scrollLayer->content->getContentWidth() - oldWidth;

        scrollLayer->moveBy(ccp(delta / 2, 0));
    }
}

void DTLayer::organizeLayout(){
    if (!canOrganize) return;
    organizationListener.spawn(
        "DT-organize-layout-task",
        organizeLayoutTask(),
        [this](organizationFuture::Output result){
            if (scrollLayer->isAtMinZoom())
                scrollLayer->zoomBy(0.001f);


            float fixedhighest = result.highestColumn - (isEditingLayout ? 0 : LayoutColumn::topHeight + LayoutColumn::addNewBtnOffset * 2);
            for (const auto& column : columns){
                column->setContentHeight(fixedhighest);
            }

            float cappedHeight = std::max(fixedhighest, ogLimits.height);

            float oldHeightLimits = scrollLayer->content->getContentHeight();

            float oldTop = scrollLayer->getMaximumPosition(false);
            float oldBottom = scrollLayer->getMinimumPosition(false);
            float oldHeight = oldBottom - oldTop;

            float offsetFromTop = scrollLayer->content->getPositionY() - oldTop;

            scrollLayer->setLimitsHeight(cappedHeight);
            columnHolder->setPositionY(cappedHeight);
            columnHolder->updateLayout();
            labelsHolder->setPosition(columnHolder->getPosition());

            float delta = 0;

            delta = oldHeightLimits - scrollLayer->content->getContentHeight();

            scrollLayer->moveBy(ccp(0, delta / 2));
            float newTop = scrollLayer->getMaximumPosition(false);
            float newBottom = scrollLayer->getMinimumPosition(false);
            float newHeight = newBottom - newTop;

            float newOffset = offsetFromTop * (newHeight / oldHeight);

            scrollLayer->content->setPositionY(newTop + newOffset);

            for (const auto& [label, newPos, newWidth] : result.labelData)
            {
                const int MOVEMENT_TAG = 2;
                const int RESIZE_TAG = 7;

                auto taggedMovementAction = label->getActionByTag(MOVEMENT_TAG);
                auto taggedResizeAction = label->getActionByTag(RESIZE_TAG);

                bool doCreateNewMoveAction = true;
                bool doCreateNewResizeAction = true;

                //movement action check
                if (taggedMovementAction != nullptr){
                    auto currentMovementActionEase = static_cast<CCEaseInOut*>(taggedMovementAction);
                    auto currentMovementAction = static_cast<CCMoveTo*>(currentMovementActionEase->getInnerAction());

                    if (!currentMovementAction->m_endPosition.equals(newPos)){
                        label->stopActionByTag(MOVEMENT_TAG);
                    }
                    else doCreateNewMoveAction = false;
                }

                if (doCreateNewMoveAction){
                    auto pos = newPos + ccp(0, isEditingLayout ? 0 : LayoutColumn::topHeight);
                    if (!cornerOnNextOrganization){
                        auto movementAction = CCEaseInOut::create(CCMoveTo::create(DTLayer::transitionTime, pos), 2);
                        movementAction->setTag(MOVEMENT_TAG);

                        label->runAction(movementAction);
                    }
                    else{
                        label->setPosition(pos);
                    }
                }

                //resize action check
                if (taggedResizeAction != nullptr){
                    auto currentResizeActionEase = static_cast<CCEaseInOut*>(taggedResizeAction);
                    auto currentResizeAction = static_cast<CCResizeWidthTo*>(currentResizeActionEase->getInnerAction());

                    if (currentResizeAction->endWidth != newWidth){
                        label->stopActionByTag(RESIZE_TAG);
                    }
                    else doCreateNewResizeAction = false;
                }

                if (doCreateNewResizeAction){
                    if (!cornerOnNextOrganization){
                        auto resizeAction = CCEaseInOut::create(CCResizeWidthTo::create(DTLayer::transitionTime, newWidth), 2);
                        resizeAction->setTag(RESIZE_TAG);

                        label->runAction(resizeAction);
                    }
                    else{
                        label->setContentWidth(newWidth);
                    }
                }
            }

            for (const auto& [target, _] : onOrganizationCompleteEvent)
            {
                onOrganizationCompleteEvent[target](delta);
            }

            if (cornerOnNextOrganization){
                cornerOnNextOrganization = false;
                scrollLayer->moveToCorner(true, false);
            }

            if (firstTime == 0){
                firstTime = 1;
                scrollLayer->setVisible(true);
                lc->removeMeAndCleanup();
                cornerOnNextOrganization = false;

                this->organizeLayout();
            }
            else if (firstTime == 1){
                firstTime = 2;

                scrollLayer->moveToCorner(true, false);
            }
        }
    );
}

organizationFuture DTLayer::organizeLayoutTask(){
    struct LabelData {
        DTLabel* label;
        std::set<LayoutColumn*> holders;
    };

    struct ColumnData {
        LayoutColumn* column;
        int orderPos;
        std::vector<std::pair<int, DTLabel*>> labels;
    };

    std::vector<ColumnData> columnSnapshots;
    std::map<DTLabel*, LabelData> labelSnapshots;

    for (const auto& column : columns)
    {
        ColumnData colData;
        colData.column = column;
        colData.orderPos = column->info.orderPos;

        for (const auto& [layer, label] : column->labels)
        {
            colData.labels.push_back({layer, label});

            if (labelSnapshots.find(label) == labelSnapshots.end())
            {
                auto holders = label->getHolders();
                std::set<LayoutColumn*> holderSet;
                for (const auto& holder : holders)
                {
                    holderSet.insert(holder);
                }
                labelSnapshots[label] = {label, holderSet};
            }
        }

        columnSnapshots.push_back(colData);
    }

    co_await arc::yield();

    std::set<DTLabel*> allLabels{};
    std::map<DTLabel*, std::set<LayoutColumn*>> labelHolders{};

    for (const auto& colData : columnSnapshots)
    {
        for (const auto& [layer, label] : colData.labels)
        {
            co_await arc::yield();
            if (allLabels.contains(label)) continue;

            allLabels.insert(label);
            label->tempPos = ccp(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            label->tempWidth = 0;
        }

        co_await arc::yield();
    }

    for (const auto& [label, data] : labelSnapshots)
    {
        labelHolders[label] = data.holders;

        co_await arc::yield();
    }

    std::map<LayoutColumn*, DTLabel*> lastVisitedLabelForColumn{};
    std::map<DTLabel*, std::map<LayoutColumn*, std::optional<int>>> labelAwaitingColumnValues{};
    std::set<DTLabel*> processedLabels{};

    auto UpdateTempPos = [&](LayoutColumn* column, DTLabel* label, DTLabel* prevLabel) -> arc::Future<> {
        if (column == nullptr || label == nullptr || label->getParent() == nullptr) co_return;
        auto startPosInLabelSpace = label->getParent()->convertToNodeSpace(column->convertToWorldSpace(column->bgSpr->getPosition()));

        float prevHeight = startPosInLabelSpace.y;

        if (label->info.layer != 0 && prevLabel != nullptr) prevHeight = prevLabel->tempPos.y - prevLabel->getContentHeight();

        if (prevHeight < label->tempPos.y) label->tempPos.y = prevHeight;

        float newX = startPosInLabelSpace.x;

        if (newX < label->tempPos.x) label->tempPos.x = newX;
    };

    auto UpdateTempWidth = [](DTLabel* label) -> arc::Future<> {
        label->tempWidth = 0;
        for (const auto& labelColumn : label->getHolders())
        {
            label->tempWidth += labelColumn->getContentWidth();

            co_await arc::yield();
        }
    };

    while (true){
        for (const auto& colData : columnSnapshots)
        {
            const auto& column = colData.column;
            // log::info("goind over column {}", colData.orderPos);
            DTLabel* prevLabel = nullptr;

            bool foundLastLabel = false;

            for (const auto& [labelLayer, label] : colData.labels)
            {
                // log::info("going through label");
                if (!lastVisitedLabelForColumn.contains(column)){
                    lastVisitedLabelForColumn.insert({column, label});
                    foundLastLabel = true;
                }
                else if (label != lastVisitedLabelForColumn[column] && !foundLastLabel){
                    co_await UpdateTempPos(column, label, prevLabel);
                    prevLabel = label;
                    continue;
                }
                else foundLastLabel  = true;

                if (processedLabels.contains(label)){
                    co_await UpdateTempPos(column, label, prevLabel);
                    prevLabel = label;
                    continue;
                }

                // log::info("label valid");

                lastVisitedLabelForColumn[column] = label;

                int newLayer = prevLabel == nullptr ? 0 : prevLabel->info.layer + 1;

                // log::info("destenation layer {}", newLayer);

                auto& holdersBack = labelHolders[label];

                if (holdersBack.size() > 1){
                    // log::info("label is multicolumn");
                    if (!labelAwaitingColumnValues.contains(label)){
                        std::map<LayoutColumn*, std::optional<int>> mapToSet{};

                        for (const auto& holder : holdersBack){
                            mapToSet.insert({holder, std::nullopt});
                            co_await arc::yield();
                        }

                        // log::info("populated list for label with {} holders", mapToSet.size());

                        labelAwaitingColumnValues.insert({label, mapToSet});
                    }

                    // log::info("adding label value?");

                    if (labelAwaitingColumnValues[label].contains(column) && !labelAwaitingColumnValues[label][column].has_value()){
                        // log::info("label value added");
                        labelAwaitingColumnValues[label][column] = newLayer;
                    }

                    int highestOptLayer = 0;
                    bool wereAllLayersFound = true;

                    co_await UpdateTempPos(column, label, prevLabel);

                    // log::info("checking conclusion..");

                    for (const auto& [column, optLayer] : labelAwaitingColumnValues[label]){
                        if (!optLayer.has_value()){
                            wereAllLayersFound = false;
                            co_await arc::yield();
                            break;
                        }

                        highestOptLayer = std::max(highestOptLayer, optLayer.value());
                        co_await arc::yield();
                    }

                    if (!wereAllLayersFound){
                        // log::info("invalid labels were found");
                        co_await arc::yield();
                        break;
                    }

                    // log::info("all layer values were found for label");

                    label->info.layer = highestOptLayer;
                    processedLabels.insert(label);

                    co_await UpdateTempWidth(label);

                    // log::info("combo found at {}", highestOptLayer);

                    prevLabel = label;
                    continue;
                }

                // log::info("non double found! adding..");

                label->info.layer = newLayer;
                processedLabels.insert(label);

                co_await UpdateTempPos(column, label, prevLabel);
                co_await UpdateTempWidth(label);

                // log::info("single found at {}", newLayer);

                prevLabel = label;
            }
            co_await arc::yield();
        }

        // log::info("res: {} | {}", processedLabels.size(), allLabels.size());

        if (processedLabels.size() == allLabels.size()) break;
        co_await arc::yield();
    }

    for (const auto& colData : columnSnapshots){
        colData.column->refreshAllLabelsLayer();
        co_await arc::yield();
    }

    float heighestHeight = 0;

    for (const auto& colData : columnSnapshots){
        for (const auto& [_, label] : colData.labels)
        {
            auto height = std::abs(label->tempPos.y) + label->getContentHeight() + LayoutColumn::addNewBtnOffset * 2;
            if (heighestHeight < height) heighestHeight = height;
            co_await arc::yield();
        }
        co_await arc::yield();
    }

    organizationResult data{};
    data.highestColumn = heighestHeight;

    for (const auto& label : allLabels)
    {
        auto targetPosition = label->tempPos;
        auto targetWidth = label->tempWidth - LayoutColumn::borderWidth;

        data.labelData.push_back({label, targetPosition, targetWidth});
        co_await arc::yield();
    }

    co_await arc::yield();
    co_return data;
}

std::pair<LayoutColumn*, int> DTLayer::getColumnLayerFromPosition(CCPoint posInWorldSpace){

    auto posInColumnHolderSpace = columnHolder->convertToNodeSpace(posInWorldSpace);

    LayoutColumn* columnFound = nullptr;

    for (const auto& column : columns)
    {
        if (column->boundingBox().containsPoint(posInColumnHolderSpace + ccp(DTColumnInfo::minWidth / 2, 0))){
            columnFound = column;
            // log::info("found column {}", columnFound->orderPos);
            break;
        }
    }

    if (columnFound == nullptr) return {nullptr, 0};

    auto posInLabelHolderSpace = labelsHolder->convertToNodeSpace(posInWorldSpace);

    if (!columnFound->labels.size()) return {columnFound, 0};

    int highestLayerFound = 0;

    for (const auto& [labelLayer, label] : columnFound->labels)
    {
        auto highestLabelPoint = label->getPositionY() - label->getContentHeight() + DTLabel::labelTitleHeight / 2;

        if (highestLayerFound < labelLayer) highestLayerFound = labelLayer;

        if (highestLabelPoint < posInLabelHolderSpace.y){

            return {columnFound, labelLayer};
        }
    }

    return {columnFound, highestLayerFound + 1};
}

std::set<LayoutColumn*, ColumnComperator> DTLayer::getColumnsBetween(CCPoint a, CCPoint b){

    std::set<LayoutColumn*, ColumnComperator> toReturn{};

    a = columnHolder->convertToNodeSpace(a);
    b = columnHolder->convertToNodeSpace(b);

    auto x = std::min(a.x, b.x);
    auto y = std::min(a.y, b.y);
    auto w = fabs(b.x - a.x);
    auto h = fabs(b.y - a.y);

    auto rect = CCRect{x, y, w, h};

    // log::info("checking rects... {}", rect);

    for (const auto& column : columns)
    {
        if (rect.intersectsRect(column->boundingBox()))
            toReturn.insert(column);
    }

    return toReturn;
}

void DTLayer::addColumnBtnClicked(CCObject*){
    addColumn();
}

LayoutColumn* DTLayer::addColumn(std::optional<DTColumnInfo> info){
    if (!info.has_value()){
        int position = 0;

        if (columnHolder->getChildrenCount() > 1){
            auto highestColumn = static_cast<CCNode*>(columnHolder->getChildren()->objectAtIndex(columnHolder->getChildrenCount() - 2));
            position = highestColumn->getZOrder() + 1;
        }

        info = DTColumnInfo{
            .orderPos = position
        };
    }

    auto column = LayoutColumn::create(info.value(), isEditingLayout, scrollLayer->getContentHeight());
    columnHolder->addChild(column);
    columns.insert(column);

    return column;
}


DTLabel* DTLayer::createNewLabel(DTLabelInfo info){
    auto newLabel = DTLabel::create(info);
    labelsHolder->addChild(newLabel);
    if (isEditingLayout) newLabel->setEditable(true);

    return newLabel;
}


void DTLayer::subscribeToOrganizationEvent(CCNode* target, geode::Function<void(float)> callback){
    if (onOrganizationCompleteEvent.contains(target)) return;

    onOrganizationCompleteEvent.insert({target, std::move(callback)});
}
void DTLayer::unsubscribeToOrganizationEvent(CCNode* target){
    if (!onOrganizationCompleteEvent.contains(target)) return;

    onOrganizationCompleteEvent.erase(target);
}

void DTLayer::setLayoutBy(const DTLayoutV3& layout)
{
    auto columnsCopy = columns;
    for (auto* column : columnsCopy)
    {
        column->destroyColumnAndCleanup();
    }
    columns.clear();

    for (const auto& column : layout.columns)
    {
        addColumn(column);
    }

    fixUpColumnPositions();
    columnHolder->updateLayout();

    std::map<LayoutColumn*, std::vector<DTLabel*>> labelsForColumns;
    std::vector<DTLabel*> allLabels;
    allLabels.reserve(layout.labels.size());

    for (const auto& label : layout.labels)
    {
        DTLabel* labelNode = createNewLabel(label);
        allLabels.push_back(labelNode);

        const int minPos = labelNode->info.minPlacementRange;
        const int maxPos = labelNode->info.maxPlacementRange;

        for (auto* column : columns)
        {
            const int pos = column->info.orderPos;

            if (pos < minPos)
                continue;

            if (pos > maxPos)
                break;

            labelsForColumns[column].push_back(labelNode);
        }
    }

    for (auto& [column, labels] : labelsForColumns)
    {
        for (auto* label : labels)
        {
            column->addLabel(label);
        }
    }

    for (auto* label : allLabels)
    {
        if (label->isAlone())
        {
            label->removeMeAndCleanup();
        }
    }

    cornerOnNextOrganization = true;
}

void DTLayer::fixUpColumnPositions(){

    int supposedOrder = 0;

    for (const auto& column : columns)
    {
        column->info.orderPos = supposedOrder;
        column->setZOrder(column->info.orderPos);

        supposedOrder++;
    }
}

void DTLayer::saveCurrentLayout(){
    Save::setLayout(currentLayout());
}

DTLayoutV3 DTLayer::currentLayout(){
    std::set<DTLabel*> labels{};

    DTLayoutV3 layout{};

    for (const auto& column : columns)
    {
        layout.columns.push_back(column->info);

        for (const auto& [labelLayer, label] : column->labels)
        {
            if (labels.contains(label)) continue;

            labels.insert(label);

            layout.labels.push_back(label->info);
        }
    }

    return layout;
}

void DTLayer::specialKeyUpdateStarted(const std::shared_ptr<SpecialKey>& key){
    for (const auto& label : keyListeners)
    {
        label->setLoading(key);
    }
}

void DTLayer::specialKeyUpdateCompleted(const std::shared_ptr<SpecialKey>& key){

    for (const auto& [_, otherKey] : specialStrings)
    {
        if (otherKey->doesRefreshWith(key->getKey()))
            otherKey->updateContent();
    }

    for (const auto& label : keyListeners)
    {
        label->completeLoading(key);
    }
}

void DTLayer::setOptionsLayerTo(DTLabel* label){
    if (layoutOptionsLayer == nullptr) return;
    if (!layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-70.5f, 0))
            )
        );
        layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );

        colorChangeBG->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );
    }

    layoutOptionsLayer->setEditedNodeTo(label);
}
void DTLayer::setOptionsLayerTo(LayoutColumn* column){
    if (layoutOptionsLayer == nullptr) return;
    if (!layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-70.5f, 0))
            )
        );
        layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );

        colorChangeBG->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );
    }

    layoutOptionsLayer->setEditedNodeTo(column);
}
void DTLayer::closeOptionsLayer(){
    if (layoutOptionsLayer == nullptr) return;
    if (layoutOptionsLayer->isEditingNode()){
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(70.5f, 0))
            )
        );
        layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(160, 0))
            )
        );

        colorChangeBG->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(160, 0))
            )
        );
    }

    layoutOptionsLayer->close();
}
void DTLayer::removeColumn(LayoutColumn* column){
    if (!columns.contains(column)) return;

    columns.erase(column);
}

void DTLayer::onApplyLayoutChanges(CCObject*){

    saveCurrentLayout();

    exitLayoutEditing();
}
void DTLayer::onDiscardLayoutChanges(CCObject*){

    Save::setNewBestColor(originalNewBestColor);
    Save::setSessionBestColor(originalSessionBestColor);

    setLayoutBy(Save::getLayout());

    specialStrings["general"]->updateContent();
    specialStrings["s0"]->updateContent();

    exitLayoutEditing();
}

void DTLayer::exitLayoutEditing(){
    isEditingLayout = false;

    closeOptionsLayer();
    
    std::set<DTLabel*> visitedLabels{};

    for (const auto& column : columns)
    {
        column->setVisibility(false);

        for (const auto& [_, label] : column->labels)
        {
            if (visitedLabels.contains(label)) continue;

            visitedLabels.insert(label);
            label->setEditable(false);
        }
    }

    bottomLeftMenu->setEnabled(true);
    bottomLeftMenu->stopAllActions();
    bottomLeftMenu->runAction(CCFadeTo::create(.15f, 255));

    bottomRightMenu->setEnabled(true);
    bottomRightMenu->stopAllActions();
    bottomRightMenu->runAction(CCFadeTo::create(.15f, 255));

    m_buttonMenu->setEnabled(true);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 255));
    editLayoutBtnSpr->stopAllActions();
    editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 255));

    groupsHolder->setEnabled(true);

    colorChangeBG->stopActionByTag(2);
    auto fa = CCFadeTo::create(.15f, 0);
    fa->setTag(2);
    colorChangeBG->runAction(fa);
    colorMenu->stopAllActions();
    colorMenu->runAction(CCFadeTo::create(.15f, 0));
    colorMenu->setEnabled(false);
    resetLayoutMenu->stopAllActions();
    resetLayoutMenu->runAction(CCFadeTo::create(.15f, 0));
    resetLayoutMenu->setEnabled(false);

    editLayoutMenu->setEnabled(false);
    layoutInfo->stopAllActions();
    layoutInfo->runAction(CCFadeTo::create(.15f, 0));
    layoutInfo->setEnabled(false);
    applyChangesButtonSpr->m_BGSprite->stopAllActions();
    applyChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 0));
    applyChangesButtonSpr->m_label->stopAllActions();
    applyChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 0));
    discardChangesButtonSpr->m_BGSprite->stopAllActions();
    discardChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 0));
    discardChangesButtonSpr->m_label->stopAllActions();
    discardChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 0));

    addColumnButton->stopAllActions();
    addColumnButton->runAction(CCFadeTo::create(.15f, 0));
    columnHolder->setEnabled(false);

    scrollLayer->moveBy(ccp(0, LayoutColumn::topHeight));

    presetList->close();

    this->organizeLayout();
}

void DTLayer::subscribeKeyListener(DTLabel* label){
    if (keyListeners.contains(label)) return;

    keyListeners.insert(label);
}
void DTLayer::unsubscribeKeyListener(DTLabel* label){
    if (!keyListeners.contains(label)) return;

    keyListeners.erase(label);
}

void DTLayer::modifyRun(int startPer, int amount, std::optional<int> sessionNumber){
    if (m_MyLevelStats.isErr()) return;

    auto processRun = [&, amount, startPer, sessionNumber](Deaths& data) -> bool {
        auto runStr = std::to_string(startPer);
        if (!data.contains(runStr)){
            if (amount < 0)
                return false;

            data.insert({runStr, amount});
            return true;
        }

        auto newNum = data[runStr] + amount;
        //log::info("{} | {} | {}", amount, data[runStr], newNum);

        if (newNum <= 0){
            data.erase(runStr);
            modifyNewBest(startPer, false, sessionNumber);
            return true;
        }

        data[runStr] = newNum;

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = getCurrentGrouping().grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()){
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.data.deaths.contains(std::to_string(startPer))) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()){
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        if (!processRun(session.value().data.deaths)) return;

        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processRun(from0Stats.deaths)){
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        //log::info("pros linked");
        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr() || linkedLevel.levelKey == stats.levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.deaths)){
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
                return;
            }
        }
    }
}
void DTLayer::modifyRun(int startPer, int endPer, int amount, std::optional<int> sessionNumber){
    if (m_MyLevelStats.isErr()) return;

    auto processRun = [&, amount, startPer, endPer](Deaths& data) -> bool {
        auto runStr = fmt::format("{}-{}", startPer, endPer);
        if (!data.contains(runStr)){
            if (amount < 0)
                return false;

            data.insert({runStr, amount});
            return true;
        }

        auto newNum = data[runStr] + amount;

        if (newNum <= 0){
            data.erase(runStr);
            return true;
        }

        data[runStr] = newNum;

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = getCurrentGrouping().grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()){
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.data.runs.contains(fmt::format("{}-{}", startPer, endPer))) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()){
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        if (!processRun(session.value().data.runs)) return;

        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processRun(from0Stats.runs)){
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr() || linkedLevel.levelKey == stats.levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.runs)){
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
                return;
            }
        }
    }
}

void DTLayer::modifyNewBest(int percent, bool makeTrue, std::optional<int> sessionNumber){
    if (m_MyLevelStats.isErr()) return;

    auto processBest = [&, percent, makeTrue](NewBests& bests) -> bool {
        if (makeTrue){
            if (bests.contains(percent)) return true;

            bests.insert(percent);
        }
        else{
            if (!bests.contains(percent)) return true;

            bests.erase(percent);
        }

        return true;
    };

    if (sessionNumber.has_value()){
        auto it = getCurrentGrouping().grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()){
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.data.newBests.contains(percent)) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()){
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        processBest(session.value().data.newBests);

        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else{
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.from0.isErr()) return;
        auto& from0Stats = stats.from0.unwrap();

        if (processBest(from0Stats.newBests)){
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        for (auto& linkedLevel : linkedLevelsData)
        {
            if (linkedLevel.from0.isErr() || linkedLevel.levelKey == stats.levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.from0.unwrap();

            if (processBest(linkedLevelFrom0Stats.newBests)){
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
                return;
            }
        }
    }
}

bool DTLayer::DeleteSave(){
    auto lvlRes = StatsManager::getLevelKey(m_Level);
    if (lvlRes.isErr()) return false;

    auto deleteRes = StatsManager::deleteLevelStats(lvlRes.unwrap());
    if (deleteRes.isErr()){
        log::error("{}", deleteRes.unwrapErr());
        return false;
    }

    SaveDeletionEvent().send();

    onClose(nullptr);
    return true;
}

UpdateFuture DTLayer::onNLKey(std::map<std::string, std::any> payload){
    co_return Ok(std::string("\n"));
}
UpdateFuture DTLayer::onATTKey(std::map<std::string, std::any> payload){
    long long totalAttempts = 0;
    for (const auto& lebel : linkedLevelsData)
        totalAttempts += lebel.metadata.attempts;

    co_return Ok(std::to_string(totalAttempts));
}
UpdateFuture DTLayer::onLVLNKey(std::map<std::string, std::any> payload){
    co_return Ok(std::string(m_Level->m_levelName));
}
UpdateFuture DTLayer::onGeneralKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto combinedDeaths, co_await getTFor<std::pair<Deaths, NewBests>>([](GeneralData const& data){
        return std::make_pair(data.deaths, data.newBests);
    },
    [](auto const& a, auto const& b){
        auto map = a.first;
        StatsManager::mergeMapsAdd(map, b.first);

        auto nbs = a.second;
        nbs.insert(b.second.begin(), b.second.end());

        return std::make_pair(map, nbs);
    }, false));

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(combinedDeaths.first, Save::getFrom0Customazations(), out, combinedDeaths.second, Save::getNewBestColor(), ignoreExtraSettings))
        co_return Err("Failed to create from0 deaths string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onRUNSKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto sharedRuns, co_await getTFor<Deaths>([](GeneralData const& data){
        return data.runs;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(sharedRuns, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings))
        co_return Err("Failed to create run deaths string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onS0Key(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto sessioNDeaths, co_await getTFor<std::pair<Deaths, NewBests>>([](GeneralData const& data){
        return std::make_pair(data.deaths, data.newBests);
    },
    NULL, true));
    
    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(sessioNDeaths.first, Save::getSessionF0Customazations(), out, sessioNDeaths.second, Save::getSessionBestColor(), ignoreExtraSettings))
        co_return Err("Failed to create session from0 deaths string");

    co_return Ok(out);
}
UpdateFuture DTLayer::onSRUNSKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto sruns, co_await getTFor<Deaths>([](GeneralData const& data){
        return data.runs;
    },
    NULL, true));

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(sruns, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings))
        co_return Err("Failed to create session run deaths string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onDTATTKey(std::map<std::string, std::any> payload){
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.deaths) + deathsFunc(data.runs);
    }, false);
}

UpdateFuture DTLayer::onDTF0ATTKey(std::map<std::string, std::any> payload){
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.deaths);
    }, false);
}

UpdateFuture DTLayer::onDTRunsATTKey(std::map<std::string, std::any> payload){
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.runs);
    }, false);
}

UpdateFuture DTLayer::onSAttKey(std::map<std::string, std::any> payload){
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.deaths) + deathsFunc(data.runs);
    }, true);
}

UpdateFuture DTLayer::onSF0AttKey(std::map<std::string, std::any> payload){
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.deaths);
    }, true);
}
UpdateFuture DTLayer::onSRunsAttKey(std::map<std::string, std::any> payload){
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.runs);
    }, true);
}

long long DTLayer::calcPlaytime(const Deaths& deaths){
    long double playtime = 0;

    // auto __calc_start_ms = timeInMs();

    float wt;
    if (!cachedLevelLength.has_value()){
        wt = m_Level->m_timestamp
            ? m_Level->m_timestamp / 240
            : m_Level->isPlatformer() ? 0 : std::ceil(timeForLevelString(m_Level->m_levelString));

        cachedLevelLength = wt;
    }
    else{
        wt = cachedLevelLength.value();
    }

    for (const auto& death : deaths)
    {
        auto runSplitRes = StatsManager::splitRunKey(death.first);
        if (runSplitRes.isErr()) continue;

        float runLength;
        if (runSplitRes.unwrap().start == std::nullopt)
            runLength = runSplitRes.unwrap().end;
        else
            runLength = (runSplitRes.unwrap().end - runSplitRes.unwrap().start.value());
        // log::info("{} | {} | {}", runLength, runSplitRes.unwrap().end, runSplitRes.unwrap().start);
        // log::info("{}", death.second);

        long double runOverallPlaytime = 0;

        if (runLength != 100)
            runLength += 0.5f;

        runOverallPlaytime = wt * (runLength / 100.0f) * death.second;

        playtime += runOverallPlaytime;
    }

    // auto __calc_elapsed_ms = timeInMs() - __calc_start_ms;
    // log::info("calcPlaytime took {} ms", __calc_elapsed_ms);

    return playtime;
}

UpdateFuture DTLayer::onAPTALLSKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    auto pt = StatsManager::workingTime(calcPlaytime(deaths));
    co_return Ok(pt);
}

UpdateFuture DTLayer::onAPTF0SKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        return data.deaths;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTRUNSKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        return data.runs;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}

UpdateFuture DTLayer::onAPTSALLSKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    NULL, true));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTSF0Key(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        return data.deaths;
    },
    NULL, true));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTSRUNSKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto runs, co_await getTFor<Deaths>([](GeneralData const& data){
        return data.runs;
    },
    NULL, true));

    co_return Ok(StatsManager::workingTime(calcPlaytime(runs)));
}

Result<Session, UpdateFutureError> DTLayer::loadSessionFromSave(std::optional<int> sessionIndex){
    if (!getCurrentGrouping().grouping.size()) return Err(UpdateFutureError("No sessions saved!", false));
    int i = sessionIndex.has_value() ? sessionIndex.value() : sessionSelector->getCurrentCount();

    if (i == 0 || i > getCurrentGrouping().grouping.size())
        return Err("Failed to get session, not in range");

    auto it = getCurrentGrouping().grouping.begin();
    std::advance(it, i - 1);

    Result<Session, UpdateFutureError> sess = Err("");

    for (const auto& [SDate, lvlKeys] : it->second.group)
    {
        for (const auto& lvlKey : lvlKeys)
        {
            auto sessionRes = StatsManager::getSession(lvlKey, SDate);
            if (sessionRes.isErr()){
                sess = Err(sessionRes.unwrapErr().error);
                break;
            }

            if (sess.isErr()){
                sess = Ok(sessionRes.unwrap());
            }
            else{
                auto& overallRef = sess.unwrap();

                overallRef.data += sessionRes.unwrap().data;
            }
        }
    }

    if (sess.isOk()){
        auto& sessRef = sess.unwrap();
        sessRef.groupID = it->first;
    }
    
    return sess;
}

UpdateFuture DTLayer::onRunsTo100Key(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    if (m_MyLevelStats.isErr()) co_return Err("Failed getting runs to 100");

    Deaths to100Deaths{};

    for (const auto& death : deaths)
    {
        co_await arc::yield();
        auto splitRunRes = StatsManager::splitRunKey(death.first);
        if (splitRunRes.isErr()) continue;
        auto splitRun = splitRunRes.unwrap();

        if (splitRun.end != 100) continue;

        to100Deaths.insert(death);
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(to100Deaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings))
        co_return Err("Failed to create runs to 100 string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onBestRunsKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    std::map<int, int> bestRuns{};

    for (const auto& death : deaths)
    {
        co_await arc::yield();
        auto splitRunRes = StatsManager::splitRunKey(death.first);
        if (splitRunRes.isErr()) continue;
        auto splitRun = splitRunRes.unwrap();

        auto realStart = splitRun.start.value_or(-1);

        if (!bestRuns.contains(realStart))
            bestRuns.insert({realStart, splitRun.end});
        else if (bestRuns[realStart] < splitRun.end){
            bestRuns[realStart] = splitRun.end;
        }
    }

    Deaths bestRunDeaths{};

    for (const auto& [bestRunStart, bestRunEnd] : bestRuns)
    {
        co_await arc::yield();
        auto runStringRes = StatsManager::createRunKey(Run{bestRunStart == -1 ? std::nullopt : std::make_optional(bestRunStart), bestRunEnd});
        if (runStringRes.isErr()) continue;
        auto runString = runStringRes.unwrap();
        if (!deaths.contains(runString)) continue;

        bestRunDeaths.insert({runString, deaths[runString]});
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(bestRunDeaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings))
        co_return Err("Failed best runs string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onSessionRunsTo100Key(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    NULL, true));

    if (m_MyLevelStats.isErr()) co_return Err("Failed getting session runs to 100");

    Deaths to100Deaths{};

    for (const auto& death : deaths)
    {
        co_await arc::yield();
        auto splitRunRes = StatsManager::splitRunKey(death.first);
        if (splitRunRes.isErr()) continue;
        auto splitRun = splitRunRes.unwrap();

        if (splitRun.end != 100) continue;

        to100Deaths.insert(death);
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(to100Deaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings))
        co_return Err("Failed to create session runs to 100 string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onSessionBestRunsKey(std::map<std::string, std::any> payload){
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    NULL, true));

    std::map<int, int> bestRuns{};

    for (const auto& death : deaths)
    {
        co_await arc::yield();
        auto splitRunRes = StatsManager::splitRunKey(death.first);
        if (splitRunRes.isErr()) continue;
        auto splitRun = splitRunRes.unwrap();

        auto realStart = splitRun.start.value_or(-1);

        if (!bestRuns.contains(realStart))
            bestRuns.insert({realStart, splitRun.end});
        else if (bestRuns[realStart] < splitRun.end){
            bestRuns[realStart] = splitRun.end;
        }
    }

    Deaths bestRunDeaths{};

    for (const auto& [bestRunStart, bestRunEnd] : bestRuns)
    {
        co_await arc::yield();
        auto runStringRes = StatsManager::createRunKey(Run{bestRunStart, bestRunEnd});
        if (runStringRes.isErr()) continue;
        auto runString = runStringRes.unwrap();
        if (!deaths.contains(runString)) continue;

        bestRunDeaths.insert({runString, deaths[runString]});
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")){
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(bestRunDeaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings))
        co_return Err("Failed best session runs string");

    co_return Ok(out);
}

void DTLayer::foreachLinkedLevel(geode::Function<void(LevelData&)> onLevelVisit){
    if (m_MyLevelStats.isErr()) return;
    auto& myStats = m_MyLevelStats.unwrap();

    for (auto& levelData : linkedLevelsData)
    {
        if (levelData.levelKey == myStats.levelKey) continue;
        onLevelVisit(levelData);
    }
}

UpdateFuture DTLayer::onPTALLSKey(std::map<std::string, std::any> payload){
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0 + data.playtimeGeneral.playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPTF0SKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0;
    }, false);
}

// same thing here, no distinction between playtime from 0 and playtime from runs
UpdateFuture DTLayer::onPTRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPTSALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0 + data.playtimeGeneral.playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onPTSF0Key(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeF0;
    }, true);
}

UpdateFuture DTLayer::onPTSRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeGeneral.playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onDeadPTALLSKey(std::map<std::string, std::any> payload){
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeDead.playtimeF0 + data.playtimeDead.playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onDeadPTF0SKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeDead.playtimeF0;
    }, false);
}

// same thing here, no distinction between playtime from 0 and playtime from runs
UpdateFuture DTLayer::onDeadPTRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeDead.playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onDeadPTSALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeDead.playtimeF0 + data.playtimeDead.playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onDeadPTSF0Key(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeDead.playtimeF0;
    }, true);
}

UpdateFuture DTLayer::onDeadPTSRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimeDead.playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onPausedPTALLSKey(std::map<std::string, std::any> payload){
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimePaused.playtimeF0 + data.playtimePaused.playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPausedPTF0SKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimePaused.playtimeF0;
    }, false);
}

// same thing here, no distinction between playtime from 0 and playtime from runs
UpdateFuture DTLayer::onPausedPTRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimePaused.playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPausedPTSALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimePaused.playtimeF0 + data.playtimePaused.playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onPausedPTSF0Key(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimePaused.playtimeF0;
    }, true);
}

UpdateFuture DTLayer::onPausedPTSRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data){
        return data.playtimePaused.playtimeRuns;
    }, true);
}


UpdateFuture DTLayer::onSectionKey(std::map<std::string, std::any> payload){
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();

    std::vector<Section> validSections{};
    for (const auto& section : myStats.metadata.sections)
    {
        if (!section.isValid()) continue;

        validSections.push_back(section);
    }
    
    if (validSections.size() <= 1) co_return Err(UpdateFutureError("No sections specified!", false));

    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    std::unordered_map<std::string, int> deathsPerSection{};

    auto CreateSectioIDForSectionPair = [&](const Run& splitDeath, const std::optional<Section>& startingSection, int deaths){
        Section endingSection;
        for (const auto& section : validSections)
        {
            if (!section.isPercentInSection(splitDeath.end)) continue;

            endingSection = section;
            break;
        }

        std::string sectionID = "";
        if (!startingSection.has_value()){
            sectionID = fmt::format("{}", endingSection.name);
        }
        else
            sectionID = fmt::format("{}-{}", startingSection.value().name, endingSection.name);
        if (!deathsPerSection.contains(sectionID))
            deathsPerSection.insert({sectionID, deaths});
        else
            deathsPerSection[sectionID] += deaths;
    };

    for (const auto& death : deaths){
        auto splitDeathRes = StatsManager::splitRunKey(death.first);
        if (splitDeathRes.isErr()) continue;
        auto splitDeath = splitDeathRes.unwrap();

        if (splitDeath.start == std::nullopt){
            CreateSectioIDForSectionPair(splitDeath, std::nullopt, death.second);
            continue;
        }
        
        std::vector<Section> startingSectionsForDeath{};

        for (const auto& section : validSections)
        {
            if (!section.isPercentInSection(splitDeath.start.value_or(-1))) continue;

            startingSectionsForDeath.push_back(section);
        }

        for (const auto& startingSection : startingSectionsForDeath)
        {
            CreateSectioIDForSectionPair(splitDeath, startingSection, death.second);
        }
    }

    std::string out;

    std::vector<int> order;
    order.reserve(validSections.size());
    for (size_t i = 0; i < validSections.size(); ++i) order.push_back(i);
    std::sort(order.begin(), order.end(), [&](int a, int b){
        return validSections[a].startPercent < validSections[b].startPercent;
    });

    auto custom = Save::getFrom0Customazations();

    int indexofyeah = 0;

    for (const auto& startIdx : order)
    {
        for (const auto& endIdx : order)
        {
            auto sectionID = (startIdx == endIdx) ? validSections[startIdx].name : fmt::format("{}-{}", validSections[startIdx].name, validSections[endIdx].name);
            auto it = deathsPerSection.find(sectionID);
            if (it == deathsPerSection.end()) continue;

            auto format = custom.format;
            format = std::regex_replace(format, std::regex("\\{per\\}"), sectionID);
            format = std::regex_replace(format, std::regex("\\{d\\}"), std::to_string(it->second));

            std::string keysStart = "";
            std::string keysEnd = "";

            if (indexofyeah % 2 == 0){
                keysEnd = "</cplus>";
                keysStart = fmt::format("<cplus={}>", custom.alternateStrength);
            }

            out += fmt::format("{}{}{}{}", keysStart, format, custom.seperator, keysEnd);

            indexofyeah++;
        }

        if (startIdx != order.back())
            out += "----------{nl}";
    }

    if (!out.empty())
        out.erase(out.length() - custom.seperator.length());

    co_return Ok(out);
}

void DTLayer::onCalculator(CCObject*){
    CalculatorPopup::create()->show();
}

UpdateFuture DTLayer::onLevelRunsKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data){
        auto runs = data.runs;
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    [](auto const& a, auto const& b){
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    const int max = 4;
    std::vector<Run> validRuns;

    for (auto const& [key, count] : deaths) {
        auto res = StatsManager::splitRunKey(key);
        if (res.isOk()) {
            Run run = res.unwrap();
            auto runStartReal = run.start.value_or(-1);
            if (runStartReal <= -1) runStartReal = 0;
            if (count > 0 && run.end > runStartReal) {
                validRuns.push_back(run);
            }
        }
        co_await arc::yield();
    }

    std::sort(validRuns.begin(), validRuns.end(), [](const Run& a, const Run& b) {
        return std::tie(a.start, a.end) < std::tie(b.start, b.end);
    });
    validRuns.erase(std::unique(validRuns.begin(), validRuns.end(), [](const Run& a, const Run& b) {
        return a.start == b.start && a.end == b.end;
    }), validRuns.end());

    std::map<int, std::array<uint64_t, max + 1>> dp;
    dp[0][0] = 1;

    for (int currentPercent = 0; currentPercent < 100; ++currentPercent) {
        if (dp.find(currentPercent) == dp.end()) continue;

        for (const auto& run : validRuns) {
            if (run.start <= currentPercent && run.end > currentPercent) {
                int target = std::min(run.end, 100);
                for (int i = 0; i < max; ++i) {
                    if (dp[currentPercent][i] > 0) {
                        dp[target][i + 1] += dp[currentPercent][i];
                    }
                }
            }
        }
    }

    auto& finalCounts = dp[100];
    std::string toReturn = "";
    bool foundAny = false;

    for (int i = 1; i <= max; ++i) {
        if (finalCounts[i] > 0) {
            if (foundAny) toReturn += "{nl}";
            
            toReturn += fmt::format("{} runs X{}", i, static_cast<uint64_t>(finalCounts[i]));
            foundAny = true;
        }
    }

    if (!foundAny) {
        co_return Ok(fmt::format("Level not done in {} runs or less.", max));
    }

    co_return Ok(toReturn);
}

UpdateFuture DTLayer::getPlaytimeFor(geode::Function<uint64_t(GeneralData const&)>&& dataGetter, bool session){
    auto gotten = co_await getTFor<uint64_t>(std::move(dataGetter), [](uint64_t const& a, uint64_t const& b){
        return a + b;
    }, session);

    if (gotten.isErr()) co_return Err(gotten.unwrapErr());
    co_return Ok(StatsManager::workingTime(gotten.unwrap()));
}

UpdateFuture DTLayer::getAttemptsFor(geode::Function<unsigned long long(GeneralData const&, geode::FunctionRef<unsigned long long(const Deaths&)> const&)>&& dataGetter, bool session){
    auto deaths = [](const Deaths& d) -> unsigned long long {
        unsigned long long attempts = 0;

        for (const auto& [_, count] : d){
            attempts += count;
        }

        return attempts;
    };

    GEODE_CO_UNWRAP_INTO(auto attempts, co_await getTFor<unsigned long long>([&deaths, &dataGetter](GeneralData const& data){
        return dataGetter(data, deaths);
    },
    [](auto a, auto b){
        return a + b;
    }, session));

    co_return Ok(std::to_string(attempts));
}

SessionCategory& DTLayer::getCurrentGrouping(){
    if (m_MyLevelStats.isErr()) return sessionsOrder;
    auto& myStats = m_MyLevelStats.unwrap();

    if (currentGrouping == -3) return daySGroup;
    if (currentGrouping == -2) return weekSGroup;
    if (currentGrouping == -1) return monthSGroup;

    if (!myStats.metadata.sessionGroups.size() || currentGrouping < 0 || currentGrouping >= myStats.metadata.sessionGroups.size()) return sessionsOrder;

    return myStats.metadata.sessionGroups[currentGrouping];
}

UpdateFuture DTLayer::onSessionDateKey(std::map<std::string, std::any> payload){
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err(sessionRes.unwrapErr());

    auto session = sessionRes.unwrap();

    co_await arc::yield();

    time_t time = static_cast<time_t>(session.groupID);

    std::tm tp{};
    #if defined(_WIN32)
        localtime_s(&tp, &time);
    #else
        localtime_r(&time, &tp);
    #endif

    auto dateStr = DateFormatter::format(tp);

    co_return Ok(dateStr);
}

void DTLayer::onGroups(CCObject*){
    if (groupsList->isOpened())
        groupsList->close();
    else
        groupsList->open();
}

void DTLayer::onGroupSelected(int const& id){
    groupsList->close();

    currentGrouping = id;
    specialStrings["s0"]->updateContent();
    specialStrings["sruns"]->updateContent();

    auto opt = groupsList->getItemForID(id);

    auto newSpr = ButtonSprite::create(
        opt.value().text.c_str(),
        100,
        100,
        1,
        false,
        opt.value().font.c_str(), 
        opt.value().BGTexture.c_str()
    );
    newSpr->setCascadeOpacityEnabled(true);
    newSpr->setScale(.45f);

    groupsBtn->setSprite(newSpr);

    bottomLeftMenu->updateLayout();
    bottomRightMenu->updateLayout();

    sessionSelector->setMaximumCount(getCurrentGrouping().grouping.size(), true);
}

void DTLayer::CleanGetStats(){
    m_MyLevelStats = StatsManager::getLevelData(m_Level);
    if (m_MyLevelStats.isErr() && m_MyLevelStats.unwrapErr().code == 1){
        LevelData newData;
        newData.levelKey = StatsManager::getLevelKey(m_Level).unwrap();
        m_MyLevelStats = Ok(newData);
    }
    else if (m_MyLevelStats.isErr()){
        auto notif = geode::Notification::create(fmt::format("Failed to load DT level data! {}", m_MyLevelStats.unwrapErr().error), NotificationIcon::Error, 3);
        notif->show();
        notif->setZOrder(101);
    }

    StatsManager::transferPlaytimeFromPT(m_MyLevelStats, m_Level);

    if (m_MyLevelStats.isOk()){
        auto stats = m_MyLevelStats.unwrap();
        stats.metadata.levelName = m_Level->m_levelName;
        stats.metadata.attempts = m_Level->m_attempts;
        stats.metadata.difficulty = StatsManager::getDifficulty(m_Level);
        (void)StatsManager::setMetadata(stats.metadata, stats.levelKey);
        m_MyLevelStats = Ok(stats);
    }

    StatsManager::setCurrentLevel(m_Level);

    DTLayer::UpdateSharedStats();
}

void DTLayer::onNewBestColor(CCObject* sender){
    auto popup = ColorPickPopup::create(Save::getNewBestColor());
    popup->setCallback([&](auto color){
        Save::setNewBestColor({color.r, color.g, color.b});

        specialStrings["general"]->updateContent();
    });
    popup->setColorTarget(newBestColorBtnSpr);
    popup->show();
}
void DTLayer::onSessionBestColor(CCObject* sender){
    auto popup = ColorPickPopup::create(Save::getSessionBestColor());
    popup->setCallback([&](auto color){
        Save::setSessionBestColor({color.r, color.g, color.b});

        specialStrings["s0"]->updateContent();
    });
    popup->setColorTarget(sessionBestColorBtnSpr);
    popup->show();
}

void DTLayer::onResetLayout(CCObject*){
    FLAlertLayer::create(this, "Warning", "This will reset the layout back to the default layout!\nare you sure you want to do this?", "CANCEL", "OK")->show();
}

void DTLayer::FLAlert_Clicked(FLAlertLayer* layer, bool btn2){
    if (!btn2) return;

    resetLayoutTo(DTLayoutPreset{
        .name = "Default",
        .layout = Save::getDefaultLayout(),
        .newBestColor = {255, 255, 0},
        .sessionBestColor = { 255, 136, 0 }
    });
}


void DTLayer::onLayoutPresets(CCObject*){
    if (presetList->isOpened())
        presetList->close();
    else{
        presetList->clearAllItems();

        std::vector<FloatingListItem> items{};

        presets.clear();

        auto savedPresets = Save::getLayoutPresets();
        int index = 0;
        for (const auto& preset : savedPresets)
        {
            FloatingListItem newItem{};

            newItem.id = index;
            newItem.text = preset.name;
            newItem.sideButtonSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
            newItem.sideButtonSprite->setScale(.35f);
            newItem.sideButtonCallback = [&](auto _){DTLayer::deletePreset(_);};

            index++;

            items.push_back(newItem);
            presets.insert({index, preset});
        }

        presetList->addItems(items);
        presetList->open();
    }
}
void DTLayer::onAddPreset(CCObject*){
    TextInputPopup::create(
        "Save Layout",
        "Layout Name",
        "OK",
        "",
        [&](const auto& str){
            savePreset(DTLayoutPreset{
                .name = str,
                .layout = currentLayout(),
                .newBestColor = newBestColorBtnSpr->getColor(),
                .sessionBestColor = sessionBestColorBtnSpr->getColor()
            });

            presetList->close();
        }
    )->show();
}
void DTLayer::onImportPreset(CCObject*){
    auto importPresetHandle = async::spawn(
        file::pick(
            file::PickMode::OpenFile,
            file::FilePickOptions{
                .defaultPath = ".dtl",
                .filters = {
                    file::FilePickOptions::Filter{
                        .description = "Death Tracker Layout file",
                        .files = {
                            "*.dtl"
                        }
                    }
                }
            }
        ),
        [&](file::PickResult result){
            if (result.isErr()){
                log::error("{}", result.unwrapErr());
                return;
            }

            auto pickOpt = result.unwrap();
            if (!pickOpt.has_value()) return;
            auto pick = pickOpt.value();

            if (pick.extension() != ".dtl" || !std::filesystem::exists(pick)) return;

            auto preset = file::readFromJson<DTLayoutPreset>(pick);
            if (preset.isErr()){
                log::error("{}", preset.unwrapErr());
                return;
            }

            savePreset(preset.unwrap());

            presetList->close();
        }
    );
    importPresetHandle.setName("DT-import-preset-task");
}
void DTLayer::onExportPreset(CCObject*){
    TextInputPopup::create(
        "Export Layout",
        "Layout Name",
        "EXPORT",
        "",
        [&](const auto& str){
            auto layout = currentLayout();
            
            auto preset = DTLayoutPreset{
                .name = str,
                .layout = layout,
                .newBestColor = newBestColorBtnSpr->getColor(),
                .sessionBestColor = sessionBestColorBtnSpr->getColor()
            };

            auto exportHandle = async::spawn(
                file::pick(
                    file::PickMode::SaveFile,
                    file::FilePickOptions{
                        .defaultPath = str + ".dtl",
                        .filters = {
                            file::FilePickOptions::Filter{
                                .description = "Death Tracker Layout file",
                                .files = {
                                    "*.dtl"
                                }
                            }
                        }
                    }
                ),
                [preset](file::PickResult result){
                    if (result.isErr()){
                        log::error("{}", result.unwrapErr());
                        return;
                    }

                    auto pickOpt = result.unwrap();
                    if (!pickOpt.has_value()) return;
                    auto pick = pickOpt.value();

                    pick = pick.replace_extension(".dtl");

                    StatsManager::createFile(pick, std::nullopt);

                    if (file::writeToJson(pick, preset).isErr()){
                        Notification::create("Failed to export layout!", NotificationIcon::Error)->show();
                    }
                }
            );
            exportHandle.setName("DT-export-preset-task");
        }
    )->show();
}

void DTLayer::savePreset(DTLayoutPreset mypreset){
    auto presets = Save::getLayoutPresets();
    for (const auto& preset : presets)
    {
        if (mypreset.name == preset.name){
            Notification::create("A preset with that name already exists!", NotificationIcon::Error)->show();
            return;
        }
    }
    
    presets.push_back(mypreset);
    Save::setLayoutPresets(presets);
}

void DTLayer::deletePreset(int id){
    geode::createQuickPopup("Warning!", "Are you sure you want to delete this preset?", "NO", "YES", [&](auto _, bool b2){
        if (!b2) return;

        auto optItem = presetList->getItemForID(id);
        if (!optItem.has_value()) return;

        auto presets = Save::getLayoutPresets();
        int index = 0;
        for (const auto& preset : presets)
        {
            if (optItem.value().text == preset.name){
                presets.erase(std::next(presets.begin(), index));
                break;
            }

            index++;
        }

        Save::setLayoutPresets(presets);
        presetList->close();
    });
}

void DTLayer::resetLayoutTo(DTLayoutPreset const& preset){
    newBestColorBtnSpr->setColor(preset.newBestColor);
    Save::setNewBestColor(newBestColorBtnSpr->getColor());
    sessionBestColorBtnSpr->setColor(preset.sessionBestColor);
    Save::setSessionBestColor(sessionBestColorBtnSpr->getColor());

    setLayoutBy(preset.layout);
    organizeLayout();

    specialStrings["general"]->updateContent();
    specialStrings["s0"]->updateContent();
}