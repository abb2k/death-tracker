#include "SectionOptions.hpp"
#include <nodes/DTLayer.hpp>
#include <nodes/options/pages/section/SectionCell.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>

SectionOptions* SectionOptions::create(const CCSize& size) {
    auto ret = new SectionOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SectionOptions::setup() {

    auto dtlayer = DTLayer::get();

    m_mainScroll = ScrollLayer::create(m_size / 1.2f);
    m_mainScroll->ignoreAnchorPointForPosition(false);
    m_mainScroll->setPosition({m_size / 2 - ccp(0, 20)});
    
    m_mainScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(m_mainScroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
        ->ignoreInvisibleChildren(false)
    );
    this->addChild(m_mainScroll);

    m_scrollBG = CCScale9Sprite::create("square02_001.png");
    m_scrollBG->setContentSize(m_mainScroll->getContentSize() + ccp(15, 10));
    m_scrollBG->setPosition(m_mainScroll->getPosition());
    m_scrollBG->setOpacity(0);
    m_scrollBG->setID("BG");
    m_scrollBG->setZOrder(-1);
    m_scrollBG->setCascadeOpacityEnabled(false);
    this->addChild(m_scrollBG);

    auto sectionsLabel = geode::Label::create("Sections", "bigFont.fnt");
    sectionsLabel->setPosition(m_mainScroll->getPosition() + ccp(0, m_mainScroll->getContentSize().height / 2 + sectionsLabel->getScaledContentHeight() / 2 + 5));
    sectionsLabel->setScale(.75f);
    this->addChild(sectionsLabel);

    auto startPosBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto spSpr = CCSprite::createWithSpriteFrameName("edit_eStartPosBtn_001.png");
    spSpr->setScale(.75f);
    startPosBtnSpr->setScale(.6f);
    startPosBtnSpr->addChild(spSpr);
    startPosBtnSpr->setCascadeOpacityEnabled(true);
    spSpr->setPosition(startPosBtnSpr->getContentSize() / 2);
    auto startPosBtn = CCMenuItemSpriteExtra::create(
        startPosBtnSpr,
        this,
        menu_selector(SectionOptions::onStartPoses)
    );
    startPosBtn->setPosition(sectionsLabel->getPosition() - ccp(sectionsLabel->getContentWidth() / 2 + startPosBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(startPosBtn);

    if (dtlayer->m_MyLevelStats.isErr()) {

    }
    else {
        auto myStats = dtlayer->m_MyLevelStats.unwrap();
        for (const auto& section : myStats.m_metadata.m_sections)
        {
            addSection(section);
        }
    }

    auto addSectionBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    //addSectionBtnSpr->setScale(.75f);
    auto addSectionBtn = CCMenuItemSpriteExtra::create(
        addSectionBtnSpr,
        this,
        menu_selector(SectionOptions::onAddSection)
    );
    addSectionBtn->setPosition(m_mainScroll->getPosition() + ccp(m_mainScroll->getScaledContentWidth() / 2, m_mainScroll->getScaledContentHeight() / 2));
    this->addChild(addSectionBtn);

    auto sectionsInfo = TutorialButton::create(.75f, "section-overall", [
        &,
        addSectionBtn
    ](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("Here you can <cy>name</c> ranges of percentages as sections!", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("You can click the <cg>+</c> button to add a <cy>new section</c>", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPTop, .75f)
            ->joinHighlight(addSectionBtn)
            ->appendDialogue("Each section has a <cy>starting</c> and <cc>ending percent</c>", TutorialCharacterFace::TCFNormal);
        for (const auto& child : m_mainScroll->m_contentLayer->getChildrenExt<CCMenu*>())
        {
            tutorialLayer->joinHighlight(child->getChildByID("start-per"))
                ->joinHighlight(child->getChildByID("end-per"));
        }
        tutorialLayer->appendDialogue("And an input for the <cf>name</c> of the section", TutorialCharacterFace::TCFNormalTilted);
        for (const auto& child : m_mainScroll->m_contentLayer->getChildrenExt<CCMenu*>())
        {
            tutorialLayer->joinHighlight(child->getChildByID("name"));
        }
        tutorialLayer->appendDialogue("And if a sections percent <cy>range is invalid</c>, the section will turn <cr>red</c>", TutorialCharacterFace::TCFNormal);
    });
    sectionsInfo->setPosition(m_size - sectionsInfo->getScaledContentSize() / 2 + ccp(2, 2));
    this->addChild(sectionsInfo);

    for (const auto& cell : m_mainScroll->m_contentLayer->getChildrenExt<SectionCell*>())
    {
        cell->fade(false, 0);
    }
    
    this->setOpacity(0);
    this->setEnabled(false);

    m_mainScroll->setMouseEnabled(false);

    m_mainScroll->m_contentLayer->updateLayout();

    return true;
}

void SectionOptions::onOpened() {
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));
    m_mainScroll->moveToTop();
    m_mainScroll->setMouseEnabled(true);

    m_scrollBG->runAction(CCFadeTo::create(fadeTime, 120));

    for (const auto& cell : m_mainScroll->m_contentLayer->getChildrenExt<SectionCell*>())
    {
        cell->fade(true, fadeTime);
    }
}
void SectionOptions::onClosed() {
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));

    m_mainScroll->setMouseEnabled(false);
    m_scrollBG->runAction(CCFadeTo::create(fadeTime, 0));

    for (const auto& cell : m_mainScroll->m_contentLayer->getChildrenExt<SectionCell*>())
    {
        cell->fade(false, fadeTime);
    }
}

