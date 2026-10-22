#include "DTTutorialLayer.hpp"
#include <hooks/DTDialogLayer.hpp>
#include <nodes/TouchSender.hpp>

DTTutorialLayer* DTTutorialLayer::create() {
    auto ret = new DTTutorialLayer();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool DTTutorialLayer::init() {
    CCTouchDispatcher::get()->addPrioTargetedDelegate(this, 1000, true);
    if (!CCLayer::init()) return false;

    auto winSize = CCDirector::get()->getWinSize();

    lightRT = CCRenderTexture::create(winSize.width, winSize.height, CCTexture2DPixelFormat::kCCTexture2DPixelFormat_RGBA8888);
    lightRT->setPosition(winSize / 2);
    lightRT->getSprite()->setOpacity(0);
    this->addChild(lightRT);

    auto skipButtonSpr = ButtonSprite::create("Skip Tutorial");
    skipButtonSpr->setScale(.5f);
    skipButtonSpr->setCascadeOpacityEnabled(true);
    skipButton = CCMenuItemSpriteExtra::create(
        skipButtonSpr,
        this,
        menu_selector(DTTutorialLayer::skipTutorial)
    );
    skipButton->setAnchorPoint({1, 0});
    skipButton->setOpacity(0);
    skipButton->runAction(CCFadeIn::create(.5f));

    auto menu = CCMenu::create();
    this->addChild(menu);

    menu->addChildAtPosition(skipButton, Anchor::BottomRight);

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
    TutorialSegment segment{
        .dialogue = dialogue
    };
    
    allSegments.push_back(std::move(segment));

    return this;
}

DTTutorialLayer* DTTutorialLayer::joinTransform(TutorialBoxPlacement alignment, float boxScale) {
    if (!allSegments.size()) return this;

    allSegments[allSegments.size() - 1].alignment = alignment;
    allSegments[allSegments.size() - 1].boxScale = boxScale;

    return this;
}

DTTutorialLayer* DTTutorialLayer::joinHighlight(CCNode* targetObject, float delayTime, bool allowTouches, bool ignoreDisabled) {
    return insertHighlight(allSegments.size() - 1, targetObject, delayTime, allowTouches, ignoreDisabled);
}
DTTutorialLayer* DTTutorialLayer::joinPreviousHighlight() {
    if (allSegments.size() - 2 <= 0) return this;

    for (const auto& [highlight, options] : allSegments[allSegments.size() - 2].targetObjects)
    {
        insertHighlight(allSegments.size() - 1, highlight, std::get<0>(options), std::get<1>(options), std::get<2>(options));
    }
    
    return this;
}

DTTutorialLayer* DTTutorialLayer::joinTextToHighlight(const std::string& text, float size, TutorialTextPlacement alignment) {
    return insertJoinTextToHighlight(allSegments.size() - 1, text, size, alignment);
}

DTTutorialLayer* DTTutorialLayer::joinCallback(geode::Function<void()> callback, bool beforeDialogue) {
    if (!allSegments.size()) return this;

    if (beforeDialogue)
        allSegments[allSegments.size() - 1].beforeCallback = std::move(callback);
    else
        allSegments[allSegments.size() - 1].afterCallback = std::move(callback);

    return this;
}

DTTutorialLayer* DTTutorialLayer::insertHighlight(int dialogueIndex, CCNode* targetObject, float delayTime, bool allowTouches, bool ignoreDisabled) {
    if (targetObject == nullptr) return this;
    if (!allSegments.size() || allSegments.size() <= dialogueIndex) return this;
    
    allSegments[dialogueIndex].targetObjects.insert({targetObject, {delayTime, allowTouches, ignoreDisabled}});

    allSegments[dialogueIndex].lastAddedHighlight = targetObject;

    return this;
}

DTTutorialLayer* DTTutorialLayer::insertJoinTextToHighlight(int dialogueIndex, const std::string& text, float size, TutorialTextPlacement alignment) {
    if (!allSegments.size() || allSegments.size() <= dialogueIndex) return this;

    auto& targetObjects = allSegments[dialogueIndex].targetObjects;
    if (!targetObjects.size()) return this;

    allSegments[dialogueIndex].textForTargets[allSegments[dialogueIndex].lastAddedHighlight] = HighlightText{
        .text = text,
        .placement = alignment,
        .size = size
    };

    return this;
}

void DTTutorialLayer::show() {
    if (!allSegments.size()) return;
    auto highestZ = CCScene::get()->getHighestChildZ();

    this->setZOrder(highestZ + 1);
    CCScene::get()->addChild(this);

    auto dialogueArray = CCArray::create();
    int i = 0;
    for (const auto& segment : allSegments)
    {
        dialogueArray->addObject(segment.dialogue);

        dialogueSegmentIndexes.insert({segment.dialogue, i});
        i++;
    }
    
    dialogueLayer = DTDialogLayer::createWithTaggedSprites(dialogueArray, 1, {
        {"normal.png"_spr, static_cast<int>(TutorialCharacterFace::TCFNormal), 2},
        {"tiltedNormal.png"_spr, static_cast<int>(TutorialCharacterFace::TCFNormalTilted), 2.15f},
        {"happy.png"_spr, static_cast<int>(TutorialCharacterFace::TCFHappy), 2.15f},
        {"angry.png"_spr, static_cast<int>(TutorialCharacterFace::TCFAngry), 2},
        {"worried.png"_spr, static_cast<int>(TutorialCharacterFace::TCFWorried), 2},
        {"sleeping.png"_spr, static_cast<int>(TutorialCharacterFace::TCFSleeping), 2},
        {"bored.png"_spr, static_cast<int>(TutorialCharacterFace::TCFBored), 2}
    });
    static_cast<DTDialogLayer*>(dialogueLayer)->setProgressCallback(std::bind(&DTTutorialLayer::onProgress, this, std::placeholders::_1));
    dialogueLayer->addToMainScene();
    dialogueLayer->m_delegate = this;
    dialogueLayer->setTouchEnabled(false);
    dialogueLayer->setKeypadEnabled(false);
    dialogueLayer->setOpacity(0);
}

void DTTutorialLayer::onProgress(DialogObject* dObject) {
    if (prevDialogue != nullptr && dialogueSegmentIndexes.contains(dObject) && allSegments[dialogueSegmentIndexes[dObject]].afterCallback != NULL) {
        auto& objToRunOn = allSegments[dialogueSegmentIndexes[dObject]];
        objToRunOn.afterCallback();
    }
    
    auto segmentIndex = dialogueSegmentIndexes[dObject];

    if (allSegments[segmentIndex].beforeCallback != NULL) {
        auto& objToRunOn = allSegments[segmentIndex];
        objToRunOn.beforeCallback();
    }

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    float time = segmentIndex == 0.1f ? 0 : 0.5f;

    bool firstRN = false;

    if (!firstDialogue) {
        firstDialogue = true;
        firstRN = true;
        lightRT->getSprite()->runAction(CCEaseInOut::create(CCFadeTo::create(.25f, 255), 2));
    }
    
    if (allSegments[segmentIndex].alignment.has_value()) {
        dialogueLayer->m_mainLayer->stopActionByTag(10);
        CCPoint toMoveTo;

        auto bg = static_cast<CCScale9Sprite*>(dialogueLayer->m_mainLayer->getChildren()->objectAtIndex(0));

        float scaleMultiplier = allSegments[segmentIndex].boxScale.has_value() ? allSegments[segmentIndex].boxScale.value() : dialogueLayer->m_mainLayer->getScale();

        switch (allSegments[segmentIndex].alignment.value()) {
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

        if (firstRN) {
            dialogueLayer->m_mainLayer->setPosition(toMoveTo);
        }
        else {
            auto movement = CCEaseInOut::create(CCMoveTo::create(time, toMoveTo), 2);
            movement->setTag(10);
            dialogueLayer->m_mainLayer->runAction(movement);
        }
    }

    if (allSegments[segmentIndex].boxScale.has_value()) {
        dialogueLayer->m_mainLayer->stopActionByTag(11);
        
        if (firstRN) {
            dialogueLayer->m_mainLayer->setScale(allSegments[segmentIndex].boxScale.value());
        }
        else {
            auto scaleEase = CCEaseInOut::create(CCScaleTo::create(time, allSegments[segmentIndex].boxScale.value()), 2);
            scaleEase->setTag(11);
            dialogueLayer->m_mainLayer->runAction(scaleEase);
        }   
    }

    for (const auto& glow : retainedSprites) {
        if (prevHighlights.contains(glow) && allSegments[segmentIndex].targetObjects.contains(prevHighlights[glow])) continue;
        glow->stopActionByTag(14);
        glow->runAction(CCFadeTo::create(.25f, 0));
        glow->setTag(1);

        if (textsForHighlights.contains(glow)) {
            textsForHighlights[glow]->stopActionByTag(14);
            textsForHighlights[glow]->runAction(CCFadeTo::create(.25f, 0));
        }
    }

    prevHighlights.clear();

    for (const auto& [highlightTarget, dataPair] : allSegments[segmentIndex].targetObjects)
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
        retainedSprites.insert(glow);

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

        prevHighlights.insert({glow, highlightTarget});
        if (touchesAllowed)
            touchAllowedNodes.insert({glow, highlightTarget});

        prevDialogue = dObject;

        if (!allSegments[segmentIndex].textForTargets.contains(highlightTarget)) continue;

        auto& textInfo = allSegments[segmentIndex].textForTargets[highlightTarget];

        auto label = CCLabelBMFont::create(textInfo.text.c_str(), "bigFont.fnt");
        label->setAlignment(textInfo.placement == TutorialTextPlacement::TTLeft ? 
            CCTextAlignment::kCCTextAlignmentLeft :
            (textInfo.placement == TutorialTextPlacement::TTRight ? 
                CCTextAlignment::kCCTextAlignmentRight : 
                CCTextAlignment::kCCTextAlignmentCenter
            ) 
        );
        auto anchotPoint = CCPoint{.5f, .5f};
        if (textInfo.placement == TutorialTextPlacement::TTRight) anchotPoint.x = 0;
        if (textInfo.placement == TutorialTextPlacement::TTLeft) anchotPoint.x = 1;
        if (textInfo.placement == TutorialTextPlacement::TTTop) anchotPoint.y = 0;
        if (textInfo.placement == TutorialTextPlacement::TTBottom) anchotPoint.y = 1;
        label->setAnchorPoint(anchotPoint);
        label->setPosition(this->convertToNodeSpace(
                highlightTarget->convertToWorldSpace(
                    {0, 0}
                )
            ) + realContentSize * ccp(std::abs(anchotPoint.x - 1), std::abs(anchotPoint.y - 1))
        );
        label->setScale(textInfo.size);
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

        textsForHighlights.insert({glow, label});
    }

}

void DTTutorialLayer::dialogClosed(DialogLayer* layer) {
    if (isClosing) return;

    isClosing = true;
    
    lightRT->getSprite()->runAction(CCSequence::create(
        CCEaseInOut::create(CCFadeTo::create(.25f, 0), 2),
        CCCallFunc::create(this, callfunc_selector(DTTutorialLayer::close)),
        nullptr
    ));

    skipButton->runAction(CCFadeOut::create(.5f));

    dialogueLayer = nullptr;
}

void DTTutorialLayer::close() {
    this->removeMeAndCleanup();
}

void DTTutorialLayer::update(float dt) {
    lightRT->beginWithClear(0, 0, 0, 0.85f);

    for (const auto& glow : retainedSprites)
        glow->visit();
    
    lightRT->end();
}

DTTutorialLayer::~DTTutorialLayer() {
    for (const auto& glow : retainedSprites) {
        glow->release();
    }
    
    CCTouchDispatcher::get()->removeDelegate(this);
}

bool DTTutorialLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return false;
    if (static_cast<CCScale9Sprite*>(dialogueLayer->m_mainLayer->getChildren()->objectAtIndex(0))->boundingBox().containsPoint(
        dialogueLayer->m_mainLayer->convertTouchToNodeSpace(pTouch)))
        return dialogueLayer->ccTouchBegan(pTouch, pEvent);

    for (const auto& glow : retainedSprites) {
        if (!touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto touchDelegate = dynamic_cast<CCTouchDelegate*>(touchAllowedNodes[glow]);
        if (touchDelegate != nullptr && touchDelegate->ccTouchBegan(pTouch, pEvent)) {
            touchedHighlight = touchDelegate;
            return true;
        }
    }

    return dialogueLayer->ccTouchBegan(pTouch, pEvent);
}
void DTTutorialLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return;

    if (touchedHighlight != nullptr) {
        touchedHighlight->ccTouchMoved(pTouch, pEvent);
        return;
    }
    dialogueLayer->ccTouchMoved(pTouch, pEvent);
}
void DTTutorialLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return;

    if (touchedHighlight != nullptr) {
        touchedHighlight->ccTouchEnded(pTouch, pEvent);
        touchedHighlight = nullptr;
        return;
    }
    dialogueLayer->ccTouchEnded(pTouch, pEvent);
}
void DTTutorialLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return;

    if (touchedHighlight != nullptr) {
        touchedHighlight->ccTouchCancelled(pTouch, pEvent);
        touchedHighlight = nullptr;
        return;
    }
    dialogueLayer->ccTouchCancelled(pTouch, pEvent);
}

