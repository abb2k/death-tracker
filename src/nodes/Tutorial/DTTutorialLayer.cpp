#include "DTTutorialLayer.hpp"

#include <hooks/DTDialogLayer.hpp>
#include <timestepyt.deltarune_textboxes/API.hpp>
#include <timestepyt.heisbehindthetree/api/API.hpp>

std::vector<DTTutorialLayer::TutorialPortrait> const& DTTutorialLayer::tutorialPortraits() {
    static std::vector<TutorialPortrait> const portraits = {
        {"normal.png"_spr, static_cast<int>(TutorialCharacterFace::TCFNormal), 2},
        {"tiltedNormal.png"_spr, static_cast<int>(TutorialCharacterFace::TCFNormalTilted), 2.15f},
        {"happy.png"_spr, static_cast<int>(TutorialCharacterFace::TCFHappy), 2.15f},
        {"angry.png"_spr, static_cast<int>(TutorialCharacterFace::TCFAngry), 2},
        {"worried.png"_spr, static_cast<int>(TutorialCharacterFace::TCFWorried), 2},
        {"sleeping.png"_spr, static_cast<int>(TutorialCharacterFace::TCFSleeping), 2},
        {"bored.png"_spr, static_cast<int>(TutorialCharacterFace::TCFBored), 2}
    };
    return portraits;
}

DTTutorialLayer* DTTutorialLayer::create() {
    auto ret = new DTTutorialLayer();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool DTTutorialLayer::init() {
    CCTouchDispatcher::get()->addPrioTargetedDelegate(this, 1000, true);
    if (!CCLayer::init()) return false;

    auto winSize = CCDirector::get()->getWinSize();

    m_lightRT = CCRenderTexture::create(winSize.width, winSize.height, CCTexture2DPixelFormat::kCCTexture2DPixelFormat_RGBA8888);
    m_lightRT->setPosition(winSize / 2);
    m_lightRT->getSprite()->setOpacity(0);
    this->addChild(m_lightRT);

    auto skipButtonSpr = ButtonSprite::create("Skip Tutorial");
    skipButtonSpr->setScale(.5f);
    skipButtonSpr->setCascadeOpacityEnabled(true);
    m_skipButton = CCMenuItemSpriteExtra::create(
        skipButtonSpr,
        this,
        menu_selector(DTTutorialLayer::skipTutorial)
    );
    m_skipButton->setAnchorPoint({1, 0});
    m_skipButton->setOpacity(0);
    m_skipButton->runAction(CCFadeIn::create(.5f));

    m_skipMenu = CCMenu::create();
    this->addChild(m_skipMenu);

    m_skipMenu->addChildAtPosition(m_skipButton, Anchor::BottomRight);

    scheduleUpdate();
    this->setMouseEnabled(true);
    this->setKeyboardEnabled(true);
    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);

    return true;
}

DTTutorialLayer* DTTutorialLayer::appendDialogue(const std::string& text, TutorialCharacterFace face, const ccColor3B& textColor, float textSize) {
    int idOfFace = static_cast<int>(face);
    
    auto dObj = DialogObject::create("abb2k", text, 1, textSize, false, textColor);
    dObj->setTag(idOfFace);

    return appendDialogue(dObj);
}

DTTutorialLayer* DTTutorialLayer::appendDialogue(DialogObject* dialogue) {
    dialogue->retain();
    TutorialSegment segment {
        .m_dialogue = dialogue
    };
    
    m_allSegments.push_back(std::move(segment));

    return this;
}

DTTutorialLayer* DTTutorialLayer::joinTransform(TutorialBoxPlacement alignment, float boxScale) {
    if (!m_allSegments.size()) return this;

    m_allSegments[m_allSegments.size() - 1].m_alignment = alignment;
    m_allSegments[m_allSegments.size() - 1].m_boxScale = boxScale;

    return this;
}

DTTutorialLayer* DTTutorialLayer::joinHighlight(CCNode* targetObject, float delayTime, bool allowTouches, bool ignoreDisabled) {
    return insertHighlight(m_allSegments.size() - 1, targetObject, delayTime, allowTouches, ignoreDisabled);
}
DTTutorialLayer* DTTutorialLayer::joinPreviousHighlight() {
    if (m_allSegments.size() - 2 <= 0) return this;

    for (const auto& [highlight, options] : m_allSegments[m_allSegments.size() - 2].m_targetObjects)
    {
        insertHighlight(m_allSegments.size() - 1, highlight, std::get<0>(options), std::get<1>(options), std::get<2>(options));
    }
    
    return this;
}

