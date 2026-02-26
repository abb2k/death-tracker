#include "../layers/DTLayer.hpp"
#include "../managers/DTPopupManager.hpp"
#include "../utils/Settings.hpp"
#include "../layers/LabelLayoutWindow.hpp"
#include "../layers/DTGraphLayer.hpp"
#include "../layers/DTLinkLayer.hpp"
#include <Geode/ui/GeodeUI.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>
#include "../hooks/DTColorSelectPopup.hpp"
#include "../layers/DTLevelSpecificSettingsLayer.hpp"

DTLayer* DTLayer::create(GJGameLevel* const& level) {
    auto popup = new DTLayer;
    if (popup->init(level)) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool DTLayer::init(GJGameLevel* const& level) {
    if (!Popup::init(368, 280, "square01_001.png", {0.f, 0.f, 94.f, 94.f}))
        return false;
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    m_Level = level;

    // ================================== //
    // loading data
    m_MyLevelStats = StatsManager::getLevelStats(m_Level);

    DTLayer::UpdateSharedStats();
    // ================================== //

    /*
     * main page
    */

    this->setID("dt-layer");

    this->setZOrder(100);
    this->m_buttonMenu->setZOrder(1);
    m_buttonMenu->setPosition(m_size);
    m_closeBtn->setPosition(m_closeBtn->getPosition() - m_size / 2);
    m_mainLayer->setZOrder(1);

    alignmentNode = CCNode::create();
    alignmentNode->setPosition(m_size / 2);
    alignmentNode->setZOrder(2);
    m_mainLayer->addChild(alignmentNode);

    auto sideBG = CCScale9Sprite::create("square01_001.png", {0,0, 94, 94});
    sideBG->setContentSize({154, 145});
    sideBG->setPosition(alignmentNode->getPosition() + ccp(233.5f, 10));
    sideBG->setScale(0.65f);
    m_mainLayer->addChild(sideBG);

    //texts bg
    m_TextBG = CCScale9Sprite::create("GJ_square05.png", {0,0, 80, 80});
    m_TextBG->setContentSize({520 / 1.6f, m_size.height / 1.15f});
    m_TextBG->setPosition({m_size.width / 2, m_size.height / 2});
    m_TextBG->setOpacity(100);
    m_TextBG->setZOrder(10);
    m_mainLayer->addChild(m_TextBG);

    //create a scroll layer for the text
    m_ScrollLayer = ScrollLayer::create({520 / 1.6f, m_size.height / 1.15f});
    m_ScrollLayer->setPosition(m_TextBG->getPosition() - m_ScrollLayer->getContentSize() / 2);
    m_ScrollLayer->setZOrder(11);
    m_mainLayer->addChild(m_ScrollLayer);

    m_ScrollBar = Scrollbar::create(m_ScrollLayer);
    m_ScrollBar->setScale(0.95f);
    m_ScrollBar->setZOrder(11);
    m_ScrollBar->setPosition({154, 0});
    alignmentNode->addChild(m_ScrollBar);

    auto editLayoutBtnS = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto editLayoutBtnSS = CCSprite::create("layout_button.png"_spr);
    editLayoutBtnSS->setPosition(editLayoutBtnS->getContentSize() / 2);
    editLayoutBtnS->addChild(editLayoutBtnSS);
    editLayoutBtnS->setScale(0.8f);
    m_EditLayoutBtn = CCMenuItemSpriteExtra::create(
        editLayoutBtnS,
        nullptr,
        this,
        menu_selector(DTLayer::onEditLayout)
    );
    m_EditLayoutBtn->setPosition(m_size.width / 2 - 3.0f, m_size.height / 2 - 3.0f);
    this->m_buttonMenu->addChild(m_EditLayoutBtn);

    auto LinkLevelsButtonS = CCSprite::createWithSpriteFrameName("gj_linkBtn_001.png");
    LinkLevelsButtonS->setScale(1.2f);
    LinkLevelsButton = CCMenuItemSpriteExtra::create(
        LinkLevelsButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::OnLinkButtonClicked)
    );
    LinkLevelsButton->setPosition({-m_size.width / 2 + 3.0f, -m_size.height / 2 + 3.0f});
    m_buttonMenu->addChild(LinkLevelsButton);

    auto settingsBS = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsBS->setScale(0.75f);
    settingsButton = CCMenuItemSpriteExtra::create(
        settingsBS,
        nullptr,
        this,
        menu_selector(DTLayer::onSettings)
    );
    settingsButton->setPosition({m_size.width / 2 - 8.5f, -m_size.height / 2 + 8.5f});
    this->m_buttonMenu->addChild(settingsButton);

    auto levelSettingsBS = CCSprite::createWithSpriteFrameName("GJ_editBtn_001.png");
    levelSettingsBS->setScale(0.4f);

    levelSettingsBSArrow = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    levelSettingsBSArrow->setPosition({-15, levelSettingsBS->getContentHeight() / 2});
    levelSettingsBSArrow->setScale(0.75f);
    levelSettingsBS->addChild(levelSettingsBSArrow);

    auto levelSettingsButton = CCMenuItemSpriteExtra::create(
        levelSettingsBS,
        nullptr,
        this,
        menu_selector(DTLayer::onSpecificSettings)
    );
    levelSettingsButton->setPosition({-m_size.width / 2 + 3.0f, 0});
    this->m_buttonMenu->addChild(levelSettingsButton);

    /*
     * edit layout
    */

    m_EditLayoutMenu = CCMenu::create();
    m_EditLayoutMenu->setVisible(false);
    m_EditLayoutMenu->setZOrder(12);
    m_EditLayoutMenu->setPosition(m_buttonMenu->getPosition() / 2);
    m_mainLayer->addChild(m_EditLayoutMenu);

    m_BlackSquare = CCSprite::create("square02_001.png");
    m_BlackSquare->setZOrder(9);
    m_BlackSquare->setVisible(false);
    m_BlackSquare->setPosition(winSize / 2);
    m_BlackSquare->setScale(100);
    m_BlackSquare->setOpacity(150);
    alignmentNode->addChild(m_BlackSquare);

    auto editLayoutApplyBtnS = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto editLayoutApplyBtnSS = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
    editLayoutApplyBtnSS->setPosition(editLayoutBtnS->getContentSize() / 2);
    editLayoutApplyBtnS->addChild(editLayoutApplyBtnSS);
    editLayoutApplyBtnS->setScale(0.65f);
    editLayoutApplyBtn = CCMenuItemSpriteExtra::create(
        editLayoutApplyBtnS,
        nullptr,
        this,
        menu_selector(DTLayer::onEditLayoutApply)
    );
    editLayoutApplyBtn->setPosition({159, -120});
    m_EditLayoutMenu->addChild(editLayoutApplyBtn);

    auto addWindowButtonS = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
    addWindowButtonS->setScale(0.6f);
    addWindowButton = CCMenuItemSpriteExtra::create(
        addWindowButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::addBox)
    );
    addWindowButton->setPosition({-157, 114});
    m_EditLayoutMenu->addChild(addWindowButton);

    auto layoutInfoBS = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    layoutInfoBS->setScale(0.8f);
    layoutInfoButton = CCMenuItemSpriteExtra::create(
        layoutInfoBS,
        nullptr,
        this,
        menu_selector(DTLayer::onLayoutInfo)
    );
    layoutInfoButton->setPosition({159, 118});
    m_EditLayoutMenu->addChild(layoutInfoButton);

    auto resetLayoutBS = CCSprite::createWithSpriteFrameName("GJ_replayBtn_001.png");
    resetLayoutBS->setScale(0.5f);
    resetLayoutButton = CCMenuItemSpriteExtra::create(
        resetLayoutBS,
        nullptr,
        this,
        menu_selector(DTLayer::onResetLayout)
    );
    resetLayoutButton->setPosition({-157, -112});
    m_EditLayoutMenu->addChild(resetLayoutButton);

    auto editLayoutSideBG = CCScale9Sprite::create("square01_001.png", {0,0, 94, 94});
    editLayoutSideBG->setContentSize({154, 145});
    editLayoutSideBG->setPosition(ccp(233.5f, 10));
    editLayoutSideBG->setScale(0.65f);
    m_EditLayoutMenu->addChild(editLayoutSideBG);

    colorSpritenb = ColorChannelSprite::create();
    colorSpritenb->setColor(Save::getNewBestColor());
    colorSpritenb->setScale(0.65f);
    auto nbcPicker = CCMenuItemSpriteExtra::create(
        colorSpritenb,
        nullptr,
        this,
        menu_selector(DTLayer::editnbcColor)
    );
    nbcPicker->setPosition({233, 38});
    m_EditLayoutMenu->addChild(nbcPicker);

    colorSpritesb = ColorChannelSprite::create();
    colorSpritesb->setColor(Save::getSessionBestColor());
    colorSpritesb->setScale(0.65f);
    auto sbcPicker = CCMenuItemSpriteExtra::create(
        colorSpritesb,
        nullptr,
        this,
        menu_selector(DTLayer::editsbcColor)
    );
    sbcPicker->setPosition({233, -3});
    m_EditLayoutMenu->addChild(sbcPicker);

    auto nbcColorPickerLabel = CCLabelBMFont::create("New best", "bigFont.fnt");
    nbcColorPickerLabel->setPosition({233, 22});
    nbcColorPickerLabel->setScale(0.4f);
    nbcColorPickerLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_EditLayoutMenu->addChild(nbcColorPickerLabel);

    auto sbcColorPickerLabel = CCLabelBMFont::create("Session best", "bigFont.fnt");
    sbcColorPickerLabel->setPosition({233, -19});
    sbcColorPickerLabel->setScale(0.35f);
    sbcColorPickerLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_EditLayoutMenu->addChild(sbcColorPickerLabel);

    //session selection

    auto SessionSelectCont = CCNode::create();
    SessionSelectCont->setID("Session-Select-Container");
    SessionSelectCont->setPosition({233, 24});
    SessionSelectCont->setZOrder(1);
    alignmentNode->addChild(SessionSelectCont);

    m_SessionSelectMenu = CCMenu::create();
    m_SessionSelectMenu->setPosition({0, 0});
    SessionSelectCont->addChild(m_SessionSelectMenu);

    m_SessionsAmount = m_SharedLevelStats.sessions.size();
    m_SessionSelected = 1;

    m_SessionSelectionInput = TextInput::create(120, "Session");
    if (m_SessionsAmount == 0)
        m_SessionSelectionInput->setString("No sessions.");
    else
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));

    m_SessionSelectionInput->getInputNode()->setDelegate(this);
    m_SessionSelectionInput->setCommonFilter(CommonFilter::Uint);
    m_SessionSelectionInput->setScale(0.45f);
    SessionSelectCont->addChild(m_SessionSelectionInput);

    auto SessionSelectionRightS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    SessionSelectionRightS->setScaleX(0.35f);
    SessionSelectionRightS->setScaleY(0.2f);
    auto SessionSelectionRight = CCMenuItemSpriteExtra::create(
        SessionSelectionRightS,
        nullptr,
        this,
        menu_selector(DTLayer::SwitchSessionRight)
    );
    SessionSelectionRight->setPosition({34, 0});
    m_SessionSelectMenu->addChild(SessionSelectionRight);

    auto SessionSelectionLeftS = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    SessionSelectionLeftS->setScaleX(0.35f);
    SessionSelectionLeftS->setScaleY(0.2f);
    auto SessionSelectionLeft = CCMenuItemSpriteExtra::create(
        SessionSelectionLeftS,
        nullptr,
        this,
        menu_selector(DTLayer::SwitchSessionLeft)
    );
    SessionSelectionLeft->setPosition({-34, 0});
    SessionSelectionLeft->setRotation(180);
    m_SessionSelectMenu->addChild(SessionSelectionLeft);

    auto SessionSelectionLabel = CCLabelBMFont::create("Session", "bigFont.fnt");
    SessionSelectionLabel->setPosition({0, 16});
    SessionSelectionLabel->setScale(0.45f);
    SessionSelectCont->addChild(SessionSelectionLabel);

    auto GraphButtonS = CCSprite::create("graph_button.png"_spr);
    GraphButtonS->setScale(0.85f);
    auto GraphButton = CCMenuItemSpriteExtra::create(
        GraphButtonS,
        nullptr,
        this,
        menu_selector(DTLayer::openGraphMenu)
    );
    GraphButton->setPosition({233, -9});
    m_buttonMenu->addChild(GraphButton);

    //copy text

    auto copyTextBS = ButtonSprite::create("Copy Text");
    copyTextBS->setScale(0.575f);
    copyTextButton = CCMenuItemSpriteExtra::create(
        copyTextBS,
        nullptr,
        this,
        menu_selector(DTLayer::copyText)
    );
    copyTextButton->setPosition({0, -m_size.height / 2 + 3.0f});
    this->m_buttonMenu->addChild(copyTextButton);

    auto copyInfoBS = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    copyInfoBS->setScale(0.8f);
    copyInfoButton = CCMenuItemSpriteExtra::create(
        copyInfoBS,
        nullptr,
        this,
        menu_selector(DTLayer::onCopyInfo)
    );
    copyInfoButton->setPosition({159, 118});
    copyInfoButton->setVisible(false);
    m_EditLayoutMenu->addChild(copyInfoButton);

    DTLayer::createLayoutBlocks();

    refreshLoadingCircle = LoadingCircle::create();
    refreshLoadingCircle->setParentLayer(m_mainLayer);
    refreshLoadingCircle->show();
    refreshLoadingCircle->setZOrder(1000);
    refreshLoadingCircle->setVisible(false);
    refreshLoadingCircle->setPosition(-refreshLoadingCircle->getContentSize() / 2 + m_size / 2);

    DTLayer::refreshAll(true);

    this->scheduleUpdate();

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString()){
        Save::setLastOpenedVersion(Mod::get()->getVersion().toNonVString());
        FLAlertLayer::create(nullptr, "To all death tracker users!", fmt::format(
            "{}",
            "Hello! I am here to say I am sorry for the lack of updates and support for Death Tracker over the past while, I've been busy :(\n\n<cr>But!</c> I am here to announce that I've been working on a <cy>new beta</c> for the <cg>big death tracker V3 update!</c>\nIf you want to help <co>test out the new features</c>, or just <cy>get notified about death tracker news</c>, feel free to <cj>join the discord server</c> on the main mod page!"
        ), "OK", nullptr, 415, false, 200, 0.75f)->show();
    }

    return true;
}

