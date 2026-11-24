#include "LinkOptions.hpp"
#include <managers/StatsManager.hpp>
#include <nodes/DTLayer.hpp>
#include <nodes/options/pages/link/LevelLinkCell.hpp>
#include <utils/Dev.hpp>
#include <actions/CCTextAreaFadeTo.hpp>
#include <arc/task/Yield.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>

LinkOptions* LinkOptions::create(const CCSize& size) {
    auto ret = new LinkOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

LinkOptions::~LinkOptions() {
    m_callbackOwner->store(nullptr, std::memory_order_release);
}

bool LinkOptions::setup() {
    m_callbackOwner->store(this, std::memory_order_release);

    m_linkedScroll = ScrollLayer::create({m_size.width / 2 - 5, m_size.height - 70});
    auto linkedScrollBG = CCScale9Sprite::create("square02_small.png");
    linkedScrollBG->setContentSize(m_linkedScroll->getContentSize());
    linkedScrollBG->setZOrder(-1);
    linkedScrollBG->ignoreAnchorPointForPosition(true);
    linkedScrollBG->setID("bg");
    m_linkedScroll->addChild(linkedScrollBG);
    m_linkedScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_linkedScroll->getContentHeight())
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)
    );
    m_linkedScroll->setMouseEnabled(false);
    m_linkedScroll->ignoreAnchorPointForPosition(true);
    this->addChild(m_linkedScroll);
    
    m_playedScroll = ScrollLayer::create({m_size.width / 2 - 5, m_size.height - 70});
    auto playedScrollBG = CCScale9Sprite::create("square02_small.png");
    playedScrollBG->setContentSize(m_playedScroll->getContentSize());
    playedScrollBG->setZOrder(-1);
    playedScrollBG->ignoreAnchorPointForPosition(true);
    playedScrollBG->setID("bg");
    m_playedScroll->addChild(playedScrollBG);
    m_playedScroll->setPositionX(m_linkedScroll->getContentWidth() + 10);
    m_playedScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_playedScroll->getContentHeight())
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)
    );
    m_playedScroll->ignoreAnchorPointForPosition(true);
    m_playedScroll->setMouseEnabled(false);
    this->addChild(m_playedScroll);

    m_nonePlayedFoundText = SimpleTextArea::create("No unlinked played levels found!", "bigFont.fnt", .5f);
    m_nonePlayedFoundText->setPosition(m_playedScroll->getPosition() + m_playedScroll->getContentSize() / 2);
    m_nonePlayedFoundText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_nonePlayedFoundText->setWidth(m_playedScroll->getContentWidth() / 1.2f);
    m_nonePlayedFoundText->setWrappingMode(WrappingMode::SPACE_WRAP);
    this->addChild(m_nonePlayedFoundText);
    
    m_noneLinkedFoundText = SimpleTextArea::create("No linked levels found!", "bigFont.fnt", .5f);
    m_noneLinkedFoundText->setPosition(m_linkedScroll->getPosition() + m_linkedScroll->getContentSize() / 2);
    m_noneLinkedFoundText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_noneLinkedFoundText->setWidth(m_linkedScroll->getContentWidth() / 1.2f);
    m_noneLinkedFoundText->setWrappingMode(WrappingMode::SPACE_WRAP);
    this->addChild(m_noneLinkedFoundText);

    m_linkedScrollPageLabel = geode::Label::create("0/0", "bigFont.fnt");
    m_linkedScrollPageLabel->setScale(.55f);
    m_linkedScrollPageLabel->setPosition(m_linkedScroll->getPosition() + ccp(m_linkedScroll->getContentWidth() / 2, m_linkedScroll->getContentHeight() + m_linkedScrollPageLabel->getScaledContentHeight() / 2 + 5));
    this->addChild(m_linkedScrollPageLabel);

    auto linkedScrollLabel = geode::Label::create("Linked", "bigFont.fnt");
    linkedScrollLabel->setScale(.55f);
    linkedScrollLabel->setPosition(m_linkedScrollPageLabel->getPosition() + ccp(0, m_linkedScrollPageLabel->getScaledContentHeight() / 2 + linkedScrollLabel->getScaledContentHeight() / 2 + 5));
    this->addChild(linkedScrollLabel);

    float arrowOffset = 60;

    auto linkPageForwardSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    linkPageForwardSpr->setScale(.6f);
    auto linkPageForward = CCMenuItemSpriteExtra::create(
        linkPageForwardSpr,
        this,
        menu_selector(LinkOptions::onSwitchedPage)
    );
    linkPageForward->setTag(1);
    linkPageForward->setID("link");
    linkPageForward->setPosition(m_linkedScrollPageLabel->getPosition() + ccp(arrowOffset, 0));
    linkPageForward->setRotation(180);
    this->addChild(linkPageForward);

    auto linkPageBackwardSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    linkPageBackwardSpr->setScale(.6);
    auto linkPageBackward = CCMenuItemSpriteExtra::create(
        linkPageBackwardSpr,
        this,
        menu_selector(LinkOptions::onSwitchedPage)
    );
    linkPageBackward->setTag(-1);
    linkPageBackward->setID("link");
    linkPageBackward->setPosition(m_linkedScrollPageLabel->getPosition() - ccp(arrowOffset, 0));
    this->addChild(linkPageBackward);

    m_playedScrollPageLabel = geode::Label::create("0/0", "bigFont.fnt");
    m_playedScrollPageLabel->setScale(.55f);
    m_playedScrollPageLabel->setPosition(m_playedScroll->getPosition() + ccp(m_playedScroll->getContentWidth() / 2, m_playedScroll->getContentHeight() + m_playedScrollPageLabel->getScaledContentHeight() / 2 + 5));
    this->addChild(m_playedScrollPageLabel);

    auto playedScrollLabel = geode::Label::create("Played", "bigFont.fnt");
    playedScrollLabel->setScale(.55f);
    playedScrollLabel->setPosition(m_playedScrollPageLabel->getPosition() + ccp(0, m_playedScrollPageLabel->getScaledContentHeight() / 2 + playedScrollLabel->getScaledContentHeight() / 2 + 5));
    this->addChild(playedScrollLabel);

    auto playedPageForwardSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    playedPageForwardSpr->setScale(.6f);
    auto playedPageForward = CCMenuItemSpriteExtra::create(
        playedPageForwardSpr,
        this,
        menu_selector(LinkOptions::onSwitchedPage)
    );
    playedPageForward->setTag(1);
    playedPageForward->setID("played");
    playedPageForward->setPosition(m_playedScrollPageLabel->getPosition() + ccp(arrowOffset, 0));
    playedPageForward->setRotation(180);
    this->addChild(playedPageForward);

    auto playedPageBackwardSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    playedPageBackwardSpr->setScale(.6);
    auto playedPageBackward = CCMenuItemSpriteExtra::create(
        playedPageBackwardSpr,
        this,
        menu_selector(LinkOptions::onSwitchedPage)
    );
    playedPageBackward->setTag(-1);
    playedPageBackward->setID("played");
    playedPageBackward->setPosition(m_playedScrollPageLabel->getPosition() - ccp(arrowOffset, 0));
    this->addChild(playedPageBackward);

    m_searchBar = TextInput::create(m_size.width / 1.5f, "Search", "bigFont.fnt");
    m_searchBar->setScale(.75f);
    m_searchBar->setPosition(ccp(m_size.width / 2, m_size.height - m_searchBar->getScaledContentHeight() / 2 - 1));
    m_searchBar->setCallback([&](const auto& newStr) {
        m_searchStr = newStr;

        if (m_levelsLoaded) updateScrollsContent(m_cachedLevels);
    });
    this->addChild(m_searchBar);

    auto idToggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .6f
    );
    idToggler->setPosition(m_searchBar->getPosition() + ccp(m_searchBar->getScaledContentWidth() / 2 + idToggler->getScaledContentWidth() / 2 + 5, 0));
    idToggler->setCallback([&](bool toggled) {
        m_searchForID = toggled;

        if (m_levelsLoaded && !m_searchStr.empty())
            updateScrollsContent(m_cachedLevels);
    });
    this->addChild(idToggler);

    auto idTogglerLabel = geode::Label::create("ID", "bigFont.fnt");
    idTogglerLabel->setScale(.5f);
    idTogglerLabel->setPosition(idToggler->getPosition() + ccp(idToggler->getScaledContentWidth() / 2 + idTogglerLabel->getScaledContentWidth() / 2 + 5, 0));
    this->addChild(idTogglerLabel);

    auto parentLayer = CCLayer::create();
    parentLayer->ignoreAnchorPointForPosition(false);
    parentLayer->setContentSize({0, 0});
    this->addChild(parentLayer);

    m_loadingShadow = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_loadingShadow->setColor({0,0,0});
    m_loadingShadow->setOpacity(100);
    m_loadingShadow->setAnchorPoint({0,0});
    m_loadingShadow->setContentSize({m_size.width, m_linkedScroll->getContentHeight()});
    parentLayer->addChild(m_loadingShadow);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(parentLayer);
    m_loadingCircle->ignoreAnchorPointForPosition(false);
    m_loadingCircle->setPosition(m_linkedScroll->getPosition() + m_linkedScroll->getContentSize() + ccp(5, -m_linkedScroll->getContentHeight() / 4));
    m_loadingCircle->show();

    m_loadingLabel = geode::Label::create("Loading levels...", "bigFont.fnt");
    m_loadingLabel->setScale(.75f);
    m_loadingLabel->setPosition(m_loadingCircle->getPosition() - ccp(0, m_loadingCircle->m_sprite->getScaledContentHeight() / 2 + m_loadingLabel->getScaledContentHeight() / 2 + 5));
    m_loadingLabel->setAlignment(geode::Label::Alignment::Center);
    parentLayer->addChild(m_loadingLabel);

    auto linkInfo = TutorialButton::create(.75f, "link-overall", [
        &,
        playedScrollBG,
        linkedScrollBG,
        idToggler,
        idTogglerLabel
    ](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("In this menu you can link your progress between levels!", TutorialCharacterFace::TCFHappy)
            ->appendDialogue("Here you have a list of all the levels you <cy>have played</c> and tracked", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(playedScrollBG)
            ->joinTransform(TutorialBoxPlacement::TBPTop, .75f)
            ->appendDialogue("And a list of all levels you have <cc>linked</c> already", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(linkedScrollBG)
            ->appendDialogue("Each levels has <cg>arrow</c> buttons to move that level <cy>between the two lists</c>", TutorialCharacterFace::TCFNormalTilted);
        for (const auto& child : m_playedScroll->m_contentLayer->getChildrenExt<CCMenu*>())
        {
            if (!child->isEnabled()) continue;

            tutorialLayer->joinHighlight(child->getChildByID("arrow"));
        }
        for (const auto& child : m_linkedScroll->m_contentLayer->getChildrenExt<CCMenu*>())
        {
            if (!child->isEnabled()) continue;
            
            tutorialLayer->joinHighlight(child->getChildByID("arrow"));
        }
        tutorialLayer->appendDialogue("Levels that are in the <cy>played</c> list arent linked", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(playedScrollBG)
            ->appendDialogue("And levels that are in the <cc>linked</c> list are linked!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(linkedScrollBG)
            ->appendDialogue("When viewing stats you will see the <cy>combined stats of all your linked levels</c>", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, 1)
            ->appendDialogue("You also have a <cf>search bar</c> to search for the name of a level you would like to link", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(m_searchBar)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->appendDialogue("You can also search by a <cf>level ID</c> when toggling the <cy>\"ID\" checkbox</c>", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(idToggler)
            ->joinHighlight(idTogglerLabel);
    });
    linkInfo->setPosition(m_size - linkInfo->getScaledContentSize() / 2 + ccp(2, 2));
    this->addChild(linkInfo);

    this->setEnabled(false);
    this->setOpacity(0);

    m_linkedScroll->getChildByID("bg")->runAction(CCFadeTo::create(0, 0));
    m_playedScroll->getChildByID("bg")->runAction(CCFadeTo::create(0, 0));

    Dev::fadeTextInput(m_searchBar, false, 0);

    for (const auto& child : CCArrayExt<CCMenu*>(m_linkedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->setOpacity(0);
    }
    for (const auto& child : CCArrayExt<CCMenu*>(m_playedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->setOpacity(0);
    }

    m_nonePlayedFoundText->runAction(CCTextAreaFadeTo::create(0, 0));
    m_noneLinkedFoundText->runAction(CCTextAreaFadeTo::create(0, 0));
    
    m_loadingLabel->setOpacity(0);
    m_loadingShadow->setOpacity(0);
    m_loadingCircle->m_sprite->setOpacity(0);

    m_loadingLabel->setVisible(false);
    m_loadingShadow->setVisible(false);
    m_loadingCircle->m_sprite->setVisible(false);

    scheduleUpdate();

    return true;
}

void LinkOptions::onOpened() {
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));

    m_linkedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 150));
    m_playedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 150));

    for (const auto& child : CCArrayExt<LevelLinkCell*>(m_linkedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(child->isEnabledAndFade());
        child->runAction(CCFadeTo::create(fadeTime, child->isEnabledAndFade() ? 255 : 120));
    }
    for (const auto& child : CCArrayExt<LevelLinkCell*>(m_playedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(child->isEnabledAndFade());
        child->runAction(CCFadeTo::create(fadeTime, child->isEnabledAndFade() ? 255 : 120));
    }

    m_playedScroll->setMouseEnabled(true);
    m_linkedScroll->setMouseEnabled(true);

    m_nonePlayedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 255));
    m_noneLinkedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 255));
    m_loadingLabel->runAction(CCFadeTo::create(fadeTime, 255));
    m_loadingShadow->runAction(CCFadeTo::create(fadeTime, 100));
    m_loadingCircle->m_sprite->runAction(CCFadeTo::create(fadeTime, 200));

    Dev::fadeTextInput(m_searchBar, true, fadeTime);

    if (!m_levelsLoaded && !m_levelsLoading)
        getAllLevelsData();
}
void LinkOptions::onClosed() {
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));

    m_linkedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 0));
    m_playedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 0));

    for (const auto& child : CCArrayExt<CCMenu*>(m_linkedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->runAction(CCFadeTo::create(fadeTime, 0));
    }
    for (const auto& child : CCArrayExt<CCMenu*>(m_playedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->runAction(CCFadeTo::create(fadeTime, 0));
    }

    m_playedScroll->setMouseEnabled(false);
    m_linkedScroll->setMouseEnabled(false);

    m_nonePlayedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 0));
    m_noneLinkedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 0));

    m_loadingLabel->runAction(CCFadeTo::create(fadeTime, 0));
    m_loadingShadow->runAction(CCFadeTo::create(fadeTime, 0));
    m_loadingCircle->m_sprite->runAction(CCFadeTo::create(fadeTime, 0));

    Dev::fadeTextInput(m_searchBar, false, fadeTime);
}