void SectionOptions::saveSections() {
    auto dtlayer = DTLayer::get();
    if (dtlayer->m_MyLevelStats.isErr()) return;

    auto& myStats = dtlayer->m_MyLevelStats.unwrap();

    std::vector<Section> newSections;
    for (const auto& cell : m_mainScroll->m_contentLayer->getChildrenExt<SectionCell*>()) {
        newSections.push_back(cell->getSection());
    }

    myStats.m_metadata.m_sections = newSections;
    (void)StatsManager::setMetadata(myStats.m_metadata, myStats.m_levelKey);

    dtlayer->foreachLinkedLevel([&](auto& lvlData) {
        lvlData.m_metadata.m_sections = newSections;
        (void)StatsManager::setMetadata(lvlData.m_metadata, lvlData.m_levelKey);
    });

    DTLayer::get()->m_specialStrings["section"]->updateContent();
}

void SectionOptions::onAddSection(CCObject* sender) {
    addSection(Section{});
    m_mainScroll->m_contentLayer->updateLayout();
    saveSections();
}

void SectionOptions::addSection(const Section& section) {
    auto cell = SectionCell::create(m_mainScroll->getContentWidth(), section);
    cell->m_onDeleted = [&]() {
        m_mainScroll->m_contentLayer->updateLayout();
        this->saveSections();
    };
    cell->m_onChanged = [&]() {
        this->saveSections();
    };
    m_mainScroll->m_contentLayer->addChild(cell);
    m_mainScroll->m_contentLayer->updateLayout();
    m_mainScroll->m_contentLayer->setPositionY(0);
}

void SectionOptions::onStartPoses(CCObject*) {
    auto currLvl = PlayLayer::get();
    if (currLvl == nullptr) {
        FLAlertLayer::create("Cant add StartPos sections", "You must enter the level itself to automatically add StartPos sections!", "OK")->show();
        return;
    }

    int anythingAdded = false;

    std::set<int> startPosPercents{};
    
    for (const auto& child : currLvl->m_objects->asExt<GameObject*>()) {
        if (auto sp = typeinfo_cast<StartPosObject*>(child)) {
            float LLength = PlayLayer::get()->m_levelLength;
            float startPosX = sp->getPositionX();

            if (currLvl->m_level->m_timestamp > 0) {
                float startPosTime = PlayLayer::get()->timeForPos({startPosX, 0}, 0, 0, true, 0);
                float LTime = PlayLayer::get()->timeForPos({LLength, 0}, 0, 0, true, 0);

                startPosPercents.insert(startPosTime / LTime * 100);
            } else {
                startPosPercents.insert(startPosX / LLength * 100);
            }
        }
    }

    if (startPosPercents.size()) startPosPercents.insert(100);
    else {
        Notification::create("No StartPoses detected!", NotificationIcon::Info)->show();
        return;
    }

    int prevRunPercent = 0;

    int index = 0;

    for (const auto& percent : startPosPercents) {
        Section section{
            .m_startPercent = prevRunPercent,
            .m_endPercent = percent,
        };

        section.m_name = fmt::format("#{}", 1 + index);
        index++;

        prevRunPercent = percent;

        if (doesSectionExist(section)) continue;

        addSection(section);
        anythingAdded = true;
    }

    
    if (anythingAdded) {
        saveSections();
        Notification::create("Added StartPos sections!", NotificationIcon::Success)->show();
    }
    else
        Notification::create("StartPos sections were already added", NotificationIcon::Info)->show();
}

bool SectionOptions::doesSectionExist(const Section& section) {
    std::vector<Section> newSections;
    for (const auto& cell : m_mainScroll->m_contentLayer->getChildrenExt<SectionCell*>()) {
        newSections.push_back(cell->getSection());
    }

    for (const auto& existingSection : newSections) {
        if (existingSection.m_startPercent == section.m_startPercent && existingSection.m_endPercent == section.m_endPercent) {
            return true;
        }
    }

    return false;
}