DTTutorialLayer* DTTutorialLayer::joinTextToHighlight(const std::string& text, float size, TutorialTextPlacement alignment) {
    return insertJoinTextToHighlight(m_allSegments.size() - 1, text, size, alignment);
}

DTTutorialLayer* DTTutorialLayer::joinCallback(Function<void()> callback, bool beforeDialogue) {
    if (!m_allSegments.size()) return this;

    if (beforeDialogue)
        m_allSegments[m_allSegments.size() - 1].m_beforeCallback = std::move(callback);
    else
        m_allSegments[m_allSegments.size() - 1].m_afterCallback = std::move(callback);

    return this;
}

DTTutorialLayer* DTTutorialLayer::insertHighlight(int dialogueIndex, CCNode* targetObject, float delayTime, bool allowTouches, bool ignoreDisabled) {
    if (targetObject == nullptr) return this;
    if (!m_allSegments.size() || m_allSegments.size() <= dialogueIndex) return this;
    
    m_allSegments[dialogueIndex].m_targetObjects.insert({targetObject, {delayTime, allowTouches, ignoreDisabled}});

    m_allSegments[dialogueIndex].m_lastAddedHighlight = targetObject;

    return this;
}

DTTutorialLayer* DTTutorialLayer::insertJoinTextToHighlight(int dialogueIndex, const std::string& text, float size, TutorialTextPlacement alignment) {
    if (!m_allSegments.size() || m_allSegments.size() <= dialogueIndex) return this;

    auto& targetObjects = m_allSegments[dialogueIndex].m_targetObjects;
    if (!targetObjects.size()) return this;

    m_allSegments[dialogueIndex].m_textForTargets[m_allSegments[dialogueIndex].m_lastAddedHighlight] = HighlightText{
        .m_text = text,
        .m_placement = alignment,
        .m_size = size
    };

    return this;
}

void DTTutorialLayer::show() {
    if (!m_allSegments.size()) return;

    m_isEggTutorial =
        Loader::get()->isModLoaded("timestepyt.heisbehindthetree") &&
        heisbehindthetree::wasEggReceived().unwrapOr(false) &&
        !heisbehindthetree::wasEggStored().unwrapOr(false);
    if (m_isEggTutorial) {
        m_skipButton->setVisible(false);
        m_skipButton->setEnabled(false);
        appendEggTutorialExtension();
    }

    auto highestZ = CCScene::get()->getHighestChildZ();

    this->setZOrder(highestZ + 1);
    CCScene::get()->addChild(this);

    auto dialogueArray = CCArray::create();
    int i = 0;
    for (const auto& segment : m_allSegments)
    {
        dialogueArray->addObject(segment.m_dialogue);

        m_dialogueSegmentIndexes.insert({segment.m_dialogue, i});
        i++;
    }

    if (Loader::get()->isModLoaded("timestepyt.deltarune_textboxes")) {
        showDeltaruneDialogueStep(0);
        return;
    }
    
    m_dialogueLayer = DTDialogLayer::createWithTaggedSprites(dialogueArray, 1, tutorialPortraits());
    m_dialogueMainLayer = m_dialogueLayer->m_mainLayer;
    m_dialogueBackground = static_cast<CCScale9Sprite*>(m_dialogueMainLayer->getChildren()->objectAtIndex(0));
    static_cast<DTDialogLayer*>(m_dialogueLayer)->setProgressCallback(std::bind(&DTTutorialLayer::onProgress, this, std::placeholders::_1));
    m_dialogueLayer->addToMainScene();
    m_dialogueLayer->m_delegate = this;
    m_dialogueLayer->setTouchEnabled(false);
    m_dialogueLayer->setKeypadEnabled(false);
    m_dialogueLayer->setOpacity(0);
}

