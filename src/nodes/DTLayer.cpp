#include <nodes/DTLayer.hpp>

#include <nodes/graphs/DTGraphLayer.hpp>
#include <nodes/options/DTLevelSpecificSettingsLayer.hpp>
#include <nodes/calculator/CalculatorPopup.hpp>

#include <nodes/overlayLabels/LabelEditLayer.hpp>

#include <Geode/ui/GeodeUI.hpp>
#include <regex>
#include <actions/CCResizeWidthTo.hpp>
#include <actions/CCParticleFadeTo.hpp>

#include <arc/task/Yield.hpp>
#include <arc/time/Sleep.hpp>

#include <utils/DateFormatter.hpp>
#include <utils/Settings.hpp>
#include <nodes/general/popup/ChangelogPopup.hpp>
#include <nodes/general/popup/TextInputPopup.hpp>

#include <managers/AuthManager.hpp>

#include <nodes/layout/layoutEditing/DTColorPickerPopup.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>

float DTLayer::transitionTime = .35f;
geode::Label* DTLayer::verText = nullptr;
CCParticleSystemQuad* DTLayer::verStarsParticle = nullptr;
CCParticleSystemQuad* DTLayer::verSmokeParticle = nullptr;

bool ColumnComperator::operator()(LayoutColumn* a, LayoutColumn* b) const {
    if (!a || !b) {
        return a < b;
    }
    return a->m_info.m_orderPos < b->m_info.m_orderPos;
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
    if (!Popup::init(winSize.width - 150, winSize.height - 30, "geode.loader/GE_square01.png"))
        return false;

    m_Level = level;

    // ================================== //
    // loading data

    CleanGetStats();

    if (!StatsManager::getSavesFolderPath().exists()) {
        Notification::create("Error! Invalid save directory!", NotificationIcon::Error)->show();
    }

    // ================================== //

    /*
     * main page
    */

    instance = this;

    this->setID("dt-layer");

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString()) {
        bool isNewPlayer = Save::getLastOpenedVersion() == "";
        Save::setLastOpenedVersion(Mod::get()->getVersion().toNonVString());
        Mod::get()->getVersion();
        if (!isNewPlayer) {
            auto versionHandle = async::spawn(
                []() -> arc::Future<> {
                    co_await arc::yield();
                },
                []() {
                    auto changelog = ChangelogPopup::create();
                    if (changelog != nullptr)
                        changelog->show();
                }
            );
            versionHandle.setName("DT-version-popup-delay-task");
        }
    }
    
    if (verText == nullptr) {
        auto verContainer = CCNode::create();
        verContainer->setPosition({5, 5});
        verContainer->setScale(.5f);

        verText = geode::Label::create(Mod::get()->getVersion().toVString(true), "gjFont17.fnt");
        verText->setAnchorPoint({0, 0});
        verText->setOpacity(0);
        verContainer->addChild(verText);

        verSmokeParticle = GameToolbox::particleFromString("8a-1a2a0a4a0a0a0a20a40a15a0a0a0a0a0a0a50a12a90a0a0.603922a0a0.560784a0a0.0627451a0a0.11a0a50a7a90a0a0.34902a0a0.309804a0a0.027451a0a0.06a0a0.15a0a0.15a0a0a0a0a0a0a0a0a2a1a0a0a0a46a0a0.63a0a0a0a0a0a0a0a0a0a0a0a0", NULL, false);
        verSmokeParticle->setOpacity(0);
        verSmokeParticle->setPosition(verText->getContentSize() / 2);
        verContainer->addChild(verSmokeParticle);

        verStarsParticle = GameToolbox::particleFromString("7a-1a2a0a3a0a0a3a18a27a20a0a0a0a0a0a0a10a12a90a80a1a0a0.92549a0a0a0a0.6a0a3a7a90a20a0.847059a0a0.74902a0a0.0509804a0a0.7a0a0.15a0a0.15a0a0a0a0a0a0a0a0a2a1a0a0a0a83a0a1.93a0a0a0a0a0a0a0a0a0a0a0a0", NULL, false);
        verStarsParticle->setOpacity(0);
        verStarsParticle->setPosition(verText->getContentSize() / 2);
        verContainer->addChild(verStarsParticle);
        
        OverlayManager::get()->addChild(verContainer);
    }

    verText->stopAllActions();
    verText->runAction(CCFadeTo::create(.5f, 120));

    verSmokeParticle->stopAllActions();
    verSmokeParticle->runAction(CCParticleFadeTo::create(.5f, 255));

    verStarsParticle->stopAllActions();
    verStarsParticle->runAction(CCParticleFadeTo::create(.5f, 255));

    float height = 60;
    m_ogLimits = CCSize{m_size.width - 30 + 1, m_size.height - height  + 1};
    m_scrollLayer = AdvancedScrollLayer::create({m_size.width - 30, m_size.height - height}, m_ogLimits);
    //m_scrollLayer->drawGrid(50, .5f, ccColor4B{ 143, 143, 143, 255 });
    m_scrollLayer->setPosition(m_size / 2 - m_scrollLayer->getContentSize() / 2 + ccp(0, height / 4));
    m_scrollLayer->setZOrder(2);
    m_scrollLayer->m_maxZoom = 0.05f;
    m_scrollLayer->m_minZoom = 10;
    m_mainLayer->addChild(m_scrollLayer);

    std::vector<CCPoint> points{
        m_scrollLayer->getPosition(),
        m_scrollLayer->getPosition() + ccp(m_scrollLayer->getContentSize().width, 0),

        m_scrollLayer->getPosition() + ccp(m_scrollLayer->getContentSize().width, 0),
        m_scrollLayer->getPosition() + m_scrollLayer->getContentSize(),

        m_scrollLayer->getPosition() + m_scrollLayer->getContentSize(),
        m_scrollLayer->getPosition() + ccp(0, m_scrollLayer->getContentSize().height),

        m_scrollLayer->getPosition() + ccp(0, m_scrollLayer->getContentSize().height),
        m_scrollLayer->getPosition(),
    };

    m_decimalPopup = DecimalPopup::create({200, m_scrollLayer->m_content->getParent()->getContentHeight() / 2.f});
    m_decimalPopup->setZOrder(3);
    m_scrollLayer->m_content->addChild(m_decimalPopup);

    auto outline = CCDrawNode::create();
    outline->m_bUseArea = false;
    outline->drawLines(&points[0], points.size(), .5f, ccc4FFromccc4B({ 143, 143, 143, 255 }));
    outline->setZOrder(2);
    m_mainLayer->addChild(outline);

    auto shadow = CCScale9Sprite::create("square.png");
    shadow->setContentSize(m_scrollLayer->getContentSize());
    shadow->setAnchorPoint({0, 0});
    shadow->setPosition(m_scrollLayer->getPosition());
    shadow->setColor({0, 0, 0});
    shadow->setOpacity(100);
    m_mainLayer->addChild(shadow);

    m_sessionSelector = SessionSelector::create(getCurrentGrouping().m_grouping.size());
    m_sessionSelector->setCallback([&](int newSession) { onSessionSelected(newSession, true); });
    m_sessionSelector->setScale(.75f);
    m_sessionSelector->setPosition({m_size.width / 2, 20});
    m_mainLayer->addChild(m_sessionSelector);

    m_bottomLeftMenu = CCMenu::create();
    m_bottomLeftMenu->setContentSize({m_size.width / 2 - m_sessionSelector->getScaledContentWidth() / 2 - 10, height / 2.4f});
    m_bottomLeftMenu->setAnchorPoint({0, .5f});
    m_bottomLeftMenu->setPosition(ccp(10, height / 2.5f  / 2) + ccp(0, 7.5f));
    m_bottomLeftMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(10)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Even)
        ->setMinRelativeScale(std::nullopt)
        ->setMaxRelativeScale(std::nullopt)
    );
    m_bottomLeftMenu->setID("bottom-left-menu");
    m_mainLayer->addChild(m_bottomLeftMenu);

    m_bottomRightMenu = CCMenu::create();
    m_bottomRightMenu->setContentSize({m_size.width / 2 - m_sessionSelector->getScaledContentWidth() / 2 - 10, height / 2.4f});
    m_bottomRightMenu->setAnchorPoint({1, .5f});
    m_bottomRightMenu->setPosition(ccp(m_size.width - 10, height / 2.5f  / 2) + ccp(0, 7.5f));
    m_bottomRightMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(10)
        ->setMainAxisScaling(AxisScaling::ScaleDown)
        ->setCrossAxisScaling(AxisScaling::ScaleDown)
        ->setMainAxisAlignment(MainAxisAlignment::Even)
        ->setMinRelativeScale(std::nullopt)
        ->setMaxRelativeScale(std::nullopt)
    );
    m_bottomRightMenu->setID("bottom-right-menu");
    m_mainLayer->addChild(m_bottomRightMenu);

    auto levelSpecificOptionsSpr = CCSprite::createWithSpriteFrameName("DTLevelOptionsBtn.png"_spr);
    auto levelSpecificOptionsBtn = CCMenuItemSpriteExtra::create(
        levelSpecificOptionsSpr,
        this,
        menu_selector(DTLayer::onLSOClicked)
    );
    m_bottomLeftMenu->addChild(levelSpecificOptionsBtn);

    auto graphBtnSpr = CCSprite::createWithSpriteFrameName("graph_button.png"_spr);
    auto graphBtn = CCMenuItemSpriteExtra::create(
        graphBtnSpr,
        this,
        menu_selector(DTLayer::graphBtnClicked)
    );
    m_bottomLeftMenu->addChild(graphBtn);

    auto calculatorSpr = CCSprite::createWithSpriteFrameName("caluclator.png"_spr);
    auto calculatorBtnSpr = CCSprite::create("GJ_button_01.png");
    calculatorBtnSpr->setCascadeOpacityEnabled(true);
    calculatorSpr->setPosition(calculatorBtnSpr->getContentSize() / 2);
    calculatorSpr->setScale(.9f);
    calculatorBtnSpr->addChild(calculatorSpr);
    auto calculatorBtn = CCMenuItemSpriteExtra::create(
        calculatorBtnSpr,
        this,
        menu_selector(DTLayer::onCalculator)
    );
    m_bottomLeftMenu->addChild(calculatorBtn);

    onSessionSelected(1, false);

    auto groupsBtnSpr = ButtonSprite::create(
        m_sessionsOrder.m_groupName.c_str(),
        100,
        100,
        1,
        false,
        "bigFont.fnt", 
        "GJ_button_04.png"
    );
    groupsBtnSpr->setCascadeOpacityEnabled(true);
    groupsBtnSpr->setScale(.45f);
    m_groupsBtn = CCMenuItemSpriteExtra::create(
        groupsBtnSpr,
        this,
        menu_selector(DTLayer::onGroups)
    );

    m_groupsHolder = CCMenu::create();
    m_groupsHolder->setContentSize(ccp(45, 0));
    m_groupsBtn->setPosition(m_groupsHolder->getContentSize() / 2);
    m_sessionSelector->addChild(m_groupsHolder);
    m_groupsHolder->setPosition({m_sessionSelector->getContentWidth() / 2, -4});
    m_groupsHolder->setScale(.85f);
    m_groupsHolder->ignoreAnchorPointForPosition(false);
    m_groupsHolder->addChild(m_groupsBtn);

    auto labelsBtnSprBG = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    m_labelsBtnSpr = CCSprite::createWithSpriteFrameName("labelsBtn.png"_spr);
    m_labelsBtnSpr->setPosition(labelsBtnSprBG->getContentSize() / 2);
    labelsBtnSprBG->addChild(m_labelsBtnSpr);
    labelsBtnSprBG->setScale(.75f);
    auto labelsBtn = CCMenuItemSpriteExtra::create(
        labelsBtnSprBG,
        this,
        menu_selector(DTLayer::onLabels)
    );
    labelsBtn->setPosition(m_scrollLayer->getPosition() + m_scrollLayer->getContentSize());
    m_bottomRightMenu->addChild(labelsBtn);

    auto editLayoutBtnSprBG = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    m_editLayoutBtnSpr = CCSprite::createWithSpriteFrameName("layout_button.png"_spr);
    m_editLayoutBtnSpr->setPosition(editLayoutBtnSprBG->getContentSize() / 2);
    editLayoutBtnSprBG->addChild(m_editLayoutBtnSpr);
    editLayoutBtnSprBG->setScale(.75f);
    auto editLayoutBtn = CCMenuItemSpriteExtra::create(
        editLayoutBtnSprBG,
        this,
        menu_selector(DTLayer::onEditLayout)
    );
    editLayoutBtn->setPosition(m_scrollLayer->getPosition() + m_scrollLayer->getContentSize());
    m_bottomRightMenu->addChild(editLayoutBtn);

    auto settingsBtnSpr = CCSprite::createWithSpriteFrameName("DTSettingsBtn.png"_spr);
    settingsBtnSpr->setScale(.75f);
    auto settingsBtn = CCMenuItemSpriteExtra::create(
        settingsBtnSpr,
        this,
        menu_selector(DTLayer::onSettings)
    );
    settingsBtn->setPosition({m_size.width - 3.f, 3.f});
    m_bottomRightMenu->addChild(settingsBtn);

    m_columnHolder = CCMenu::create();
    m_columnHolder->setAnchorPoint({0, 1});
    m_columnHolder->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisScaling(AxisScaling::Fit)
        ->setCrossAxisScaling(AxisScaling::None)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
        ->setCrossAxisAlignment(CrossAxisAlignment::Start)
    );
    m_columnHolder->setPosition({0, m_scrollLayer->m_content->getContentHeight()});
    m_scrollLayer->m_content->addChild(m_columnHolder);

    m_labelsHolder = CCNode::create();
    m_labelsHolder->setPosition(m_columnHolder->getPosition());
    m_scrollLayer->m_content->addChild(m_labelsHolder);

    setLayoutBy(Save::getLayout());

    auto addColumnButtonSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    m_addColumnButton = CCMenuItemSpriteExtra::create(
        addColumnButtonSpr,
        this,
        menu_selector(DTLayer::addColumnBtnClicked)
    );
    m_addColumnButton->setZOrder(1000);
    m_columnHolder->addChild(m_addColumnButton);
    m_addColumnButton->setOpacity(0);
    m_columnHolder->setEnabled(false);

    m_lc = LoadingCircle::create();
    m_lc->setContentSize({0, 0});
    m_lc->m_sprite->setPosition(m_size / 2);
    m_mainLayer->addChild(m_lc);

    CCTouchDispatcher::get()->removeDelegate(m_scrollLayer);

    this->setKeypadEnabled(true);
    this->setTouchEnabled(true);

    populateSpecialStrings();

    this->organizeLayout();

    this->scheduleUpdate();

    m_bottomRightMenu->updateLayout();
    m_bottomLeftMenu->updateLayout();

    m_groupsList = FloatingList::create({
        m_groupsBtn->getScaledContentWidth(),
        120
    });
    m_groupsList->setZOrder(2);
    m_groupsList->addItems({
        FloatingListItem{
            .m_id = -4,
            .m_text = m_sessionsOrder.m_groupName,
            .m_BGTexture = "GJ_button_04.png"
        },
        FloatingListItem{
            .m_id = -3,
            .m_text = m_daySGroup.m_groupName,
            .m_BGTexture = "GJ_button_05.png"
        },
        FloatingListItem{
            .m_id = -2,
            .m_text = m_weekSGroup.m_groupName,
            .m_BGTexture = "GJ_button_05.png"
        },
        FloatingListItem{
            .m_id = -1,
            .m_text = m_monthSGroup.m_groupName,
            .m_BGTexture = "GJ_button_05.png"
        },
    });
    m_groupsList->setCallback([&](auto id) {
        this->onGroupSelected(id);
    });
    m_groupsList->setAnchorPoint({.5f, 0});
    m_mainLayer->addChild(m_groupsList);
    m_groupsList->setPosition(
        m_groupsList->getParent()->convertToNodeSpace(
            m_groupsBtn->convertToWorldSpace({m_groupsBtn->getContentWidth() / 2,m_groupsBtn->getContentHeight()})
        )
    );

    m_editLayoutMenu = CCMenu::create();
    m_editLayoutMenu->setEnabled(false);
    m_editLayoutMenu->setPosition({winSize.width / 2,160});
    m_editLayoutMenu->ignoreAnchorPointForPosition(false);
    m_editLayoutMenu->setAnchorPoint({0,0});
    m_mainLayer->addChild(m_editLayoutMenu);

    m_applyChangesButtonSpr = ButtonSprite::create("Apply Changes", "goldFont.fnt", "GJ_button_01.png");
    m_applyChangesButtonSpr->m_BGSprite->setOpacity(0);
    m_applyChangesButtonSpr->m_label->setOpacity(0);
    m_applyChangesButtonSpr->setScale(.5f);
    auto applyChangesButton = CCMenuItemSpriteExtra::create(
        m_applyChangesButtonSpr,
        this,
        menu_selector(DTLayer::onApplyLayoutChanges)
    );
    applyChangesButton->setPosition({41, -140});
    m_editLayoutMenu->addChild(applyChangesButton);

    m_discardChangesButtonSpr = ButtonSprite::create("Discard Changes", "goldFont.fnt", "GJ_button_06.png");
    m_discardChangesButtonSpr->m_BGSprite->setOpacity(0);
    m_discardChangesButtonSpr->m_label->setOpacity(0);
    m_discardChangesButtonSpr->setScale(.45f);
    auto discardChangesButton = CCMenuItemSpriteExtra::create(
        m_discardChangesButtonSpr,
        this,
        menu_selector(DTLayer::onDiscardLayoutChanges)
    );
    discardChangesButton->setPosition({-191, -140});
    m_editLayoutMenu->addChild(discardChangesButton);

    auto mainInfo = TutorialButton::create(
        1,
        "main-overall",
        [
            &,
            levelSpecificOptionsBtn,
            graphBtn,
            editLayoutBtn,
            settingsBtn,
            calculatorBtn,
            labelsBtn
        ](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("Welcome to the <cy>main death tracker page!</c>", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("This is the <cy>main view</c> where you can view <cg>all your data!</c>", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .75f)
            ->joinHighlight(m_scrollLayer)
            ->joinTextToHighlight("Main Scroll View", .5f, TutorialTextPlacement::TTTop)
#if !defined(GEODE_IS_MOBILE)
            ->appendDialogue("You can hold <cg>control</c> and <cp>scroll</c> to zoom in! and <cc>shift</c> and <cp>scroll</c> to move side to side.", TutorialCharacterFace::TCFNormalTilted)
#else
            ->appendDialogue("You can <cg>pinch</c> to zoom in! and <cc>swipe</c> to move side to side.", TutorialCharacterFace::TCFNormalTilted)
#endif
            ->joinHighlight(m_scrollLayer)

            ->appendDialogue("Of course you can also scroll normally :D", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter)

            ->appendDialogue("You also have many options <cy>at the bottom</c> here!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->joinHighlight(m_bottomRightMenu)
            ->joinHighlight(m_bottomLeftMenu)
            ->joinHighlight(m_sessionSelector)

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
            ->joinHighlight(m_sessionSelector)
            ->joinTextToHighlight("Session Selector", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("The <cy>higher</c> the number, the <cr>older</c> the session! so <co>session 1</c> is the most recent and <co>the last</c> is the oldest!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(m_sessionSelector)

            ->appendDialogue("There are the session groups! which allow you to group session by specific categories         ", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_groupsBtn)
            ->joinTextToHighlight("Session Groups", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("There are the <cf>overlay labels</c>, which allow you to dynamically display info while playing", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(labelsBtn)
            ->joinTextToHighlight("Overlay Labels", .3f, TutorialTextPlacement::TTTop)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)

            ->appendDialogue("You also have the option to <cy>edit how your data is layed out</c> using this button!", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(editLayoutBtn)
            ->joinTextToHighlight("Layout Editor", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("And lastly you have quick access to the mod settings for death tracker right here!", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(settingsBtn)
            ->joinTextToHighlight("Mod Options", .3f, TutorialTextPlacement::TTTop)

            ->appendDialogue("You can also <cy>double click</c> an of the <cf>labels top parts</c> to copy their contents", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .75f);
        std::set<DTLabel*> allLabels{};

        int index = 0;
        for (const auto& column : m_columns)
        {
            for (const auto& [_, label] : column->m_labels)
            {
                if (allLabels.contains(label)) continue;

                allLabels.insert(label);
                tutorialLayer->joinHighlight(label->m_labelTitleBG, .1f * index);
                index++;
            }
        }

            tutorialLayer->appendDialogue("Have fun playing around with the features!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPCenter);
    });
    mainInfo->setPosition(m_size);
    m_buttonMenu->addChild(mainInfo);

    m_resetLayoutMenu = CCMenu::create();
    m_resetLayoutMenu->ignoreAnchorPointForPosition(false);
    m_resetLayoutMenu->setPosition({0, 0});
    m_resetLayoutMenu->setContentSize({0, 0});
    m_resetLayoutMenu->setCascadeOpacityEnabled(true);
    m_resetLayoutMenu->setEnabled(false);
    m_resetLayoutMenu->setZOrder(3);
    m_mainLayer->addChild(m_resetLayoutMenu);
    
    auto resetLayoutBtnSpr = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
    resetLayoutBtnSpr->setScale(.5f);
    auto resetLayoutBtn = CCMenuItemSpriteExtra::create(
        resetLayoutBtnSpr,
        this,
        menu_selector(DTLayer::onResetLayout)
    );
    resetLayoutBtn->setPosition({0, 0});
    m_resetLayoutMenu->addChild(resetLayoutBtn);

    auto layoutPresetsBtnSpr = ButtonSprite::create("Presets", "bigFont.fnt", "GJ_button_05.png");
    layoutPresetsBtnSpr->setScale(.4f);
    layoutPresetsBtnSpr->setCascadeOpacityEnabled(true);
    auto layoutPresetsBtn = CCMenuItemSpriteExtra::create(
        layoutPresetsBtnSpr,
        this,
        menu_selector(DTLayer::onLayoutPresets)
    );
    layoutPresetsBtn->setPosition({m_size.width, 0});
    m_resetLayoutMenu->addChild(layoutPresetsBtn);

    m_presetList = FloatingList::create({
        layoutPresetsBtn->getContentWidth() + 10,
        90
    });
    m_presetList->setPosition(layoutPresetsBtn->getPosition() + ccp(0, layoutPresetsBtn->getContentHeight() / 2 + 2));
    m_presetList->setAnchorPoint({.5f, 0});
    m_presetList->setCallback([&](int id) {
        auto optItem = m_presetList->getItemForID(id);
        if (!optItem.has_value()) return;

        auto m_presets = Save::getLayoutPresets();
        int index = 0;
        for (const auto& preset : m_presets)
        {
            if (optItem.value().m_text == preset.m_name) {
                resetLayoutTo(preset);
                break;
            }

            index++;
        }
    });
    m_resetLayoutMenu->addChild(m_presetList);

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
    m_resetLayoutMenu->addChild(addPresetBtn);

    auto importBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
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
    m_resetLayoutMenu->addChild(importPresetBtn);

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
    m_resetLayoutMenu->addChild(exportPresetBtn);

    auto presetsTutorial = TutorialButton::create(.75f, "layout-m_presets", [
        &,
        exportPresetBtn,
        importPresetBtn,
        addPresetBtn,
        layoutPresetsBtn,
        discardChangesButton
    ](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("Here you are able to save <cy>m_presets</c> of your layout!", TutorialCharacterFace::TCFHappy)
            ->joinTransform(TutorialBoxPlacement::TBPRight, .75f)
            ->joinHighlight(exportPresetBtn)
            ->joinHighlight(importPresetBtn)
            ->joinHighlight(addPresetBtn)
            ->joinHighlight(layoutPresetsBtn)

            ->appendDialogue("You can click the <cy>m_presets</c> button to look at a <cf>list of all the m_presets</c> you have", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(layoutPresetsBtn)

            ->appendDialogue("In that list, you can click the <cg>m_presets name</c> to <cy>switch</c> to it, and the <cr>trash</c> button to <cr>delete</c> it!", TutorialCharacterFace::TCFHappy)
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
    m_resetLayoutMenu->addChild(presetsTutorial);


    m_resetLayoutMenu->setOpacity(0);

    m_colorChangeBG = CCScale9Sprite::create("GJ_square05.png");
    m_colorChangeBG->setContentSize({100, 120});
    m_colorChangeBG->setAnchorPoint({0, .5f});
    m_colorChangeBG->setPosition(m_mainLayer->convertToNodeSpace({5, winSize.height / 2}));
    m_colorChangeBG->setZOrder(101);
    m_colorChangeBG->setOpacity(0);
    m_colorChangeBG->setScale(.85f);
    m_colorChangeBG->setCascadeOpacityEnabled(true);
    m_mainLayer->addChild(m_colorChangeBG);

    m_colorMenu = CCMenu::create();
    m_colorMenu->setPosition({0, 0});
    m_colorMenu->ignoreAnchorPointForPosition(false);
    m_colorMenu->setContentSize({0, 0});
    m_colorMenu->setEnabled(false);
    m_colorMenu->setCascadeOpacityEnabled(true);
    m_colorChangeBG->addChild(m_colorMenu);

    m_newBestColorBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    m_newBestColorBtnSpr->setColor(to3B(Save::getNewBestCustomizations().m_color));
    m_newBestColorBtnSpr->setScale(.75f);
    m_newBestColorBtnSpr->setCascadeOpacityEnabled(true);
    auto newBestColorBtn = CCMenuItemSpriteExtra::create(
        m_newBestColorBtnSpr,
        this,
        menu_selector(DTLayer::onNewBestColor)
    );
    newBestColorBtn->setPosition({
        m_colorChangeBG->getContentWidth() / 2,
        m_colorChangeBG->getContentHeight() - newBestColorBtn->getContentHeight() / 2 - 20
    });
    m_colorMenu->addChild(newBestColorBtn);

    auto newBestColorBtnLabel = geode::Label::create("New Best Color", "bigFont.fnt");
    newBestColorBtnLabel->setScale(.3f);
    newBestColorBtnLabel->setAnchorPoint({.5f, 0});
    newBestColorBtnLabel->setCascadeOpacityEnabled(true);
    newBestColorBtnLabel->setPosition(newBestColorBtn->getPosition() + ccp(
        0,
        newBestColorBtn->getContentHeight() / 2 + 1
    ));
    m_colorMenu->addChild(newBestColorBtnLabel);
    
    m_sessionBestColorBtnSpr = CCSprite::createWithSpriteFrameName("GJ_colorBtn_001.png");
    m_sessionBestColorBtnSpr->setColor(to3B(Save::getSessionBestCustomizations().m_color));
    m_sessionBestColorBtnSpr->setScale(.75f);
    m_sessionBestColorBtnSpr->setCascadeOpacityEnabled(true);
    auto sessionBestColorBtn = CCMenuItemSpriteExtra::create(
        m_sessionBestColorBtnSpr,
        this,
        menu_selector(DTLayer::onSessionBestColor)
    );
    sessionBestColorBtn->setPosition({
        m_colorChangeBG->getContentWidth() / 2,
        sessionBestColorBtn->getContentHeight() / 2 + 20
    });
    m_colorMenu->addChild(sessionBestColorBtn);

    auto sessionBestColorBtnLabel = geode::Label::create("Session Best\nColor", "bigFont.fnt");
    sessionBestColorBtnLabel->setScale(.3f);
    sessionBestColorBtnLabel->setAnchorPoint({.5f, 0});
    sessionBestColorBtnLabel->setAlignment(geode::Label::Alignment::Center);
    sessionBestColorBtnLabel->setCascadeOpacityEnabled(true);
    sessionBestColorBtnLabel->setPosition(sessionBestColorBtn->getPosition() + ccp(
        0,
        sessionBestColorBtn->getContentHeight() / 2 + 1
    ));
    m_colorMenu->addChild(sessionBestColorBtnLabel);
    m_colorMenu->setOpacity(0);

    m_scrollLayer->setVisible(false);

    m_layoutInfo = TutorialButton::create(1, "layout-overall", [&, applyChangesButton, discardChangesButton, resetLayoutBtn](DTTutorialLayer* tutorialLayer) {
        tutorialLayer
            ->appendDialogue("This is where you can <cg>Edit how death tracker looks!</c>", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("Here you have different labels! which can display any text you want!", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottom, .65f);

        std::set<DTLabel*> allLabels{};

        int index = 0;
        for (const auto& column : m_columns)
        {
            for (const auto& [_, label] : column->m_labels)
            {
                if (allLabels.contains(label)) continue;

                allLabels.insert(label);
                tutorialLayer->joinHighlight(label->m_labelTitleBG, .1f * index);
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
        for (const auto& label : allLabels) {
            tutorialLayer->joinHighlight(label->m_leftExpandLine, .2f * index, false, false);
            tutorialLayer->joinHighlight(label->m_rightExpandLine, .2f * index, false, false);

            index++;
        }

        tutorialLayer
            ->appendDialogue("Every <cy>label</c> has to be attached to some <cp>column</c>", TutorialCharacterFace::TCFNormalTilted)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .65f);

        index = 0;
        for (const auto& column : m_columns)
        {
            tutorialLayer->joinHighlight(column->m_topSpr, .1f * index);
            index++;
        }

        tutorialLayer
            ->appendDialogue("Each <cp>column</c> can be <cy>clicked</c> to enter its settings", TutorialCharacterFace::TCFHappy)
            ->joinPreviousHighlight()
            ->appendDialogue("And can be <cg>expanded</c> when <cy>dragging its edges</c>", TutorialCharacterFace::TCFNormal);

        index = 0;
        for (const auto& column : m_columns)
        {
            tutorialLayer->joinHighlight(column->m_topBorder2, .2f * index);
            index++;
        }

        tutorialLayer
            ->appendDialogue("You can also edit the <cy>new best</c> and <co>session best</c> colors", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(m_colorChangeBG)
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
    m_layoutInfo->setPosition(m_size);
    m_layoutInfo->setOpacity(0);
    m_layoutInfo->setEnabled(false);
    auto layoutInfoMenu = CCMenu::createWithItem(m_layoutInfo);
    layoutInfoMenu->setPosition({0, 0});
    m_mainLayer->addChild(layoutInfoMenu);

    m_donerListenerHandle = OnDonerStateChangedEvent().listen([&](bool state) { return onDonerStateChanged(state); });

    onDonerStateChanged(AuthManager::isDoner());

    AuthManager::authForDoner();

    return true;
}

void DTLayer::onEditLayout(CCObject*) {
    if (m_layoutOptionsLayer == nullptr) {
        m_layoutOptionsLayer = LayoutOptionsLayer::create({150, m_size.height});
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        m_layoutOptionsLayer->setPosition({winSize.width + 10, (winSize.height - m_layoutOptionsLayer->getContentHeight()) / 2});
        m_layoutOptionsLayer->m_onBackedOut = [&]() {closeOptionsLayer();};
        this->addChild(m_layoutOptionsLayer);
    }

    m_isEditingLayout = true;

    std::set<DTLabel*> visitedLabels{};

    for (const auto& column : m_columns)
    {
        column->setVisibility(true);

        for (const auto& [_, label] : column->m_labels)
        {
            if (visitedLabels.contains(label)) continue;

            visitedLabels.insert(label);
            label->setEditable(true);
        }
    }

    m_decimalPopup->close();

    m_bottomLeftMenu->setEnabled(false);
    m_bottomLeftMenu->stopAllActions();
    m_bottomLeftMenu->runAction(CCFadeTo::create(.15f, 0));

    m_bottomRightMenu->setEnabled(false);
    m_bottomRightMenu->stopAllActions();
    m_bottomRightMenu->runAction(CCFadeTo::create(.15f, 0));

    m_buttonMenu->setEnabled(false);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 0));
    m_editLayoutBtnSpr->stopAllActions();
    m_editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 0));

    m_groupsList->close();
    
    m_colorChangeBG->stopActionByTag(2);
    auto fa = CCFadeTo::create(.15f, 255);
    fa->setTag(2);
    m_colorChangeBG->runAction(fa);
    m_colorMenu->stopAllActions();
    m_colorMenu->runAction(CCFadeTo::create(.15f, 255));
    m_colorMenu->setEnabled(true);
    m_resetLayoutMenu->stopAllActions();
    m_resetLayoutMenu->runAction(CCFadeTo::create(.15f, 255));
    m_resetLayoutMenu->setEnabled(true);

    m_editLayoutMenu->setEnabled(true);
    m_layoutInfo->stopAllActions();
    m_layoutInfo->runAction(CCFadeTo::create(.15f, 255));
    m_layoutInfo->setEnabled(true);
    m_applyChangesButtonSpr->m_BGSprite->stopAllActions();
    m_applyChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 255));
    m_applyChangesButtonSpr->m_label->stopAllActions();
    m_applyChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 255));
    m_discardChangesButtonSpr->m_BGSprite->stopAllActions();
    m_discardChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 255));
    m_discardChangesButtonSpr->m_label->stopAllActions();
    m_discardChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 255));

    m_addColumnButton->stopAllActions();
    m_addColumnButton->runAction(CCFadeTo::create(.15f, 255));
    m_columnHolder->setEnabled(true);

    m_originalNewBestCustomization = Save::getNewBestCustomizations();
    m_originalSessionBestCustomization = Save::getSessionBestCustomizations();
    m_newBestColorBtnSpr->setColor(to3B(m_originalNewBestCustomization.m_color));
    m_sessionBestColorBtnSpr->setColor(to3B(m_originalSessionBestCustomization.m_color));

    m_scrollLayer->moveBy(ccp(0, -LayoutColumn::topHeight));

    this->organizeLayout();
}

bool DTLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    if (m_doMoveScroll && !m_decimalPopup->isTouchOnMe(pTouch, true))
        m_scrollLayer->ccTouchBegan(pTouch, pEvent);

    return true;
}

void DTLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
    if (m_doMoveScroll && !m_decimalPopup->isTouchOnMe(pTouch, false))
        m_scrollLayer->ccTouchMoved(pTouch, pEvent);
}

void DTLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
    m_scrollLayer->ccTouchEnded(pTouch, pEvent);
}

void DTLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) {
    m_scrollLayer->ccTouchCancelled(pTouch, pEvent);
}

