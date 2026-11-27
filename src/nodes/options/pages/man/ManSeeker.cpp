#include "ManSeeker.hpp"

ManSeeker* ManSeeker::create() {
    auto ret = new ManSeeker();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool ManSeeker::init() {
    if (!CCNodeRGBA::init()) return false;

    this->setCascadeColorEnabled(true);
    this->setCascadeOpacityEnabled(true);

    this->setContentSize({42, 42});
    this->setAnchorPoint({0.5f, 0.1f});

    m_visualsHolder = CCNodeRGBA::create();
    m_visualsHolder->setPosition(this->getContentSize() / 2.f);
    m_visualsHolder->setCascadeOpacityEnabled(true);
    this->addChild(m_visualsHolder);

    auto manSeekerC1 = CCSprite::createWithSpriteFrameName("manSeeker_1.png"_spr);
    manSeekerC1->setColor({117, 251, 237});
    manSeekerC1->setCascadeOpacityEnabled(true);
    m_visualsHolder->addChild(manSeekerC1);

    auto manSeekerC2 = CCSprite::createWithSpriteFrameName("manSeeker_0.png"_spr);
    manSeekerC2->setColor({11, 11, 59});
    manSeekerC2->setCascadeOpacityEnabled(true);
    m_visualsHolder->addChild(manSeekerC2);

    auto manSeekerExtra = CCSprite::createWithSpriteFrameName("manSeeker_2.png"_spr);
    manSeekerExtra->setCascadeOpacityEnabled(true);
    m_visualsHolder->addChild(manSeekerExtra);

    auto manSeekerGlow = CCSprite::createWithSpriteFrameName("manSeeker_3.png"_spr);
    manSeekerGlow->setCascadeOpacityEnabled(true);
    m_visualsHolder->addChild(manSeekerGlow);



    schedule(schedule_selector(ManSeeker::update), 1.0f / m_movementTickSpeed);

    return true;
}

void ManSeeker::onEnter() {
    CCNode::onEnter();
    CCTouchDispatcher::get()->addTargetedDelegate(this, 0, false);
}

void ManSeeker::onExit() {
    CCTouchDispatcher::get()->removeDelegate(this);
    CCNode::onExit();
}

void ManSeeker::handleKeyDown(enumKeyCodes key) {
    if (!m_movementEnabled) return;

    switch (key) {
        case enumKeyCodes::KEY_Up:
            m_holdingUpArrow = true;
            break;
        case enumKeyCodes::KEY_Down:
            m_holdingDownArrow = true;
            break;
        case enumKeyCodes::KEY_Left:
            m_holdingLeftArrow = true;
            break;
        case enumKeyCodes::KEY_Right:
            m_holdingRightArrow = true;
            break;
        default:
            break;
    }

    this->updateMovementVectorKeyboard();
}

void ManSeeker::handleKeyUp(enumKeyCodes key) {
    switch (key) {
        case enumKeyCodes::KEY_Up:
            m_holdingUpArrow = false;
            break;
        case enumKeyCodes::KEY_Down:
            m_holdingDownArrow = false;
            break;
        case enumKeyCodes::KEY_Left:
            m_holdingLeftArrow = false;
            break;
        case enumKeyCodes::KEY_Right:
            m_holdingRightArrow = false;
            break;
        default:
            break;
    }

    this->updateMovementVectorKeyboard();
}


void ManSeeker::updateMovementVectorKeyboard() {
    m_movementVector = ccp(0, 0);
    if (m_holdingUpArrow) m_movementVector.y += 1;
    if (m_holdingDownArrow) m_movementVector.y -= 1;
    if (m_holdingLeftArrow) m_movementVector.x -= 1;
    if (m_holdingRightArrow) m_movementVector.x += 1;
}

bool ManSeeker::ccTouchBegan(CCTouch* pTouch, CCEvent* pEvent) {
#if defined(GEODE_IS_MOBILE)
    m_movementVector = ccp(0, 0);
    if (!m_movementEnabled) return false;

    m_touchStartLocation = pTouch->getLocation();
    m_movementVector = ccp(0, 0);
    return true;
#else
    return false;
#endif
}
void ManSeeker::ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent) {
    this->updateMovementVectorTouch(pTouch);
}
void ManSeeker::ccTouchEnded(CCTouch* pTouch, CCEvent* pEvent) {
    m_movementVector = ccp(0, 0);
}
void ManSeeker::ccTouchCancelled(CCTouch* pTouch, CCEvent* pEvent) {
    m_movementVector = ccp(0, 0);
}

