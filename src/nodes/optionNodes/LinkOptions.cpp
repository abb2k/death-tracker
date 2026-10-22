#include <nodes/optionNodes/LinkOptions.hpp>
#include <managers/StatsManager.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/LevelLinkCell.hpp>
#include <utils/Dev.hpp>
#include <utils/CCTextAreaFadeTo.hpp>
#include <arc/task/Yield.hpp>

const int LinkOptions::PER_PAGE_COUNT = 10;

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

bool LinkOptions::setup() {

    linkedScroll = ScrollLayer::create({size.width / 2 - 5, size.height - 70});
    auto linkedScrollBG = CCScale9Sprite::create("square02_small.png");
    linkedScrollBG->setContentSize(linkedScroll->getContentSize());
    linkedScrollBG->setZOrder(-1);
    linkedScrollBG->ignoreAnchorPointForPosition(true);
    linkedScrollBG->setID("bg");
    linkedScroll->addChild(linkedScrollBG);
    linkedScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(linkedScroll->getContentHeight())
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)
    );
    linkedScroll->setMouseEnabled(false);
    this->addChild(linkedScroll);
    
    playedScroll = ScrollLayer::create({size.width / 2 - 5, size.height - 70});
    auto playedScrollBG = CCScale9Sprite::create("square02_small.png");
    playedScrollBG->setContentSize(playedScroll->getContentSize());
    playedScrollBG->setZOrder(-1);
    playedScrollBG->ignoreAnchorPointForPosition(true);
    playedScrollBG->setID("bg");
    playedScroll->addChild(playedScrollBG);
    playedScroll->setPositionX(linkedScroll->getContentWidth() + 10);
    playedScroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(playedScroll->getContentHeight())
        ->setAxisReverse(true)
        ->setAxisAlignment(AxisAlignment::End)
    );
    playedScroll->setMouseEnabled(false);
    this->addChild(playedScroll);

    nonePlayedFoundText = SimpleTextArea::create("No unlinked played levels found!", "bigFont.fnt", .5f);
    nonePlayedFoundText->setPosition(playedScroll->getPosition() + playedScroll->getContentSize() / 2);
    nonePlayedFoundText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    nonePlayedFoundText->setWidth(playedScroll->getContentWidth() / 1.2f);
    nonePlayedFoundText->setWrappingMode(WrappingMode::SPACE_WRAP);
    this->addChild(nonePlayedFoundText);
    
    noneLinkedFoundText = SimpleTextArea::create("No linked levels found!", "bigFont.fnt", .5f);
    noneLinkedFoundText->setPosition(linkedScroll->getPosition() + linkedScroll->getContentSize() / 2);
    noneLinkedFoundText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    noneLinkedFoundText->setWidth(linkedScroll->getContentWidth() / 1.2f);
    noneLinkedFoundText->setWrappingMode(WrappingMode::SPACE_WRAP);
    this->addChild(noneLinkedFoundText);

    linkedScrollPageLabel = CCLabelBMFont::create("0/0", "bigFont.fnt");
    linkedScrollPageLabel->setScale(.55f);
    linkedScrollPageLabel->setPosition(linkedScroll->getPosition() + ccp(linkedScroll->getContentWidth() / 2, linkedScroll->getContentHeight() + linkedScrollPageLabel->getScaledContentHeight() / 2 + 5));
    this->addChild(linkedScrollPageLabel);

    auto linkedScrollLabel = CCLabelBMFont::create("Linked", "bigFont.fnt");
    linkedScrollLabel->setScale(.55f);
    linkedScrollLabel->setPosition(linkedScrollPageLabel->getPosition() + ccp(0, linkedScrollPageLabel->getScaledContentHeight() / 2 + linkedScrollLabel->getScaledContentHeight() / 2 + 5));
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
    linkPageForward->setPosition(linkedScrollPageLabel->getPosition() + ccp(arrowOffset, 0));
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
    linkPageBackward->setPosition(linkedScrollPageLabel->getPosition() - ccp(arrowOffset, 0));
    this->addChild(linkPageBackward);

    playedScrollPageLabel = CCLabelBMFont::create("0/0", "bigFont.fnt");
    playedScrollPageLabel->setScale(.55f);
    playedScrollPageLabel->setPosition(playedScroll->getPosition() + ccp(playedScroll->getContentWidth() / 2, playedScroll->getContentHeight() + playedScrollPageLabel->getScaledContentHeight() / 2 + 5));
    this->addChild(playedScrollPageLabel);

    auto playedScrollLabel = CCLabelBMFont::create("Played", "bigFont.fnt");
    playedScrollLabel->setScale(.55f);
    playedScrollLabel->setPosition(playedScrollPageLabel->getPosition() + ccp(0, playedScrollPageLabel->getScaledContentHeight() / 2 + playedScrollLabel->getScaledContentHeight() / 2 + 5));
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
    playedPageForward->setPosition(playedScrollPageLabel->getPosition() + ccp(arrowOffset, 0));
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
    playedPageBackward->setPosition(playedScrollPageLabel->getPosition() - ccp(arrowOffset, 0));
    this->addChild(playedPageBackward);

    searchBar = TextInput::create(size.width / 1.5f, "Search", "bigFont.fnt");
    searchBar->setScale(.75f);
    searchBar->setPosition(ccp(size.width / 2, size.height - searchBar->getScaledContentHeight() / 2 - 1));
    searchBar->setCallback([&](const auto& newStr) {
        searchStr = newStr;

        updateScrollsContent();
    });
    this->addChild(searchBar);

    auto idToggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .6f
    );
    idToggler->setPosition(searchBar->getPosition() + ccp(searchBar->getScaledContentWidth() / 2 + idToggler->getScaledContentWidth() / 2 + 5, 0));
    idToggler->setCallback([&](bool toggled) {
        searchForID = toggled;

        if (searchStr != "")
            updateScrollsContent();
    });
    this->addChild(idToggler);

    auto idTogglerLabel = CCLabelBMFont::create("ID", "bigFont.fnt");
    idTogglerLabel->setScale(.5f);
    idTogglerLabel->setPosition(idToggler->getPosition() + ccp(idToggler->getScaledContentWidth() / 2 + idTogglerLabel->getScaledContentWidth() / 2 + 5, 0));
    this->addChild(idTogglerLabel);

    auto parentLayer = CCLayer::create();
    parentLayer->ignoreAnchorPointForPosition(false);
    parentLayer->setContentSize({0, 0});
    this->addChild(parentLayer);

    loadingShadow = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    loadingShadow->setColor({0,0,0});
    loadingShadow->setOpacity(100);
    loadingShadow->setAnchorPoint({0,0});
    loadingShadow->setContentSize({size.width, linkedScroll->getContentHeight()});
    parentLayer->addChild(loadingShadow);

    loadingCircle = LoadingCircle::create();
    loadingCircle->setParentLayer(parentLayer);
    loadingCircle->ignoreAnchorPointForPosition(false);
    loadingCircle->setPosition(linkedScroll->getPosition() + linkedScroll->getContentSize() + ccp(5, -linkedScroll->getContentHeight() / 4));
    loadingCircle->show();

    loadingLabel = CCLabelBMFont::create("Loading levels...", "bigFont.fnt");
    loadingLabel->setScale(.75f);
    loadingLabel->setPosition(loadingCircle->getPosition() - ccp(0, loadingCircle->m_sprite->getScaledContentHeight() / 2 + loadingLabel->getScaledContentHeight() / 2 + 5));
    loadingLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    parentLayer->addChild(loadingLabel);

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
        for (const auto& child : playedScroll->m_contentLayer->getChildrenExt<CCMenu*>())
        {
            if (!child->isEnabled()) continue;

            tutorialLayer->joinHighlight(child->getChildByID("arrow"));
        }
        for (const auto& child : linkedScroll->m_contentLayer->getChildrenExt<CCMenu*>())
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
            ->joinHighlight(searchBar)
            ->joinTransform(TutorialBoxPlacement::TBPCenter, .75f)
            ->appendDialogue("You can also search by a <cf>level ID</c> when toggling the <cy>\"ID\" checkbox</c>", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(idToggler)
            ->joinHighlight(idTogglerLabel);
    });
    linkInfo->setPosition(size - linkInfo->getScaledContentSize() / 2 + ccp(2, 2));
    this->addChild(linkInfo);

    this->setEnabled(false);
    this->setOpacity(0);

    linkedScroll->getChildByID("bg")->runAction(CCFadeTo::create(0, 0));
    playedScroll->getChildByID("bg")->runAction(CCFadeTo::create(0, 0));

    Dev::fadeTextInput(searchBar, false, 0);

    for (const auto& child : CCArrayExt<CCMenu*>(linkedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->setOpacity(0);
    }
    for (const auto& child : CCArrayExt<CCMenu*>(playedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->setOpacity(0);
    }

    nonePlayedFoundText->runAction(CCTextAreaFadeTo::create(0, 0));
    noneLinkedFoundText->runAction(CCTextAreaFadeTo::create(0, 0));
    
    loadingLabel->setOpacity(0);
    loadingShadow->setOpacity(0);
    loadingCircle->m_sprite->setOpacity(0);

    loadingLabel->setVisible(false);
    loadingShadow->setVisible(false);
    loadingCircle->m_sprite->setVisible(false);

    scheduleUpdate();

    return true;
}

void LinkOptions::onOpened() {
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));

    linkedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 150));
    playedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 150));

    for (const auto& child : CCArrayExt<LevelLinkCell*>(linkedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(child->isEnabledAndFade());
        child->runAction(CCFadeTo::create(fadeTime, child->isEnabledAndFade() ? 255 : 120));
    }
    for (const auto& child : CCArrayExt<LevelLinkCell*>(playedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(child->isEnabledAndFade());
        child->runAction(CCFadeTo::create(fadeTime, child->isEnabledAndFade() ? 255 : 120));
    }

    playedScroll->setMouseEnabled(true);
    linkedScroll->setMouseEnabled(true);

    nonePlayedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 255));
    noneLinkedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 255));
    loadingLabel->runAction(CCFadeTo::create(fadeTime, 255));
    loadingShadow->runAction(CCFadeTo::create(fadeTime, 100));
    loadingCircle->m_sprite->runAction(CCFadeTo::create(fadeTime, 200));

    Dev::fadeTextInput(searchBar, true, fadeTime);

    getAllLevelsData();
}
void LinkOptions::onClosed() {
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));

    linkedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 0));
    playedScroll->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 0));

    for (const auto& child : CCArrayExt<CCMenu*>(linkedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->runAction(CCFadeTo::create(fadeTime, 0));
    }
    for (const auto& child : CCArrayExt<CCMenu*>(playedScroll->m_contentLayer->getChildren())) {
        child->setEnabled(false);
        child->runAction(CCFadeTo::create(fadeTime, 0));
    }

    playedScroll->setMouseEnabled(false);
    linkedScroll->setMouseEnabled(false);

    nonePlayedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 0));
    noneLinkedFoundText->runAction(CCTextAreaFadeTo::create(fadeTime, 0));

    loadingLabel->runAction(CCFadeTo::create(fadeTime, 0));
    loadingShadow->runAction(CCFadeTo::create(fadeTime, 0));
    loadingCircle->m_sprite->runAction(CCFadeTo::create(fadeTime, 0));

    Dev::fadeTextInput(searchBar, false, fadeTime);
}