void DTLayer::ccTouchesMoved(CCSet* touches, CCEvent* event) {
    if (m_doMoveScroll && !m_decimalPopup->isTouchOnMe(static_cast<CCTouch*>(touches->anyObject()), false))
        m_scrollLayer->ccTouchesMoved(touches, event);
}

void DTLayer::graphBtnClicked(CCObject*) {
    DTGraphLayer::create()->show();
}

void DTLayer::addSpecialString(const std::shared_ptr<SpecialKey>& key) {
    key->setUpdateStartedCallback([&](const std::shared_ptr<SpecialKey>& k) { this->specialKeyUpdateStarted(k); });
    key->setUpdateCompletedCallback([&](const std::shared_ptr<SpecialKey>& k) { this->specialKeyUpdateCompleted(k); });
    m_specialStrings.emplace(key->getKey(), key);
    key->updateContent();
}

void DTLayer::populateSpecialStrings() {
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

    auto cheatedGeneralKey = std::make_shared<SpecialKey>("cheatedGeneral", "Adds all your cheated runs from 0% (shared with linked levels)");
    cheatedGeneralKey->refreshWith(runsKey->getKey());
    cheatedGeneralKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onCheatedGeneralKey));
    addSpecialString(cheatedGeneralKey);

    auto cheatedRunsKey = std::make_shared<SpecialKey>("cheatedRuns", "Adds all your cheated runs (shared with linked levels)");
    cheatedRunsKey->refreshWith(runsKey->getKey());
    cheatedRunsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onCheatedRunsKey));
    addSpecialString(cheatedRunsKey);

    auto cheatedAttemptsKey = std::make_shared<SpecialKey>("cheatedAttempts", "Adds your cheated attempt count (shared with linked levels)");
    cheatedAttemptsKey->refreshWith(runsKey->getKey());
    cheatedAttemptsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onCheatedAttemptsKey));
    addSpecialString(cheatedAttemptsKey);

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

    auto runAttKey = std::make_shared<SpecialKey>("runatt", "Gets how many death tracker attempts youve played from each start position");
    runAttKey->refreshWith(generalKey->getKey());
    runAttKey->refreshWith(runsKey->getKey());
    runAttKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onRunAttemptsKey));
    addSpecialString(runAttKey);

    auto dynamicDecimalKey = std::make_shared<SpecialKey>("dynamicDecimal", "Gets how many death tracker attempts youve played from each start position", false);
    dynamicDecimalKey->refreshWith(generalKey->getKey());
    dynamicDecimalKey->refreshWith(runsKey->getKey());
    dynamicDecimalKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onDynamicDecimalKey));
    addSpecialString(dynamicDecimalKey);

    auto recentDeathsKey = std::make_shared<SpecialKey>("recentDeaths", "Get your top 10 recent deaths");
    recentDeathsKey->refreshWith(generalKey->getKey());
    recentDeathsKey->refreshWith(runsKey->getKey());
    recentDeathsKey->setUpdateFunction(BIND_UPDATE_FUNC(DTLayer::onRecentDeathsKey));
    addSpecialString(recentDeathsKey);
}