void DTLayer::update(float delta){
    m_LayoutStuffCont->setVisible(m_EditLayoutMenu->isVisible());
    if (m_TextCont)
        m_TextCont->setVisible(!m_EditLayoutMenu->isVisible());
}

void DTLayer::onEditLayout(CCObject* sender){
    DTLayer::EditLayoutEnabled(true);
}

void DTLayer::textChanged(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInputNode() && m_SessionsAmount > 0){
        auto res = utils::numFromString<int>(input->getString());
        int selected = res.unwrapOr(1);
        
        if (selected > m_SessionsAmount){
            selected = m_SessionsAmount;
            input->setString(fmt::format("{}", m_SessionsAmount));
        }

        if (selected < 1){
            selected = 1;
            input->setString("1");
        }

        m_SessionSelected = selected;
        DTLayer::refreshSession();
    }
}

void DTLayer::textInputOpened(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInputNode() && m_SessionsAmount > 0){
        input->setString(fmt::format("{}", m_SessionSelected));
        m_SessionSelectionInputSelected = true;
    }

    
}

void DTLayer::textInputClosed(CCTextInputNode* input){
    if (input == m_SessionSelectionInput->getInputNode() && m_SessionsAmount > 0){
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
        m_SessionSelectionInputSelected = false;
    }
}