void DTTutorialLayer::appendEggTutorialExtension() {
    m_eggTutorialDialogueStartIndex = m_allSegments.size();
    appendDialogue("...", TutorialCharacterFace::TCFSleeping);
    joinTransform(TutorialBoxPlacement::TBPCenter);
    appendDialogue("Huh?", TutorialCharacterFace::TCFNormal);
    appendDialogue("Whats that?", TutorialCharacterFace::TCFNormalTilted);
    appendDialogue("You want to give me an \"Egg\"?", TutorialCharacterFace::TCFWorried);
    appendDialogue("Im not sure why you would wan-", TutorialCharacterFace::TCFNormal);
    appendDialogue("...", TutorialCharacterFace::TCFNormal);
    appendDialogue("You want me to keep it safe?", TutorialCharacterFace::TCFBored);
    appendDialogue("Umm", TutorialCharacterFace::TCFWorried);
    appendDialogue("Alright sure..", TutorialCharacterFace::TCFNormal);
    appendDialogue("Im not too sure about this, but im here to help so sure i will!", TutorialCharacterFace::TCFHappy);
    joinCallback([this] {
        m_eggTutorialCompleted = true;
        (void)heisbehindthetree::storeEgg();
        FMODAudioEngine::sharedEngine()->playEffect("timestepyt.heisbehindthetree/snd_egg.wav");
    }, true);
    appendDialogue("Ill keep it safe dont worry! you go enjoy the rest of death tracker!", TutorialCharacterFace::TCFHappy);
}

void DTTutorialLayer::setEggDialogueSkipKeybindDisabled(bool disabled) {
    if (disabled) {
        if (m_previousDeltaruneSkipKeybinds) return;

        auto textboxesMod = Loader::get()->getLoadedMod("timestepyt.deltarune_textboxes");
        if (!textboxesMod) {
            log::error("Deltarune Textboxes was not loaded while showing the egg tutorial");
            return;
        }

        auto setting = geode::cast::typeinfo_pointer_cast<KeybindSettingV3>(
            textboxesMod->getSetting("keybind-skip")
        );
        if (!setting) {
            log::error("Deltarune Textboxes is missing its Skip Text keybind setting");
            return;
        }

        m_previousDeltaruneSkipKeybinds = setting->getValue();
        setting->setValue({});
        return;
    }

    if (!m_previousDeltaruneSkipKeybinds) return;

    auto textboxesMod = Loader::get()->getLoadedMod("timestepyt.deltarune_textboxes");
    if (!textboxesMod) {
        m_previousDeltaruneSkipKeybinds.reset();
        return;
    }

    auto setting = geode::cast::typeinfo_pointer_cast<KeybindSettingV3>(
        textboxesMod->getSetting("keybind-skip")
    );
    if (!setting) {
        m_previousDeltaruneSkipKeybinds.reset();
        return;
    }

    setting->setValue(*m_previousDeltaruneSkipKeybinds);
    m_previousDeltaruneSkipKeybinds.reset();
}