void DTLayer::UpdateSharedStats() {
    if (m_MyLevelStats.isErr()) {
        m_linkedLevelsData.clear();
        return;
    }
    auto sharedStats = m_MyLevelStats.unwrap();

    m_linkedLevelsData.clear();
    m_sessionsOrder.m_grouping.clear();
    m_sessionsOrder.m_groupName = "None";

    std::set<std::string> linkedLevels{};
    std::map<std::string, LevelData> visitedLevels{};
    linkedLevels.insert(sharedStats.m_metadata.m_linkedLevels.begin(), sharedStats.m_metadata.m_linkedLevels.end());

    while (true) {
        auto startSize = linkedLevels.size();
        for (const auto& linkedLevel : linkedLevels)
        {
            if (visitedLevels.contains(linkedLevel)) continue;

            auto currStatsRes = StatsManager::getLevelData(linkedLevel);
            if (currStatsRes.isErr()) {
                Notification::create(fmt::format("failed to get data for linked level - {} | {}", linkedLevel, currStatsRes.unwrapErr().m_error))->show();
                continue;
            }
            auto currStats = currStatsRes.unwrap();

            linkedLevels.insert(currStats.m_metadata.m_linkedLevels.begin(), currStats.m_metadata.m_linkedLevels.end());
            visitedLevels.insert({currStats.m_levelKey, currStats});
        }

        if (startSize == linkedLevels.size()) break;
    }

    visitedLevels.insert({sharedStats.m_levelKey, sharedStats});

    for (const auto& [_, level] : visitedLevels) {
        std::for_each(level.m_sessionNames.begin(), level.m_sessionNames.end(), [&](long long key) {
            if (this->m_sessionsOrder.m_grouping.contains(key)) {
                if (!this->m_sessionsOrder.m_grouping[key].m_group[key].contains(level.m_levelKey))
                    this->m_sessionsOrder.m_grouping[key].m_group[key].insert(level.m_levelKey);
            }
            else {
                SessionGrouping grouping{};
                grouping.m_group.insert({key, {level.m_levelKey}});
                this->m_sessionsOrder.m_grouping.insert({key, grouping});
            }
        });

        m_linkedLevelsData.push_back(level);
    }

    if (m_sessionSelector != nullptr)
        m_sessionSelector->setMaximumCount(m_sessionsOrder.m_grouping.size(), false);

    updateStaticGroupings();

    UpdateDeathRelatedStrings();
}

void DTLayer::onSettings(CCObject*) {
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::onLabels(CCObject*) {
    LabelEditLayer::create()->show();
}

void DTLayer::keyBackClicked() {
    if (m_isEditingLayout) {
        if (m_layoutOptionsLayer != nullptr) {
            if (!m_layoutOptionsLayer->isEditingNode())
                onApplyLayoutChanges(nullptr);
            else {
                m_layoutOptionsLayer->keyBackClicked();
            }
        }
    }
    else {
        DTLayer::onClose(nullptr);
    }
}

void DTLayer::onClose(CCObject* sender) {
    if (m_MyLevelStats.isOk() && Settings::getAutoBackupEnabled() && Settings::getAutoBackupAtDTExit()) {
        if (m_MyLevelStats.unwrap().m_metadata.m_autoBackup) {
            StatsManager::addBackup(
                m_MyLevelStats.unwrap().m_levelKey,
                Settings::getAutoBackupGeneral(),
                Settings::getAutoBackupSessionAmount()
            );
        }
    }

    m_organizationListener.cancel();

    for (auto& [_, key] : m_specialStrings) {
        if (key)
        {
            key->setUpdateCompletedCallback(nullptr);
            key->setUpdateStartedCallback(nullptr);
            key->cancel();
        }
    }

    m_keyListeners.clear();

    instance = nullptr;

    verText->stopAllActions();
    verText->runAction(CCFadeTo::create(.2f, 0));

    verSmokeParticle->stopAllActions();
    verSmokeParticle->runAction(CCParticleFadeTo::create(.2f, 0));

    verStarsParticle->stopAllActions();
    verStarsParticle->runAction(CCParticleFadeTo::create(.2f, 0));

    Popup::onClose(sender);
}

void DTLayer::show() {
    Popup::show();
    this->setZOrder(100);

    m_mainLayer->stopAllActions();
    m_mainLayer->setScale(1);
}

void DTLayer::keyDown(enumKeyCodes key, double d) {
    m_scrollLayer->keyDown(key, d);
}
void DTLayer::keyUp(enumKeyCodes key, double d) {
    m_scrollLayer->keyUp(key, d);
}

void DTLayer::onLSOClicked(CCObject*) {
    DTLevelSpecificSettingsLayer::create()->show();
}

DTLayer* DTLayer::get() { return instance; }

AdvancedScrollLayer* DTLayer::getScrollLayer() {
    return m_scrollLayer;
}

void DTLayer::UpdateDeathRelatedStrings() {
    if (m_MyLevelStats.isErr()) return;

    for (const auto& [_, key] : m_specialStrings)
    {
        if (key->getKey() == "general" || key->getKey() == "s0" || key->getKey() == "runs" || key->getKey() == "sruns"
            || key->getKey() == "cheatedGeneral" || key->getKey() == "cheatedRuns" || key->getKey() == "cheatedAttempts")
            key->updateContent();
    }
}

void DTLayer::updateStaticGroupings() {
    m_daySGroup.m_groupName = "Day";
    m_daySGroup.m_grouping.clear();
    m_weekSGroup.m_groupName = "Week";
    m_weekSGroup.m_grouping.clear();
    m_monthSGroup.m_groupName = "Month";
    m_monthSGroup.m_grouping.clear();

    for (const auto& [date, group] : m_sessionsOrder.m_grouping)
    {
        for (const auto& [date, lvls] : group.m_group)
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
            
            if (!m_daySGroup.m_grouping.contains(startOfDay)) {
                SessionGrouping grouping{};
                m_daySGroup.m_grouping.insert({startOfDay, grouping});
            }
            m_daySGroup.m_grouping[startOfDay].m_group.insert({date, lvls});

            if (!m_weekSGroup.m_grouping.contains(startOfWeek)) {
                SessionGrouping grouping{};
                m_weekSGroup.m_grouping.insert({startOfWeek, grouping});
            }
            m_weekSGroup.m_grouping[startOfWeek].m_group.insert({date, lvls});

            if (!m_monthSGroup.m_grouping.contains(startOfMonth)) {
                SessionGrouping grouping{};
                m_monthSGroup.m_grouping.insert({startOfMonth, grouping});
            }
            m_monthSGroup.m_grouping[startOfMonth].m_group.insert({date, lvls});
        }
    }  
}

bool DTLayer::createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests, const CustomizationData& newBestCustomizations, bool ignoreExtraSettings, bool isSession) {
    out = "";
    if (m_MyLevelStats.isErr()) return false;
    auto& stats = m_MyLevelStats.unwrap();

    auto toReturn = createDeathsString(deaths, stats.m_metadata, custom, out, newBests, newBestCustomizations, ignoreExtraSettings, isSession);

    return toReturn;
}