ResultFuture DTLayer::updateSessionString(int session){
    if (session - 1 < 0 || session - 1 >= m_SharedLevelStats.sessions.size() || !m_SharedLevelStats.sessions.size()) co_return Err("couldent get current session!");

    Session currentSession = m_SharedLevelStats.sessions[session - 1];

    auto value = co_await CreateDeathsInfo(currentSession.deaths, currentSession.newBests);

    if (value.isErr()){
        selectedSessionString = "No Saved Attempts!";
    }
    else{
        selectedSessionInfo = value.unwrap();

        std::string mergedString = "No Saved Attempts! ";
        for (const auto& SDeathI : selectedSessionInfo)
        {
            if (SDeathI.run.end < m_MyLevelStats.hideUpto) continue;
            if (mergedString == "No Saved Attempts! ")
                mergedString = "";

            if (SDeathI.isNewBest)
                mergedString += "<sbc>";
            mergedString += fmt::format("{}% x{}\n", SDeathI.run.end, SDeathI.deaths);
        }
        mergedString = mergedString.substr(0, mergedString.size() - 1);
        selectedSessionString = mergedString;
    }

    auto value2 = co_await CreateRunsInfo(currentSession.runs);

    if (value2.isErr()){
        selectedSessionRunString = "No Saved Runs!";
    }
    else{
        m_SelectedSessionRunInfo = value2.unwrap();

        std::string mergedString = "No Saved Runs! ";
        for (const auto& SRunI : m_SelectedSessionRunInfo)
        {
            if (SRunI.run.end - SRunI.run.start < m_MyLevelStats.hideRunLength) continue;
            if (mergedString == "No Saved Runs! ")
                mergedString = "";

            mergedString += fmt::format("{}% - {}% x{}\n", SRunI.run.start, SRunI.run.end, SRunI.deaths);
        }
        mergedString = mergedString.substr(0, mergedString.size() - 1);
        selectedSessionRunString = mergedString;
    }

    co_return Ok();
}

void DTLayer::onEditLayoutApply(CCObject*){
    DTLayer::EditLayoutEnabled(false);

    std::vector<LabelLayout> layout;
    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        auto IWindow = static_cast<LabelLayoutWindow*>(m_LayoutLines[i]);
        layout.push_back(IWindow->m_MyLayout);
    }

    Save::setNewBestColor(colorSpritenb->getColor());
    Save::setSessionBestColor(colorSpritesb->getColor());

    Save::setLayout(layout);

    DTLayer::refreshAll(true);
}

bool DTLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = true;
    if (pTouch->getLocation() != ccp(0, CCDirector::sharedDirector()->getWinSize().height))
        ClickPos = pTouch;
    return true;
}

void DTLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent){
    if (pTouch->getLocation() != ccp(0, CCDirector::sharedDirector()->getWinSize().height))
        ClickPos = pTouch;
}

void DTLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = false;
}

void DTLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent){
    m_IsClicking = false;
}

void DTLayer::EditLayoutEnabled(const bool& b){
    this->m_buttonMenu->setEnabled(!b);
    m_SessionSelectMenu->setEnabled(!b);
    m_EditLayoutMenu->setVisible(b);
    m_EditLayoutBtn->setEnabled(!b);
    m_BlackSquare->setVisible(b);
    m_LayoutStuffCont->setVisible(b);
    editLayoutApplyBtn->setEnabled(b);
    layoutInfoButton->setVisible(b);
    addWindowButton->setVisible(b);
    resetLayoutButton->setVisible(b);
    if (m_SessionSelectionInput)
        m_SessionSelectionInput->setEnabled(!b);

    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        auto IWindow = static_cast<LabelLayoutWindow*>(m_LayoutLines[i]);
        IWindow->setMoveEnabled(b);
    }

    float transitionFadeSpeed = 0.5f;

    if (b)
    {
        DTLayer::changeScrollSizeByBoxes(true);
        m_TextBG->setOpacity(200);
        colorSpritenb->setColor(Save::getNewBestColor());
        colorSpritesb->setColor(Save::getSessionBestColor());
    }
    else{
        m_TextBG->setOpacity(100);
        DTLayer::refreshAll(true);
        auto sprite = static_cast<CCSprite*>(editLayoutApplyBtn->getChildren()->objectAtIndex(0));
        sprite->setOpacity(255);
        static_cast<CCSprite*>(sprite->getChildren()->objectAtIndex(0))->setOpacity(255);
        isInCopyMenu = false;
    }
}

void DTLayer::changeScrollSizeByBoxes(const bool& moveToTop){
    float hight = 0;
    float lineH = 0;
    LabelLayoutWindow* PrevLine = nullptr;

    std::vector<LabelLayoutWindow*> sortedLayoutLines;
    sortedLayoutLines.reserve(m_LayoutLines.size());
    for (int i = 0; i < m_LayoutLines.size(); i++)
        sortedLayoutLines.push_back(static_cast<LabelLayoutWindow*>(m_LayoutLines[i]));
    
    std::ranges::sort(sortedLayoutLines, [](const LabelLayoutWindow* a, const LabelLayoutWindow* b) {
        return a->m_MyLayout.line < b->m_MyLayout.line;
    });

    for (int i = 0; i < sortedLayoutLines.size(); i++)
    {
        if (PrevLine){
            float calH = abs(sortedLayoutLines[i]->getPositionY() - PrevLine->getPositionY());
            hight += calH;
        }
        else{
            lineH = m_LayoutLines[i]->getContentSize().height;
        }

        PrevLine = sortedLayoutLines[i];
    }
    hight += lineH * 2;

    if (hight < m_ScrollLayer->getContentSize().height){
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, m_ScrollLayer->getContentSize().height});
    }
    else{
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, hight});
    }
    
    m_LayoutStuffCont->setPositionX(m_ScrollLayer->getContentSize().width / 2);
    m_LayoutStuffCont->setPositionY(m_ScrollLayer->m_contentLayer->getContentSize().height);

    if (moveToTop)
        m_ScrollLayer->moveToTop();
}