void LinkOptions::getAllLevelsData() {
    m_levelsLoading = true;
    m_loadingLabel->setVisible(true);
    m_loadingShadow->setVisible(true);
    m_loadingCircle->m_sprite->setVisible(true);

    auto callbackOwner = this->m_callbackOwner;
    LinkedLevelsCacheManager::getLinkedLevels(
        [callbackOwner](const GetLevelsTaskProgress& progress) {
            auto progressCopy = progress;
            geode::queueInMainThread([callbackOwner, progress = std::move(progressCopy)]() {
                if (auto* self = callbackOwner->load(std::memory_order_acquire))
                    self->onGetLevelsProgress(progress);
            });
        },
        [callbackOwner](GetLevelsFuture::Output const& val) {
            auto levels = val;
            geode::queueInMainThread([callbackOwner, levels = std::move(levels)]() {
                if (auto* self = callbackOwner->load(std::memory_order_acquire))
                    self->onGetLevels(levels);
            });
        }
    );
}

void LinkOptions::onGetLevelsProgress(const GetLevelsTaskProgress& progress) {
    m_loadingLabel->setString(fmt::format("{}\n{}/{}", progress.m_message, progress.m_current, progress.m_max).c_str());
}

void LinkOptions::onGetLevels(GetLevelsFuture::Output const& out) {
    m_levelsLoading = false;
    m_levelsLoaded = true;
    m_cachedLevels = out;

    m_loadingLabel->setVisible(false);
    m_loadingShadow->setVisible(false);
    m_loadingCircle->m_sprite->setVisible(false);

    updateScrollsContent(m_cachedLevels);
}