void LinkOptions::getAllLevelsData() {
    if (getallLevelsListener.isPending() || data.size()) return;

    loadingLabel->setVisible(true);
    loadingShadow->setVisible(true);
    loadingCircle->m_sprite->setVisible(true);

    getallLevelsListener.spawn(
        "DT-get-all-levels-task",
        getLevelsFuture(),
        [&](GetLevelsFuture::Output val) {
            this->onGetLevels(val);
        }
    );
}

void LinkOptions::onGetLevelsProgress(const GetLevelsTaskProgress& progress) {
    geode::queueInMainThread([&, progress]() {
        loadingLabel->setString(fmt::format("{}\n{}/{}", progress.message, progress.current, progress.max).c_str());
    });
}

void LinkOptions::onGetLevels(GetLevelsFuture::Output out) {
    data = std::move(out);

    loadingLabel->setVisible(false);
    loadingShadow->setVisible(false);
    loadingCircle->m_sprite->setVisible(false);

    updateScrollsContent();
}

GetLevelsFuture LinkOptions::getLevelsFuture() {
    auto progresObject = GetLevelsTaskProgress{"Getting list of played levels...", -1, -1};

    onGetLevelsProgress(progresObject);

    auto allLevels = StatsManager::allV3FileLevelKeys();
    if (allLevels.empty()) co_return std::vector<LinkData>{};

    progresObject.message = "Getting level data...";
    progresObject.current = 0;
    progresObject.max = static_cast<int>(allLevels.size());

    onGetLevelsProgress(progresObject);

    std::vector<LinkData> metadatas{};
    metadatas.reserve(allLevels.size());

    int i = 0;
    for (const auto& levelKey : allLevels)
    {
        auto meta = StatsManager::getMetadata(levelKey);
        if (meta.isOk()) {
            LinkData ldata{};
            ldata.metadata = meta.unwrap();
            ldata.levelKey = levelKey;

            metadatas.emplace_back(std::move(ldata));
        }

        co_await arc::yield();

        i++;
        progresObject.current = i;
        onGetLevelsProgress(progresObject);
    }
    
    co_return metadatas;
}