void DTTutorialLayer::ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return;

    if (touchedHighlight != nullptr) {
        touchedHighlight->ccTouchesBegan(pTouches, pEvent);
        return;
    }
    dialogueLayer->ccTouchesBegan(pTouches, pEvent);
}
void DTTutorialLayer::ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return;

    if (touchedHighlight != nullptr) {
        touchedHighlight->ccTouchesMoved(pTouches, pEvent);
        return;
    }
    dialogueLayer->ccTouchesMoved(pTouches, pEvent);
}
void DTTutorialLayer::ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return;

    if (touchedHighlight != nullptr) {
        touchedHighlight->ccTouchesEnded(pTouches, pEvent);
        return;
    }
    dialogueLayer->ccTouchesEnded(pTouches, pEvent);
}
void DTTutorialLayer::ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent) {
    if (dialogueLayer == nullptr) return;

    if (touchedHighlight != nullptr) {
        touchedHighlight->ccTouchesCancelled(pTouches, pEvent);
        return;
    }
    dialogueLayer->ccTouchesCancelled(pTouches, pEvent);
}

void DTTutorialLayer::keyBackClicked() {
    if (dialogueLayer == nullptr) return;
    dialogueLayer->handleDialogTap();
}

void DTTutorialLayer::keyDown(enumKeyCodes key, double d) {
    if (dialogueLayer == nullptr) return;
    for (const auto& glow : retainedSprites) {
        if (!touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto keyDel = dynamic_cast<CCKeyboardDelegate*>(touchAllowedNodes[glow]);
        if (keyDel != nullptr) {
            keyDel->keyDown(key, d);
            return;
        }
    }

    CCLayer::keyDown(key, d);
}
void DTTutorialLayer::keyUp(enumKeyCodes key, double d) {
    if (dialogueLayer == nullptr) return;
    for (const auto& glow : retainedSprites) {
        if (!touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto keyDel = dynamic_cast<CCKeyboardDelegate*>(touchAllowedNodes[glow]);
        if (keyDel != nullptr) {
            keyDel->keyUp(key, d);
            return;
        }
    }

    CCLayer::keyUp(key, d);
}

void DTTutorialLayer::rightKeyDown() {
    if (dialogueLayer == nullptr) return;
    for (const auto& glow : retainedSprites) {
        if (!touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto mDel = dynamic_cast<CCMouseDelegate*>(touchAllowedNodes[glow]);
        if (mDel != nullptr) {
            mDel->rightKeyDown();
            return;
        }
    }
}

void DTTutorialLayer::rightKeyUp() {
    if (dialogueLayer == nullptr) return;
    for (const auto& glow : retainedSprites) {
        if (!touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto mDel = dynamic_cast<CCMouseDelegate*>(touchAllowedNodes[glow]);
        if (mDel != nullptr) {
            mDel->rightKeyUp();
            return;
        }
    }
}

void DTTutorialLayer::scrollWheel(float x, float y) {
    if (dialogueLayer == nullptr) return;
    for (const auto& glow : retainedSprites) {
        if (!touchAllowedNodes.contains(glow)) continue;
        if (glow->getTag() != 2) continue;

        auto mDel = dynamic_cast<CCMouseDelegate*>(touchAllowedNodes[glow]);
        if (mDel != nullptr) {
            mDel->scrollWheel(x, y);
            return;
        }
    }
}

void DTTutorialLayer::skipTutorial(CCObject*) {
    if (isClosing) return;
    dialogueLayer->removeMeAndCleanup();
    dialogClosed(nullptr);
}