void DTLayer::createLayoutBlocks(){
    auto m_CurretLayout = Save::getLayout();

    if (m_LayoutStuffCont)
        m_LayoutStuffCont->removeMeAndCleanup();

    m_LayoutStuffCont = CCNode::create();
    m_LayoutStuffCont->setPositionX(m_ScrollLayer->getContentSize().width / 2);
    m_LayoutStuffCont->setPositionY(m_ScrollLayer->m_contentLayer->getContentSize().height);
    m_LayoutStuffCont->setVisible(false);

    m_ScrollLayer->m_contentLayer->addChild(m_LayoutStuffCont);

    m_LayoutLines.clear();
    m_LayoutLines.reserve(m_CurretLayout.size());

    for (int i = 0; i < m_CurretLayout.size(); i++)
    {
        auto currentWindow = LabelLayoutWindow::create(m_CurretLayout[i], this);
        m_LayoutStuffCont->addChild(currentWindow);
        geode::cocos::handleTouchPriority(this);
        m_LayoutLines.push_back(currentWindow);
    }

    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->setPositionBasedOnLayout(static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->m_MyLayout);
    }
}

void DTLayer::RefreshText(bool moveToTop){
    if (m_TextCont) m_TextCont->removeMeAndCleanup();

    m_TextCont = CCNode::create();
    m_ScrollLayer->m_contentLayer->addChild(m_TextCont);

    float overallHight = 0;

    auto layout = Save::getLayout();

    std::vector<std::tuple<SimpleTextArea*, int, int>> lables;

    std::map<int, float> positioning;
    std::map<int, bool> twoLabelLines;

    for (const auto& labelSettings : layout)
    {
        auto modifiedString = DTLayer::modifyString(labelSettings.text);

        auto label = SimpleTextArea::create(modifiedString.c_str(), StatsManager::getFont(labelSettings.font).c_str());
        label->setAlignment(labelSettings.alignment);
        label->setAnchorPoint({0.5f, 1});
        label->setWrappingMode(WrappingMode::WORD_WRAP);
        label->setColor(labelSettings.color);
        label->setScale(labelSettings.fontSize);
        label->setWidth(label->getWidth());
        lables.push_back(std::make_tuple(label, labelSettings.line, labelSettings.position));
        m_TextCont->addChild(label);

        float contentSize = label->getScaledContentHeight();

        if (positioning.contains(labelSettings.line)){
            twoLabelLines.insert(std::make_pair(labelSettings.line, true));
            if (positioning[labelSettings.line] < contentSize){
                positioning[labelSettings.line] = contentSize;
            }
        }
        else
            positioning.insert(std::make_pair(labelSettings.line, contentSize));
    }

    for (int i = 0; i < lables.size(); i++)
    {
        if (twoLabelLines[std::get<1>(lables[i])]){
            SimpleTextArea* currentArea = std::get<0>(lables[i]);

            currentArea->setWidth(currentArea->getWidth() / 2);
            if (std::get<2>(lables[i]) == 1){
                currentArea->setPositionX(currentArea->getPositionX() + currentArea->getWidth() / 2);
            }
            else{
                currentArea->setPositionX(currentArea->getPositionX() - currentArea->getWidth() / 2);
            }
            currentArea->setFont(currentArea->getFont());

            if (positioning.contains(std::get<1>(lables[i]))){
                if (positioning[std::get<1>(lables[i])] < std::get<0>(lables[i])->getScaledContentHeight()){
                    positioning[std::get<1>(lables[i])] = std::get<0>(lables[i])->getScaledContentHeight();
                }
            }
        }
    }

    for (int i = 0; i < lables.size(); i++)
    {
        for (const auto& posI : positioning)
        {
            if (std::get<1>(lables[i]) > posI.first){
                std::get<0>(lables[i])->setPositionY(std::get<0>(lables[i])->getPositionY() - posI.second);
            }
            else{
                break;
            }
        }

        auto label = std::get<0>(lables[i]);
        for (const auto& line : label->getLines())
        {
            std::string s = line->getString();
            if (s != "<" && s.length() > 1){
                //new best color
                if (StatsManager::isKeyInIndex(s, 1, "nbc>")){
                    s.erase(0, 5);
                    line->setString(s.c_str());
                    line->setColor(Save::getNewBestColor());
                }
                //sessions best color
                if (StatsManager::isKeyInIndex(s, 1, "sbc>")){
                    s.erase(0, 5);
                    line->setString(s.c_str());
                    line->setColor(Save::getSessionBestColor()); 
                }
            }

            s = line->getString();

            if (m_Level->isPlatformer()){
                for (int i = 0; i < s.length(); i++)
                {
                    if (s[i] == '%'){
                        auto PS = dynamic_cast<CCNode*>(line->getChildren()->objectAtIndex(i));
                        if (!PS) break;
                        PS->setVisible(false);
                        auto PSCont = CCNode::create();
                        PSCont->setPosition(line->getPosition());
                        PSCont->setScale(line->getScale());
                        PSCont->setAnchorPoint(line->getAnchorPoint());
                        PSCont->setContentSize(line->getContentSize());
                        line->getParent()->addChild(PSCont);

                        auto cpS = CCSprite::createWithSpriteFrameName("checkpoint_01_001.png");
                        cpS->setPosition(PS->getPosition());
                        PSCont->addChild(cpS);
                    }
                }
                
            }
        }
    }

    for (const auto& posI : positioning)
    {
        overallHight += posI.second;
    }

    if (overallHight < m_ScrollLayer->getContentSize().height){
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, m_ScrollLayer->getContentSize().height});
    }
    else{
        m_ScrollLayer->m_contentLayer->setContentSize({m_ScrollLayer->m_contentLayer->getContentSize().width, overallHight});
    }

    m_TextCont->setPositionX(m_ScrollLayer->getContentSize().width / 2);
    m_TextCont->setPositionY(m_ScrollLayer->m_contentLayer->getContentSize().height);

    if (moveToTop || m_ScrollLayer->m_contentLayer->getContentHeight() - m_ScrollLayer->getContentHeight() < -m_ScrollLayer->m_contentLayer->getPositionY())
        m_ScrollLayer->moveToTop();
        
    /*
    create labels by settings

    know how high every line
        get the highest if the two are in the same line
    
    place the labels at the bottom of the previewse line postions added up
    
    place the pos 1 on the right and pos 0 on the left
    */
}