void LinkOptions::updateScrollsContent(GetLevelsFuture::Output const& levels) {
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;
    struct LevelMetaNameCompare {
        bool operator()(const LinkData* a, const LinkData* b) const {
            if (a->m_metadata.m_levelName != b->m_metadata.m_levelName)
                return a->m_metadata.m_levelName < b->m_metadata.m_levelName;
            return a->m_levelKey < b->m_levelKey;
        }
    };

    std::set<const LinkData*, LevelMetaNameCompare> linkedLevels{};
    std::set<const LinkData*, LevelMetaNameCompare> playedLevels{};

    auto& myStats = DTLayer::get()->m_MyLevelStats.unwrap();

    auto makeLower = [](std::string str) -> std::string {
        for (size_t i = 0; i < str.length(); ++i) {
            str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
        }
        return str;
    };

    for (const auto& meta : levels)
    {
        if (myStats.m_levelKey == meta.m_levelKey) continue;

        if (!m_searchStr.empty()) {
            if (!m_searchForID) {
                auto lowerSearchStr = makeLower(m_searchStr);
                auto lowerLevelName = makeLower(meta.m_metadata.m_levelName);

                if (lowerLevelName.find(lowerSearchStr) == std::string::npos)
                    continue;
            }
            else {
                if (meta.m_levelKey.find(m_searchStr) == std::string::npos)
                    continue;
            }
        }

        if (myStats.m_metadata.m_linkedLevels.contains(meta.m_levelKey))
            linkedLevels.insert(&meta);
        else
            playedLevels.insert(&meta);
    }

    m_maxLinkedPage = !linkedLevels.size() ? std::nullopt : std::make_optional(static_cast<unsigned int>(static_cast<float>(linkedLevels.size()) / LinkOptions::PER_PAGE_COUNT + 0.99f));
    m_maxPlayedPage = !playedLevels.size() ? std::nullopt : std::make_optional(static_cast<unsigned int>(static_cast<float>(playedLevels.size()) / LinkOptions::PER_PAGE_COUNT + 0.99f));
    m_currentLinkedPage = m_maxLinkedPage.has_value() ? std::min(m_currentLinkedPage, m_maxLinkedPage.value() - 1) : 0;
    m_currentPlayedPage = m_maxPlayedPage.has_value() ? std::min(m_currentPlayedPage, m_maxPlayedPage.value() - 1) : 0;

    m_nonePlayedFoundText->setVisible(!playedLevels.size());
    m_noneLinkedFoundText->setVisible(!linkedLevels.size());
    
    m_linkedScroll->m_contentLayer->removeAllChildrenWithCleanup(true);
    m_playedScroll->m_contentLayer->removeAllChildrenWithCleanup(true);

    if (m_currentLinkedPage == 0) {
        auto cell = LevelLinkCell::create(m_linkedScroll->m_contentLayer->getContentWidth(), myStats.m_levelKey, myStats.m_metadata);
        cell->setAlignment(true);
        cell->setEnabledAndFade(false);
        if (!this->isEnabled()) {
            cell->setEnabled(false);
            cell->setOpacity(0);
        }
        
        m_linkedScroll->m_contentLayer->addChild(cell, -1);
    }

    auto addCellsAsChildren = [&](CCNode* parent, const std::set<const LinkData*, LevelMetaNameCompare>& setToUse, int pageNum, bool alignToLeft) {
        auto it = setToUse.begin();
        std::advance(it, pageNum * LinkOptions::PER_PAGE_COUNT);

        if (it == setToUse.end()) return;

        int z = 0;

        for (int i = 0; i < LinkOptions::PER_PAGE_COUNT; i++)
        {
            auto cell = LevelLinkCell::create(parent->getContentWidth(), (**it).m_levelKey, (**it).m_metadata);
            cell->setCallback([&](const std::string& levelKey, bool isLeftAligned) {onLinkCellClicked(levelKey, isLeftAligned);});
            
            cell->setAlignment(alignToLeft);

            if (!this->isEnabled()) {
                cell->setEnabled(false);
                cell->setOpacity(0);
            }
            
            parent->addChild(cell, z);
            z++;
            std::advance(it, 1);
            if (it == setToUse.end()) break;
        }
    };

    addCellsAsChildren(m_linkedScroll->m_contentLayer, linkedLevels, m_currentLinkedPage, true);
    addCellsAsChildren(m_playedScroll->m_contentLayer, playedLevels, m_currentPlayedPage, false);

    m_linkedScroll->m_contentLayer->updateLayout();
    m_playedScroll->m_contentLayer->updateLayout();
    m_linkedScroll->moveToTop();
    m_playedScroll->moveToTop();

    updatePageLabels();
}