bool DTLayer::createDeathsString(const Deaths& deaths, const LevelMetadata& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests,  const CustomizationData& newBestCustomizations, bool ignoreExtraSettings, bool isSession) {
    out = "";

    const auto isNewBest = [&](double percent) {
        if (!newBests)
            return false;
        if (newBests->contains(percent))
            return true;

        return std::any_of(newBests->begin(), newBests->end(), [percent](double bestPercent) {
            return std::floor(bestPercent) == percent;
        });
    };

    std::vector<std::pair<std::string, int>> deathVec;

    std::unordered_map<std::string, int> mergedDeaths;
    for (const auto& [originalRunStr, count] : deaths) {
        auto runSplitRes = StatsManager::splitRunKey(originalRunStr);
        if (runSplitRes.isErr()) continue;
        auto runSplit = runSplitRes.unwrap();

        runSplit.m_end = std::floor(runSplit.m_end);
        if (runSplit.m_start)
            runSplit.m_start = std::floor(runSplit.m_start.value());

        if (!ignoreExtraSettings) {
            if (runSplit.m_end >= meta.m_realEndPercent)
                runSplit.m_end = 100;

            if (runSplit.m_start && runSplit.m_start.value() >= meta.m_realEndPercent)
                runSplit.m_start = 100;
        }

        auto res = StatsManager::createRunKey(runSplit);
        if (res.isErr()) continue;
        mergedDeaths[res.unwrap()] += count;
    }

    deathVec.assign(mergedDeaths.begin(), mergedDeaths.end());
        

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

        if (runA.m_start == runB.m_start)
            return runA.m_end < runB.m_end;

        return runA.m_start < runB.m_start;
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

        bool includeRunStart = runSplit.m_start.has_value();

        std::string nbDeColor;
        std::string nbColor;

        if (includeRunStart && !ignoreExtraSettings) {
            int startValue = static_cast<int>(std::floor(runSplit.m_start.value()));

            if (!meta.m_showAnyRun && !meta.m_runsToShow.empty()) {
                auto it = meta.m_runsToShow.find(startValue);

                if (it != meta.m_runsToShow.end()) {
                    if (it->second > runSplit.m_end) {
                        if (std::numeric_limits<uint64_t>::max() - hiddenRuns < amount) {
                            log::error("Hidden death count overflow while formatting decimal deaths");
                            return false;
                        }
                        hiddenRuns += amount;
                        continue;
                    }
                }
                else {
                    if (std::numeric_limits<uint64_t>::max() - hiddenRuns < amount) {
                        log::error("Hidden death count overflow while formatting decimal deaths");
                        return false;
                    }
                    hiddenRuns += amount;
                    continue;
                }
            }
            else {
                if (meta.m_sharedRunToShow > runSplit.m_end - startValue) {
                    if (std::numeric_limits<uint64_t>::max() - hiddenRuns < amount) {
                        log::error("Hidden death count overflow while formatting decimal deaths");
                        return false;
                    }
                    hiddenRuns += amount;
                    continue;
                }
            }

            if (runIndex % 2 == 0) {
                nbDeColor = "</cplus>";
                nbColor = fmt::format("<cplus={}>", custom.m_alternateStrength);
            }
        }
        else if (!includeRunStart && !ignoreExtraSettings) {
            if (meta.m_hideUpto > runSplit.m_end && runSplit.m_end >= 0) {
                if (std::numeric_limits<uint64_t>::max() - hiddenRuns < amount) {
                    log::error("Hidden death count overflow while formatting decimal deaths");
                    return false;
                }
                hiddenRuns += amount;
                continue;
            }

            if (isNewBest(runSplit.m_end)) {
                nbDeColor = fmt::format("</color>{}{}",
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? "</gradient>" : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? "</wave>" : ""
                );

                std::string colorKey = fmt::format("<color={}>", cc4bToHexString(newBestCustomizations.m_color));
                std::string gradientKey = fmt::format("<gradient={},{},{},{}>",
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorA),
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorB),
                    newBestCustomizations.m_gradient.m_duration,
                    newBestCustomizations.m_gradient.m_stagger
                );
                std::string waveKey = fmt::format("<wave={},{},{},{}>",
                    newBestCustomizations.m_wave.m_speed,
                    newBestCustomizations.m_wave.m_distanceY,
                    newBestCustomizations.m_wave.m_distanceX,
                    newBestCustomizations.m_wave.m_offsetPerIndex
                );

                nbColor = fmt::format(
                    "{}{}{}",
                    colorKey,
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? gradientKey : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? waveKey : ""
                );
            }
            else if (runIndex % 2 == 0) {
                nbDeColor = "</cplus>";
                nbColor = fmt::format("<cplus={}>", custom.m_alternateStrength);
            }
        }
        else {
            if (isNewBest(runSplit.m_end)) {
                nbDeColor = fmt::format("</color>{}{}",
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? "</gradient>" : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? "</wave>" : ""
                );

                std::string colorKey = fmt::format("<color={}>", cc4bToHexString(newBestCustomizations.m_color));
                std::string gradientKey = fmt::format("<gradient={},{},{},{}>",
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorA),
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorB),
                    newBestCustomizations.m_gradient.m_duration,
                    newBestCustomizations.m_gradient.m_stagger
                );
                std::string waveKey = fmt::format("<wave={},{},{},{}>",
                    newBestCustomizations.m_wave.m_speed,
                    newBestCustomizations.m_wave.m_distanceY,
                    newBestCustomizations.m_wave.m_distanceX,
                    newBestCustomizations.m_wave.m_offsetPerIndex
                );

                nbColor = fmt::format(
                    "{}{}{}",
                    colorKey,
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? gradientKey : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? waveKey : ""
                );
            }
        }

        auto runKey = StatsManager::createRunKey(runSplit);

        if (runKey.isOk()) {
            // nbColor += fmt::format("<runBtn={}>", runKey.unwrap());
            // nbDeColor += "</runBtn>";

            nbDeColor += fmt::format(" <runMoreBtn={},{}>C</runMoreBtn>", runKey.unwrap(), isSession);
        }

        int currentStart = runSplit.m_start.value_or(-1);

        if (prevStart != currentStart) {
            if (prevStart != -2) {
                out += "----------{nl}";
            }

            prevStart = currentStart;
        }

        auto format = custom.m_format;

        std::string toReplaceWith =
            includeRunStart
                ? fmt::format("{}-{}", runSplit.m_start.value(), runSplit.m_end)
                : fmt::format("{}", runSplit.m_end);

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
            custom.m_seperator
        );

        runIndex++;
    }

    if (out.empty() && hiddenRuns == 0) {
        out = "No Deaths Found!";
    }
    else {
        if (!custom.m_seperator.empty() && out.size() >= custom.m_seperator.size()) {
            out.erase(out.size() - custom.m_seperator.size());
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

bool DTLayer::createDecimalDeathsString(const ComplexDeathMap& deaths, const Run& selectedRun, const LevelMetadata& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests,  const CustomizationData& newBestCustomizations, bool ignoreExtraSettings, uint8_t decimalDepth) {
    out = "";

    if (decimalDepth > ComplexPercent::maxDecimalDepth) {
        log::error("Decimal depth must be between 0 and {}", ComplexPercent::maxDecimalDepth);
        return false;
    }

    ComplexPercent::Units decimalFactor = 1;
    for (auto depth = decimalDepth; depth < ComplexPercent::maxDecimalDepth; ++depth)
        decimalFactor *= 10;

    const auto floorToDecimalDepth = [decimalFactor](ComplexPercent::Units units) {
        auto quotient = units / decimalFactor;
        if (units < 0 && units % decimalFactor != 0)
            --quotient;
        return quotient * decimalFactor;
    };

    const auto isNewBest = [&](ComplexPercent::Units percentUnits) {
        if (!newBests)
            return false;

        return std::any_of(newBests->begin(), newBests->end(), [&](double bestPercent) {
            auto parsedBest = ComplexPercent::fromPercent(bestPercent);
            return parsedBest.isOk() && floorToDecimalDepth(parsedBest.unwrap().m_units) == percentUnits;
        });
    };

    std::vector<std::pair<std::string, uint64_t>> deathVec;

    std::unordered_map<std::string, uint64_t> mergedDeaths;
    for (const auto& [run, death] : deaths.filterCheatedRuns().m_records) {
        Run runSplit{
            run.second.has_value()
                ? std::optional<double>(ComplexPercent{floorToDecimalDepth(*run.second)}.asPercent())
                : std::nullopt,
            ComplexPercent{floorToDecimalDepth(run.first)}.asPercent()
        };

        if (!ignoreExtraSettings) {
            if (runSplit.m_end >= meta.m_realEndPercent)
                runSplit.m_end = 100;

            if (runSplit.m_start && runSplit.m_start.value() >= meta.m_realEndPercent)
                runSplit.m_start = 100;
        }

        if (std::floor(runSplit.m_end) != std::floor(selectedRun.m_end))
            continue;
        if (runSplit.m_start.has_value() != selectedRun.m_start.has_value())
            continue;
        if (runSplit.m_start && std::floor(runSplit.m_start.value()) != std::floor(selectedRun.m_start.value()))
            continue;

        auto res = StatsManager::createRunKey(runSplit);
        if (res.isErr()) continue;
        auto& mergedCount = mergedDeaths[res.unwrap()];
        if (std::numeric_limits<uint64_t>::max() - mergedCount < death.m_deaths) {
            log::error("Death count overflow while formatting decimal deaths");
            return false;
        }
        mergedCount += death.m_deaths;
    }

    deathVec.assign(mergedDeaths.begin(), mergedDeaths.end());
        

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

        if (runA.m_start == runB.m_start)
            return runA.m_end < runB.m_end;

        return runA.m_start < runB.m_start;
    });

    std::optional<double> prevStart;
    uint64_t hiddenRuns = 0;
    int runIndex = 0;

    for (const auto& [run, amount] : deathVec) {
        auto runSplitRes = StatsManager::splitRunKey(run);

        if (runSplitRes.isErr()) {
            log::error("Failed to split run key! {}", runSplitRes.unwrapErr());
            continue;
        }

        auto& runSplit = runSplitRes.unwrap();

        bool includeRunStart = runSplit.m_start.has_value();

        std::string nbDeColor;
        std::string nbColor;

        if (includeRunStart && !ignoreExtraSettings) {
            int startValue = static_cast<int>(std::floor(runSplit.m_start.value()));

            if (!meta.m_showAnyRun && !meta.m_runsToShow.empty()) {
                auto it = meta.m_runsToShow.find(startValue);

                if (it != meta.m_runsToShow.end()) {
                    if (it->second > runSplit.m_end) {
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
                if (meta.m_sharedRunToShow > runSplit.m_end - startValue) {
                    hiddenRuns += amount;
                    continue;
                }
            }

            if (runIndex % 2 == 0) {
                nbDeColor = "</cplus>";
                nbColor = fmt::format("<cplus={}>", custom.m_alternateStrength);
            }
        }
        else if (!includeRunStart && !ignoreExtraSettings) {
            if (meta.m_hideUpto > runSplit.m_end && runSplit.m_end >= 0) {
                hiddenRuns += amount;
                continue;
            }

            auto endPercent = ComplexPercent::fromPercent(runSplit.m_end);
            if (endPercent.isOk() && isNewBest(endPercent.unwrap().m_units)) {
                nbDeColor = fmt::format("</color>{}{}",
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? "</gradient>" : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? "</wave>" : ""
                );

                std::string colorKey = fmt::format("<color={}>", cc4bToHexString(newBestCustomizations.m_color));
                std::string gradientKey = fmt::format("<gradient={},{},{},{}>",
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorA),
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorB),
                    newBestCustomizations.m_gradient.m_duration,
                    newBestCustomizations.m_gradient.m_stagger
                );
                std::string waveKey = fmt::format("<wave={},{},{},{}>",
                    newBestCustomizations.m_wave.m_speed,
                    newBestCustomizations.m_wave.m_distanceY,
                    newBestCustomizations.m_wave.m_distanceX,
                    newBestCustomizations.m_wave.m_offsetPerIndex
                );

                nbColor = fmt::format(
                    "{}{}{}",
                    colorKey,
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? gradientKey : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? waveKey : ""
                );
            }
            else if (runIndex % 2 == 0) {
                nbDeColor = "</cplus>";
                nbColor = fmt::format("<cplus={}>", custom.m_alternateStrength);
            }
        }
        else {
            auto endPercent = ComplexPercent::fromPercent(runSplit.m_end);
            if (endPercent.isOk() && isNewBest(endPercent.unwrap().m_units)) {
                nbDeColor = fmt::format("</color>{}{}",
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? "</gradient>" : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? "</wave>" : ""
                );

                std::string colorKey = fmt::format("<color={}>", cc4bToHexString(newBestCustomizations.m_color));
                std::string gradientKey = fmt::format("<gradient={},{},{},{}>",
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorA),
                    cc4bToHexString(newBestCustomizations.m_gradient.m_colorB),
                    newBestCustomizations.m_gradient.m_duration,
                    newBestCustomizations.m_gradient.m_stagger
                );
                std::string waveKey = fmt::format("<wave={},{},{},{}>",
                    newBestCustomizations.m_wave.m_speed,
                    newBestCustomizations.m_wave.m_distanceY,
                    newBestCustomizations.m_wave.m_distanceX,
                    newBestCustomizations.m_wave.m_offsetPerIndex
                );

                nbColor = fmt::format(
                    "{}{}{}",
                    colorKey,
                    newBestCustomizations.m_gradientEnabled && AuthManager::isDoner() ? gradientKey : "",
                    newBestCustomizations.m_waveEnabled && AuthManager::isDoner() ? waveKey : ""
                );
            }
        }

        auto currentStart = runSplit.m_start;

        if (prevStart != currentStart) {
            if (runIndex != 0) {
                out += "----------{nl}";
            }

            prevStart = currentStart;
        }

        auto format = custom.m_format;

        std::string toReplaceWith =
            includeRunStart
                ? fmt::format("{}-{}", runSplit.m_start.value(), runSplit.m_end)
                : fmt::format("{}", runSplit.m_end);

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
            custom.m_seperator
        );

        runIndex++;
    }

    if (out.empty() && hiddenRuns == 0) {
        out = "No Deaths Found!";
    }
    else {
        if (!custom.m_seperator.empty() && out.size() >= custom.m_seperator.size()) {
            out.erase(out.size() - custom.m_seperator.size());
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

int DTLayer::getCurrentSelectedSession() {
    return m_sessionSelector->getCurrentCount();
}

void DTLayer::onSessionSelected(int sessionNum, bool updateContent) {
    auto it = getCurrentGrouping().m_grouping.begin();
    std::advance(it, sessionNum - 1);

    if (it == getCurrentGrouping().m_grouping.end()) return;

    if (sessionNum - 1 == m_currentSession) return;
    m_currentSession = sessionNum - 1;

    if (m_sessionSelector->getCurrentCount() != m_currentSession + 1) m_sessionSelector->setCurrentCount(m_currentSession + 1);

    if (updateContent) {
        m_specialStrings["s0"]->updateContent();
        m_specialStrings["sruns"]->updateContent();
    }
}

//better info time calc

uint64_t DTLayer::timeInMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string DTLayer::decodeBase64Gzip(std::string_view input) {
    return ZipUtils::decompressString(std::string(input), false, 0);
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

float DTLayer::timeForLevelString(std::string_view levelString) {
    struct SpeedPortalObject {
        int m_id;
        float m_xPos;
        bool m_checked;
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
        while(getline(responseStream, currentObject, ';')) {
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
            return a.m_xPos < b.m_xPos;
        });

        for(const auto& portal : speedPortals) {
            //log::info("Object ID: {}, X Position: {}, Portal ID: {}", portal.id, portal.xPos, prevPortalId);
            timeFull += (portal.m_xPos - prevPortalX) / travelForPortalId(prevPortalId);
            prevPortalId = portal.m_id;
            prevPortalX = portal.m_xPos;
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

void DTLayer::update(float dt) {
    m_columnHolder->updateLayout();

    float oldWidth = m_scrollLayer->m_content->getContentWidth();

    float width = std::max(m_ogLimits.width, m_columnHolder->getContentWidth() - (m_isEditingLayout ? 0 : m_addColumnButton->getContentWidth()));

    if (oldWidth != width) {
        m_scrollLayer->setLimitsWidth(width);
    }
}

void DTLayer::organizeLayout() {
    if (!m_canOrganize) return;
    m_organizationListener.spawn(
        "DT-organize-layout-task",
        organizeLayoutTask(),
        [this](organizationFuture::Output result) {
            if (m_scrollLayer->isAtMinZoom())
                m_scrollLayer->zoomBy(0.001f);


            float fixedhighest = result.m_highestColumn - (m_isEditingLayout ? 0 : LayoutColumn::topHeight + LayoutColumn::addNewBtnOffset * 2);
            for (const auto& column : m_columns) {
                column->setContentHeight(fixedhighest);
            }

            float cappedHeight = std::max(fixedhighest, m_ogLimits.height);
            float oldHeightLimits = m_scrollLayer->m_content->getContentHeight();
            float oldMinimumPosition = m_scrollLayer->getMinimumPosition(false);
            float oldMaximumPosition = m_scrollLayer->getMaximumPosition(false);
            float oldPositionRange = oldMaximumPosition - oldMinimumPosition;
            float scrollProgress = oldPositionRange > 0
                ? std::clamp((m_scrollLayer->m_content->getPositionY() - oldMinimumPosition) / oldPositionRange, 0.f, 1.f)
                : 0.5f;

            m_scrollLayer->setLimitsHeight(cappedHeight);
            float delta = oldHeightLimits - m_scrollLayer->m_content->getContentHeight();
            float newMinimumPosition = m_scrollLayer->getMinimumPosition(false);
            float newMaximumPosition = m_scrollLayer->getMaximumPosition(false);
            float newPositionY = newMinimumPosition + scrollProgress * (newMaximumPosition - newMinimumPosition);
            m_scrollLayer->moveTo({m_scrollLayer->m_content->getPositionX(), newPositionY});

            m_columnHolder->setPositionY(cappedHeight);
            m_columnHolder->updateLayout();
            m_labelsHolder->setPosition(m_columnHolder->getPosition());

            for (const auto& [label, newPos, newWidth] : result.m_labelData)
            {
                constexpr int MOVEMENT_TAG = 2;
                constexpr int RESIZE_TAG = 7;

                auto taggedMovementAction = label->getActionByTag(MOVEMENT_TAG);
                auto taggedResizeAction = label->getActionByTag(RESIZE_TAG);

                bool doCreateNewMoveAction = true;
                bool doCreateNewResizeAction = true;

                //movement action check
                if (taggedMovementAction != nullptr) {
                    auto currentMovementActionEase = static_cast<CCEaseInOut*>(taggedMovementAction);
                    auto currentMovementAction = static_cast<CCMoveTo*>(currentMovementActionEase->getInnerAction());

                    if (!currentMovementAction->m_endPosition.equals(newPos)) {
                        label->stopActionByTag(MOVEMENT_TAG);
                    }
                    else doCreateNewMoveAction = false;
                }

                if (doCreateNewMoveAction) {
                    auto pos = newPos + ccp(0, m_isEditingLayout ? 0 : LayoutColumn::topHeight);
                    if (!m_cornerOnNextOrganization) {
                        auto movementAction = CCEaseInOut::create(CCMoveTo::create(DTLayer::transitionTime, pos), 2);
                        movementAction->setTag(MOVEMENT_TAG);

                        label->runAction(movementAction);
                    }
                    else {
                        label->setPosition(pos);
                    }
                }

                //resize action check
                if (taggedResizeAction != nullptr) {
                    auto currentResizeActionEase = static_cast<CCEaseInOut*>(taggedResizeAction);
                    auto currentResizeAction = static_cast<CCResizeWidthTo*>(currentResizeActionEase->getInnerAction());

                    if (currentResizeAction->getEndWidth() != newWidth) {
                        label->stopActionByTag(RESIZE_TAG);
                    }
                    else doCreateNewResizeAction = false;
                }

                if (doCreateNewResizeAction) {
                    if (!m_cornerOnNextOrganization) {
                        auto resizeAction = CCEaseInOut::create(CCResizeWidthTo::create(DTLayer::transitionTime, newWidth), 2);
                        resizeAction->setTag(RESIZE_TAG);

                        label->runAction(resizeAction);
                    }
                    else {
                        label->setContentWidth(newWidth);
                    }
                }
            }

            for (const auto& [target, _] : m_onOrganizationCompleteEvent)
            {
                m_onOrganizationCompleteEvent[target](delta);
            }

            if (m_cornerOnNextOrganization) {
                m_cornerOnNextOrganization = false;
                m_scrollLayer->moveToCorner(true, false);
            }

            if (m_firstTime == 0) {
                m_firstTime = 1;
                m_scrollLayer->setVisible(true);
                m_lc->removeMeAndCleanup();
                m_cornerOnNextOrganization = false;

                this->organizeLayout();
            }
            else if (m_firstTime == 1) {
                m_firstTime = 2;

                m_scrollLayer->moveToCorner(true, false);
            }
        }
    );
}

organizationFuture DTLayer::organizeLayoutTask() {
    struct LabelData {
        DTLabel* m_label;
        std::set<LayoutColumn*> m_holders;
    };

    struct ColumnData {
        LayoutColumn* m_column;
        int m_orderPos;
        std::vector<std::pair<int, DTLabel*>> m_labels;
    };

    std::vector<ColumnData> columnSnapshots;
    std::map<DTLabel*, LabelData> labelSnapshots;

    for (const auto& column : m_columns)
    {
        ColumnData colData;
        colData.m_column = column;
        colData.m_orderPos = column->m_info.m_orderPos;

        for (const auto& [layer, label] : column->m_labels)
        {
            colData.m_labels.push_back({layer, label});

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
        for (const auto& [layer, label] : colData.m_labels)
        {
            co_await arc::yield();
            if (allLabels.contains(label)) continue;

            allLabels.insert(label);
            label->m_tempPos = ccp(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            label->m_tempWidth = 0;
        }

        co_await arc::yield();
    }

    for (const auto& [label, data] : labelSnapshots)
    {
        labelHolders[label] = data.m_holders;

        co_await arc::yield();
    }

    std::map<LayoutColumn*, DTLabel*> lastVisitedLabelForColumn{};
    std::map<DTLabel*, std::map<LayoutColumn*, std::optional<int>>> labelAwaitingColumnValues{};
    std::set<DTLabel*> processedLabels{};

    auto UpdateTempPos = [&](LayoutColumn* column, DTLabel* label, DTLabel* prevLabel) -> arc::Future<> {
        if (column == nullptr || label == nullptr || label->getParent() == nullptr) co_return;
        auto startPosInLabelSpace = label->getParent()->convertToNodeSpace(column->convertToWorldSpace(column->m_bgSpr->getPosition()));

        float prevHeight = startPosInLabelSpace.y;

        if (label->m_info.m_layer != 0 && prevLabel != nullptr) prevHeight = prevLabel->m_tempPos.y - prevLabel->getContentHeight();

        if (prevHeight < label->m_tempPos.y) label->m_tempPos.y = prevHeight;

        float newX = startPosInLabelSpace.x;

        if (newX < label->m_tempPos.x) label->m_tempPos.x = newX;
    };

    auto UpdateTempWidth = [](DTLabel* label) -> arc::Future<> {
        label->m_tempWidth = 0;
        for (const auto& labelColumn : label->getHolders())
        {
            label->m_tempWidth += labelColumn->getContentWidth();

            co_await arc::yield();
        }
    };

    while (true) {
        for (const auto& colData : columnSnapshots)
        {
            const auto& column = colData.m_column;
            // log::info("goind over column {}", colData.m_orderPos);
            DTLabel* prevLabel = nullptr;

            bool foundLastLabel = false;

            for (const auto& [labelLayer, label] : colData.m_labels)
            {
                // log::info("going through label");
                if (!lastVisitedLabelForColumn.contains(column)) {
                    lastVisitedLabelForColumn.insert({column, label});
                    foundLastLabel = true;
                }
                else if (label != lastVisitedLabelForColumn[column] && !foundLastLabel) {
                    co_await UpdateTempPos(column, label, prevLabel);
                    prevLabel = label;
                    continue;
                }
                else foundLastLabel  = true;

                if (processedLabels.contains(label)) {
                    co_await UpdateTempPos(column, label, prevLabel);
                    prevLabel = label;
                    continue;
                }

                // log::info("label valid");

                lastVisitedLabelForColumn[column] = label;

                int newLayer = prevLabel == nullptr ? 0 : prevLabel->m_info.m_layer + 1;

                // log::info("destenation layer {}", newLayer);

                auto& holdersBack = labelHolders[label];

                if (holdersBack.size() > 1) {
                    // log::info("label is multicolumn");
                    if (!labelAwaitingColumnValues.contains(label)) {
                        std::map<LayoutColumn*, std::optional<int>> mapToSet{};

                        for (const auto& holder : holdersBack) {
                            mapToSet.insert({holder, std::nullopt});
                            co_await arc::yield();
                        }

                        // log::info("populated list for label with {} holders", mapToSet.size());

                        labelAwaitingColumnValues.insert({label, mapToSet});
                    }

                    // log::info("adding label value?");

                    if (labelAwaitingColumnValues[label].contains(column) && !labelAwaitingColumnValues[label][column].has_value()) {
                        // log::info("label value added");
                        labelAwaitingColumnValues[label][column] = newLayer;
                    }

                    int highestOptLayer = 0;
                    bool wereAllLayersFound = true;

                    co_await UpdateTempPos(column, label, prevLabel);

                    // log::info("checking conclusion..");

                    for (const auto& [column, optLayer] : labelAwaitingColumnValues[label]) {
                        if (!optLayer.has_value()) {
                            wereAllLayersFound = false;
                            co_await arc::yield();
                            break;
                        }

                        highestOptLayer = std::max(highestOptLayer, optLayer.value());
                        co_await arc::yield();
                    }

                    if (!wereAllLayersFound) {
                        // log::info("invalid labels were found");
                        co_await arc::yield();
                        break;
                    }

                    // log::info("all layer values were found for label");

                    label->m_info.m_layer = highestOptLayer;
                    processedLabels.insert(label);

                    co_await UpdateTempWidth(label);

                    // log::info("combo found at {}", highestOptLayer);

                    prevLabel = label;
                    continue;
                }

                // log::info("non double found! adding..");

                label->m_info.m_layer = newLayer;
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

    for (const auto& colData : columnSnapshots) {
        colData.m_column->refreshAllLabelsLayer();
        co_await arc::yield();
    }

    float heighestHeight = 0;

    for (const auto& colData : columnSnapshots) {
        for (const auto& [_, label] : colData.m_labels)
        {
            auto height = std::abs(label->m_tempPos.y) + label->getContentHeight() + LayoutColumn::addNewBtnOffset * 2;
            if (heighestHeight < height) heighestHeight = height;
            co_await arc::yield();
        }
        co_await arc::yield();
    }

    organizationResult data{};
    data.m_highestColumn = heighestHeight;

    for (const auto& label : allLabels)
    {
        auto targetPosition = label->m_tempPos;
        auto targetWidth = label->m_tempWidth - LayoutColumn::borderWidth;

        data.m_labelData.push_back({label, targetPosition, targetWidth});
        co_await arc::yield();
    }

    co_await arc::yield();
    co_return data;
}

std::pair<LayoutColumn*, int> DTLayer::getColumnLayerFromPosition(CCPoint posInWorldSpace) {

    auto posInColumnHolderSpace = m_columnHolder->convertToNodeSpace(posInWorldSpace);

    LayoutColumn* columnFound = nullptr;

    for (const auto& column : m_columns)
    {
        if (column->boundingBox().containsPoint(posInColumnHolderSpace + ccp(DTColumnInfo::minWidth / 2, 0))) {
            columnFound = column;
            // log::info("found column {}", columnFound->m_orderPos);
            break;
        }
    }

    if (columnFound == nullptr) return {nullptr, 0};

    auto posInLabelHolderSpace = m_labelsHolder->convertToNodeSpace(posInWorldSpace);

    if (!columnFound->m_labels.size()) return {columnFound, 0};

    int highestLayerFound = 0;

    for (const auto& [labelLayer, label] : columnFound->m_labels)
    {
        auto highestLabelPoint = label->getPositionY() - label->getContentHeight() + DTLabel::labelTitleHeight / 2;

        if (highestLayerFound < labelLayer) highestLayerFound = labelLayer;

        if (highestLabelPoint < posInLabelHolderSpace.y) {

            return {columnFound, labelLayer};
        }
    }

    return {columnFound, highestLayerFound + 1};
}

std::set<LayoutColumn*, ColumnComperator> DTLayer::getColumnsBetween(CCPoint a, CCPoint b) {

    std::set<LayoutColumn*, ColumnComperator> toReturn{};

    a = m_columnHolder->convertToNodeSpace(a);
    b = m_columnHolder->convertToNodeSpace(b);

    auto x = std::min(a.x, b.x);
    auto y = std::min(a.y, b.y);
    auto w = fabs(b.x - a.x);
    auto h = fabs(b.y - a.y);

    auto rect = CCRect{x, y, w, h};

    // log::info("checking rects... {}", rect);

    for (const auto& column : m_columns)
    {
        if (rect.intersectsRect(column->boundingBox()))
            toReturn.insert(column);
    }

    return toReturn;
}

void DTLayer::addColumnBtnClicked(CCObject*) {
    addColumn();
}

LayoutColumn* DTLayer::addColumn(std::optional<DTColumnInfo> info) {
    if (!info.has_value()) {
        int position = 0;

        if (m_columnHolder->getChildrenCount() > 1) {
            auto highestColumn = static_cast<CCNode*>(m_columnHolder->getChildren()->objectAtIndex(m_columnHolder->getChildrenCount() - 2));
            position = highestColumn->getZOrder() + 1;
        }

        info = DTColumnInfo {
            .m_orderPos = position
        };
    }
    else {
        
        for (const auto& column : m_columns)
        {
            if (column->m_info.m_orderPos >= info.value().m_orderPos) {
                column->m_info.m_orderPos++;
                column->setZOrder(column->m_info.m_orderPos);
            }
        }
    }

    auto column = LayoutColumn::create(info.value(), m_isEditingLayout, m_scrollLayer->getContentHeight());
    m_columnHolder->addChild(column);
    for (const auto& label : CCArrayExt<DTLabel*>(m_labelsHolder->getChildren())) {
        if (label->m_info.m_maxPlacementRange >= info.value().m_orderPos && label->m_info.m_minPlacementRange < info.value().m_orderPos) {
            column->addLabel(label);
        }
    }
    
    m_columns.insert(column);

    for (const auto& label : CCArrayExt<DTLabel*>(m_labelsHolder->getChildren())) {
        label->updateInfoWithColumnData();
    }

    organizeLayout();

    return column;
}


DTLabel* DTLayer::createNewLabel(DTLabelInfo info) {
    auto newLabel = DTLabel::create(info);
    m_labelsHolder->addChild(newLabel);
    newLabel->attachedDecimalPopup(m_decimalPopup);
    if (m_isEditingLayout) newLabel->setEditable(true);

    return newLabel;
}


void DTLayer::subscribeToOrganizationEvent(CCNode* target, Function<void(float)> callback) {
    if (m_onOrganizationCompleteEvent.contains(target)) return;

    m_onOrganizationCompleteEvent.insert({target, std::move(callback)});
}
void DTLayer::unsubscribeToOrganizationEvent(CCNode* target) {
    if (!m_onOrganizationCompleteEvent.contains(target)) return;

    m_onOrganizationCompleteEvent.erase(target);
}

void DTLayer::setLayoutBy(const DTLayoutV3& layout)
{
    auto columnsCopy = m_columns;
    for (auto* column : columnsCopy)
    {
        column->destroyColumnAndCleanup();
    }
    m_columns.clear();

    for (const auto& column : layout.m_columns)
    {
        addColumn(column);
    }

    fixUpColumnPositions();
    m_columnHolder->updateLayout();

    std::map<LayoutColumn*, std::vector<DTLabel*>> labelsForColumns;
    std::vector<DTLabel*> allLabels;
    allLabels.reserve(layout.m_labels.size());

    for (const auto& label : layout.m_labels)
    {
        DTLabel* labelNode = createNewLabel(label);
        allLabels.push_back(labelNode);

        const int minPos = labelNode->m_info.m_minPlacementRange;
        const int maxPos = labelNode->m_info.m_maxPlacementRange;

        for (auto* column : m_columns)
        {
            const int pos = column->m_info.m_orderPos;

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

    m_cornerOnNextOrganization = true;
}

void DTLayer::fixUpColumnPositions() {

    int supposedOrder = 0;

    for (const auto& column : m_columns)
    {
        column->m_info.m_orderPos = supposedOrder;
        column->setZOrder(column->m_info.m_orderPos);

        supposedOrder++;
    }
}

void DTLayer::saveCurrentLayout() {
    Save::setLayout(currentLayout());
}

DTLayoutV3 DTLayer::currentLayout() {
    std::set<DTLabel*> labels{};

    DTLayoutV3 layout{};

    for (const auto& column : m_columns)
    {
        layout.m_columns.push_back(column->m_info);

        for (const auto& [labelLayer, label] : column->m_labels)
        {
            if (labels.contains(label)) continue;

            labels.insert(label);

            layout.m_labels.push_back(label->m_info);
        }
    }

    return layout;
}

void DTLayer::specialKeyUpdateStarted(const std::shared_ptr<SpecialKey>& key) {
    for (const auto& label : m_keyListeners)
    {
        label->setLoading(key);
    }
}

void DTLayer::specialKeyUpdateCompleted(const std::shared_ptr<SpecialKey>& key) {

    for (const auto& [_, otherKey] : m_specialStrings)
    {
        if (otherKey->doesRefreshWith(key->getKey()))
            otherKey->updateContent();
    }

    for (const auto& label : m_keyListeners)
    {
        label->completeLoading(key);
    }
}

void DTLayer::setOptionsLayerTo(DTLabel* label) {
    if (m_layoutOptionsLayer == nullptr) return;
    if (!m_layoutOptionsLayer->isEditingNode()) {
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-70.5f, 0))
            )
        );
        m_layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );

        m_colorChangeBG->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );
    }

    m_layoutOptionsLayer->setEditedNodeTo(label);
}
void DTLayer::setOptionsLayerTo(LayoutColumn* column) {
    if (m_layoutOptionsLayer == nullptr) return;
    if (!m_layoutOptionsLayer->isEditingNode()) {
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-70.5f, 0))
            )
        );
        m_layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );

        m_colorChangeBG->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(-160, 0))
            )
        );
    }

    m_layoutOptionsLayer->setEditedNodeTo(column);
}
void DTLayer::closeOptionsLayer() {
    if (m_layoutOptionsLayer == nullptr) return;
    if (m_layoutOptionsLayer->isEditingNode()) {
        m_mainLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(70.5f, 0))
            )
        );
        m_layoutOptionsLayer->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(160, 0))
            )
        );

        m_colorChangeBG->runAction(
            CCEaseBackOut::create(
                CCMoveBy::create(0.5f, ccp(160, 0))
            )
        );
    }

    m_layoutOptionsLayer->close();
}
void DTLayer::removeColumn(LayoutColumn* column) {
    if (!m_columns.contains(column)) return;

    m_columns.erase(column);
}