void DTTutorialLayer::showDeltaruneDialogueStep(std::size_t index) {
    if (index >= m_allSegments.size()) {
        m_eggTutorialCompleted = m_isEggTutorial;
        dialogClosed(nullptr);
        return;
    }

    m_deltaruneDialogueIndex = index;
    auto const isEggDialogue = m_eggTutorialDialogueStartIndex &&
        index >= m_eggTutorialDialogueStartIndex.value();
    setEggDialogueSkipKeybindDisabled(isEggDialogue);
    auto dialogue = m_allSegments[index].m_dialogue;

    auto const& portraits = tutorialPortraits();
    auto portraitInfo = std::find_if(portraits.begin(), portraits.end(), [dialogue](auto const& portrait) {
        return std::get<1>(portrait) == dialogue->getTag();
    });
    auto const& portraitName = portraitInfo == portraits.end() ? std::get<0>(portraits.front()) : std::get<0>(*portraitInfo);

    auto portrait = CCSprite::createWithSpriteFrameName(portraitName.c_str());
    if (!portrait)
        portrait = CCSprite::create(portraitName.c_str());
    if (!portrait) {
        dialogClosed(nullptr);
        return;
    }

    auto portraitScale = deltarune_textboxes::getCharacterSpriteScaleFrom(portrait);
    if (portraitScale.isErr()) {
        dialogClosed(nullptr);
        return;
    }
    portrait->setScale(portraitScale.unwrap() * 0.7f);

    auto dialogueObject = deltarune_textboxes::createDialogObject(portrait, "Default", "abb2k", dialogue->m_text);
    if (dialogueObject.isErr()) {
        dialogClosed(nullptr);
        return;
    }

    std::array<deltarune_textboxes::DialogObjectPtr, 1> dialogueObjects{
        std::move(dialogueObject).unwrap()
    };
    auto textboxesMod = isEggDialogue
        ? Loader::get()->getLoadedMod("timestepyt.deltarune_textboxes")
        : nullptr;
    if (isEggDialogue && !textboxesMod) {
        log::error("Deltarune Textboxes was not loaded while showing the egg tutorial");
        dialogClosed(nullptr);
        return;
    }

    auto createDialog = [&] {
        return deltarune_textboxes::createFullDialog(
            std::span<deltarune_textboxes::DialogObjectPtr>(dialogueObjects)
        );
    };
    auto alertResult = [&] {
        if (!textboxesMod)
            return createDialog();

        auto const previousDisableClickToProgress =
            textboxesMod->getSettingValue<bool>("disableClickToProgress");
        textboxesMod->setSettingValue<bool>("disableClickToProgress", true);
        auto result = createDialog();
        textboxesMod->setSettingValue<bool>("disableClickToProgress", previousDisableClickToProgress);
        return result;
    }();
    if (alertResult.isErr()) {
        dialogClosed(nullptr);
        return;
    }

    m_deltaruneAlert = alertResult.unwrap();
    m_deltaruneAlert->m_alertProtocol = this;
    m_dialogueMainLayer = m_deltaruneAlert->m_mainLayer;
    m_dialogueBackground = m_dialogueMainLayer
        ? static_cast<CCScale9Sprite*>(m_dialogueMainLayer->getChildByID("background"))
        : nullptr;
    moveSkipMenuTo(m_deltaruneAlert, 1000);
    if (m_hasDialoguePlacement) {
        m_dialogueMainLayer->setScale(m_dialogueBoxScale);
        m_dialogueMainLayer->setPosition(dialogueMainLayerPositionForCenter(m_dialogueCenter, m_dialogueBoxScale));
    }
    else if (m_dialogueBoxScale != 1.f) {
        m_dialogueMainLayer->setScale(m_dialogueBoxScale);
    }
    onProgress(dialogue);
    DTDialogLayer::addStoredEggBadge(m_dialogueMainLayer, m_dialogueBackground, true);
}

void DTTutorialLayer::FLAlert_Clicked(FLAlertLayer* layer, bool) {
    if (m_isClosing || layer != m_deltaruneAlert) return;

    m_deltaruneAlert = nullptr;
    m_dialogueMainLayer = nullptr;
    m_dialogueBackground = nullptr;
    showDeltaruneDialogueStep(m_deltaruneDialogueIndex + 1);
}

CCPoint DTTutorialLayer::dialogueMainLayerPositionForCenter(CCPoint center, float scale) {
    if (!m_deltaruneAlert)
        return center;

    auto parent = m_dialogueMainLayer->getParent();
    if (!parent) {
        return m_dialogueMainLayer->getPosition();
    }

    auto const currentPosition = m_dialogueMainLayer->getPosition();
    auto const currentScale = m_dialogueMainLayer->getScale();
    m_dialogueMainLayer->setScale(scale);
    auto const backgroundCenter = ccp(
        m_dialogueBackground->getContentWidth() / 2,
        m_dialogueBackground->getContentHeight() / 2
    );
    auto const backgroundCenterInParent = parent->convertToNodeSpace(
        m_dialogueBackground->convertToWorldSpace(backgroundCenter)
    );
    m_dialogueMainLayer->setScale(currentScale);
    auto const desiredCenterInParent = parent->convertToNodeSpace(this->convertToWorldSpace(center));

    return currentPosition + desiredCenterInParent - backgroundCenterInParent;
}