void LinkOptions::onLinkCellClicked(const std::string& levelKey, bool isLeftAligned) {
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;

    // log::info("[LinkOptions] onLinkCellClicked: {} (isLeftAligned: {})", levelKey, isLeftAligned);

    if (!isLeftAligned) { // link
        auto& myData = DTLayer::get()->m_MyLevelStats.unwrap();
        myData.m_metadata.m_linkedLevels.insert(levelKey);
        
        DTLayer::get()->UpdateSharedStats();
        
        (void)StatsManager::setMetadata(myData.m_metadata, myData.m_levelKey);

        for (auto& other : DTLayer::get()->m_linkedLevelsData) {
            if (other.m_levelKey == myData.m_levelKey) continue;
            bool changed = false;

            for (const auto& k : myData.m_metadata.m_linkedLevels) {
                if (k == other.m_levelKey) continue;
                if (!other.m_metadata.m_linkedLevels.contains(k)) {
                    other.m_metadata.m_linkedLevels.insert(k);
                    changed = true;
                }
            }

            if (other.m_levelKey == levelKey) {
                other.m_metadata.m_linkedLevels.insert(myData.m_levelKey);
                changed = true;
            }

            if (changed) {
                (void)StatsManager::setMetadata(other.m_metadata, other.m_levelKey);
            }
        }

        DTLayer::get()->foreachLinkedLevel([&myData](LevelData& linkedLevel) {
            linkedLevel.m_metadata.m_showAnyRun = myData.m_metadata.m_showAnyRun;
            linkedLevel.m_metadata.m_runsToShow = myData.m_metadata.m_runsToShow;

            linkedLevel.m_metadata.m_sections = myData.m_metadata.m_sections;

            (void)StatsManager::setMetadata(linkedLevel.m_metadata, linkedLevel.m_levelKey);
        });
    }
    else { // unlink
        auto& myData = DTLayer::get()->m_MyLevelStats.unwrap();
        myData.m_metadata.m_linkedLevels.erase(levelKey);

        std::set<std::string> toRemoveFromTarget{};

        LevelData* targetToRemove = nullptr;

        for (auto& other : DTLayer::get()->m_linkedLevelsData) {
            if (other.m_metadata.m_linkedLevels.contains(levelKey)) {
                other.m_metadata.m_linkedLevels.erase(levelKey);

                toRemoveFromTarget.insert(other.m_levelKey);

                (void)StatsManager::setMetadata(other.m_metadata, other.m_levelKey);
            }

            if (other.m_levelKey == levelKey) {
                targetToRemove = &other;
            }
        }

        if (targetToRemove != nullptr) {
            for (const auto& lvlKey : toRemoveFromTarget)
            {
                targetToRemove->m_metadata.m_linkedLevels.erase(lvlKey);
            }

            (void)StatsManager::setMetadata(targetToRemove->m_metadata, targetToRemove->m_levelKey);
        }

        DTLayer::get()->UpdateSharedStats();

        (void)StatsManager::setMetadata(myData.m_metadata, myData.m_levelKey);
    }

    if (m_levelsLoaded) updateScrollsContent(m_cachedLevels);
}