void DTLayer::onApplyLayoutChanges(CCObject*) {

    saveCurrentLayout();

    exitLayoutEditing();
}
void DTLayer::onDiscardLayoutChanges(CCObject*) {

    Save::setNewBestCustomizations(m_originalNewBestCustomization);
    Save::setSessionBestCustomizations(m_originalSessionBestCustomization);

    setLayoutBy(Save::getLayout());

    m_specialStrings["general"]->updateContent();
    m_specialStrings["s0"]->updateContent();

    exitLayoutEditing();
}

void DTLayer::exitLayoutEditing() {
    m_isEditingLayout = false;

    closeOptionsLayer();
    
    std::set<DTLabel*> visitedLabels{};

    for (const auto& column : m_columns)
    {
        column->setVisibility(false);

        for (const auto& [_, label] : column->m_labels)
        {
            if (visitedLabels.contains(label)) continue;

            visitedLabels.insert(label);
            label->setEditable(false);
        }
    }

    m_bottomLeftMenu->setEnabled(true);
    m_bottomLeftMenu->stopAllActions();
    m_bottomLeftMenu->runAction(CCFadeTo::create(.15f, 255));

    m_bottomRightMenu->setEnabled(true);
    m_bottomRightMenu->stopAllActions();
    m_bottomRightMenu->runAction(CCFadeTo::create(.15f, 255));

    m_buttonMenu->setEnabled(true);
    m_buttonMenu->stopAllActions();
    m_buttonMenu->runAction(CCFadeTo::create(.15f, 255));
    m_editLayoutBtnSpr->stopAllActions();
    m_editLayoutBtnSpr->runAction(CCFadeTo::create(.15f, 255));

    m_colorChangeBG->stopActionByTag(2);
    auto fa = CCFadeTo::create(.15f, 0);
    fa->setTag(2);
    m_colorChangeBG->runAction(fa);
    m_colorMenu->stopAllActions();
    m_colorMenu->runAction(CCFadeTo::create(.15f, 0));
    m_colorMenu->setEnabled(false);
    m_resetLayoutMenu->stopAllActions();
    m_resetLayoutMenu->runAction(CCFadeTo::create(.15f, 0));
    m_resetLayoutMenu->setEnabled(false);

    m_editLayoutMenu->setEnabled(false);
    m_layoutInfo->stopAllActions();
    m_layoutInfo->runAction(CCFadeTo::create(.15f, 0));
    m_layoutInfo->setEnabled(false);
    m_applyChangesButtonSpr->m_BGSprite->stopAllActions();
    m_applyChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 0));
    m_applyChangesButtonSpr->m_label->stopAllActions();
    m_applyChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 0));
    m_discardChangesButtonSpr->m_BGSprite->stopAllActions();
    m_discardChangesButtonSpr->m_BGSprite->runAction(CCFadeTo::create(.15f, 0));
    m_discardChangesButtonSpr->m_label->stopAllActions();
    m_discardChangesButtonSpr->m_label->runAction(CCFadeTo::create(.15f, 0));

    m_addColumnButton->stopAllActions();
    m_addColumnButton->runAction(CCFadeTo::create(.15f, 0));
    m_columnHolder->setEnabled(false);

    m_scrollLayer->moveBy(ccp(0, LayoutColumn::topHeight));

    m_presetList->close();

    this->organizeLayout();
}