void DTTutorialLayer::moveSkipMenuTo(CCNode* parent, int zOrder) {
    if (!m_skipMenu || !parent || m_skipMenu->getParent() == parent) return;

    auto const anchor = m_skipMenu->getAnchorPoint();
    auto const size = m_skipMenu->getContentSize();
    auto const anchorInPoints = ccp(size.width * anchor.x, size.height * anchor.y);
    auto const worldAnchor = m_skipMenu->convertToWorldSpace(anchorInPoints);

    m_skipMenu->retain();
    m_skipMenu->removeFromParentAndCleanup(false);
    parent->addChild(m_skipMenu, zOrder);
    m_skipMenu->setPosition(parent->convertToNodeSpace(worldAnchor));
    m_skipMenu->setTouchPriority(parent == m_deltaruneAlert ? -1000 : 0);
    m_skipMenu->release();
}

void DTTutorialLayer::onProgress(DialogObject* dObject) {
    if (m_prevDialogue != nullptr) {
        auto previousSegment = m_dialogueSegmentIndexes.find(m_prevDialogue);
        if (previousSegment != m_dialogueSegmentIndexes.end()) {
            auto& completedSegment = m_allSegments[previousSegment->second];
            if (completedSegment.m_afterCallback != NULL)
                completedSegment.m_afterCallback();
        }
    }
    
    auto segmentIndex = m_dialogueSegmentIndexes[dObject];
    if (m_dialogueLayer) {
        auto const isEggDialogue = m_eggTutorialDialogueStartIndex &&
            segmentIndex >= m_eggTutorialDialogueStartIndex.value();
        static_cast<DTDialogLayer*>(m_dialogueLayer)->setFastForwardDisabled(isEggDialogue);
    }

    if (m_allSegments[segmentIndex].m_beforeCallback != NULL) {
        auto& objToRunOn = m_allSegments[segmentIndex];
        objToRunOn.m_beforeCallback();
    }

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    float time = segmentIndex == 0.1f ? 0 : 0.5f;

    bool firstRN = false;

    if (!m_firstDialogue) {
        m_firstDialogue = true;
        firstRN = true;
        m_lightRT->getSprite()->runAction(CCEaseInOut::create(CCFadeTo::create(.25f, 255), 2));
    }
    
    if (m_allSegments[segmentIndex].m_alignment.has_value() && m_dialogueMainLayer && m_dialogueBackground) {
        m_dialogueMainLayer->stopActionByTag(10);
        CCPoint toMoveTo;

        auto bg = m_dialogueBackground;

        float scaleMultiplier = m_allSegments[segmentIndex].m_boxScale.has_value() ? m_allSegments[segmentIndex].m_boxScale.value() : m_dialogueBoxScale;

        switch (m_allSegments[segmentIndex].m_alignment.value()) {
            case TutorialBoxPlacement::TBPCenter:
                toMoveTo = CCPoint{winSize.width * 0.5F, winSize.height * 0.5F};
            break;
        case TutorialBoxPlacement::TBPTop:
                toMoveTo = CCPoint{winSize.width * 0.5F, (winSize.height - 50.F) - 20.F};
            break;
        case TutorialBoxPlacement::TBPBottom:
                toMoveTo = CCPoint{winSize.width * 0.5F, 70.F};
            break;
        case TutorialBoxPlacement::TBPLeft:
                toMoveTo = CCPoint{bg->getContentWidth() * scaleMultiplier / 2, winSize.height * 0.5F};
            break;
        case TutorialBoxPlacement::TBPRight:
                toMoveTo = CCPoint{winSize.width - bg->getContentWidth() * scaleMultiplier / 2, winSize.height * 0.5F};
            break;
        case TutorialBoxPlacement::TBPTopRight:
                toMoveTo = CCPoint{winSize.width - bg->getContentWidth() * scaleMultiplier / 2, (winSize.height - 50.F) - 20.F};
            break;
        case TutorialBoxPlacement::TBPTopLeft:
                toMoveTo = CCPoint{bg->getContentWidth() * scaleMultiplier / 2, (winSize.height - 50.F) - 20.F};
            break;
        case TutorialBoxPlacement::TBPBottomLeft:
                toMoveTo = CCPoint{bg->getContentWidth() * scaleMultiplier / 2, 70.F};
            break;
        case TutorialBoxPlacement::TBPBottomRight:
                toMoveTo = CCPoint{winSize.width - bg->getContentWidth() * scaleMultiplier / 2, 70.F};
            break;
        }

        m_dialogueCenter = toMoveTo;
        m_hasDialoguePlacement = true;
        auto const targetPosition = dialogueMainLayerPositionForCenter(toMoveTo, scaleMultiplier);
        if (firstRN) {
            m_dialogueMainLayer->setPosition(targetPosition);
        }
        else {
            auto movement = CCEaseInOut::create(CCMoveTo::create(time, targetPosition), 2);
            movement->setTag(10);
            m_dialogueMainLayer->runAction(movement);
        }
    }

    if (m_allSegments[segmentIndex].m_boxScale.has_value() && m_dialogueMainLayer) {
        m_dialogueBoxScale = m_allSegments[segmentIndex].m_boxScale.value();
        m_dialogueMainLayer->stopActionByTag(11);
        
        if (firstRN) {
            m_dialogueMainLayer->setScale(m_dialogueBoxScale);
        }
        else {
            auto scaleEase = CCEaseInOut::create(CCScaleTo::create(time, m_dialogueBoxScale), 2);
            scaleEase->setTag(11);
            m_dialogueMainLayer->runAction(scaleEase);
        }   
    }

    for (const auto& glow : m_retainedSprites) {
        if (m_prevHighlights.contains(glow) && m_allSegments[segmentIndex].m_targetObjects.contains(m_prevHighlights[glow])) continue;
        glow->stopActionByTag(14);
        glow->runAction(CCFadeTo::create(.25f, 0));
        glow->setTag(1);

        if (m_textsForHighlights.contains(glow)) {
            m_textsForHighlights[glow]->stopActionByTag(14);
            m_textsForHighlights[glow]->runAction(CCFadeTo::create(.25f, 0));
        }
    }

    m_prevHighlights.clear();

    for (const auto& [highlightTarget, dataPair] : m_allSegments[segmentIndex].m_targetObjects)
    {
        auto& [delayTime, touchesAllowed, ignoreDisabled] = dataPair;

        auto glow = CCScale9Sprite::createWithSpriteFrameName("squareGlow.png"_spr);

        auto cornerMin = this->convertToNodeSpace(highlightTarget->convertToWorldSpace({0, 0}));
        auto cornerMax = this->convertToNodeSpace(highlightTarget->convertToWorldSpace(highlightTarget->getContentSize()));

        auto realContentSize = CCSize{
            std::abs(cornerMax.x - cornerMin.x),
            std::abs(cornerMax.y - cornerMin.y)
        };

        glow->setContentSize(realContentSize + ccp(15, 15));
        glow->setPosition(
            this->convertToNodeSpace(
                highlightTarget->convertToWorldSpace(
                    {0, 0}
                )
            ) + realContentSize / 2
        );

        glow->_scale9Image->setBlendFunc({GL_ZERO, GL_ONE_MINUS_SRC_ALPHA});
        glow->retain();
        glow->onEnter();
        m_retainedSprites.insert(glow);

        glow->setOpacity(0);
        CCSequence* fadeSeq;
        if (!ignoreDisabled || ignoreDisabled && nodeIsVisible(highlightTarget)) {
            fadeSeq = CCSequence::create(
                CCDelayTime::create(delayTime),
                CCFadeTo::create(.5f, 255),
                nullptr
            );
            fadeSeq->setTag(14);
            glow->runAction(fadeSeq);
        }
        glow->setTag(2);

        m_prevHighlights.insert({glow, highlightTarget});
        if (touchesAllowed)
            m_touchAllowedNodes.insert({glow, highlightTarget});

        if (!m_allSegments[segmentIndex].m_textForTargets.contains(highlightTarget)) continue;

        auto& textInfo = m_allSegments[segmentIndex].m_textForTargets[highlightTarget];

        auto label = geode::Label::create(textInfo.m_text.c_str(), "bigFont.fnt");
        label->setAlignment(textInfo.m_placement == TutorialTextPlacement::TTLeft ?
            geode::Label::Alignment::Left :
            (textInfo.m_placement == TutorialTextPlacement::TTRight ?
                geode::Label::Alignment::Right : 
                geode::Label::Alignment::Center
            ) 
        );
        auto anchotPoint = CCPoint{.5f, .5f};
        if (textInfo.m_placement == TutorialTextPlacement::TTRight) anchotPoint.x = 0;
        if (textInfo.m_placement == TutorialTextPlacement::TTLeft) anchotPoint.x = 1;
        if (textInfo.m_placement == TutorialTextPlacement::TTTop) anchotPoint.y = 0;
        if (textInfo.m_placement == TutorialTextPlacement::TTBottom) anchotPoint.y = 1;
        label->setAnchorPoint(anchotPoint);
        label->setPosition(this->convertToNodeSpace(
                highlightTarget->convertToWorldSpace(
                    {0, 0}
                )
            ) + realContentSize * ccp(std::abs(anchotPoint.x - 1), std::abs(anchotPoint.y - 1))
        );
        label->setScale(textInfo.m_size);
        label->setOpacity(0);
        if (!ignoreDisabled || ignoreDisabled && nodeIsVisible(highlightTarget)) {
            fadeSeq = CCSequence::create(
                CCDelayTime::create(delayTime),
                CCFadeTo::create(.5f, 255),
                nullptr
            );
            fadeSeq->setTag(14);
            label->runAction(fadeSeq);
        }
        this->addChild(label);

        m_textsForHighlights.insert({glow, label});
    }

    m_prevDialogue = dObject;
}