void LinkOptions::onSwitchedPage(CCObject* sender) {
    if (auto nodeSender = typeinfo_cast<CCNode*>(sender)) {
        auto amount = nodeSender->getTag();

        if (nodeSender->getID() == "link") {
            m_currentLinkedPage += amount;
            
            if (m_maxLinkedPage.has_value() && m_currentLinkedPage == m_maxLinkedPage.value()) m_currentLinkedPage = 0;
            
            m_currentLinkedPage = std::clamp(m_currentLinkedPage, static_cast<unsigned int>(0), m_maxLinkedPage.has_value() ? m_maxLinkedPage.value() - 1 : 0);
        }
        else if (nodeSender->getID() == "played") {
            m_currentPlayedPage += amount;
                        
            if (m_maxPlayedPage.has_value() && m_currentPlayedPage == m_maxPlayedPage.value()) m_currentPlayedPage = 0;
        
            m_currentPlayedPage = std::clamp(m_currentPlayedPage, static_cast<unsigned int>(0), m_maxPlayedPage.has_value() ? m_maxPlayedPage.value() - 1 : 0);
        }

        if (m_levelsLoaded) updateScrollsContent(m_cachedLevels);
    }
}

void LinkOptions::updatePageLabels() {
    m_linkedScrollPageLabel->setString(fmt::format("{}/{}", m_maxLinkedPage.has_value() ? m_currentLinkedPage + 1 : 0, m_maxLinkedPage.has_value() ? m_maxLinkedPage.value() : 0).c_str());
    m_playedScrollPageLabel->setString(fmt::format("{}/{}", m_maxPlayedPage.has_value() ? m_currentPlayedPage + 1 : 0, m_maxPlayedPage.has_value() ? m_maxPlayedPage.value() : 0).c_str());
}

void LinkOptions::update(float dt) {
    if (!this->isEnabled()) {
        m_linkedScroll->setMouseEnabled(false);
        m_playedScroll->setMouseEnabled(false);

        return;
    }
    auto mousePos = getMousePos();

    CCRect playedScrollRect = {m_playedScroll->getPositionX(), m_playedScroll->getPositionY(), m_playedScroll->getScaledContentSize().width, m_playedScroll->getScaledContentSize().height};

    CCRect linkedScrollRect = {m_linkedScroll->getPositionX(), m_linkedScroll->getPositionY(), m_linkedScroll->getScaledContentSize().width, m_linkedScroll->getScaledContentSize().height};

    if (linkedScrollRect.containsPoint(m_linkedScroll->getParent()->convertToNodeSpace(mousePos))) {
        m_linkedScroll->setMouseEnabled(true);
        m_playedScroll->setMouseEnabled(false);
    }
    else if (playedScrollRect.containsPoint(m_playedScroll->getParent()->convertToNodeSpace(mousePos))) {
        m_linkedScroll->setMouseEnabled(false);
        m_playedScroll->setMouseEnabled(true);
    }
}