void DTLayer::subscribeKeyListener(DTLabel* label) {
    if (m_keyListeners.contains(label)) return;

    m_keyListeners.insert(label);
}
void DTLayer::unsubscribeKeyListener(DTLabel* label) {
    if (!m_keyListeners.contains(label)) return;

    m_keyListeners.erase(label);
}

void DTLayer::modifyRun(double startPer, int amount, std::optional<int> sessionNumber) {
    if (m_MyLevelStats.isErr()) return;

    auto processRun = [&, amount, startPer, sessionNumber](ComplexDeathMap& data) -> bool {
        if (!data.adjust(startPer, std::nullopt, amount))
            return false;
        if (amount < 0 && !data.contains(startPer, std::nullopt))
            modifyNewBest(startPer, false, sessionNumber);
        return true;
    };

    if (sessionNumber.has_value()) {
        auto it = getCurrentGrouping().m_grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().m_grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.m_group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()) {
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.m_data.m_deaths.contains(startPer, std::nullopt)) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()) {
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        if (!processRun(session.value().m_data.m_deaths)) return;

        session.value().m_dataIsOverride = true;
        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else {
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.m_from0.isErr()) return;
        auto& from0Stats = stats.m_from0.unwrap();

        if (processRun(from0Stats.m_deaths)) {
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.m_levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        //log::info("pros linked");
        for (auto& linkedLevel : m_linkedLevelsData)
        {
            if (linkedLevel.m_from0.isErr() || linkedLevel.m_levelKey == stats.m_levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.m_from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.m_deaths)) {
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.m_levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
                return;
            }
        }
    }
}
void DTLayer::modifyRun(double startPer, double endPer, int amount, std::optional<int> sessionNumber) {
    if (m_MyLevelStats.isErr()) return;

    auto processRun = [&, amount, startPer, endPer](ComplexDeathMap& data) -> bool {
        return data.adjust(endPer, startPer, amount);
    };

    if (sessionNumber.has_value()) {
        auto it = getCurrentGrouping().m_grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().m_grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.m_group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()) {
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.m_data.m_runs.contains(endPer, startPer)) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()) {
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        if (!processRun(session.value().m_data.m_runs)) return;

        session.value().m_dataIsOverride = true;
        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else {
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.m_from0.isErr()) return;
        auto& from0Stats = stats.m_from0.unwrap();

        if (processRun(from0Stats.m_runs)) {
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.m_levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        for (auto& linkedLevel : m_linkedLevelsData)
        {
            if (linkedLevel.m_from0.isErr() || linkedLevel.m_levelKey == stats.m_levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.m_from0.unwrap();

            if (processRun(linkedLevelFrom0Stats.m_runs)) {
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.m_levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
                return;
            }
        }
    }
}

void DTLayer::modifyNewBest(double percent, bool makeTrue, std::optional<int> sessionNumber) {
    if (m_MyLevelStats.isErr()) return;

    auto parsedPercent = ComplexPercent::fromPercent(percent);
    if (parsedPercent.isErr()) {
        log::error("Failed to modify new best at invalid percent {}: {}", percent, parsedPercent.unwrapErr());
        return;
    }
    percent = parsedPercent.unwrap().asPercent();

    auto processBest = [&, percent, makeTrue](NewBests& bests) -> bool {
        if (makeTrue) {
            if (bests.contains(percent)) return true;

            bests.insert(percent);
        }
        else {
            if (!bests.contains(percent)) return true;

            bests.erase(percent);
        }

        return true;
    };

    if (sessionNumber.has_value()) {
        auto it = getCurrentGrouping().m_grouping.begin();
        std::advance(it, sessionNumber.value() - 1);

        if (it == getCurrentGrouping().m_grouping.end()) return;

        std::optional<Session> session = std::nullopt;
        std::string keyToUse;

        std::optional<Session> firstOKSess = std::nullopt;
        std::string firstOKSessLvlKey;

        for (const auto& [SDate, lvlKey] : it->second.m_group)
        {
            for (const auto& lvlKey : lvlKey)
            {
                auto sessionRes = StatsManager::getSession(lvlKey, SDate);
                if (sessionRes.isErr()) continue;
                auto currSess = sessionRes.unwrap();

                if (!firstOKSess.has_value()) {
                    firstOKSess = currSess;
                    firstOKSessLvlKey = lvlKey;
                }

                if (!currSess.m_data.m_newBests.contains(percent)) continue;

                session = currSess;
                keyToUse = lvlKey;

                break;
            }
        }

        if (!session.has_value()) {
            session = firstOKSess;
            keyToUse = firstOKSessLvlKey;
        }
        
        if (!session.has_value()) return;

        processBest(session.value().m_data.m_newBests);

        session.value().m_dataIsOverride = true;
        auto setSessionRes = StatsManager::setSession(session.value(), keyToUse, it->first, false);
        if (setSessionRes.isErr()) log::error("{}", setSessionRes.unwrapErr());
    }
    else {
        auto& stats = m_MyLevelStats.unwrap();
        if (stats.m_from0.isErr()) return;
        auto& from0Stats = stats.m_from0.unwrap();

        if (processBest(from0Stats.m_newBests)) {
            auto setGeneralRes = StatsManager::setGeneral(from0Stats, stats.m_levelKey);
            if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
            return;
        }

        for (auto& linkedLevel : m_linkedLevelsData)
        {
            if (linkedLevel.m_from0.isErr() || linkedLevel.m_levelKey == stats.m_levelKey) continue;
            auto& linkedLevelFrom0Stats = linkedLevel.m_from0.unwrap();

            if (processBest(linkedLevelFrom0Stats.m_newBests)) {
                auto setGeneralRes = StatsManager::setGeneral(linkedLevelFrom0Stats, linkedLevel.m_levelKey);
                if (setGeneralRes.isErr()) log::error("{}", setGeneralRes.unwrapErr());
                return;
            }
        }
    }
}

bool DTLayer::DeleteSave() {
    auto lvlRes = StatsManager::getLevelKey(m_Level);
    if (lvlRes.isErr()) return false;

    auto deleteRes = StatsManager::deleteLevelStats(lvlRes.unwrap());
    if (deleteRes.isErr()) {
        log::error("{}", deleteRes.unwrapErr());
        return false;
    }

    SaveDeletionEvent().send();

    onClose(nullptr);
    return true;
}

UpdateFuture DTLayer::onNLKey(std::map<std::string, std::any> payload) {
    co_return Ok(std::string("\n"));
}
UpdateFuture DTLayer::onATTKey(std::map<std::string, std::any> payload) {
    long long totalAttempts = 0;
    for (const auto& lebel : m_linkedLevelsData)
        totalAttempts += lebel.m_metadata.m_attempts;

    co_return Ok(std::to_string(totalAttempts));
}
UpdateFuture DTLayer::onLVLNKey(std::map<std::string, std::any> payload) {
    co_return Ok(std::string(m_Level->m_levelName));
}
UpdateFuture DTLayer::onGeneralKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto combinedDeaths, co_await getTFor<std::pair<Deaths, NewBests>>([](GeneralData const& data) {
        return std::make_pair(data.m_deaths.toLegacyDeaths(), data.m_newBests);
    },
    [](auto const& a, auto const& b) {
        auto map = a.first;
        StatsManager::mergeMapsAdd(map, b.first);

        auto nbs = a.second;
        nbs.insert(b.second.begin(), b.second.end());

        return std::make_pair(map, nbs);
    }, false));

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(combinedDeaths.first, Save::getFrom0Customazations(), out, combinedDeaths.second, Save::getNewBestCustomizations(), ignoreExtraSettings, false))
        co_return Err("Failed to create from0 deaths string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onRUNSKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto sharedRuns, co_await getTFor<Deaths>([](GeneralData const& data) {
        return data.m_runs.toLegacyDeaths();
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(sharedRuns, Save::getRunsCustomazations(), out, std::nullopt, CustomizationData{.m_color = to4B({255, 255, 255})}, ignoreExtraSettings, false))
        co_return Err("Failed to create run deaths string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onCheatedGeneralKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto cheatedFrom0Runs, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto cheatedRuns = data.m_runs.filterOnlyCheatedRuns().toLegacyDeaths(true);
        Deaths from0Runs;
        for (const auto& [key, count] : cheatedRuns) {
            auto run = StatsManager::splitRunKey(key);
            if (run.isErr() || run.unwrap().m_start != 0)
                continue;
            from0Runs[std::to_string(run.unwrap().m_end)] = count;
        }
        return from0Runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    bool ignoreExtraSettings = false;
    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(cheatedFrom0Runs, Save::getFrom0Customazations(), out, std::nullopt, Save::getNewBestCustomizations(), ignoreExtraSettings, false))
        co_return Err("Failed to create cheated from0 runs string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onCheatedRunsKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto cheatedRuns, co_await getTFor<Deaths>([](GeneralData const& data) {
        return data.m_runs.filterOnlyCheatedRuns().toLegacyDeaths(true);
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    bool ignoreExtraSettings = false;
    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(cheatedRuns, Save::getRunsCustomazations(), out, std::nullopt, CustomizationData{.m_color = to4B({255, 255, 255})}, ignoreExtraSettings, false))
        co_return Err("Failed to create cheated runs string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onCheatedAttemptsKey(std::map<std::string, std::any> payload) {
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.m_runs.filterOnlyCheatedRuns().toLegacyDeaths(true));
    }, false);
}

UpdateFuture DTLayer::onS0Key(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto sessioNDeaths, co_await getTFor<std::pair<Deaths, NewBests>>([](GeneralData const& data) {
        return std::make_pair(data.m_deaths.toLegacyDeaths(), data.m_newBests);
    },
    NULL, true));
    
    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(sessioNDeaths.first, Save::getSessionF0Customazations(), out, sessioNDeaths.second, Save::getSessionBestCustomizations(), ignoreExtraSettings, true))
        co_return Err("Failed to create session from0 deaths string");

    co_return Ok(out);
}
UpdateFuture DTLayer::onSRUNSKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto sruns, co_await getTFor<Deaths>([](GeneralData const& data) {
        return data.m_runs.toLegacyDeaths();
    },
    NULL, true));

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(sruns, Save::getRunsCustomazations(), out, std::nullopt, CustomizationData{.m_color = to4B({255, 255, 255})}, ignoreExtraSettings, true))
        co_return Err("Failed to create session run deaths string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onDTATTKey(std::map<std::string, std::any> payload) {
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.m_deaths.toLegacyDeaths()) + deathsFunc(data.m_runs.toLegacyDeaths());
    }, false);
}

UpdateFuture DTLayer::onDTF0ATTKey(std::map<std::string, std::any> payload) {
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.m_deaths.toLegacyDeaths());
    }, false);
}

UpdateFuture DTLayer::onDTRunsATTKey(std::map<std::string, std::any> payload) {
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.m_runs.toLegacyDeaths());
    }, false);
}

UpdateFuture DTLayer::onSAttKey(std::map<std::string, std::any> payload) {
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.m_deaths.toLegacyDeaths()) + deathsFunc(data.m_runs.toLegacyDeaths());
    }, true);
}

UpdateFuture DTLayer::onSF0AttKey(std::map<std::string, std::any> payload) {
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.m_deaths.toLegacyDeaths());
    }, true);
}
UpdateFuture DTLayer::onSRunsAttKey(std::map<std::string, std::any> payload) {
    co_return co_await getAttemptsFor([](GeneralData const& data, auto deathsFunc) {
        return deathsFunc(data.m_runs.toLegacyDeaths());
    }, true);
}

long long DTLayer::calcPlaytime(const Deaths& deaths) {
    long double playtime = 0;

    // auto __calc_start_ms = timeInMs();

    float wt;
    if (!m_cachedLevelLength.has_value()) {
        wt = m_Level->m_timestamp
            ? m_Level->m_timestamp / 240
            : m_Level->isPlatformer() ? 0 : std::ceil(timeForLevelString(m_Level->m_levelString));

        m_cachedLevelLength = wt;
    }
    else {
        wt = m_cachedLevelLength.value();
    }

    for (const auto& death : deaths)
    {
        auto runSplitRes = StatsManager::splitRunKey(death.first);
        if (runSplitRes.isErr()) continue;

        float runLength;
        if (runSplitRes.unwrap().m_start == std::nullopt)
            runLength = runSplitRes.unwrap().m_end;
        else
            runLength = (runSplitRes.unwrap().m_end - runSplitRes.unwrap().m_start.value());
        // log::info("{} | {} | {}", runLength, runSplitRes.unwrap().m_end, runSplitRes.unwrap().m_start);
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

UpdateFuture DTLayer::onAPTALLSKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    auto pt = StatsManager::workingTime(calcPlaytime(deaths));
    co_return Ok(pt);
}

UpdateFuture DTLayer::onAPTF0SKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        return data.m_deaths.toLegacyDeaths();
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTRUNSKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        return data.m_runs.toLegacyDeaths();
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}

UpdateFuture DTLayer::onAPTSALLSKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    NULL, true));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTSF0Key(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        return data.m_deaths.toLegacyDeaths();
    },
    NULL, true));

    co_return Ok(StatsManager::workingTime(calcPlaytime(deaths)));
}
UpdateFuture DTLayer::onAPTSRUNSKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto runs, co_await getTFor<Deaths>([](GeneralData const& data) {
        return data.m_runs.toLegacyDeaths();
    },
    NULL, true));

    co_return Ok(StatsManager::workingTime(calcPlaytime(runs)));
}

Result<Session, UpdateFutureError> DTLayer::loadSessionFromSave(std::optional<int> sessionIndex) {
    if (!getCurrentGrouping().m_grouping.size()) return Err(UpdateFutureError("No sessions saved!", false));
    int i = sessionIndex.has_value() ? sessionIndex.value() : m_sessionSelector->getCurrentCount();

    if (i == 0 || i > getCurrentGrouping().m_grouping.size())
        return Err("Failed to get session, not in range");

    auto it = getCurrentGrouping().m_grouping.begin();
    std::advance(it, i - 1);

    Result<Session, UpdateFutureError> sess = Err("");

    for (const auto& [SDate, lvlKeys] : it->second.m_group)
    {
        for (const auto& lvlKey : lvlKeys)
        {
            auto sessionRes = StatsManager::getSession(lvlKey, SDate);
            if (sessionRes.isErr()) {
                sess = Err(sessionRes.unwrapErr().m_error);
                break;
            }

            if (sess.isErr()) {
                sess = Ok(sessionRes.unwrap());
            }
            else {
                auto& overallRef = sess.unwrap();

                overallRef.m_data += sessionRes.unwrap().m_data;
            }
        }
    }

    if (sess.isOk()) {
        auto& sessRef = sess.unwrap();
        sessRef.m_groupID = it->first;
    }
    
    return sess;
}