void ManSeeker::updateMovementVectorTouch(CCTouch* pTouch) {
    auto touchDelta = pTouch->getLocation() - m_touchStartLocation;
    if (ccpLength(touchDelta) <= m_touchMovementThreshold) {
        m_movementVector = ccp(0, 0);
        return;
    }

    m_movementVector = ccpNormalize(touchDelta);
}

void ManSeeker::update(float delta) {
    if (!this->getParent()) return;

    if (m_movementEnabled) {
        if (m_movementVector.x > 0)
            m_visualsHolder->setScaleX(1.f);
        else if (m_movementVector.x < 0)
            m_visualsHolder->setScaleX(-1.f);

        auto current = this->getPosition();
        auto next = current + m_movementVector * m_movementSpeed * delta;

        auto currentBounds = this->getCollisionBounds();
        auto left = currentBounds.getMinX() - current.x;
        auto right = currentBounds.getMaxX() - current.x;
        auto bottom = currentBounds.getMinY() - current.y;
        auto top = currentBounds.getMaxY() - current.y;

        if (m_hasMovementBounds) {
            next.x = std::clamp(next.x, m_movementBounds.getMinX() - left, m_movementBounds.getMaxX() - right);
            next.y = std::clamp(next.y, m_movementBounds.getMinY() - bottom, m_movementBounds.getMaxY() - top);
        }

        if (m_hasObstacleBounds) {
            auto obstacleLeft = m_obstacleBounds.getMinX();
            auto obstacleRight = m_obstacleBounds.getMaxX();
            auto obstacleBottom = m_obstacleBounds.getMinY();
            auto obstacleTop = m_obstacleBounds.getMaxY();

            auto verticalOverlap = current.y + top > obstacleBottom && current.y + bottom < obstacleTop;
            if (verticalOverlap) {
                if (m_movementVector.x > 0.f && current.x + right <= obstacleLeft && next.x + right > obstacleLeft) {
                    next.x = obstacleLeft - right;
                }
                else if (m_movementVector.x < 0.f && current.x + left >= obstacleRight && next.x + left < obstacleRight) {
                    next.x = obstacleRight - left;
                }
            }

            auto horizontalOverlap = next.x + right > obstacleLeft && next.x + left < obstacleRight;
            if (horizontalOverlap) {
                if (m_movementVector.y > 0.f && current.y + top <= obstacleBottom && next.y + top > obstacleBottom) {
                    next.y = obstacleBottom - top;
                }
                else if (m_movementVector.y < 0.f && current.y + bottom >= obstacleTop && next.y + bottom < obstacleTop) {
                    next.y = obstacleTop - bottom;
                }
            }
        }

        this->setPosition(next);
    }
}

void ManSeeker::setMovementBounds(cocos2d::CCRect bounds) {
    m_movementBounds = bounds;
    m_hasMovementBounds = true;

    auto position = this->getPosition();
    auto collisionBounds = this->getCollisionBounds();
    auto left = collisionBounds.getMinX() - position.x;
    auto right = collisionBounds.getMaxX() - position.x;
    auto bottom = collisionBounds.getMinY() - position.y;
    auto top = collisionBounds.getMaxY() - position.y;
    position.x = std::clamp(position.x, bounds.getMinX() - left, bounds.getMaxX() - right);
    position.y = std::clamp(position.y, bounds.getMinY() - bottom, bounds.getMaxY() - top);
    this->setPosition(position);
}

cocos2d::CCRect ManSeeker::getCollisionBounds() {
    auto size = this->getContentSize();
    auto anchor = this->getAnchorPoint();
    auto constexpr hitboxWidthFactor = 0.9f;
    auto hitboxWidth = size.width * hitboxWidthFactor - 3;
    auto left = this->getPositionX() - anchor.x * size.width + (size.width - hitboxWidth) / 2.f;
    auto bottom = this->getPositionY() - anchor.y * size.height + 5;
    auto constexpr hitboxHeightFactor = 0.2f;
    return {left, bottom, hitboxWidth, size.height * hitboxHeightFactor};
}

void ManSeeker::setObstacleBounds(cocos2d::CCRect bounds) {
    m_obstacleBounds = bounds;
    m_hasObstacleBounds = bounds.size.width > 0.f && bounds.size.height > 0.f;
}

void ManSeeker::setMovementEnabled(bool enabled) {
    m_movementEnabled = enabled;
    if (!enabled) {
        m_holdingUpArrow = false;
        m_holdingDownArrow = false;
        m_holdingLeftArrow = false;
        m_holdingRightArrow = false;
        m_movementVector = ccp(0, 0);
    }
}