std::string DTLayer::modifyString(std::string ToModify){
    /*
    keys to check for

    {f0} - runs from 0

    {runs} - runs

    {lvln} - levels name
    
    {att} - level attempts (with linked levels attempts)

    {s0} - selected session runs from 0
    
    {sruns} - selected session runs

    {nl} - new line

    {ssd}

    {sst}
    
    */

    int overallOffset = 0;

    auto inctences = StatsManager::KMPSearch("{", ToModify);
    for (int i = 0; i < inctences.size(); i++)
    {
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "f0}")){
            ToModify.erase(inctences[i], 4);
            ToModify.insert(inctences[i], deathsString);
            overallOffset -= 4;
            overallOffset += deathsString.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "runs}")){
            ToModify.erase(inctences[i], 6);
            ToModify.insert(inctences[i], RunString);
            overallOffset -= 6;
            overallOffset += RunString.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "lvln}")){
            ToModify.erase(inctences[i], 6);
            ToModify.insert(inctences[i], m_SharedLevelStats.levelName);
            overallOffset -= 6;
            overallOffset += m_SharedLevelStats.levelName.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "att}")){
            ToModify.erase(inctences[i], 5);
            ToModify.insert(inctences[i], std::to_string(m_SharedLevelStats.attempts));
            overallOffset -= 5;
            overallOffset += std::to_string(m_SharedLevelStats.attempts).length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "s0}")){
            ToModify.erase(inctences[i], 4);
            ToModify.insert(inctences[i], selectedSessionString);
            overallOffset -= 4;
            overallOffset += selectedSessionString.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "sruns}")){
            ToModify.erase(inctences[i], 7);
            ToModify.insert(inctences[i], selectedSessionRunString);
            overallOffset -= 7;
            overallOffset += selectedSessionRunString.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "nl}")){
            ToModify.erase(inctences[i], 4);
            ToModify.insert(inctences[i], "\n");
            overallOffset -= 4;
            overallOffset += 1;
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "ptf0}")){
            ToModify.erase(inctences[i], 6);
            auto timeText = StatsManager::workingTime(playtimeFromZero);
            ToModify.insert(inctences[i], timeText);
            overallOffset -= 6;
            overallOffset += timeText.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "ptrun}")){
            ToModify.erase(inctences[i], 7);
            auto timeText = StatsManager::workingTime(playtimeFromRuns);
            ToModify.insert(inctences[i], timeText);
            overallOffset -= 7;
            overallOffset += timeText.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "ptall}")){
            ToModify.erase(inctences[i], 7);
            //log::info("{} | {}", playtimeFromRuns, playtimeFromZero);
            auto timeText = StatsManager::workingTime(playtimeFromRuns + playtimeFromZero);
            ToModify.insert(inctences[i], timeText);
            overallOffset -= 7;
            overallOffset += timeText.length();
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "ssd}")){
            ToModify.erase(inctences[i], 5);
            overallOffset -= 5;

            if (m_SharedLevelStats.sessions[m_SessionSelected - 1].sessionStartDate == -1){
                std::string toAdd = "(Unknown date)";
                ToModify.insert(inctences[i], toAdd);
                overallOffset += toAdd.length();
            }
            else{
                time_t time = m_SharedLevelStats.sessions[m_SessionSelected - 1].sessionStartDate;
                auto tp = std::chrono::system_clock::from_time_t(time);


                std::string dateStr = (Settings::getDateFormat() == -1) ? fmt::format("{:%d/%m/%Y}", tp) : fmt::format("{:%m/%d/%Y}", tp);

                ToModify.insert(inctences[i], dateStr);
                overallOffset += dateStr.length();
            }
        }
        if (StatsManager::isKeyInIndex(ToModify, inctences[i] + 1, "sst}")){
            ToModify.erase(inctences[i], 5);
            overallOffset -= 5;

            if (!m_SharedLevelStats.sessions.size()) continue;

            if (m_SharedLevelStats.sessions[m_SessionSelected - 1].sessionStartDate == -1){
                std::string toAdd = "(Unknown time)";
                ToModify.insert(inctences[i], toAdd);
                overallOffset += toAdd.length();
            }
            else{
                time_t time = m_SharedLevelStats.sessions[m_SessionSelected - 1].sessionStartDate;
                auto tp = std::chrono::system_clock::from_time_t(time);

                std::string timeStr = fmt::format("{:%I:%M%p}", tp);

                ToModify.insert(inctences[i], timeStr);
                overallOffset += timeStr.length();
            }
        }

        if (i + 1 < inctences.size()){
            inctences[i + 1] += overallOffset;
        }
    }
    
    return ToModify;
}

ResultFuture DTLayer::refreshStrings(){
    auto value = co_await DTLayer::CreateDeathsInfo(m_SharedLevelStats.deaths, m_SharedLevelStats.newBests);

    if (value.isErr()){
        deathsString = "No Saved Attempts!";
    }
    else{
        m_DeathsInfo = value.unwrap();

        DTLayer::updatePlaytime(m_DeathsInfo, false);

        std::string mergedString = "No Saved Attempts! ";
        for (const auto& deathI : m_DeathsInfo)
        {
            if (deathI.run.end < m_MyLevelStats.hideUpto) continue;
            if (mergedString == "No Saved Attempts! ")
                mergedString = "";

            if (deathI.isNewBest)
                mergedString += "<nbc>";
            mergedString += fmt::format("{}% x{}\n", deathI.run.end, deathI.deaths);
        }
        mergedString = mergedString.substr(0, mergedString.size() - 1);
        deathsString = mergedString;
    }

    auto value2 = co_await DTLayer::CreateRunsInfo(m_SharedLevelStats.runs);
    
    if (value2.isErr()){
        RunString = "No Saved Runs!";
    }
    else{
        m_RunInfo = value2.unwrap();

        DTLayer::updatePlaytime(m_RunInfo, true);

        std::string mergedString = "No Saved Runs! ";
        for (const auto& runI : m_RunInfo)
        {
            if (runI.run.end - runI.run.start < m_MyLevelStats.hideRunLength) continue;
            if (mergedString == "No Saved Runs! ")
                mergedString = "";

            mergedString += fmt::format("{}% - {}% x{}\n", runI.run.start, runI.run.end, runI.deaths);
        }
        mergedString = mergedString.substr(0, mergedString.size() - 1);
        RunString = mergedString;
    }

    
    auto updateSessRes = co_await DTLayer::updateSessionString(m_SessionSelected);

    co_return updateSessRes;
}