UpdateFuture DTLayer::onRunsTo100Key(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
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

        if (splitRun.m_end != 100) continue;

        to100Deaths.insert(death);
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(to100Deaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings, false))
        co_return Err("Failed to create runs to 100 string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onBestRunsKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
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

        auto realStart = splitRun.m_start.value_or(-1);

        if (!bestRuns.contains(realStart))
            bestRuns.insert({realStart, splitRun.m_end});
        else if (bestRuns[realStart] < splitRun.m_end) {
            bestRuns[realStart] = splitRun.m_end;
        }
    }

    Deaths bestRunDeaths{};

    for (const auto& [bestRunStart, bestRunEnd] : bestRuns)
    {
        co_await arc::yield();
        auto runStringRes = StatsManager::createRunKey(Run{
            bestRunStart == -1
                ? std::optional<double>{}
                : std::optional<double>{static_cast<double>(bestRunStart)},
            static_cast<double>(bestRunEnd)
        });
        if (runStringRes.isErr()) continue;
        auto runString = runStringRes.unwrap();
        if (!deaths.contains(runString)) continue;

        bestRunDeaths.insert({runString, deaths[runString]});
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(bestRunDeaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings, false))
        co_return Err("Failed best runs string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onSessionRunsTo100Key(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
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

        if (splitRun.m_end != 100) continue;

        to100Deaths.insert(death);
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(to100Deaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings, true))
        co_return Err("Failed to create session runs to 100 string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onSessionBestRunsKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
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

        auto realStart = splitRun.m_start.value_or(-1);

        if (!bestRuns.contains(realStart))
            bestRuns.insert({realStart, splitRun.m_end});
        else if (bestRuns[realStart] < splitRun.m_end) {
            bestRuns[realStart] = splitRun.m_end;
        }
    }

    Deaths bestRunDeaths{};

    for (const auto& [bestRunStart, bestRunEnd] : bestRuns)
    {
        co_await arc::yield();
        auto runStringRes = StatsManager::createRunKey(Run{
            std::optional<double>{static_cast<double>(bestRunStart)},
            static_cast<double>(bestRunEnd)
        });
        if (runStringRes.isErr()) continue;
        auto runString = runStringRes.unwrap();
        if (!deaths.contains(runString)) continue;

        bestRunDeaths.insert({runString, deaths[runString]});
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDeathsString(bestRunDeaths, Save::getRunsCustomazations(), out, std::nullopt, {255, 255, 255}, ignoreExtraSettings, true))
        co_return Err("Failed best session runs string");

    co_return Ok(out);
}

void DTLayer::foreachLinkedLevel(Function<void(LevelData&)> onLevelVisit) {
    if (m_MyLevelStats.isErr()) return;
    auto& myStats = m_MyLevelStats.unwrap();

    for (auto& levelData : m_linkedLevelsData)
    {
        if (levelData.m_levelKey == myStats.m_levelKey) continue;
        onLevelVisit(levelData);
    }
}

UpdateFuture DTLayer::onPTALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeGeneral.m_playtimeF0 + data.m_playtimeGeneral.m_playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPTF0SKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeGeneral.m_playtimeF0;
    }, false);
}

// same thing here, no distinction between playtime from 0 and playtime from runs
UpdateFuture DTLayer::onPTRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeGeneral.m_playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPTSALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeGeneral.m_playtimeF0 + data.m_playtimeGeneral.m_playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onPTSF0Key(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeGeneral.m_playtimeF0;
    }, true);
}

UpdateFuture DTLayer::onPTSRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeGeneral.m_playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onDeadPTALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeDead.m_playtimeF0 + data.m_playtimeDead.m_playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onDeadPTF0SKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeDead.m_playtimeF0;
    }, false);
}

// same thing here, no distinction between playtime from 0 and playtime from runs
UpdateFuture DTLayer::onDeadPTRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeDead.m_playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onDeadPTSALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeDead.m_playtimeF0 + data.m_playtimeDead.m_playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onDeadPTSF0Key(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeDead.m_playtimeF0;
    }, true);
}

UpdateFuture DTLayer::onDeadPTSRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimeDead.m_playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onPausedPTALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimePaused.m_playtimeF0 + data.m_playtimePaused.m_playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPausedPTF0SKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimePaused.m_playtimeF0;
    }, false);
}

// same thing here, no distinction between playtime from 0 and playtime from runs
UpdateFuture DTLayer::onPausedPTRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimePaused.m_playtimeRuns;
    }, false);
}

UpdateFuture DTLayer::onPausedPTSALLSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimePaused.m_playtimeF0 + data.m_playtimePaused.m_playtimeRuns;
    }, true);
}

UpdateFuture DTLayer::onPausedPTSF0Key(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimePaused.m_playtimeF0;
    }, true);
}

UpdateFuture DTLayer::onPausedPTSRUNSKey(std::map<std::string, std::any> payload) {
    co_return co_await getPlaytimeFor([](GeneralData const& data) {
        return data.m_playtimePaused.m_playtimeRuns;
    }, true);
}


UpdateFuture DTLayer::onSectionKey(std::map<std::string, std::any> payload) {
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();

    std::vector<Section> validSections{};
    for (const auto& section : myStats.m_metadata.m_sections)
    {
        if (!section.isValid()) continue;

        validSections.push_back(section);
    }
    
    if (validSections.size() <= 1) co_return Err(UpdateFutureError("No sections specified!", false));

    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    std::unordered_map<std::string, int> deathsPerSection{};

    auto CreateSectioIDForSectionPair = [&](const Run& splitDeath, const std::optional<Section>& startingSection, int deaths) {
        Section endingSection;
        for (const auto& section : validSections)
        {
            if (!section.isPercentInSection(splitDeath.m_end)) continue;

            endingSection = section;
            break;
        }

        std::string sectionID = "";
        if (!startingSection.has_value()) {
            sectionID = fmt::format("{}", endingSection.m_name);
        }
        else
            sectionID = fmt::format("{}-{}", startingSection.value().m_name, endingSection.m_name);
        if (!deathsPerSection.contains(sectionID))
            deathsPerSection.insert({sectionID, deaths});
        else
            deathsPerSection[sectionID] += deaths;
    };

    for (const auto& death : deaths) {
        auto splitDeathRes = StatsManager::splitRunKey(death.first);
        if (splitDeathRes.isErr()) continue;
        auto splitDeath = splitDeathRes.unwrap();

        if (splitDeath.m_start == std::nullopt) {
            CreateSectioIDForSectionPair(splitDeath, std::nullopt, death.second);
            continue;
        }
        
        std::vector<Section> startingSectionsForDeath{};

        for (const auto& section : validSections)
        {
            if (!section.isPercentInSection(splitDeath.m_start.value_or(-1))) continue;

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
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return validSections[a].m_startPercent < validSections[b].m_startPercent;
    });

    auto custom = Save::getFrom0Customazations();

    int indexofyeah = 0;

    for (const auto& startIdx : order)
    {
        for (const auto& endIdx : order)
        {
            auto sectionID = (startIdx == endIdx) ? validSections[startIdx].m_name : fmt::format("{}-{}", validSections[startIdx].m_name, validSections[endIdx].m_name);
            auto it = deathsPerSection.find(sectionID);
            if (it == deathsPerSection.end()) continue;

            auto format = custom.m_format;
            format = std::regex_replace(format, std::regex("\\{per\\}"), sectionID);
            format = std::regex_replace(format, std::regex("\\{d\\}"), std::to_string(it->second));

            std::string keysStart = "";
            std::string keysEnd = "";

            if (indexofyeah % 2 == 0) {
                keysEnd = "</cplus>";
                keysStart = fmt::format("<cplus={}>", custom.m_alternateStrength);
            }

            out += fmt::format("{}{}{}{}", keysStart, format, custom.m_seperator, keysEnd);

            indexofyeah++;
        }

        if (startIdx != order.back())
            out += "----------{nl}";
    }

    if (!out.empty())
        out.erase(out.length() - custom.m_seperator.length());

    co_return Ok(out);
}

void DTLayer::onCalculator(CCObject*) {
    CalculatorPopup::create()->show();
}

