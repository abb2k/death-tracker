#include "ImportCompletePopup.hpp"
#include <nodes/layers/DTLayer.hpp>
#include <nodes/SessionCell.hpp>

ImportCompletePopup* ImportCompletePopup::create(LevelData&& data, std::vector<Session> const& sessions) {
    auto ret = new ImportCompletePopup();
    if (ret->init(std::move(data), std::move(sessions))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ImportCompletePopup::init(LevelData&& _data, std::vector<Session> const& _sessions) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(250, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Import Complete");

    this->data = std::move(_data);
    this->sessions = std::move(_sessions);

    generalView = ScrollLayer::create({m_size.width / 2.25f, 150});
    generalView->ignoreAnchorPointForPosition(false);
    generalView->setAnchorPoint({0, 1});
    generalView->setZOrder(1);
    generalView->setPosition({
        10,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight()
    });
    generalView->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(generalView->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    m_mainLayer->addChild(generalView);

    auto generalViewBG = CCScale9Sprite::create("square02_small.png");
    generalViewBG->setAnchorPoint(generalView->getAnchorPoint());
    generalViewBG->setPosition(generalView->getPosition() - ccp(5, -5) / 2);
    generalViewBG->setContentSize(generalView->getContentSize() + ccp(5, 5));
    generalViewBG->setOpacity(120);
    m_mainLayer->addChild(generalViewBG);

    auto sessionLabel = CCLabelBMFont::create("Sessions      ", "bigFont.fnt");
    sessionLabel->setScale(.35f);
    sessionLabel->setAnchorPoint({1, 1});
    sessionLabel->setPosition({
        m_size.width - 10,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight()
    });
    m_mainLayer->addChild(sessionLabel);

    sessionView = ScrollLayer::create({m_size.width / 2.25f, 150  - sessionLabel->getScaledContentHeight() - 5});
    sessionView->ignoreAnchorPointForPosition(false);
    sessionView->setAnchorPoint({1, 1});
    sessionView->setZOrder(1);
    sessionView->setPosition({
        m_size.width - 10,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight() - sessionLabel->getScaledContentHeight() - 5
    });
    sessionView->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(sessionView->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    m_mainLayer->addChild(sessionView);

    auto sessionViewBG = CCScale9Sprite::create("square02_small.png");
    sessionViewBG->setAnchorPoint(sessionView->getAnchorPoint());
    sessionViewBG->setPosition(sessionView->getPosition() - ccp(-5, -5) / 2);
    sessionViewBG->setContentSize(sessionView->getContentSize() + ccp(5, 5));
    sessionViewBG->setOpacity(120);
    m_mainLayer->addChild(sessionViewBG);

    DTLabelInfo info;
    info.labelName = "General Data";
    info.scale = .3f;
    info.wrapping = WrappingMode::SPACE_WRAP;
    info.isExpanded = true;

    if (data.from0.isErr()) {
        info.text = "No found general data!";
    }
    else {
        auto& f0 = data.from0.unwrap();

        std::string str = "Playtime{nl}" + StatsManager::workingTime(
            f0.playtimeGeneral.playtimeF0 + f0.playtimeGeneral.playtimeRuns
        );
        
        str += "{nl} {nl}From 0:{nl}";

        std::string f0Str;

        DTLayer::get()->createDeathsString(
            f0.deaths,
            data.metadata,
            Save::getFrom0Customazations(),
            f0Str,
            f0.newBests,
            Save::getNewBestCustomizations(),
            true
        );

        str += f0Str;

        std::string runsStr;

        DTLayer::get()->createDeathsString(
            f0.runs,
            data.metadata,
            Save::getRunsCustomazations(),
            runsStr
        );

        str += "{nl} {nl}Runs:{nl}";

        str += runsStr;

        info.text = str;
    }

    auto label = DTLabel::create(info);
    label->setExpandable(false);
    label->setContentWidth(generalView->getContentWidth());
    generalView->m_contentLayer->addChild(label);

    for (const auto& session : sessions)
    {
        sessionView->m_contentLayer->addChild(SessionCell::create(sessionView->getContentWidth(), session));
    }
    
    sessionView->m_contentLayer->updateLayout();

    auto btnMenu = CCMenu::create();
    btnMenu->setPosition({0,0});
    m_mainLayer->addChild(btnMenu);

    DoAddGeneralToggler = SimpleToggler::createWithDefaults(
        .6f,
        true
    );
    DoAddGeneralToggler->setPosition(generalViewBG->getPosition() + ccp(
        DoAddGeneralToggler->getScaledContentWidth() / 2 + 5,
        -generalViewBG->getScaledContentHeight() - DoAddGeneralToggler->getScaledContentHeight() / 2 - 5)
    );
    btnMenu->addChild(DoAddGeneralToggler);

    auto DoAddGeneralTogglerLabel = CCLabelBMFont::create("Add General", "bigFont.fnt");
    DoAddGeneralTogglerLabel->setScale(.35f);
    DoAddGeneralTogglerLabel->setPosition(DoAddGeneralToggler->getPosition() + ccp(DoAddGeneralTogglerLabel->getScaledContentWidth() / 2 + DoAddGeneralToggler->getScaledContentWidth() / 2 + 5, 0));
    btnMenu->addChild(DoAddGeneralTogglerLabel);

    auto deselectAllSessionBtnSpr = ButtonSprite::create("Deselect All", "goldFont.fnt", "GJ_button_06.png");
    deselectAllSessionBtnSpr->setScale(.3f);
    auto deselectAllSessionBtn = CCMenuItemSpriteExtra::create(
        deselectAllSessionBtnSpr,
        this,
        menu_selector(ImportCompletePopup::deselectAllSessions)
    );
    deselectAllSessionBtn->setPosition(sessionViewBG->getPosition() + ccp(
        -deselectAllSessionBtn->getScaledContentWidth() / 2 - 5,
        -sessionViewBG->getScaledContentHeight() - deselectAllSessionBtn->getScaledContentHeight() / 2 - 5)
    );
    btnMenu->addChild(deselectAllSessionBtn);

    auto selectAllSessionBtnSpr = ButtonSprite::create("Select All");
    selectAllSessionBtnSpr->setScale(.3f);
    auto selectAllSessionBtn = CCMenuItemSpriteExtra::create(
        selectAllSessionBtnSpr,
        this,
        menu_selector(ImportCompletePopup::selectAllSessions)
    );
    selectAllSessionBtn->setPosition(deselectAllSessionBtn->getPosition() + ccp(
            -selectAllSessionBtn->getScaledContentWidth() / 2 - 5 - deselectAllSessionBtn->getScaledContentWidth() / 2,
            0
        )
    );
    btnMenu->addChild(selectAllSessionBtn);

    float offFromSides = 30;

    auto combineBtnSpr = ButtonSprite::create("Combine", "goldFont.fnt", "GJ_button_02.png");
    combineBtnSpr->setScale(.6f);
    auto combineBtn = CCMenuItemSpriteExtra::create(
        combineBtnSpr,
        this,
        menu_selector(ImportCompletePopup::onCombine)
    );
    combineBtn->setPosition(btnMenu->getPosition() + ccp(
        combineBtn->getScaledContentWidth() / 2 + offFromSides,
        combineBtn->getScaledContentHeight() / 2 + 10
    ));
    btnMenu->addChild(combineBtn);

    auto overrideBtnSpr = ButtonSprite::create("Override", "goldFont.fnt", "GJ_button_06.png");
    overrideBtnSpr->setScale(.6f);
    auto overrideBtn = CCMenuItemSpriteExtra::create(
        overrideBtnSpr,
        this,
        menu_selector(ImportCompletePopup::onOverride)
    );
    overrideBtn->setPosition(btnMenu->getPosition() + ccp(
        m_size.width - overrideBtn->getScaledContentWidth() / 2 - offFromSides,
        overrideBtn->getScaledContentHeight() / 2 + 10
    ));
    btnMenu->addChild(overrideBtn);

    scheduleUpdate();

    return true;
}

void ImportCompletePopup::update(float dt) {
    float prevHeight = generalView->m_contentLayer->getContentHeight();
    generalView->m_contentLayer->updateLayout();
    if (generalView->m_contentLayer->getContentHeight() != prevHeight)
        generalView->moveToTop();

    auto mousePos = getMousePos();

    CCRect generalViewRect = {generalView->getPositionX(), generalView->getPositionY(), generalView->getScaledContentSize().width, generalView->getScaledContentSize().height};

    CCRect sessionViewRect = {sessionView->getPositionX(), sessionView->getPositionY(), sessionView->getScaledContentSize().width, sessionView->getScaledContentSize().height};

    if (generalViewRect.containsPoint(generalView->getParent()->convertToNodeSpace(mousePos))) {
        generalView->setMouseEnabled(true);
        sessionView->setMouseEnabled(false);
    }
    else if (sessionViewRect.containsPoint(sessionView->getParent()->convertToNodeSpace(mousePos))) {
        generalView->setMouseEnabled(false);
        sessionView->setMouseEnabled(true);
    }
}

void ImportCompletePopup::selectAllSessions(CCObject*) {
    for (const auto& SCell : sessionView->m_contentLayer->getChildrenExt<SessionCell*>())
    {
        SCell->setSelected(true);
    }
}
void ImportCompletePopup::deselectAllSessions(CCObject*) {
    for (const auto& SCell : sessionView->m_contentLayer->getChildrenExt<SessionCell*>())
    {
        SCell->setSelected(false);
    }
}

void ImportCompletePopup::onCombine(CCObject*) {
    combineAlert = FLAlertLayer::create(this, "Warning!", "Are you sure you want to combine these saves?\n \nCombining would merge this imported save with your existing data.\n \nThis change is not easily revertible.", "Cancel", "Combine");
    combineAlert->show();
}

void ImportCompletePopup::onOverride(CCObject*) {
    overrideAlert = FLAlertLayer::create(this, "Warning!", "Are you sure you want to override your save with this imported one?\n \nOverriding would erase all existing data from the level and replace it with the data from the imported save.\n \nThis change is irrevertible.", "Cancel", "Override");
    overrideAlert->show();
}

void ImportCompletePopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;

    std::vector<Session> selectedSessions{};
    for (const auto& SCell : sessionView->m_contentLayer->getChildrenExt<SessionCell*>())
    {
        if (SCell->isSelected())
            selectedSessions.push_back(SCell->getSession());
    }
    

    if (combineAlert == layer) {
        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();
        auto key = stats.levelKey;

        if (DoAddGeneralToggler->isToggled()) {
            if (stats.from0.isErr() && data.from0.isOk())
                (void)StatsManager::setGeneral(data.from0.unwrap(), key);
            else if (data.from0.isOk()) {
                auto generalData = stats.from0.unwrap();
                generalData += data.from0.unwrap();
                (void)StatsManager::setGeneral(generalData, key);
            }
        }

        for (auto& session : selectedSessions)
        {
            if (DTLayer::get()->m_MyLevelStats.unwrap().sessionNames.contains(session.sessionStartDate)) {
                auto currSession = StatsManager::getSession(key, session.sessionStartDate);
                if (currSession.isErr())
                    (void)StatsManager::setSession(session, key, session.sessionStartDate, false);
                else {
                    auto sess = currSession.unwrap();
                    sess.data += session.data;
                    (void)StatsManager::setSession(sess, key, session.sessionStartDate, false);
                }
            }
            else {
                (void)StatsManager::setSession(session, key, session.sessionStartDate, false);
            }
        }
    }
    else if (overrideAlert == layer) {
        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();
        auto key = stats.levelKey;
        if (selectedSessions.size()) {
            auto deleteRes = StatsManager::deleteAllSessions(key);
            if (deleteRes.isErr()) return;
        }

        if (DoAddGeneralToggler->isToggled()) {
            if (data.from0.isErr()) {
                stats.from0 = Ok(GeneralData{});
            }
            else {
                stats.from0 = Ok(data.from0.unwrap());
            }

            (void)StatsManager::setGeneral(stats.from0.unwrap(), key);
        }
        
        for (auto& session : selectedSessions)
        {
            (void)StatsManager::setSession(session, key, session.sessionStartDate, false);
        }
    }

    DTLayer::get()->CleanGetStats();

    for (const auto& [_, key] : DTLayer::get()->specialStrings)
    {
        key->updateContent();
    }

    this->keyBackClicked();
}