DeathStringFuture DTLayer::CreateDeathsInfo(const Deaths& deaths, const NewBests& newBests){
    if (!m_Level) co_return Err("invalid level");
    if (deaths.size() == 0) co_return Err("No Saved Progress");

    int totalDeaths = 0;
    int bestRun = 0;
    std::map<int, int> sortedDeaths{};

    // sort the deaths
    for (const auto& [percentKey, count] : deaths) {
        auto numRes = geode::utils::numFromString<int>(percentKey);
        if (numRes.isErr()) co_return Err(numRes.unwrapErr());
        int percentInt = numRes.unwrap();

        sortedDeaths.insert(std::make_pair(percentInt, count));
        totalDeaths += count;

        if (percentInt > bestRun) bestRun = percentInt;
    }

    // create output
    int offset = m_Level->m_normalPercent.value() == 100
        ? 1
        : 0;

    std::vector<DeathInfo> output{};

    for (const auto& [percent, count] : sortedDeaths) {
        // calculate pass rate
        totalDeaths -= count;

        float passCount = totalDeaths;
        float passRate = (passCount + offset) / (passCount + count + offset) * 100;

        bool nb = newBests.contains(percent);
            
        if (percent == bestRun){
            if (bestRun != 100)
                passRate = 0;
            else
                passRate = 100;
        }

        auto info = DeathInfo(Run(0, percent), nb, count, passRate);
        output.push_back(info);
    }

    if (output.size() == 0) co_return Err("No Saved Progress");
    
    std::ranges::sort(output, [](const DeathInfo a, const DeathInfo b) {
        return a.run.end < b.run.end;
    });

    co_return Ok(output);
}

DeathStringFuture DTLayer::CreateRunsInfo(const Runs runs){
    if (!m_Level) co_return Err("invalid level");
    if (runs.size() == 0) co_return Err("No Saved Progress");

    std::map<int, int> totalDeaths{};
    std::vector<std::tuple<std::string, int>> sortedRuns{};

    for (const auto [runKey, count] : runs){
        sortedRuns.push_back(std::make_tuple(runKey, count));
        
        Run currRun = StatsManager::splitRunKey(runKey);

        totalDeaths[currRun.start] += count;
    }

    // sort the runs
    std::ranges::sort(sortedRuns, [](const std::tuple<std::string, int> a, const std::tuple<std::string, int> b) {
        auto runA = StatsManager::splitRunKey(std::get<0>(a));
        auto runB = StatsManager::splitRunKey(std::get<0>(b));

        // start is equal, compare end
        if (runA.start == runB.start) return runA.end < runB.end;
        return runA.start < runB.start;
    });

    // create output
    std::vector<DeathInfo> output{};

    for (const auto [runKey, count] : sortedRuns) {

        auto splittedRun = StatsManager::splitRunKey(runKey); 

        totalDeaths[splittedRun.start] -= count;

        float passCount = totalDeaths[splittedRun.start];
        float passRate = (passCount) / (passCount + count) * 100;

        auto info = DeathInfo(splittedRun, false, count, passRate);

        output.push_back(info);
    }

    co_return Ok(output);    
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

void DTLayer::updatePlaytime(std::vector<DeathInfo> deaths, bool runs){
    //credit to better info for the level time stuff :)
    
    if (runs)
        playtimeFromRuns = 0;
    else
        playtimeFromZero = 0;

    auto wt = m_Level->m_timestamp
        ? m_Level->m_timestamp / 240
        : std::ceil(timeForLevelString(m_Level->m_levelString));

    for (int i = 0; i < deaths.size(); i++)
    {
        int runLength = (deaths[i].run.end - deaths[i].run.start);

        long long runOverallPlaytime = 0;

        if (runLength == 0)
            runOverallPlaytime = wt * 0.005f * deaths[i].deaths;
        else
            runOverallPlaytime = wt * (runLength / 100.0f) * deaths[i].deaths;

        if (runs)
            playtimeFromRuns += runOverallPlaytime;
        else
            playtimeFromZero += runOverallPlaytime;
    }
}

void DTLayer::SwitchSessionRight(CCObject*){
    if (m_SessionSelected >= m_SessionsAmount) return;

    m_SessionSelected += 1;
    if (m_SessionSelectionInputSelected)
        m_SessionSelectionInput->setString(fmt::format("{}", m_SessionSelected));
    else
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
    DTLayer::refreshSession();
}   

void DTLayer::SwitchSessionLeft(CCObject*){
    if (m_SessionSelected - 1 < 1) return;

    m_SessionSelected -= 1;
    if (m_SessionSelectionInputSelected)
        m_SessionSelectionInput->setString(fmt::format("{}", m_SessionSelected));
    else
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
    DTLayer::refreshSession();
}

void DTLayer::addBox(CCObject*){
    int lastLine = 0;

    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        if (static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->m_MyLayout.line > lastLine){
            lastLine = static_cast<LabelLayoutWindow*>(m_LayoutLines[i])->m_MyLayout.line;
        }
    }
    
    LabelLayout currentLayout
    {
        .labelName = "Label",
        .line = lastLine + 1,
        .position = 0,
        .color = {255, 255, 255, 255},
        .alignment = CCTextAlignment::kCCTextAlignmentCenter,
        .font = 0,
        .fontSize = 0.75f
    };

    auto currentWindow = LabelLayoutWindow::create(currentLayout, this);
    m_LayoutStuffCont->addChild(currentWindow);
    m_LayoutLines.push_back(currentWindow);

    currentWindow->setPositionBasedOnLayout(currentLayout);

    DTLayer::changeScrollSizeByBoxes();
}

void DTLayer::updateRunsAllowed(){
    if (m_MyLevelStats.currentBest != -1)
        StatsManager::saveData(m_MyLevelStats, m_Level);
    DTLayer::refreshAll();
}

void DTLayer::FLAlert_Clicked(FLAlertLayer* layer, bool selected){
    if (ResetLayoutAlert == layer && selected){
        std::vector<LabelLayout> defaultLayout{
            {
                .labelName = "from 0",
                .text = "From 0:{nl}{f0}{nl} ",
                .line = 3,
                .position = 0,
                .color = {160,255,198,255},
                .alignment = CCTextAlignment::kCCTextAlignmentCenter,
                .font = 0,
                .fontSize = 0.5f
            },
            {
                .labelName = "Session",
                .text = "Session:{nl}{ssd}{nl}{s0}{nl} ",
                .line = 3,
                .position = 1,
                .color = {255,217,166,255},
                .alignment = CCTextAlignment::kCCTextAlignmentCenter,
                .font = 0,
                .fontSize = 0.5f
            },
            {
                .labelName = "Runs",
                .text = "Runs:{nl}{runs}{nl} ",
                .line = 4,
                .position = 0,
                .color = {85,168,112,255},
                .alignment = CCTextAlignment::kCCTextAlignmentCenter,
                .font = 0,
                .fontSize = 0.5f
            },
            {
                .labelName = "SRuns",
                .text = "Session Runs:{nl}{sruns}{nl} ",
                .line = 4,
                .position = 1,
                .color = {251,176,94,255},
                .alignment = CCTextAlignment::kCCTextAlignmentCenter,
                .font = 0,
                .fontSize = 0.5f
            },
            {
                .labelName = "Title",
                .text = "{lvln}:",
                .line = 0,
                .position = 1,
                .color = {255,255,255,255},
                .alignment = CCTextAlignment::kCCTextAlignmentCenter,
                .font = 2,
                .fontSize = 1
            },
            {
                .labelName = "att",
                .text = "{att} attempts",
                .line = 2,
                .position = 1,
                .color = {255,255,255,255},
                .alignment = CCTextAlignment::kCCTextAlignmentCenter,
                .font = 1,
                .fontSize = 0.75f
            },
            {
                .labelName =  "playtime",
                .text = "playtime - {ptall}",
                .line = 1,
                .position = 1,
                .color = {255,255,255,255},
                .alignment = CCTextAlignment::kCCTextAlignmentCenter,
                .font = 19,
                .fontSize = 0.45f
            }
        };

        Save::setLayout(defaultLayout);

        Save::setNewBestColor({255, 255, 0});
        Save::setSessionBestColor({ 255, 136, 0 });

        DTLayer::createLayoutBlocks();
        DTLayer::onEditLayoutApply(nullptr);
    }
}