UpdateFuture DTLayer::onLevelRunsKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    constexpr int max = 4;
    std::vector<Run> validRuns;

    for (auto const& [key, count] : deaths) {
        auto res = StatsManager::splitRunKey(key);
        if (res.isOk()) {
            Run run = res.unwrap();
            auto runStartReal = run.m_start.value_or(-1);
            if (runStartReal <= -1) runStartReal = 0;
            if (count > 0 && run.m_end > runStartReal) {
                validRuns.push_back(run);
            }
        }
        co_await arc::yield();
    }

    std::sort(validRuns.begin(), validRuns.end(), [](const Run& a, const Run& b) {
        return std::tie(a.m_start, a.m_end) < std::tie(b.m_start, b.m_end);
    });
    validRuns.erase(std::unique(validRuns.begin(), validRuns.end(), [](const Run& a, const Run& b) {
        return a.m_start == b.m_start && a.m_end == b.m_end;
    }), validRuns.end());

    std::map<int, std::array<uint64_t, max + 1>> dp;
    dp[0][0] = 1;

    for (int currentPercent = 0; currentPercent < 100; ++currentPercent) {
        if (dp.find(currentPercent) == dp.end()) continue;

        for (const auto& run : validRuns) {
            if (run.m_start <= currentPercent && run.m_end > currentPercent) {
                int target = std::min(static_cast<int>(run.m_end), 100);
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

UpdateFuture DTLayer::onRunAttemptsKey(std::map<std::string, std::any> payload) {
    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();

    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<Deaths>([](GeneralData const& data) {
        auto runs = data.m_runs.toLegacyDeaths();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    std::map<int, int> percentAttempts{};

    for (const auto& death : deaths)
    {
        co_await arc::yield();
        auto splitRunRes = StatsManager::splitRunKey(death.first);
        if (splitRunRes.isErr()) continue;
        auto splitRun = splitRunRes.unwrap();

        auto realStart = splitRun.m_start.value_or(0);

        if (!percentAttempts.contains(realStart))
            percentAttempts.insert({realStart, death.second});
        else {
            percentAttempts[realStart] += death.second;
        }
    }

    std::string out;

    auto custom = Save::getRunsCustomazations();

    int indexofyeah = 0;

    for (const auto& [percent, attempts] : percentAttempts)
    {
        if (!myStats.m_metadata.m_showAnyRun && !myStats.m_metadata.m_runsToShow.empty() && !myStats.m_metadata.m_runsToShow.contains(percent))
            continue;

        auto str = fmt::format("{}% - {}", percent, attempts);

        std::string keysStart = "";
        std::string keysEnd = "";

        if (indexofyeah % 2 == 0) {
            keysEnd = "</cplus>";
            keysStart = fmt::format("<cplus={}>", custom.m_alternateStrength);
        }

        out += fmt::format("{}{}{}{}", keysStart, str, custom.m_seperator, keysEnd);

        indexofyeah++;
        
        out += "----------{nl}";
    }

    if (!out.empty())
        out.erase(out.length() - custom.m_seperator.length());

    co_return Ok(out);
}

UpdateFuture DTLayer::onDynamicDecimalKey(std::map<std::string, std::any> payload) {
    auto decimalDepth = payload.find("decimalDepth");
    if (decimalDepth == payload.end())
        co_return Err("Missing decimalDepth");

    auto decimalDepthValue = std::any_cast<uint8_t>(&decimalDepth->second);
    if (!decimalDepthValue)
        co_return Err("Invalid decimalDepth");

    auto decimalRun = payload.find("decimalRun");
    if (decimalRun == payload.end())
        co_return Err("Missing decimalRun");

    auto decimalRunValue = std::any_cast<Run>(&decimalRun->second);
    if (!decimalRunValue)
        co_return Err("Invalid decimalRun");

    auto decimalIsSess = payload.find("decimalIsSess");
    if (decimalIsSess == payload.end())
        co_return Err("Missing decimalIsSess");

    auto decimalIsSessValue = std::any_cast<bool>(&decimalIsSess->second);
    if (!decimalIsSessValue)
        co_return Err("Invalid decimalIsSess");

    if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate runs playtime");
    auto myStats = m_MyLevelStats.unwrap();

    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<std::pair<ComplexDeathMap, NewBests>>([](GeneralData const& data) {
        auto runs = data.m_runs;
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return std::make_pair(runs, data.m_newBests);
    },
    [](auto const& a, auto const& b) {
        auto map = a.first;
        StatsManager::mergeMapsAdd(map, b.first);

        auto nbs = a.second;
        nbs.insert(b.second.begin(), b.second.end());

        return std::make_pair(map, nbs);
    }, *decimalIsSessValue));

    auto occurrenceTimestamp = payload.find("decimalOccurrenceTimestamp");
    auto occurrenceIndex = payload.find("decimalOccurrenceIndex");
    if (occurrenceTimestamp != payload.end() || occurrenceIndex != payload.end()) {
        if (occurrenceTimestamp == payload.end() || occurrenceIndex == payload.end())
            co_return Err("Incomplete recent death selection");

        auto timestampValue = std::any_cast<long long>(&occurrenceTimestamp->second);
        auto indexValue = std::any_cast<size_t>(&occurrenceIndex->second);
        if (!timestampValue || !indexValue)
            co_return Err("Invalid recent death selection");

        auto endPercent = ComplexPercent::fromPercent(decimalRunValue->m_end);
        if (endPercent.isErr())
            co_return Err("Invalid recent death end percent");

        std::optional<ComplexPercent::Units> startPercent;
        if (decimalRunValue->m_start) {
            auto parsedStart = ComplexPercent::fromPercent(*decimalRunValue->m_start);
            if (parsedStart.isErr())
                co_return Err("Invalid recent death start percent");
            startPercent = parsedStart.unwrap().m_units;
        }

        ComplexDeathMap::Run selectedRun{endPercent.unwrap().m_units, startPercent};
        auto deathIt = deaths.first.m_records.find(selectedRun);
        if (deathIt == deaths.first.m_records.end())
            co_return Err("Selected recent death no longer exists");

        size_t matchingIndex = 0;
        std::optional<ComplexDeathOccurrence> selectedOccurrence;
        for (const auto& occurrence : deathIt->second.m_occurrences) {
            if (!occurrence.m_runEndTimestamp || occurrence.m_runEndTimestamp.value() != *timestampValue)
                continue;
            if (matchingIndex++ == *indexValue) {
                selectedOccurrence = occurrence;
                break;
            }
        }
        if (!selectedOccurrence)
            co_return Err("Selected recent death occurrence no longer exists");

        auto selectedDeath = deathIt->second;
        selectedDeath.m_deaths = 1;
        selectedDeath.m_occurrences = {*selectedOccurrence};

        ComplexDeathMap selectedDeaths;
        selectedDeaths.m_records.emplace(selectedRun, std::move(selectedDeath));
        deaths.first = std::move(selectedDeaths);
    }

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    std::string out;
    if (!createDecimalDeathsString(deaths.first, *decimalRunValue, myStats.m_metadata, Save::getFrom0Customazations(), out, deaths.second, *decimalIsSessValue ? Save::getSessionBestCustomizations() : Save::getNewBestCustomizations(), ignoreExtraSettings, *decimalDepthValue))
        co_return Err("Failed best session runs string");

    co_return Ok(out);
}

UpdateFuture DTLayer::onRecentDeathsKey(std::map<std::string, std::any> payload) {
    GEODE_CO_UNWRAP_INTO(auto deaths, co_await getTFor<ComplexDeathMap>([](GeneralData const& data) {
        auto runs = data.m_runs.filterCheatedRuns();
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, false));

    struct RecentDeathOccurrence {
        long long m_timestamp;
        ComplexDeathMap::Run m_run;
        ComplexDeathOccurrence m_occurrence;
        size_t m_occurrenceIndex;
    };
    std::vector<RecentDeathOccurrence> recentOccurrences;
    std::map<std::pair<ComplexDeathMap::Run, long long>, size_t> occurrenceIndices;

    for (const auto& [run, death] : deaths.m_records) {
        for (const auto& occurrence : death.m_occurrences) {
            if (!occurrence.m_runEndTimestamp.has_value()) continue;

            auto timestamp = occurrence.m_runEndTimestamp.value();
            auto& occurrenceIndex = occurrenceIndices[{run, timestamp}];
            recentOccurrences.push_back({timestamp, run, occurrence, occurrenceIndex++});
        }
    }

    std::stable_sort(recentOccurrences.begin(), recentOccurrences.end(), [](const auto& a, const auto& b) {
        return a.m_timestamp > b.m_timestamp;
    });

    bool ignoreExtraSettings = false;

    if (payload.contains("ignoreExtraSettings")) {
        if (auto ignoreExtraSettingsTemp = std::any_cast<bool>(&payload["ignoreExtraSettings"]))
            ignoreExtraSettings = *ignoreExtraSettingsTemp;
    }

    if (m_MyLevelStats.isErr())
        co_return Err("Failed to calculate recent deaths");
    auto myStats = m_MyLevelStats.unwrap();
    auto custom = Save::getRunsCustomazations();

    std::string out;
    int hiddenDeaths = 0;
    size_t shownDeaths = 0;

    constexpr size_t maxRecentDeaths = 10;
    auto count = std::min(recentOccurrences.size(), maxRecentDeaths);

    for (size_t i = 0; i < count; ++i) {
        auto& recent = recentOccurrences[i];
        Run run;
        run.m_start = recent.m_run.second ? std::optional<double>(ComplexPercent{recent.m_run.second.value()}.asPercent()) : std::nullopt;
        run.m_end = ComplexPercent{recent.m_run.first}.asPercent();

        if (!ignoreExtraSettings) {
            if (run.m_start) {
                int startValue = static_cast<int>(run.m_start.value());
                if (!myStats.m_metadata.m_showAnyRun && !myStats.m_metadata.m_runsToShow.empty()) {
                    auto it = myStats.m_metadata.m_runsToShow.find(startValue);
                    if (it == myStats.m_metadata.m_runsToShow.end() || it->second > run.m_end) {
                        hiddenDeaths++;
                        continue;
                    }
                }
                else if (myStats.m_metadata.m_sharedRunToShow > run.m_end - startValue) {
                    hiddenDeaths++;
                    continue;
                }
            }
            else if (myStats.m_metadata.m_hideUpto > run.m_end && run.m_end >= 0) {
                hiddenDeaths++;
                continue;
            }
        }

        auto runKey = StatsManager::createRunKey(run);
        if (runKey.isErr())
            co_return Err("Failed to create recent death run key");

        auto percentText = run.m_start
            ? fmt::format("{}-{}", static_cast<int>(run.m_start.value()), static_cast<int>(run.m_end))
            : fmt::format("{}", static_cast<int>(run.m_end));
        auto format = fmt::format("{}%", percentText);

        std::string colorOpen;
        std::string colorClose;
        if (shownDeaths % 2 == 0) {
            colorOpen = fmt::format("<cplus={}>", custom.m_alternateStrength);
            colorClose = "</cplus>";
        }

        auto decimalButton = fmt::format(
            " <runMoreBtn={},{},{},{}>C</runMoreBtn>",
            runKey.unwrap(),
            false,
            recent.m_timestamp,
            recent.m_occurrenceIndex
        );
        out += fmt::format("{}{}{}{}{}", colorOpen, format, decimalButton, colorClose, custom.m_seperator);
        shownDeaths++;
    }

    if (out.empty() && hiddenDeaths == 0) {
        out = "No Deaths Found!";
    }
    else {
        if (!custom.m_seperator.empty() && out.size() >= custom.m_seperator.size())
            out.erase(out.size() - custom.m_seperator.size());
    }

    if (hiddenDeaths != 0) {
        if (!out.empty())
            out += "{nl}";
        out += fmt::format("{}x hidden", hiddenDeaths);
    }

    co_return Ok(out);
}

UpdateFuture DTLayer::getPlaytimeFor(Function<uint64_t(GeneralData const&)>&& dataGetter, bool session) {
    auto gotten = co_await getTFor<uint64_t>(std::move(dataGetter), [](uint64_t const& a, uint64_t const& b) {
        return a + b;
    }, session);

    if (gotten.isErr()) co_return Err(gotten.unwrapErr());
    co_return Ok(StatsManager::workingTime(gotten.unwrap()));
}

UpdateFuture DTLayer::getAttemptsFor(Function<unsigned long long(GeneralData const&, geode::FunctionRef<unsigned long long(const Deaths&)> const&)>&& dataGetter, bool session) {
    auto deaths = [](const Deaths& d) -> unsigned long long {
        unsigned long long attempts = 0;

        for (const auto& [_, count] : d) {
            attempts += count;
        }

        return attempts;
    };

    GEODE_CO_UNWRAP_INTO(auto attempts, co_await getTFor<unsigned long long>([&deaths, &dataGetter](GeneralData const& data) {
        return dataGetter(data, deaths);
    },
    [](auto a, auto b) {
        return a + b;
    }, session));

    co_return Ok(std::to_string(attempts));
}

SessionCategory& DTLayer::getCurrentGrouping() {
    if (m_MyLevelStats.isErr()) return m_sessionsOrder;
    auto& myStats = m_MyLevelStats.unwrap();

    if (m_currentGrouping == -3) return m_daySGroup;
    if (m_currentGrouping == -2) return m_weekSGroup;
    if (m_currentGrouping == -1) return m_monthSGroup;

    if (!myStats.m_metadata.m_sessionGroups.size() || m_currentGrouping < 0 || m_currentGrouping >= myStats.m_metadata.m_sessionGroups.size()) return m_sessionsOrder;

    return myStats.m_metadata.m_sessionGroups[m_currentGrouping];
}

UpdateFuture DTLayer::onSessionDateKey(std::map<std::string, std::any> payload) {
    auto sessionRes = loadSessionFromSave();
    if (sessionRes.isErr()) co_return Err(sessionRes.unwrapErr());

    auto session = sessionRes.unwrap();

    co_await arc::yield();

    time_t time = static_cast<time_t>(session.m_groupID);

    std::tm tp{};
    #if defined(_WIN32)
        localtime_s(&tp, &time);
    #else
        localtime_r(&time, &tp);
    #endif

    auto dateStr = DateFormatter::timeFormat(tp);

    co_return Ok(dateStr);
}

void DTLayer::onGroups(CCObject*) {
    if (m_groupsList->isOpened())
        m_groupsList->close();
    else
        m_groupsList->open();
}

void DTLayer::onGroupSelected(int const& id) {
    m_groupsList->close();

    m_currentGrouping = id;
    m_specialStrings["s0"]->updateContent();
    m_specialStrings["sruns"]->updateContent();

    auto opt = m_groupsList->getItemForID(id);

    auto newSpr = ButtonSprite::create(
        opt.value().m_text.c_str(),
        100,
        100,
        1,
        false,
        opt.value().m_font.c_str(),
        opt.value().m_BGTexture.c_str()
    );
    newSpr->setCascadeOpacityEnabled(true);
    newSpr->setScale(.45f);

    m_groupsBtn->setSprite(newSpr);

    m_bottomLeftMenu->updateLayout();
    m_bottomRightMenu->updateLayout();

    m_sessionSelector->setMaximumCount(getCurrentGrouping().m_grouping.size(), true);
}

void DTLayer::CleanGetStats() {
    m_MyLevelStats = StatsManager::getLevelData(m_Level);
    if (m_MyLevelStats.isErr() && m_MyLevelStats.unwrapErr().m_code == 1) {
        LevelData newData;
        newData.m_levelKey = StatsManager::getLevelKey(m_Level).unwrap();
        m_MyLevelStats = Ok(newData);
    }
    else if (m_MyLevelStats.isErr()) {
        auto notif = geode::Notification::create(fmt::format("Failed to load DT level data! {}", m_MyLevelStats.unwrapErr().m_error), NotificationIcon::Error, 3);
        notif->show();
        notif->setZOrder(101);
    }

    StatsManager::transferPlaytimeFromPT(m_MyLevelStats, m_Level);

    if (m_MyLevelStats.isOk()) {
        auto stats = m_MyLevelStats.unwrap();
        stats.m_metadata.m_levelName = m_Level->m_levelName;
        stats.m_metadata.m_attempts = m_Level->m_attempts;
        stats.m_metadata.m_difficulty = StatsManager::getDifficulty(m_Level);
        (void)StatsManager::setMetadata(stats.m_metadata, stats.m_levelKey);
        m_MyLevelStats = Ok(stats);
    }

    StatsManager::setCurrentLevel(m_Level);

    DTLayer::UpdateSharedStats();
}

void DTLayer::onNewBestColor(CCObject* sender) {
    auto popup = DTColorPickerPopup::create(Save::getNewBestCustomizations());
    popup->setCallback([&](auto color) {
        auto current = Save::getNewBestCustomizations();
        current.m_color = color;

        Save::setNewBestCustomizations(current);

        m_specialStrings["general"]->updateContent();
    });
    popup->setGradientChangedCallback([&](GradientData const& data) {
        auto current = Save::getNewBestCustomizations();
        current.m_gradient = data;

        Save::setNewBestCustomizations(current);

        m_specialStrings["general"]->updateContent();
    });
    popup->setGradientEnabledChangedCallback([&](bool enabled) {
        auto current = Save::getNewBestCustomizations();
        current.m_gradientEnabled = enabled;

        Save::setNewBestCustomizations(current);

        m_specialStrings["general"]->updateContent();
    });
    popup->setWaveChangedCallback([&](WaveData const& data) {
        auto current = Save::getNewBestCustomizations();
        current.m_wave = data;

        Save::setNewBestCustomizations(current);

        m_specialStrings["general"]->updateContent();
    });
    popup->setWaveEnabledChangedCallback([&](bool enabled) {
        auto current = Save::getNewBestCustomizations();
        current.m_waveEnabled = enabled;

        Save::setNewBestCustomizations(current);

        m_specialStrings["general"]->updateContent();
    });
    popup->setColorTarget(m_newBestColorBtnSpr);
    popup->show();
}
void DTLayer::onSessionBestColor(CCObject* sender) {
    auto popup = DTColorPickerPopup::create(Save::getSessionBestCustomizations());
    popup->setCallback([&](auto color) {
        auto current = Save::getSessionBestCustomizations();
        current.m_color = color;

        Save::setSessionBestCustomizations(current);

        m_specialStrings["s0"]->updateContent();
    });
    popup->setGradientChangedCallback([&](GradientData const& data) {
        auto current = Save::getSessionBestCustomizations();
        current.m_gradient = data;

        Save::setSessionBestCustomizations(current);

        m_specialStrings["s0"]->updateContent();
    });
    popup->setGradientEnabledChangedCallback([&](bool enabled) {
        auto current = Save::getSessionBestCustomizations();
        current.m_gradientEnabled = enabled;

        Save::setSessionBestCustomizations(current);

        m_specialStrings["s0"]->updateContent();
    });
    popup->setWaveChangedCallback([&](WaveData const& data) {
        auto current = Save::getSessionBestCustomizations();
        current.m_wave = data;

        Save::setSessionBestCustomizations(current);

        m_specialStrings["s0"]->updateContent();
    });
    popup->setWaveEnabledChangedCallback([&](bool enabled) {
        auto current = Save::getSessionBestCustomizations();
        current.m_waveEnabled = enabled;

        Save::setSessionBestCustomizations(current);

        m_specialStrings["s0"]->updateContent();
    });
    popup->setColorTarget(m_sessionBestColorBtnSpr);
    popup->show();
}

void DTLayer::onResetLayout(CCObject*) {
    FLAlertLayer::create(this, "Warning", "This will reset the layout back to the default layout!\nare you sure you want to do this?", "CANCEL", "OK")->show();
}

void DTLayer::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;

    resetLayoutTo(DTLayoutPreset{
        .m_name = "Default",
        .m_layout = Save::getDefaultLayout(),
        .m_newBestCustomization= Save::getDefaultNewBestCustomizations(),
        .m_sessionBestCustomization = Save::getDefaultSessionBestCustomizations()
    });
}


void DTLayer::onLayoutPresets(CCObject*) {
    if (m_presetList->isOpened())
        m_presetList->close();
    else {
        m_presetList->clearAllItems();

        std::vector<FloatingListItem> items{};

        m_presets.clear();

        auto savedPresets = Save::getLayoutPresets();
        int index = 0;
        for (const auto& preset : savedPresets)
        {
            FloatingListItem newItem{};

            newItem.m_id = index;
            newItem.m_text = preset.m_name;
            newItem.m_sideButtonSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
            newItem.m_sideButtonSprite->setScale(.35f);
            newItem.m_sideButtonCallback = [&](auto _) {DTLayer::deletePreset(_);};

            index++;

            items.push_back(newItem);
            m_presets.insert({index, preset});
        }

        m_presetList->addItems(items);
        m_presetList->open();
    }
}
void DTLayer::onAddPreset(CCObject*) {
    TextInputPopup::create(
        "Save Layout",
        "Layout Name",
        "OK",
        "",
        [&](const auto& str) {
            savePreset(DTLayoutPreset{
                .m_name = str,
                .m_layout = currentLayout(),
                .m_newBestCustomization = Save::getNewBestCustomizations(),
                .m_sessionBestCustomization = Save::getSessionBestCustomizations()
            });

            m_presetList->close();
        }
    )->show();
}
void DTLayer::onImportPreset(CCObject*) {
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
        [&](file::PickResult result) {
            if (result.isErr()) {
                log::error("{}", result.unwrapErr());
                return;
            }

            auto pickOpt = result.unwrap();
            if (!pickOpt.has_value()) return;
            auto pick = pickOpt.value();

            if (pick.extension() != "dtl" || !pick.exists()) return;

            auto preset = file::readFromJson<DTLayoutPreset>(pick);
            if (preset.isErr()) {
                log::error("{}", preset.unwrapErr());
                return;
            }

            savePreset(preset.unwrap());

            m_presetList->close();
        }
    );
    importPresetHandle.setName("DT-import-preset-task");
}
void DTLayer::onExportPreset(CCObject*) {
    TextInputPopup::create(
        "Export Layout",
        "Layout Name",
        "EXPORT",
        "",
        [&](const auto& str) {
            auto layout = currentLayout();
            
            auto preset = DTLayoutPreset{
                .m_name = str,
                .m_layout = layout,
                .m_newBestCustomization = Save::getNewBestCustomizations(),
                .m_sessionBestCustomization = Save::getSessionBestCustomizations()
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
                [preset](file::PickResult result) {
                    if (result.isErr()) {
                        log::error("{}", result.unwrapErr());
                        return;
                    }

                    auto pickOpt = result.unwrap();
                    if (!pickOpt.has_value()) return;
                    auto pick = pickOpt.value();

                    pick = pick.replaceExtension("dtl");

                    StatsManager::createFile(pick, std::nullopt);

                    if (file::writeToJson(pick, preset).isErr()) {
                        Notification::create("Failed to export layout!", NotificationIcon::Error)->show();
                    }
                }
            );
            exportHandle.setName("DT-export-preset-task");
        }
    )->show();
}

void DTLayer::savePreset(DTLayoutPreset mypreset) {
    auto m_presets = Save::getLayoutPresets();
    for (const auto& preset : m_presets)
    {
        if (mypreset.m_name == preset.m_name) {
            Notification::create("A preset with that name already exists!", NotificationIcon::Error)->show();
            return;
        }
    }
    
    m_presets.push_back(mypreset);
    Save::setLayoutPresets(m_presets);
}

void DTLayer::deletePreset(int id) {
    geode::createQuickPopup("Warning!", "Are you sure you want to delete this preset?", "NO", "YES", [&](auto _, bool b2) {
        if (!b2) return;

        auto optItem = m_presetList->getItemForID(id);
        if (!optItem.has_value()) return;

        auto m_presets = Save::getLayoutPresets();
        int index = 0;
        for (const auto& preset : m_presets)
        {
            if (optItem.value().m_text == preset.m_name) {
                m_presets.erase(std::next(m_presets.begin(), index));
                break;
            }

            index++;
        }

        Save::setLayoutPresets(m_presets);
        m_presetList->close();
    });
}

void DTLayer::resetLayoutTo(DTLayoutPreset const& preset) {
    m_newBestColorBtnSpr->setColor(to3B(preset.m_newBestCustomization.m_color));
    Save::setNewBestCustomizations(preset.m_newBestCustomization);
    m_sessionBestColorBtnSpr->setColor(to3B(preset.m_sessionBestCustomization.m_color));
    Save::setSessionBestCustomizations(preset.m_sessionBestCustomization);

    setLayoutBy(preset.m_layout);
    organizeLayout();

    m_specialStrings["general"]->updateContent();
    m_specialStrings["s0"]->updateContent();
}

ListenerResult DTLayer::onDonerStateChanged(bool state) {
    for (const auto& label : CCArrayExt<DTLabel*>(m_labelsHolder->getChildren())) {
        label->setDonerRichTextEnabled(state);
    }

    verText->setColor(state ? ccColor3B{ 253, 255, 115 } : ccColor3B{ 255, 255, 255 });

    verSmokeParticle->setVisible(state);
    verStarsParticle->setVisible(state);

    m_specialStrings["general"]->updateContent();
    m_specialStrings["s0"]->updateContent();

    return ListenerResult::Propagate;
}