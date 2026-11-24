#include "ImportCompletePopup.hpp"

#include <nodes/DTLayer.hpp>
#include <nodes/options/pages/save/import/SessionCell.hpp>
#include <nodes/layout/usage/DTLabel.hpp>
#include <nodes/general/SimpleToggler.hpp>

ImportCompletePopup* ImportCompletePopup::create(LevelData data, std::vector<Session> sessions) {
    auto ret = new ImportCompletePopup();
    
    if (ret->init(std::move(data), std::move(sessions))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool ImportCompletePopup::init(LevelData data, std::vector<Session> sessions) {
    if (!Popup::init(250, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Import Complete");

    this->m_data = std::move(data);
    this->m_sessions = std::move(sessions);

    m_generalView = ScrollLayer::create({m_size.width / 2.25f, 150});
    m_generalView->ignoreAnchorPointForPosition(false);
    m_generalView->setAnchorPoint({0, 1});
    m_generalView->setZOrder(1);
    m_generalView->setPosition({
        10,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight()
    });
    m_generalView->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_generalView->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    m_mainLayer->addChild(m_generalView);

    auto generalViewBG = CCScale9Sprite::create("square02_small.png");
    generalViewBG->setAnchorPoint(m_generalView->getAnchorPoint());
    generalViewBG->setPosition(m_generalView->getPosition() - ccp(5, -5) / 2);
    generalViewBG->setContentSize(m_generalView->getContentSize() + ccp(5, 5));
    generalViewBG->setOpacity(120);
    m_mainLayer->addChild(generalViewBG);

    auto sessionLabel = geode::Label::create("Sessions      ", "bigFont.fnt");
    sessionLabel->setScale(.35f);
    sessionLabel->setAnchorPoint({1, 1});
    sessionLabel->setPosition({
        m_size.width - 10,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight()
    });
    m_mainLayer->addChild(sessionLabel);

    m_sessionView = ScrollLayer::create({m_size.width / 2.25f, 150  - sessionLabel->getScaledContentHeight() - 5});
    m_sessionView->ignoreAnchorPointForPosition(false);
    m_sessionView->setAnchorPoint({1, 1});
    m_sessionView->setZOrder(1);
    m_sessionView->setPosition({
        m_size.width - 10,
        m_size.height - (m_size.height - m_title->getPositionY()) - m_title->getScaledContentHeight() - sessionLabel->getScaledContentHeight() - 5
    });
    m_sessionView->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_sessionView->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    m_mainLayer->addChild(m_sessionView);

    auto sessionViewBG = CCScale9Sprite::create("square02_small.png");
    sessionViewBG->setAnchorPoint(m_sessionView->getAnchorPoint());
    sessionViewBG->setPosition(m_sessionView->getPosition() - ccp(-5, -5) / 2);
    sessionViewBG->setContentSize(m_sessionView->getContentSize() + ccp(5, 5));
    sessionViewBG->setOpacity(120);
    m_mainLayer->addChild(sessionViewBG);

    DTLabelInfo info;
    info.m_labelName = "General Data";
    info.m_scale = .3f;
    info.m_wrapping = WrappingMode::SPACE_WRAP;
    info.m_isExpanded = true;

    if (m_data.m_from0.isErr()) {
        info.m_text = "No found general data!";
    }
    else {
        auto& f0 = m_data.m_from0.unwrap();

        std::string str = "Playtime{nl}" + StatsManager::workingTime(
            f0.m_playtimeGeneral.m_playtimeF0 + f0.m_playtimeGeneral.m_playtimeRuns
        );
        
        str += "{nl} {nl}From 0:{nl}";

        std::string f0Str;

        DTLayer::get()->createDeathsString(
            f0.m_deaths,
            m_data.m_metadata,
            Save::getFrom0Customazations(),
            f0Str,
            f0.m_newBests,
            Save::getNewBestCustomizations(),
            true
        );

        str += f0Str;

        std::string runsStr;

        DTLayer::get()->createDeathsString(
            f0.m_runs,
            m_data.m_metadata,
            Save::getRunsCustomazations(),
            runsStr
        );

        str += "{nl} {nl}Runs:{nl}";

        str += runsStr;

        info.m_text = str;
    }

    auto label = DTLabel::create(info);
    label->setExpandable(false);
    label->setContentWidth(m_generalView->getContentWidth());
    m_generalView->m_contentLayer->addChild(label);

    for (const auto& session : m_sessions)
    {
        m_sessionView->m_contentLayer->addChild(SessionCell::create(m_sessionView->getContentWidth(), session));
    }
    
    m_sessionView->m_contentLayer->updateLayout();

    auto btnMenu = CCMenu::create();
    btnMenu->setPosition({0,0});
    m_mainLayer->addChild(btnMenu);

    m_DoAddGeneralToggler = SimpleToggler::createWithDefaults(
        .6f,
        true
    );
    m_DoAddGeneralToggler->setPosition(generalViewBG->getPosition() + ccp(
        m_DoAddGeneralToggler->getScaledContentWidth() / 2 + 5,
        -generalViewBG->getScaledContentHeight() - m_DoAddGeneralToggler->getScaledContentHeight() / 2 - 5)
    );
    btnMenu->addChild(m_DoAddGeneralToggler);

    auto DoAddGeneralTogglerLabel = geode::Label::create("Add General", "bigFont.fnt");
    DoAddGeneralTogglerLabel->setScale(.35f);
    DoAddGeneralTogglerLabel->setPosition(m_DoAddGeneralToggler->getPosition() + ccp(DoAddGeneralTogglerLabel->getScaledContentWidth() / 2 + m_DoAddGeneralToggler->getScaledContentWidth() / 2 + 5, 0));
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
    float prevHeight = m_generalView->m_contentLayer->getContentHeight();
    m_generalView->m_contentLayer->updateLayout();
    if (m_generalView->m_contentLayer->getContentHeight() != prevHeight)
        m_generalView->moveToTop();

    auto mousePos = getMousePos();

    CCRect generalViewRect = {m_generalView->getPositionX(), m_generalView->getPositionY(), m_generalView->getScaledContentSize().width, m_generalView->getScaledContentSize().height};

    CCRect sessionViewRect = {m_sessionView->getPositionX(), m_sessionView->getPositionY(), m_sessionView->getScaledContentSize().width, m_sessionView->getScaledContentSize().height};

    if (generalViewRect.containsPoint(m_generalView->getParent()->convertToNodeSpace(mousePos))) {
        m_generalView->setMouseEnabled(true);
        m_sessionView->setMouseEnabled(false);
    }
    else if (sessionViewRect.containsPoint(m_sessionView->getParent()->convertToNodeSpace(mousePos))) {
        m_generalView->setMouseEnabled(false);
        m_sessionView->setMouseEnabled(true);
    }
}

void ImportCompletePopup::selectAllSessions(CCObject*) {
    for (const auto& SCell : m_sessionView->m_contentLayer->getChildrenExt<SessionCell*>())
    {
        SCell->setSelected(true);
    }
}
void ImportCompletePopup::deselectAllSessions(CCObject*) {
    for (const auto& SCell : m_sessionView->m_contentLayer->getChildrenExt<SessionCell*>())
    {
        SCell->setSelected(false);
    }
}

void ImportCompletePopup::onCombine(CCObject*) {
    m_combineAlert = FLAlertLayer::create(this, "Warning!", "Are you sure you want to combine these saves?\n \nCombining would merge this imported save with your existing data.\n \nThis change is not easily revertible.", "Cancel", "Combine");
    m_combineAlert->show();
}

void ImportCompletePopup::onOverride(CCObject*) {
    m_overrideAlert = FLAlertLayer::create(this, "Warning!", "Are you sure you want to override your save with this imported one?\n \nOverriding would erase all existing data from the level and replace it with the data from the imported save.\n \nThis change is irrevertible.", "Cancel", "Override");
    m_overrideAlert->show();
}

void ImportCompletePopup::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!btn2) return;

    std::vector<Session> selectedSessions{};
    for (const auto& SCell : m_sessionView->m_contentLayer->getChildrenExt<SessionCell*>())
    {
        if (SCell->isSelected())
            selectedSessions.push_back(SCell->getSession());
    }
    

    if (m_combineAlert == layer) {
        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();
        auto key = stats.m_levelKey;

        if (m_DoAddGeneralToggler->isToggled()) {
            if (stats.m_from0.isErr() && m_data.m_from0.isOk())
                (void)StatsManager::setGeneral(m_data.m_from0.unwrap(), key);
            else if (m_data.m_from0.isOk()) {
                auto generalData = stats.m_from0.unwrap();
                generalData += m_data.m_from0.unwrap();
                (void)StatsManager::setGeneral(generalData, key);
            }
        }

        for (auto& session : selectedSessions)
        {
            if (DTLayer::get()->m_MyLevelStats.unwrap().m_sessionNames.contains(session.m_sessionStartDate)) {
                auto currSession = StatsManager::getSession(key, session.m_sessionStartDate);
                if (currSession.isErr())
                    (void)StatsManager::setSession(session, key, session.m_sessionStartDate, false);
                else {
                    auto sess = currSession.unwrap();
                    sess.m_data += session.m_data;
                    sess.m_dataIsOverride = !m_DoAddGeneralToggler->isToggled();
                    (void)StatsManager::setSession(sess, key, session.m_sessionStartDate, false);
                }
            }
            else {
                session.m_dataIsOverride = !m_DoAddGeneralToggler->isToggled();
                (void)StatsManager::setSession(session, key, session.m_sessionStartDate, false);
            }
        }
    }
    else if (m_overrideAlert == layer) {
        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();
        auto key = stats.m_levelKey;
        if (selectedSessions.size()) {
            auto deleteRes = StatsManager::deleteAllSessions(key);
            if (deleteRes.isErr()) return;
        }

        if (m_DoAddGeneralToggler->isToggled()) {
            if (m_data.m_from0.isErr()) {
                stats.m_from0 = Ok(GeneralData{});
            }
            else {
                stats.m_from0 = Ok(m_data.m_from0.unwrap());
            }

            (void)StatsManager::setGeneral(stats.m_from0.unwrap(), key);
        }
        
        for (auto& session : selectedSessions)
        {
            session.m_dataIsOverride = session.m_dataIsOverride || !m_DoAddGeneralToggler->isToggled() || m_data.m_from0.isErr();
            (void)StatsManager::setSession(session, key, session.m_sessionStartDate, false);
        }
    }

    DTLayer::get()->CleanGetStats();

    for (const auto& [_, key] : DTLayer::get()->m_specialStrings)
    {
        key->updateContent();
    }

    this->keyBackClicked();
}