void DTTutorialLayer::dialogClosed(DialogLayer* layer) {
    if (m_isClosing) return;

    setEggDialogueSkipKeybindDisabled(false);

    if (m_isEggTutorial && (layer || m_eggTutorialCompleted)) {
        Mod::get()->setSavedValue("eggTutorialCompleted", true);
    }

    moveSkipMenuTo(this, 0);
    m_isClosing = true;
    
    m_lightRT->getSprite()->runAction(CCSequence::create(
        CCEaseInOut::create(CCFadeTo::create(.25f, 0), 2),
        CCCallFunc::create(this, callfunc_selector(DTTutorialLayer::close)),
        nullptr
    ));

    m_skipButton->runAction(CCFadeOut::create(.5f));

    m_dialogueLayer = nullptr;
    m_deltaruneAlert = nullptr;
    m_dialogueMainLayer = nullptr;
    m_dialogueBackground = nullptr;
}

void DTTutorialLayer::close() {
    this->removeMeAndCleanup();
}

void DTTutorialLayer::update(float dt) {
    m_lightRT->beginWithClear(0, 0, 0, 0.85f);

    for (const auto& glow : m_retainedSprites)
        glow->visit();
    
    m_lightRT->end();
}

DTTutorialLayer::~DTTutorialLayer() {
    setEggDialogueSkipKeybindDisabled(false);

    for (const auto& glow : m_retainedSprites) {
        glow->release();
    }

    for (const auto& segment : m_allSegments) {
        segment.m_dialogue->release();
    }
    
    CCTouchDispatcher::get()->removeDelegate(this);
}

