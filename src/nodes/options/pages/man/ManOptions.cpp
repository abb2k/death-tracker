#include "ManOptions.hpp"

#include <hooks/DTDialogLayer.hpp>
#include <timestepyt.deltarune_textboxes/API.hpp>
#include <timestepyt.heisbehindthetree/api/API.hpp>
#include <nodes/options/pages/man/ManHider.hpp>
#include <nodes/general/TouchSender.hpp>

#include <nodes/options/pages/man/ManSeeker.hpp>


ManOptions* ManOptions::create(CCSize size) {
    if (!Loader::get()->isModLoaded("timestepyt.heisbehindthetree"))
        return nullptr;

    auto ret = new ManOptions();

    if (ret && ret->initWithSetup(std::move(size))) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool ManOptions::setup() {
    auto bgSquare = CCScale9Sprite::create("square.png");
    bgSquare->setColor({ 0, 0, 0 });
    bgSquare->setOpacity(255);
    bgSquare->ignoreAnchorPointForPosition(false);
    bgSquare->setPosition(this->getContentSize() / 2.f);
    bgSquare->setContentWidth(60.f);
    bgSquare->setContentSize(this->getContentSize());
    bgSquare->setScale(0.9f);
    bgSquare->setCascadeOpacityEnabled(true);
    bgSquare->setID("floor-square");

    auto addEdgeFade = [&](CCPoint position, float rotation, bool flipY, bool horizontalEdge) {
        auto fade = CCSprite::createWithSpriteFrameName("d_gradient_01_001.png");
        fade->setColor({0, 0, 0});
        fade->setPosition(position);
        fade->setScaleX((horizontalEdge ? bgSquare->getContentWidth() : bgSquare->getContentHeight()) / fade->getContentWidth());
        fade->setScaleY((horizontalEdge ? bgSquare->getContentHeight() : bgSquare->getContentWidth()) * 0.1f / fade->getContentHeight());
        fade->setRotation(rotation);
        fade->setFlipY(flipY);
        fade->setAnchorPoint({0.5f, flipY ? 1.f : 0.f});
        bgSquare->addChild(fade);
    };

    addEdgeFade({0, bgSquare->getContentHeight() / 2.f}, 90.f, true, false);
    addEdgeFade({bgSquare->getContentWidth(), bgSquare->getContentHeight() / 2.f}, 90.f, false, false);
    addEdgeFade({bgSquare->getContentWidth() / 2.f, 0}, 0.f, true, true);
    addEdgeFade({bgSquare->getContentWidth() / 2.f, bgSquare->getContentHeight()}, 0.f, false, true);

    auto addCornerFade = [&](CCPoint position, CCPoint anchor, bool flipX, bool flipY) {
        auto fade = CCSprite::createWithSpriteFrameName("d_gradient_02_001.png");
        fade->setColor({0, 0, 0});
        fade->setPosition(position);
        fade->setAnchorPoint(anchor);
        fade->setFlipX(flipX);
        fade->setFlipY(flipY);
        fade->setScaleX(bgSquare->getContentWidth() * 0.1f / fade->getContentWidth());
        fade->setScaleY(bgSquare->getContentHeight() * 0.1f / fade->getContentHeight());
        bgSquare->addChild(fade);
    };

    addCornerFade({0, 0}, {1, 1}, false, true);
    addCornerFade({bgSquare->getContentWidth(), 0}, {0, 1}, true, true);
    addCornerFade({0, bgSquare->getContentHeight()}, {1, 0}, false, false);
    addCornerFade({bgSquare->getContentWidth(), bgSquare->getContentHeight()}, {0, 0}, true, false);

    this->addChild(bgSquare);

    // @geode-ignore(unknown-resource)
    CCSprite* floorCircle = CCSprite::create("timestepyt.heisbehindthetree/floorCircle.png");
    floorCircle->setPosition(this->getContentSize() / 2.f);
    floorCircle->setScale(0.7f);
    floorCircle->setID("floor-circle");

    auto floorSquare = CCScale9Sprite::create("square.png");
    floorSquare->setColor({98, 43, 98});
    floorSquare->setOpacity(255);
    floorSquare->setAnchorPoint({0.5f, 1.f});
    floorSquare->ignoreAnchorPointForPosition(false);
    floorSquare->setPosition(floorCircle->getPosition());
    floorSquare->setContentWidth(60.f);
    floorSquare->setContentHeight(this->getContentHeight() / 2);
    floorSquare->setScale(0.9f);
    floorSquare->setCascadeOpacityEnabled(true);
    floorSquare->setID("floor-square");

    this->addChild(floorCircle);
    this->addChild(floorSquare);

    m_world = CCNodeRGBA::create();
    m_world->setCascadeOpacityEnabled(true);
    this->addChild(m_world);

    m_hider = ManHider::create();
    m_hider->setScale(0.8f);
    m_hider->setAnchorPoint({0.5f, 0.35f});
    m_hider->setPosition(floorCircle->getPosition() - ccp(0, 22));
    m_hider->setID("hider");

    auto tree = m_hider->getTree();
    tree->setScale(m_hider->getScale());
    tree->setAnchorPoint(m_hider->getAnchorPoint());
    tree->setPosition(m_hider->getPosition());
    tree->setID("hider-top");

    m_world->addChild(m_hider);
    m_world->addChild(tree, std::numeric_limits<int>::max());

    auto treeTouch = TouchSender::create(-1, true);
    treeTouch->setBeginCheckCallback([this, tree](CCTouch* touch, CCEvent*) {
        auto point = tree->convertTouchToNodeSpace(touch);
        return this->isEnabled() &&
            CCRect(0.f, 0.f, tree->getContentWidth(), tree->getContentHeight()).containsPoint(point);
    });
    treeTouch->setOnTouchCallback([this, tree](TouchSenderState state, CCTouch* touch, CCEvent*) {
        if (state != TouchSenderState::Ended || !this->isEnabled()) return;
#if defined(GEODE_IS_MOBILE)
        auto point = tree->convertTouchToNodeSpace(touch);
        if (CCRect(0.f, 0.f, tree->getContentWidth(), tree->getContentHeight()).containsPoint(point))
            this->runEggDialogue();
#endif
    });
    m_world->addChild(treeTouch);
    
    m_seeker = ManSeeker::create();
    m_seeker->setID("man-seeker");
    m_world->addChild(m_seeker);

    m_seekerShadow = CCSprite::createWithSpriteFrameName("d_circle_02_001.png");
    m_seekerShadow->setScaleX(0.75f);
    m_seekerShadow->setScaleY(0.3f);
    m_seekerShadow->setOpacity(55);
    m_seekerShadow->setColor({0, 0, 0});
    m_seekerShadow->setCascadeOpacityEnabled(true);
    m_seekerShadow->setID("man-seeker-shadow");
    m_world->addChild(m_seekerShadow, std::numeric_limits<int>::min());

    m_hitboxDebug = CCDrawNode::create();
    m_hitboxDebug->setID("hitbox-debug");
    m_hitboxDebug->setVisible(m_showHitboxes);
    m_world->addChild(m_hitboxDebug, std::numeric_limits<int>::max());

    m_hider->runFade(false, 0.0f);
    this->setEnabled(false);
    this->setOpacity(0);

    scheduleUpdate();

    return true;
}

void ManOptions::onEnter() {
    OptionsNode::onEnter();
    m_seeker->setPosition(m_world->convertToNodeSpace({CCDirector::get()->getWinSize().width / 2, -20}));
    this->updateSeekerConstraints();
}

void ManOptions::onOpened() {
    float fadeTime = .2f;
    this->runAction(CCFadeIn::create(fadeTime));
    this->setEnabled(true);
    m_seeker->setMovementEnabled(true);
    m_seeker->setPosition(m_world->convertToNodeSpace({CCDirector::get()->getWinSize().width / 2, -20}));
    this->updateSeekerConstraints();
    m_interactedThisOpening = false;

    if (Loader::get()->isModLoaded("timestepyt.deltarune_textboxes"))
        deltarune_textboxes::lockTextSound("Default").unwrapOrDefault();

    GameManager::get()->fadeInMusic("man.mp3"_spr);

    m_hider->runFade(true, fadeTime);
}
void ManOptions::onClosed() {
    float fadeTime = .2f;
    this->runAction(CCFadeOut::create(fadeTime));
    this->setEnabled(false);
    m_seeker->setMovementEnabled(false);
    m_activeDialogueSequences = 0;
    m_dialogueLayer = nullptr;
    m_pendingDialogueOptions.reset();
    m_pendingDialogueOptionCallback = nullptr;

    if (Loader::get()->isModLoaded("timestepyt.deltarune_textboxes"))
        deltarune_textboxes::unlockTextSound();

    GameManager::get()->playMenuMusic();

    m_hider->runFade(false, fadeTime);
}

void ManOptions::keyDown(enumKeyCodes key, double) {
    m_seeker->handleKeyDown(key);

    if (key == KEY_Z) {
        runEggDialogue();
    }
}

void ManOptions::keyUp(enumKeyCodes key, double) {
    m_seeker->handleKeyUp(key);
}

void ManOptions::update(float dt) {
    if (!m_world) return;

    m_seekerShadow->setPosition(m_seeker->getPosition() + ccp(0, 3.5f));
    m_hider->setZOrder(static_cast<int>(-m_hider->getPositionY() * 1000));
    m_seeker->setZOrder(static_cast<int>(-m_seeker->getPositionY() * 1000));
    this->updateHitboxDebug();
}

bool ManOptions::isSeekerHidden() {
    if (!m_world || !m_hider || !m_seeker) return false;

    auto seekerBounds = m_seeker->getCollisionBounds();
    auto hidingBounds = convertRect(m_hidingHitbox, m_hider->getTree(), m_world);

    auto seekerCenter = ccp(
        (seekerBounds.getMinX() + seekerBounds.getMaxX()) / 2.f,
        (seekerBounds.getMinY() + seekerBounds.getMaxY()) / 2.f
    );
    return hidingBounds.containsPoint(seekerCenter);
}

bool ManOptions::isSeekerWatchingStamp() {
    if (!m_world || !m_hider || !m_seeker) return false;

    auto seekerBounds = m_seeker->getCollisionBounds();
    auto hidingBounds = convertRect(m_stampDialogueHitbox, m_hider->getTree(), m_world);

    auto seekerCenter = ccp(
        (seekerBounds.getMinX() + seekerBounds.getMaxX()) / 2.f,
        (seekerBounds.getMinY() + seekerBounds.getMaxY()) / 2.f
    );
    return hidingBounds.containsPoint(seekerCenter);
}

void ManOptions::setHidingHitbox(cocos2d::CCRect hitbox) {
    m_hidingHitbox = hitbox;
    this->updateHitboxDebug();
}

void ManOptions::setHitboxesVisible(bool visible) {
    m_showHitboxes = visible;
    if (m_hitboxDebug) {
        m_hitboxDebug->setVisible(visible);
        this->updateHitboxDebug();
    }
}

void ManOptions::setStumpHitbox(cocos2d::CCRect hitbox) {
    m_stumpHitbox = hitbox;
    m_hasCustomStumpHitbox = true;
    this->updateSeekerConstraints();
    this->updateHitboxDebug();
}

void ManOptions::updateSeekerConstraints() {
    if (!m_world || !m_hider || !m_seeker) return;

    auto winSize = CCDirector::get()->getWinSize();
    auto scene = CCDirector::get()->getRunningScene();
    if (!scene) return;

    constexpr float bottomScreenPadding = 20.f;
    m_screenBounds = convertRect(
        {0.f, -bottomScreenPadding, winSize.width, winSize.height + bottomScreenPadding},
        scene,
        m_world
    );
    m_seeker->setMovementBounds(m_screenBounds);

    auto stump = m_hider->getStump();
    auto stumpSize = stump->getContentSize();
    auto localStumpHitbox = m_hasCustomStumpHitbox
        ? m_stumpHitbox
        : CCRect{
            stumpSize.width * 0.275f + 2,
            stumpSize.height * 0.1f - 10.f,
            stumpSize.width * 0.3f - 2,
            stumpSize.height * 0.1f
        };
    m_stumpBounds = convertRect(localStumpHitbox, stump, m_world);
    m_seeker->setObstacleBounds(m_stumpBounds);
}

void ManOptions::updateHitboxDebug() {
    if (!m_showHitboxes || !m_hitboxDebug || !m_seeker || !m_hider) return;

    m_hitboxDebug->clear();
    drawRectOutline(m_hitboxDebug, m_screenBounds, ccc4f(0.1f, 0.7f, 1.f, 1.f));
    drawRectOutline(m_hitboxDebug, m_stumpBounds, ccc4f(1.f, 0.2f, 0.2f, 1.f));
    drawRectOutline(m_hitboxDebug, m_seeker->getCollisionBounds(), ccc4f(0.2f, 1.f, 0.3f, 1.f));
    drawRectOutline(
        m_hitboxDebug,
        convertRect(m_hidingHitbox, m_hider->getTree(), m_world),
        ccc4f(1.f, 0.85f, 0.1f, 1.f)
    );
    drawRectOutline(
        m_hitboxDebug,
        convertRect(m_stampDialogueHitbox, m_hider->getTree(), m_world),
        ccc4f(1.f, 0.85f, 0.1f, 1.f)
    );
}

CCRect ManOptions::convertRect(CCRect rect, CCNode* source, CCNode* target) {
    CCPoint corners[4]{
        CCPoint{rect.getMinX(), rect.getMinY()},
        CCPoint{rect.getMaxX(), rect.getMinY()},
        CCPoint{rect.getMinX(), rect.getMaxY()},
        CCPoint{rect.getMaxX(), rect.getMaxY()},
    };

    for (auto& corner : corners) {
        corner = target->convertToNodeSpace(source->convertToWorldSpace(corner));
    }

    auto minX = corners[0].x;
    auto maxX = corners[0].x;
    auto minY = corners[0].y;
    auto maxY = corners[0].y;
    for (auto const& corner : corners) {
        minX = std::min(minX, corner.x);
        maxX = std::max(maxX, corner.x);
        minY = std::min(minY, corner.y);
        maxY = std::max(maxY, corner.y);
    }

    return {minX, minY, maxX - minX, maxY - minY};
}

CCRect ManOptions::getNodeBounds(CCNode* node, CCNode* target) {
    auto size = node->getContentSize();
    return convertRect({0.f, 0.f, size.width, size.height}, node, target);
}

void ManOptions::drawRectOutline(CCDrawNode* drawNode, CCRect rect, ccColor4F color) {
    auto bottomLeft = ccp(rect.getMinX(), rect.getMinY());
    auto bottomRight = ccp(rect.getMaxX(), rect.getMinY());
    auto topRight = ccp(rect.getMaxX(), rect.getMaxY());
    auto topLeft = ccp(rect.getMinX(), rect.getMaxY());

    drawNode->drawSegment(bottomLeft, bottomRight, 1.5f, color);
    drawNode->drawSegment(bottomRight, topRight, 1.5f, color);
    drawNode->drawSegment(topRight, topLeft, 1.5f, color);
    drawNode->drawSegment(topLeft, bottomLeft, 1.5f, color);
}

void ManOptions::runEggDialogue() {
    auto recieved = heisbehindthetree::wasEggReceived().unwrapOr(false);
    auto stored = heisbehindthetree::wasEggStored().unwrapOr(false);

    if (isSeekerWatchingStamp()) {
        if (recieved || stored || m_interactedThisOpening)
            runDialogueSequence({"(It is a tree.)"});
        else
            runDialogueSequence({"(He is behind the tree.)"});
    }

    if (!isSeekerHidden()) return;

    if (recieved || stored || m_interactedThisOpening) {
        runDialogueSequence({"(...)"});
        return;
    }

    m_interactedThisOpening = true;


    constexpr auto text1 = "(Well, there is a man here.)";
    constexpr auto text2 = "(He offered you something.)";
    constexpr auto text3 = "(You received an \"Egg\".)";

    runDialogueSequence({text1, text2}, std::pair<std::string, std::string>{"Yes", "No"},
        [this, text3](bool secondOptionSelected) {
            if (secondOptionSelected)
                this->showDefaultMessage();
            else {
                (void)heisbehindthetree::receiveEgg();
                FMODAudioEngine::sharedEngine()->playEffect("timestepyt.heisbehindthetree/snd_egg.wav");

                runDialogueSequence({text3}, std::pair<std::string, std::string>{"Ask for more", "Do Not"}, [this, text3](bool secondOptionSelected) {
                    if (secondOptionSelected)
                        this->showDefaultMessage();
                    else
                        this->runMoreDialogue();
                });
            }
        }
    );
}

void ManOptions::runDialogueSequence(
    std::vector<std::string> texts,
    std::optional<std::pair<std::string, std::string>> options,
    Function<void(bool)> onOptionSelected
) {
    if (texts.empty()) return;

    ++m_activeDialogueSequences;
    m_seeker->setMovementEnabled(false);

    if (Loader::get()->isModLoaded("timestepyt.deltarune_textboxes")) {
        runDialogueSequenceStep(std::move(texts), 0, std::move(options), std::move(onOptionSelected));
        return;
    }

    auto dialogueObjects = CCArray::create();
    for (auto const& text : texts) {
        auto dialogueObject = DialogObject::create("", text, 0, 1, false, {0, 0, 0});
        if (!dialogueObject) {
            log::error("Failed to create a Geometry Dash dialogue object for ManOptions");
            finishDialogueSequence();
            return;
        }
        dialogueObjects->addObject(dialogueObject);
    }

    auto dialogueLayer = DialogLayer::createWithObjects(dialogueObjects, 1);
    if (!dialogueLayer) {
        log::error("Failed to create a Geometry Dash dialogue layer for ManOptions");
        finishDialogueSequence();
        return;
    }

    m_pendingDialogueOptions = std::move(options);
    m_pendingDialogueOptionCallback = std::move(onOptionSelected);
    m_dialogueLayer = dialogueLayer;
    auto dialogueHook = static_cast<DTDialogLayer*>(dialogueLayer);
    dialogueHook->setProgressOnZ(true);
    dialogueHook->setCharacterHidden(true);
    dialogueLayer->m_delegate = this;
    dialogueLayer->displayNextObject();
    dialogueLayer->updateChatPlacement(DialogChatPlacement::Bottom);
    dialogueLayer->addToMainScene();
}

void ManOptions::dialogClosed(DialogLayer* layer) {
    if (layer != m_dialogueLayer) return;

    m_dialogueLayer = nullptr;
    auto options = std::move(m_pendingDialogueOptions);
    auto onOptionSelected = std::move(m_pendingDialogueOptionCallback);
    m_pendingDialogueOptions.reset();
    m_pendingDialogueOptionCallback = nullptr;

    if (!options) {
        finishDialogueSequence();
        return;
    }

    geode::createQuickPopup("", "", options->first.c_str(), options->second.c_str(),
        [this, onOptionSelected = std::move(onOptionSelected)](FLAlertLayer*, bool secondOptionSelected) mutable {
            if (onOptionSelected)
                onOptionSelected(secondOptionSelected);
            finishDialogueSequence();
        }
    );
}

void ManOptions::runDialogueSequenceStep(
    std::vector<std::string> texts,
    std::size_t index,
    std::optional<std::pair<std::string, std::string>> options,
    Function<void(bool)> onOptionSelected
) {
    const auto text = texts[index];

    auto callback = [this, texts = std::move(texts), index, options = std::move(options), onOptionSelected = std::move(onOptionSelected)](
        FLAlertLayer*, bool secondOptionSelected
    ) mutable {
        if (index + 1 < texts.size()) {
            runDialogueSequenceStep(std::move(texts), index + 1, std::move(options), std::move(onOptionSelected));
            return;
        }

        if (options) {
            constexpr auto blankTextboxText = "<cd> </c>";
            geode::createQuickPopup("", blankTextboxText, options->first.c_str(), options->second.c_str(),
                [this, onOptionSelected = std::move(onOptionSelected)](FLAlertLayer*, bool secondOptionSelected) mutable {
                    if (onOptionSelected)
                        onOptionSelected(secondOptionSelected);
                    finishDialogueSequence();
                }
            );
        }
        else {
            finishDialogueSequence();
        }
    };

    geode::createQuickPopup("", text.c_str(), "OK", nullptr, std::move(callback));
}

void ManOptions::finishDialogueSequence() {
    if (m_activeDialogueSequences == 0) return;

    m_activeDialogueSequences--;
    if (m_activeDialogueSequences == 0 && this->isEnabled())
        m_seeker->setMovementEnabled(true);
}

void ManOptions::showDefaultMessage() {
    runDialogueSequence({"(Well, there is not a man here.)"});
}

void ManOptions::runMoreDialogue() {
    runDialogueSequence({
        "(The man nodded vigorously.)",
        "(He pointed into the future.)",
        "(Over there, was a death.)",
        "(In fact, it seemed very much like the rest of your deaths.)",
        "(Craning your head, you noticed that there were many more after that.)",
        "(It felt a little bit like looking into a neverending abyss...)",
        "(Your head began to sink.)",
        "(... the man saw this and looked concerned.)",
        "(He seemed like he wanted to speak, but...)",
        "(...)",
        "(The man puts his hand softly on your shoulder)",
        "(confidently staring at your soul, he moves his hand to your chest.)",
        "(You felt a sense of relief.)",
        "(...)",
        "(Well, the man smiled.)",
        "(And he waved goodbye, for now.)"
    });
}