void LinkOptions::updateScrollsContent() {
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;
    struct LevelMetaNameCompare {
        bool operator()(const LinkData* a, const LinkData* b) const {
            if (a->metadata.levelName != b->metadata.levelName)
                return a->metadata.levelName < b->metadata.levelName;
            return a->levelKey < b->levelKey;
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

    for (const auto& meta : data)
    {
        if (myStats.levelKey == meta.levelKey) continue;

        if (!searchStr.empty()) {
            if (!searchForID) {
                auto lowerSearchStr = makeLower(searchStr);
                auto lowerLevelName = makeLower(meta.metadata.levelName);

                if (lowerLevelName.find(lowerSearchStr) == std::string::npos)
                    continue;
            }
            else {
                if (meta.levelKey.find(searchStr) == std::string::npos)
                    continue;
            }
        }

        if (myStats.metadata.linkedLevels.contains(meta.levelKey))
            linkedLevels.insert(&meta);
        else
            playedLevels.insert(&meta);
    }

    maxLinkedPage = !linkedLevels.size() ? std::nullopt : std::make_optional(static_cast<unsigned int>(static_cast<float>(linkedLevels.size()) / LinkOptions::PER_PAGE_COUNT + 0.99f));
    maxPlayedPage = !playedLevels.size() ? std::nullopt : std::make_optional(static_cast<unsigned int>(static_cast<float>(playedLevels.size()) / LinkOptions::PER_PAGE_COUNT + 0.99f));

    nonePlayedFoundText->setVisible(!playedLevels.size());
    noneLinkedFoundText->setVisible(!linkedLevels.size());
    
    linkedScroll->m_contentLayer->removeAllChildrenWithCleanup(true);
    playedScroll->m_contentLayer->removeAllChildrenWithCleanup(true);

    if (currentLinkedPage == 0) {
        auto cell = LevelLinkCell::create(linkedScroll->m_contentLayer->getContentWidth(), myStats.levelKey, myStats.metadata);    
        cell->setAlignment(true);
        cell->setEnabledAndFade(false);
        if (!this->isEnabled()) {
            cell->setEnabled(false);
            cell->setOpacity(0);
        }
        
        linkedScroll->m_contentLayer->addChild(cell, -1);
    }

    auto addCellsAsChildren = [&](CCNode* parent, const std::set<const LinkData*, LevelMetaNameCompare>& setToUse, int pageNum, bool alignToLeft) {
        auto it = setToUse.begin();
        std::advance(it, pageNum * LinkOptions::PER_PAGE_COUNT);

        if (it == setToUse.end()) return;

        int z = 0;

        for (int i = 0; i < LinkOptions::PER_PAGE_COUNT; i++)
        {
            auto cell = LevelLinkCell::create(parent->getContentWidth(), (**it).levelKey, (**it).metadata);
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

    addCellsAsChildren(linkedScroll->m_contentLayer, linkedLevels, currentLinkedPage, true);
    addCellsAsChildren(playedScroll->m_contentLayer, playedLevels, currentPlayedPage, false);

    linkedScroll->m_contentLayer->updateLayout();
    playedScroll->m_contentLayer->updateLayout();
    linkedScroll->moveToTop();
    playedScroll->moveToTop();

    updatePageLabels();
}

void LinkOptions::onLinkCellClicked(const std::string& levelKey, bool isLeftAligned) {
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;

    // log::info("[LinkOptions] onLinkCellClicked: {} (isLeftAligned: {})", levelKey, isLeftAligned);

    if (!isLeftAligned) { // link
        auto& myData = DTLayer::get()->m_MyLevelStats.unwrap();
        myData.metadata.linkedLevels.insert(levelKey);
        
        DTLayer::get()->UpdateSharedStats();
        
        (void)StatsManager::setMetadata(myData.metadata, myData.levelKey);

        for (auto& other : DTLayer::get()->linkedLevelsData) {
            if (other.levelKey == myData.levelKey) continue;
            bool changed = false;

            for (const auto& k : myData.metadata.linkedLevels) {
                if (k == other.levelKey) continue;
                if (!other.metadata.linkedLevels.contains(k)) {
                    other.metadata.linkedLevels.insert(k);
                    changed = true;
                }
            }

            if (other.levelKey == levelKey) {
                other.metadata.linkedLevels.insert(myData.levelKey);
                changed = true;
            }

            if (changed) {
                (void)StatsManager::setMetadata(other.metadata, other.levelKey);
            }
        }

        DTLayer::get()->foreachLinkedLevel([&myData](LevelData& linkedLevel) {
            linkedLevel.metadata.showAnyRun = myData.metadata.showAnyRun;
            linkedLevel.metadata.runsToShow = myData.metadata.runsToShow;

            linkedLevel.metadata.sections = myData.metadata.sections;

            (void)StatsManager::setMetadata(linkedLevel.metadata, linkedLevel.levelKey);
        });
    }
    else { // unlink
        auto& myData = DTLayer::get()->m_MyLevelStats.unwrap();
        myData.metadata.linkedLevels.erase(levelKey);

        std::set<std::string> toRemoveFromTarget{};

        LevelData* targetToRemove = nullptr;

        for (auto& other : DTLayer::get()->linkedLevelsData) {
            if (other.metadata.linkedLevels.contains(levelKey)) {
                other.metadata.linkedLevels.erase(levelKey);

                toRemoveFromTarget.insert(other.levelKey);

                (void)StatsManager::setMetadata(other.metadata, other.levelKey);
            }

            if (other.levelKey == levelKey) {
                targetToRemove = &other;
            }
        }

        if (targetToRemove != nullptr) {
            for (const auto& lvlKey : toRemoveFromTarget)
            {
                targetToRemove->metadata.linkedLevels.erase(lvlKey);
            }

            (void)StatsManager::setMetadata(targetToRemove->metadata, targetToRemove->levelKey);
        }

        DTLayer::get()->UpdateSharedStats();

        (void)StatsManager::setMetadata(myData.metadata, myData.levelKey);
    }

    updateScrollsContent();
}

void LinkOptions::onSwitchedPage(CCObject* sender) {
    if (auto nodeSender = typeinfo_cast<CCNode*>(sender)) {
        auto amount = nodeSender->getTag();

        if (nodeSender->getID() == "link") {
            currentLinkedPage += amount;
            
            if (maxLinkedPage.has_value() && currentLinkedPage == maxLinkedPage.value()) currentLinkedPage = 0;
            
            currentLinkedPage = std::clamp(currentLinkedPage, static_cast<unsigned int>(0), maxLinkedPage.has_value() ? maxLinkedPage.value() - 1 : 0);
        }
        else if (nodeSender->getID() == "played") {
            currentPlayedPage += amount;
                        
            if (maxPlayedPage.has_value() && currentPlayedPage == maxPlayedPage.value()) currentPlayedPage = 0;
        
            currentPlayedPage = std::clamp(currentPlayedPage, static_cast<unsigned int>(0), maxPlayedPage.has_value() ? maxPlayedPage.value() - 1 : 0);
        }

        updateScrollsContent();
    }
}

void LinkOptions::updatePageLabels() {
    linkedScrollPageLabel->setString(fmt::format("{}/{}", maxLinkedPage.has_value() ? currentLinkedPage + 1 : 0, maxLinkedPage.has_value() ? maxLinkedPage.value() : 0).c_str());
    playedScrollPageLabel->setString(fmt::format("{}/{}", maxPlayedPage.has_value() ? currentPlayedPage + 1 : 0, maxPlayedPage.has_value() ? maxPlayedPage.value() : 0).c_str());
}

void LinkOptions::update(float dt) {
    if (!this->isEnabled()) {
        linkedScroll->setMouseEnabled(false);
        playedScroll->setMouseEnabled(false);

        return;
    }
    auto mousePos = getMousePos();

    CCRect playedScrollRect = {playedScroll->getPositionX(), playedScroll->getPositionY(), playedScroll->getScaledContentSize().width, playedScroll->getScaledContentSize().height};

    CCRect linkedScrollRect = {linkedScroll->getPositionX(), linkedScroll->getPositionY(), linkedScroll->getScaledContentSize().width, linkedScroll->getScaledContentSize().height};

    if (linkedScrollRect.containsPoint(linkedScroll->getParent()->convertToNodeSpace(mousePos))) {
        linkedScroll->setMouseEnabled(true);
        playedScroll->setMouseEnabled(false);
    }
    else if (playedScrollRect.containsPoint(playedScroll->getParent()->convertToNodeSpace(mousePos))) {
        linkedScroll->setMouseEnabled(false);
        playedScroll->setMouseEnabled(true);
    }
}