bool DTTutorialLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return false;

    if (static_cast<CCScale9Sprite*>(m_dialogueLayer->m_mainLayer->getChildren()->objectAtIndex(0))->boundingBox().containsPoint(m_dialogueLayer->m_mainLayer->convertTouchToNodeSpace(pTouch)))
        return m_dialogueLayer->ccTouchBegan(pTouch, pEvent);

    for (const auto& glow : m_retainedSprites) {
        if (!m_touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto touchDelegate = dynamic_cast<CCTouchDelegate*>(m_touchAllowedNodes[glow]);
        if (touchDelegate != nullptr && touchDelegate->ccTouchBegan(pTouch, pEvent)) {
            m_touchedHighlight = touchDelegate;
            return true;
        }
    }

    return m_dialogueLayer->ccTouchBegan(pTouch, pEvent);
}
void DTTutorialLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return;

    if (m_touchedHighlight != nullptr) {
        m_touchedHighlight->ccTouchMoved(pTouch, pEvent);
        return;
    }
    m_dialogueLayer->ccTouchMoved(pTouch, pEvent);
}
void DTTutorialLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return;

    if (m_touchedHighlight != nullptr) {
        m_touchedHighlight->ccTouchEnded(pTouch, pEvent);
        m_touchedHighlight = nullptr;
        return;
    }
    m_dialogueLayer->ccTouchEnded(pTouch, pEvent);
}
void DTTutorialLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return;

    if (m_touchedHighlight != nullptr) {
        m_touchedHighlight->ccTouchCancelled(pTouch, pEvent);
        m_touchedHighlight = nullptr;
        return;
    }
    m_dialogueLayer->ccTouchCancelled(pTouch, pEvent);
}