void DTLayer::onClose(cocos2d::CCObject*) {
    if (m_EditLayoutMenu->isVisible()){
        DTLayer::EditLayoutEnabled(false);
        DTLayer::createLayoutBlocks();
    }
    else if (LevelSpecificSettingsLayer){
        //exit
        DTLayer::onSpecificSettings(nullptr);
    }
    else{
        this->setKeypadEnabled(false);
        this->setTouchEnabled(false);
        this->removeFromParentAndCleanup(true);
    }
}

void DTLayer::openGraphMenu(CCObject*){
    auto graph = DTGraphLayer::create(this);
    graph->setZOrder(100);
    this->addChild(graph);
}

void DTLayer::OnLinkButtonClicked(CCObject*){
    auto lLayer = DTLinkLayer::create(this);
    lLayer->setZOrder(100);
    this->addChild(lLayer);
}

void DTLayer::UpdateSharedStats(){
    m_SharedLevelStats = m_MyLevelStats;

    for (int i = 0; i < m_MyLevelStats.LinkedLevels.size(); i++)
    {
        auto currStats = StatsManager::getLevelStats(m_MyLevelStats.LinkedLevels[i]).unwrapOrDefault();
        if (currStats.levelName == "Unknown name"){
            Notification::create("failed to get data for linked level - " + m_MyLevelStats.LinkedLevels[i])->show();
            continue;
        }

        m_SharedLevelStats.attempts += currStats.attempts;

        m_SharedLevelStats.sessions.reserve(m_SharedLevelStats.sessions.size() + std::distance(currStats.sessions.begin(),currStats.sessions.end()));
        m_SharedLevelStats.sessions.insert(m_SharedLevelStats.sessions.end(),currStats.sessions.begin(),currStats.sessions.end());

        for (int r = 0; r < m_MyLevelStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < currStats.RunsToSave.size(); r2++)
            {
                if (currStats.RunsToSave[r2] == m_MyLevelStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                currStats.RunsToSave.push_back(m_MyLevelStats.RunsToSave[r]);
        }
        for (int r = 0; r < currStats.RunsToSave.size(); r++)
        {
            bool addMeRun = true;
            for (int r2 = 0; r2 < m_MyLevelStats.RunsToSave.size(); r2++)
            {
                if (m_MyLevelStats.RunsToSave[r2] == currStats.RunsToSave[r])
                    addMeRun = false;
            }
            
            if (addMeRun)
                m_MyLevelStats.RunsToSave.push_back(currStats.RunsToSave[r]);
        }

        std::ranges::sort(m_MyLevelStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        std::ranges::sort(currStats.RunsToSave, [](const int a, const int b) {
            return a < b;
        });

        if (m_MyLevelStats.currentBest != -1)
            StatsManager::saveData(m_MyLevelStats, m_Level);
        if (currStats.currentBest != -1)
            StatsManager::saveData(currStats, m_MyLevelStats.LinkedLevels[i]);
        
        for (const auto& [death, count] : currStats.deaths)
        {
            m_SharedLevelStats.deaths[death] += count;
        }
        
        for (const auto& [run, count] : currStats.runs)
        {
            m_SharedLevelStats.runs[run] += count;
        }

        if (m_SharedLevelStats.currentBest < currStats.currentBest)
            m_SharedLevelStats.currentBest = currStats.currentBest;

        m_SharedLevelStats.newBests.insert(currStats.newBests.begin(), currStats.newBests.end());
    }

    std::ranges::sort(m_SharedLevelStats.sessions, [](const Session a, const Session b) {
        return a.lastPlayed > b.lastPlayed;
    });

    m_SessionsAmount = m_SharedLevelStats.sessions.size();

    if (m_SessionSelectionInput)
        m_SessionSelectionInput->setString(fmt::format("{}/{}", m_SessionSelected, m_SessionsAmount));
}

void DTLayer::onSettings(CCObject*){
    geode::openSettingsPopup(Mod::get());
}

void DTLayer::onLayoutInfo(CCObject*){
    auto alert = FLAlertLayer::create("Help", "The boxes (labels) here represent the text displayed.\n \nYou drag them around to change their order and <cy>double click</c> any of them for more options.\nClick the <cg>plus</c> button to add a new label.\nThe button on the bottom left will <cy>reset everything back to default.</c>", "Ok");
    alert->show();
}

void DTLayer::onCopyInfo(CCObject*){
    auto alert = FLAlertLayer::create("Help", "Click a label to copy its text.", "Ok");
    alert->show();
}

void DTLayer::copyText(CCObject*)
{
    DTLayer::EditLayoutEnabled(true);

    isInCopyMenu = true;
    editLayoutApplyBtn->setEnabled(false);
    auto sprite = static_cast<CCSprite*>(editLayoutApplyBtn->getChildren()->objectAtIndex(0));
    sprite->setOpacity(100);
    static_cast<CCSprite*>(sprite->getChildren()->objectAtIndex(0))->setOpacity(100);
    resetLayoutButton->setVisible(false);

    for (int i = 0; i < m_LayoutLines.size(); i++)
    {
        auto IWindow = static_cast<LabelLayoutWindow*>(m_LayoutLines[i]);
        IWindow->setMoveEnabled(false);
    }

    layoutInfoButton->setVisible(false);
    addWindowButton->setVisible(false);
    copyInfoButton->setVisible(true);
}

void DTLayer::clickedWindow(CCNode* nwindow){
    auto window = static_cast<LabelLayoutWindow*>(nwindow);
    if (isInCopyMenu){
        DTLayer::EditLayoutEnabled(false);

        std::string toCopy = DTLayer::modifyString(window->m_MyLayout.text);

        for (int i = 0; i < toCopy.size(); i++)
        {
            if (toCopy[i] == '<' && toCopy.length() > i + 1){
                if (StatsManager::isKeyInIndex(toCopy, i + 1, "nbc>")){
                    toCopy.erase(i, 5);
                }
                if (StatsManager::isKeyInIndex(toCopy, i + 1, "sbc>")){
                    toCopy.erase(i, 5);
                }
            }
        }
        
        clipboard::write(toCopy);

        geode::Notification::create("Copied text from " + window->m_MyLayout.labelName, CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
    }   
}

void DTLayer::onResetLayout(CCObject*){
    ResetLayoutAlert = FLAlertLayer::create(this, "Warning!", "This will reset your layout back to the default layout.\n\nAre you sure you want to do this?", "Cancel", "Reset");
    ResetLayoutAlert->setZOrder(150);
    this->addChild(ResetLayoutAlert);
}

void DTLayer::editnbcColor(CCObject*){
    openednbLast = true;
    colorSelectnb = geode::ColorPickPopup::create({colorSpritenb->getColor().r, colorSpritenb->getColor().g, colorSpritenb->getColor().b, 255});
    colorSelectnb->show();
    colorSelectnb->setCallback([&](auto color){this->updateColor(color);});
}

void DTLayer::editsbcColor(CCObject*){
    openednbLast = false;
    colorSelectsb = geode::ColorPickPopup::create({colorSpritesb->getColor().r, colorSpritesb->getColor().g, colorSpritesb->getColor().b, 255});
    colorSelectsb->show();
    colorSelectsb->setCallback([&](auto color){this->updateColor(color);});
}

void DTLayer::onSpecificSettings(CCObject*){
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (runningMoveTransition) return;
    runningMoveTransition = true;

    if (LevelSpecificSettingsLayer){
        isExitingSSLayer = true;

        settingsButton->runAction(CCFadeIn::create(0.3f));
        LinkLevelsButton->runAction(CCFadeIn::create(0.3f));
        static_cast<ButtonSprite*>(copyTextButton->getChildren()->objectAtIndex(0))->m_label->runAction(CCFadeIn::create(0.3f));
        static_cast<ButtonSprite*>(copyTextButton->getChildren()->objectAtIndex(0))->m_BGSprite->runAction(CCFadeIn::create(0.3f));
        m_closeBtn->runAction(CCFadeIn::create(0.3f));
        m_EditLayoutBtn->runAction(CCFadeIn::create(0.3f));

        levelSettingsBSArrow->runAction(CCScaleTo::create(0.3f, 0.75f, 0.75f));

        LevelSpecificSettingsLayer->runAction(CCEaseInOut::create(CCScaleTo::create(0.3f, 0), 2));
        LevelSpecificSettingsLayer->runAction(CCSequence::create(CCEaseInOut::create(CCMoveTo::create(0.3f, ccp(310, winSize.height / 2)), 2), CCCallFuncO::create(this, callfuncO_selector(DTLayer::onMoveTransitionEnded), LevelSpecificSettingsLayer), nullptr));
        m_mainLayer->runAction(CCSequence::create(CCEaseInOut::create(CCMoveTo::create(0.3f, winSize / 2), 2), nullptr));
        typeinfo_cast<DTLevelSpecificSettingsLayer*>(LevelSpecificSettingsLayer)->EnableTouch(false);

        return;
    }


    auto SLLS = DTLevelSpecificSettingsLayer::create({375, 280}, this);
    this->addChild(SLLS);
    SLLS->EnableTouch(false);

    settingsButton->setEnabled(false);
    settingsButton->runAction(CCFadeOut::create(0.3f));
    LinkLevelsButton->setEnabled(false);
    LinkLevelsButton->runAction(CCFadeOut::create(0.3f));
    copyTextButton->setEnabled(false);
    static_cast<ButtonSprite*>(copyTextButton->getChildren()->objectAtIndex(0))->m_label->runAction(CCFadeOut::create(0.3f));
    static_cast<ButtonSprite*>(copyTextButton->getChildren()->objectAtIndex(0))->m_BGSprite->runAction(CCFadeOut::create(0.3f));
    m_closeBtn->setEnabled(false);
    m_closeBtn->runAction(CCFadeOut::create(0.3f));
    m_EditLayoutBtn->setEnabled(false);
    m_EditLayoutBtn->runAction(CCFadeOut::create(0.3f));

    levelSettingsBSArrow->runAction(CCScaleTo::create(0.3f, -0.75f, 0.75f));

    SLLS->setPosition(104, 159);
    SLLS->setScale(0);
    SLLS->runAction(CCEaseInOut::create(CCScaleTo::create(0.3f, 1), 2));
    SLLS->runAction(CCSequence::create(CCEaseInOut::create(CCMoveTo::create(0.3f, ccp(205, winSize.height / 2)), 2), CCCallFuncO::create(this, callfuncO_selector(DTLayer::onMoveTransitionEnded), SLLS), nullptr));
    m_mainLayer->runAction(CCSequence::create(CCEaseInOut::create(CCMoveTo::create(0.3f, ccp(205 + SLLS->m_Size.width / 2 + m_size.width / 2 + 24, m_mainLayer->getPositionY())), 2), nullptr));

    LevelSpecificSettingsLayer = SLLS;
}

void DTLayer::onMoveTransitionEnded(CCObject* LSSL){
    auto tempLSSL = typeinfo_cast<DTLevelSpecificSettingsLayer*>(LSSL);

    runningMoveTransition = false;
    if (isExitingSSLayer){
        isExitingSSLayer = false;
        tempLSSL->removeMeAndCleanup();
        LevelSpecificSettingsLayer = nullptr;
        settingsButton->setEnabled(true);
        LinkLevelsButton->setEnabled(true);
        copyTextButton->setEnabled(true);
        m_closeBtn->setEnabled(true);
        m_EditLayoutBtn->setEnabled(true);
    }
    else{
        tempLSSL->EnableTouch(true);
    }
}

void DTLayer::updateColor(cocos2d::ccColor4B const& color){
    ccColor3B col3b = {color.r, color.g, color.b};
    if (openednbLast){
        colorSpritenb->setColor(col3b);
    }
    else{
        colorSpritesb->setColor(col3b);
    }
}

void DTLayer::refreshAll(bool moveToTop){
    refreshLoadingCircle->setVisible(true);
    refreshListener.spawn(
        updateSessionAndRefresh(moveToTop, false),
        [&](ResultFuture::Output out){
            this->onRefreshFinished(out);
        }
    );
}

void DTLayer::refreshSession(bool moveToTop){
    refreshLoadingCircle->setVisible(true);

    refreshListener.spawn(
        updateSessionAndRefresh(moveToTop, true),
        [&](ResultFuture::Output out){
            this->onRefreshFinished(out);
        }
    );
}

void DTLayer::onRefreshFinished(ResultFuture::Output event){
    refreshLoadingCircle->setVisible(false);
}
ResultFuture DTLayer::updateSessionAndRefresh(bool moveToTop, bool refreshSessionOnly){
    ResultFuture::Output updateOutput = Err("");

    if (refreshSessionOnly){
        updateOutput = co_await updateSessionString(m_SessionSelected);
    }
    else{
        updateOutput = co_await refreshStrings();
    }
    if (updateOutput.isErr()) co_return Err(updateOutput.unwrapErr());

    auto value = co_await updateSessionString(m_SessionSelected);

    if (value.isErr()) co_return Err(value.unwrapErr());

    geode::queueInMainThread([&](){
        DTLayer::RefreshText(moveToTop);
    });
    

    co_return Ok();
}