void DTTutorialLayer::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return;

    if (m_touchedHighlight != nullptr) {
        m_touchedHighlight->ccTouchesBegan(pTouches, pEvent);
        return;
    }
    m_dialogueLayer->ccTouchesBegan(pTouches, pEvent);
}
void DTTutorialLayer::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return;

    if (m_touchedHighlight != nullptr) {
        m_touchedHighlight->ccTouchesMoved(pTouches, pEvent);
        return;
    }
    m_dialogueLayer->ccTouchesMoved(pTouches, pEvent);
}
void DTTutorialLayer::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return;

    if (m_touchedHighlight != nullptr) {
        m_touchedHighlight->ccTouchesEnded(pTouches, pEvent);
        return;
    }
    m_dialogueLayer->ccTouchesEnded(pTouches, pEvent);
}
void DTTutorialLayer::ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent) {
    if (m_dialogueLayer == nullptr) return;

    if (m_touchedHighlight != nullptr) {
        m_touchedHighlight->ccTouchesCancelled(pTouches, pEvent);
        return;
    }
    m_dialogueLayer->ccTouchesCancelled(pTouches, pEvent);
}

void DTTutorialLayer::keyBackClicked() {
    if (m_dialogueLayer == nullptr) return;
    m_dialogueLayer->handleDialogTap();
}

void DTTutorialLayer::keyDown(enumKeyCodes key, double d) {
    if (m_dialogueLayer == nullptr) return;
    for (const auto& glow : m_retainedSprites) {
        if (!m_touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto keyDel = dynamic_cast<CCKeyboardDelegate*>(m_touchAllowedNodes[glow]);
        if (keyDel != nullptr) {
            keyDel->keyDown(key, d);
            return;
        }
    }

    CCLayer::keyDown(key, d);
}
void DTTutorialLayer::keyUp(enumKeyCodes key, double d) {
    if (m_dialogueLayer == nullptr) return;
    for (const auto& glow : m_retainedSprites) {
        if (!m_touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto keyDel = dynamic_cast<CCKeyboardDelegate*>(m_touchAllowedNodes[glow]);
        if (keyDel != nullptr) {
            keyDel->keyUp(key, d);
            return;
        }
    }

    CCLayer::keyUp(key, d);
}

void DTTutorialLayer::rightKeyDown() {
    if (m_dialogueLayer == nullptr) return;
    for (const auto& glow : m_retainedSprites) {
        if (!m_touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto mDel = dynamic_cast<CCMouseDelegate*>(m_touchAllowedNodes[glow]);
        if (mDel != nullptr) {
            mDel->rightKeyDown();
            return;
        }
    }
}

void DTTutorialLayer::rightKeyUp() {
    if (m_dialogueLayer == nullptr) return;
    for (const auto& glow : m_retainedSprites) {
        if (!m_touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto mDel = dynamic_cast<CCMouseDelegate*>(m_touchAllowedNodes[glow]);
        if (mDel != nullptr) {
            mDel->rightKeyUp();
            return;
        }
    }
}

bool DTTutorialLayer::scrollWheel(float x, float y) {
    if (m_dialogueLayer == nullptr) return false;
    
    for (const auto& glow : m_retainedSprites) {
        if (!m_touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto mDel = dynamic_cast<CCMouseDelegate*>(m_touchAllowedNodes[glow]);
        if (mDel != nullptr) {
            mDel->scrollWheel(x, y);
            return true;
        }
    }

    return false;
}

void DTTutorialLayer::skipTutorial(CCObject*) {
    if (m_isClosing || m_isEggTutorial) return;
    if (m_dialogueLayer)
        m_dialogueLayer->removeMeAndCleanup();
    if (m_deltaruneAlert) {
        moveSkipMenuTo(this, 0);
        m_deltaruneAlert->removeFromParentAndCleanup(true);
    }
    dialogClosed(nullptr);
}