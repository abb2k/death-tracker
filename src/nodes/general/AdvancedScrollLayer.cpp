#include "AdvancedScrollLayer.hpp"

AdvancedScrollLayer* AdvancedScrollLayer::create(CCSize size, CCSize limits) {
    auto ret = new AdvancedScrollLayer();

    if (ret && ret->init(std::move(size), std::move(limits))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool AdvancedScrollLayer::init(CCSize size, CCSize limits) {
    if (!CCLayer::init()) return false;
    
    this->setContentSize(size);

    CCDrawNode* stencil = CCDrawNode::create();

    CCPoint rect[4] = {
        ccp(0, 0),
        ccp(size.width, 0),
        ccp(size.width, size.height),
        ccp(0, size.height)
    };

    stencil->drawPolygon(rect, 4, ccc4f(1, 1, 1, 1), 0, ccc4f(1, 1, 1, 1));

    CCClippingNode* clip = CCClippingNode::create(stencil);
    clip->setAnchorPoint(ccp(0, 0));
    clip->setPosition(ccp(0, 0));
    clip->setContentSize(size);

    m_zoomParent = CCNode::create();
    m_zoomParent->setContentSize(limits);
    m_zoomParent->setAnchorPoint({.5f, .5f});
    m_zoomParent->setPosition(size / 2);
    clip->addChild(m_zoomParent);

    m_content = CCNode::create();
    m_content->setContentSize(limits);
    m_zoomParent->addChild(m_content);

    m_widthScrollExt = new CCScrollLayerExt({0, 0, size.height, size.width});
    m_widthScrollExt->autorelease();
    m_widthScrollExt->setMouseEnabled(false);
    m_widthScrollExt->setTouchEnabled(false);
    m_widthScrollExt->setKeyboardEnabled(false);
    m_widthScrollExt->setKeypadEnabled(false);
    
    m_horizontalBar = Scrollbar::create(m_widthScrollExt);
    m_horizontalBar->setPositionX(size.width / 2);
    m_horizontalBar->setRotation(90);
    setHorizontalScrollbarPosition(false);
    this->addChild(m_horizontalBar);

    m_heightScrollExt = new CCScrollLayerExt({0, 0, size.width, size.height});
    m_heightScrollExt->autorelease();
    m_heightScrollExt->setMouseEnabled(false);
    m_heightScrollExt->setTouchEnabled(false);
    m_heightScrollExt->setKeyboardEnabled(false);
    m_heightScrollExt->setKeypadEnabled(false);

    m_verticalBar = Scrollbar::create(m_heightScrollExt);
    m_verticalBar->setPositionY(size.height / 2);
    m_verticalBar->setPositionX(size.width + 4);
    this->addChild(m_verticalBar);

    updateProgressBars();

    resetPositioning();

    schedule(schedule_selector(AdvancedScrollLayer::updatePositionByProgBars));

    this->addChild(clip);

    this->setKeypadEnabled(true);
    this->setKeyboardEnabled(true);
    this->setMouseEnabled(true);
    this->setTouchEnabled(true);
    
    m_allTouches = CCSet::create();
    m_allTouches->retain();

    return true;
}

float AdvancedScrollLayer::getMinimumPosition(bool horizontal) {
    CCSize viewSize = this->getContentSize();
    CCSize contentSize = m_zoomParent->getContentSize();
    
    float scaleFactor;

    if (horizontal)
        scaleFactor = viewSize.width / contentSize.width;
    else
        scaleFactor = viewSize.height / contentSize.height;

    float length;

    if (horizontal)
        length = viewSize.width - contentSize.width;
    else
        length = viewSize.height - contentSize.height;
    
    return (m_zoomParent->getScale() - scaleFactor) / (1 - scaleFactor) * (length / 2) / m_zoomParent->getScale();
}

float AdvancedScrollLayer::getMaximumPosition(bool horizontal) {
    return -getMinimumPosition(horizontal);
}

void AdvancedScrollLayer::updateProgressBars() {
    m_widthScrollExt->m_contentLayer->setPositionY(m_content->getPositionX() - getMinimumPosition(true) * -1);
    m_widthScrollExt->m_contentLayer->setContentHeight(abs(getMinimumPosition(true) * 2) + m_zoomParent->getPositionX() * 2);

    m_heightScrollExt->m_contentLayer->setPositionY(m_content->getPositionY() - getMinimumPosition(false) * -1);
    m_heightScrollExt->m_contentLayer->setContentHeight(abs(getMinimumPosition(false) * 2) + m_zoomParent->getPositionY() * 2);
}

void AdvancedScrollLayer::updatePositionByProgBars(float dt) {
    m_content->setPositionX(m_widthScrollExt->m_contentLayer->getPositionY() - getMinimumPosition(true));

    m_content->setPositionY(m_heightScrollExt->m_contentLayer->getPositionY() - getMinimumPosition(false));
}

bool AdvancedScrollLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!m_isEnabled) return true;

    auto const size = this->getContentSize();
    auto const pos = this->convertToNodeSpace(touch->getLocation());
    auto const rect = CCRect{0, 0, size.width, size.height};

    if (!rect.containsPoint(pos)) return false;

    this->ccTouchMoved(nullptr, event);

    m_touchDown = true;

    if (m_allTouches->count() == 1) {
        auto firstLoc = static_cast<CCTouch*>(m_allTouches->anyObject())->getLocation();
        auto secondLoc = touch->getLocation();

        //super special thanks for the better edit pinch to zoom functionallity :) it much good :thumbsup:

        m_initialMinPoint = (m_zoomParent->convertToNodeSpace(firstLoc) + m_zoomParent->convertToNodeSpace(secondLoc)) / 2.f;
        m_initialScale = m_zoomParent->getScale();
        m_initialDistance = firstLoc.getDistance(secondLoc);
    }

    if (!m_allTouches->containsObject(touch))
        m_allTouches->addObject(touch);

    return true;
}

void AdvancedScrollLayer::ccTouchEnded(CCTouch* touch, CCEvent*) {
    m_touchDown = false;

    if (m_allTouches->containsObject(touch))
        m_allTouches->removeObject(touch);
}

void AdvancedScrollLayer::ccTouchCancelled(CCTouch* touch, CCEvent*) {
    m_touchDown = false;

    if (m_allTouches->containsObject(touch))
        m_allTouches->removeObject(touch);
}

void AdvancedScrollLayer::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (!m_isEnabled || touch == nullptr) return;

    if (m_allTouches->count() > 1) {
        ccTouchesMoved(m_allTouches, nullptr);
        return;
    }

    if (!this->boundingBox().containsPoint(this->getParent()->convertToNodeSpace(touch->getLocation()))) return;

    CCPoint curr = this->convertToNodeSpace(touch->getLocationInView());
    CCPoint prev = this->convertToNodeSpace(touch->m_prevPoint);
    CCPoint delta = curr - prev;
    
    moveBy(delta * m_scrollSense * ccp(1, -1) / m_zoomParent->getScale());
}

void AdvancedScrollLayer::ccTouchesMoved(CCSet* touches, CCEvent* event) {
    if (!m_isEnabled || touches == nullptr) return;
    
    if (touches->count() != 2) {
        ccTouchMoved(static_cast<CCTouch*>(touches->anyObject()), event);
        return;
    }
    CCSetIterator it = touches->begin();
    CCTouch* touch1 = static_cast<CCTouch*>(*it);
    ++it;
    CCTouch* touch2 = static_cast<CCTouch*>(*it);

    if (!this->boundingBox().containsPoint(this->getParent()->convertToNodeSpace(touch1->getLocation()))) return;

    auto firstLoc = touch1->getLocation();
    auto secondLoc = touch2->getLocation();

    auto center = (m_zoomParent->convertToNodeSpace(firstLoc) + m_zoomParent->convertToNodeSpace(secondLoc)) / 2.f;
    auto distNow = firstLoc.getDistance(secondLoc);
    
    auto const mult = m_initialDistance / distNow;

    float d1 = touch1->getLocation().getDistance(touch1->getPreviousLocation());
    float d2 = touch2->getLocation().getDistance(touch2->getPreviousLocation());

    float result;
    if (d1 + d2 == 0.0f)
        result = 0.5f;
    else
        result = d1 / (d1 + d2);

    zoomToAndMove(m_initialScale / mult, result);
}

bool AdvancedScrollLayer::scrollWheel(float y, float x) {
    if (!m_isEnabled) return false;

    if (!m_scrollMovement)
        zoomToAndMove(m_zoomParent->getScale() + -y * m_zoomSensetivity);
    else {
        if (CCKeyboardDispatcher::get()->getControlKeyPressed()) {
            zoomToAndMove(m_zoomParent->getScale() + -y * m_zoomSensetivity);
        }
        else if (!m_holdingShift)
            moveBy(ccp(x, y) * m_scrollSense);
        else
            moveBy(ccp(y, x) * m_scrollSense);
    }

    //log::info("{}", ccp(y, x));
    return true;
}

void AdvancedScrollLayer::keyDown(enumKeyCodes key, double d) {
    if (key == enumKeyCodes::KEY_LeftShift) {
        m_holdingShift = true;
    }

    CCLayer::keyDown(key, d);
}

void AdvancedScrollLayer::keyUp(enumKeyCodes key, double d) {
    if (key == enumKeyCodes::KEY_LeftShift) {
        m_holdingShift = false;
    }
}

void AdvancedScrollLayer::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 1, true);
}

void AdvancedScrollLayer::setHorizontalScrollbarPosition(bool onTop) {
    m_horizontalBar->setPositionY(onTop ? this->getContentHeight() + 4 : -4);
}

void AdvancedScrollLayer::setLimits(CCSize newLimits) {
    CCSize oldSize = m_content->getContentSize();
    CCPoint oldPosition = m_content->getPosition();
    float oldScale = m_zoomParent->getScale();

    CCSize newLimitedLimits = m_content->getContentSize();
    if (newLimits.width > this->getContentWidth()) newLimitedLimits.width = newLimits.width;
    if (newLimits.height > this->getContentHeight()) newLimitedLimits.height = newLimits.height;

    m_content->setContentSize(newLimitedLimits);
    m_zoomParent->setContentSize(newLimitedLimits);

    if (m_gridNode != nullptr)
        drawGrid(m_gridsquareSize, m_gridLineWidth, m_gridColor);

    zoomTo(oldScale);

    CCPoint oldCenterPoint = oldSize / 2 - oldPosition / oldScale;
    CCPoint newPosition = (newLimitedLimits / 2 - oldCenterPoint) * m_zoomParent->getScale();
    moveTo(newPosition);
}

void AdvancedScrollLayer::setLimitsWidth(float width) {
    setLimits({width, m_content->getContentHeight()});
}
void AdvancedScrollLayer::setLimitsHeight(float height) {
    setLimits({m_content->getContentWidth(), height});
}

void AdvancedScrollLayer::drawGrid(float squareSize, float lineWidth, ccColor4B color) {
    if (m_gridNode != nullptr) m_gridNode->clear();
    else {
        m_gridNode = CCDrawNode::create();
        m_gridNode->m_bUseArea = false;
        m_content->addChild(m_gridNode);
    }

    m_gridsquareSize = squareSize;
    m_gridLineWidth = lineWidth;
    m_gridColor = color;

    std::vector<CCPoint> gridLines{};

    float sizeX = m_content->getContentWidth();
    float sizeY = m_content->getContentHeight();

    float stepX = squareSize;
    for (float i = 0; i <= sizeX; i += stepX)
    {
        m_gridNode->drawSegment({i, 0}, {i, sizeY}, lineWidth, ccc4FFromccc4B(color));
    }

    float stepY = squareSize;
    for (float i = 0; i <= sizeY; i += stepY)
    {
        m_gridNode->drawSegment({0, i}, {sizeX, i}, lineWidth, ccc4FFromccc4B(color));
    }
}

void AdvancedScrollLayer::addFromCenter(CCNode* child, CCPoint offset) {
    m_content->addChild(child);
    child->setPosition(m_content->getContentSize() / 2 + offset);
}

void AdvancedScrollLayer::toggleHorizontalScrollbar(bool b) {
    m_horizontalBar->setVisible(b);
}

void AdvancedScrollLayer::toggleVerticalScrollbar(bool b) {
    m_verticalBar->setVisible(b);
}

void AdvancedScrollLayer::setScrollSensetivity(float sense) {
    m_scrollSense = sense;
}

void AdvancedScrollLayer::moveBy(CCPoint amount) {
    moveTo(m_content->getPosition() + amount);
}

void AdvancedScrollLayer::moveTo(CCPoint newPos) {
    CCSize viewSize = this->getContentSize();
    CCSize contentSize = m_zoomParent->getContentSize();

    float scaleFactorX = viewSize.width / contentSize.width;
    float lengthX = viewSize.width - contentSize.width;
    float minX = (m_zoomParent->getScale() - scaleFactorX) / (1 - scaleFactorX) * (lengthX / 2) / m_zoomParent->getScale();
    float maxX = -minX;

    float scaleFactorY = viewSize.height / contentSize.height;
    float lengthY = viewSize.height - contentSize.height;
    float minY = (m_zoomParent->getScale() - scaleFactorY) / (1 - scaleFactorY) * (lengthY / 2) / m_zoomParent->getScale();
    float maxY = -minY;

    float clampedX = std::clamp(newPos.x, minX, maxX);
    float clampedY = std::clamp(newPos.y, minY, maxY);

    //log::info("new pos {} | {} - {} | min ({}, {}) | max ({}, {}) | clamped ({}, {})", newPos, viewSize, contentSize, minX, minY, maxX, maxY, clampedX, clampedY);

    m_content->setPosition({clampedX, clampedY});

    updateProgressBars();
}

void AdvancedScrollLayer::resetPositioning() {
    m_content->setPosition({0, 0});
    zoomTo(1);
}

void AdvancedScrollLayer::moveToCorner(bool left, bool bottom) {
    CCSize corners = m_zoomParent->getScaledContentSize() / 2;

    float XToGoTo = corners.width * (left ? 1 : -1);
    float YToGoTo = corners.height * (bottom ? 1 : -1);
    
    //log::info("{} | {} || {}", XToGoTo, YToGoTo, corners);

    moveTo({XToGoTo, YToGoTo});
}

void AdvancedScrollLayer::zoomBy(float zoomAmount) {
    zoomTo(m_zoomParent->getScale() + zoomAmount);
}

void AdvancedScrollLayer::zoomToAndMove(float zoomAmount, float betweenDelta) {
    auto touchPos = getMousePos();

    if (m_allTouches->count() == 2) {
        CCSetIterator it = m_allTouches->begin();
        CCTouch* touch1 = static_cast<CCTouch*>(*it);
        ++it;
        CCTouch* touch2 = static_cast<CCTouch*>(*it);
        touchPos = touch1->getLocation().lerp(touch2->getLocation(), betweenDelta);
    }

    auto posBeforeZoom = m_zoomParent->convertToNodeSpace(touchPos);
    zoomTo(zoomAmount);
    auto posAfterZoom = m_zoomParent->convertToNodeSpace(touchPos);
    moveBy(posAfterZoom - posBeforeZoom);
}

void AdvancedScrollLayer::zoomTo(float zoomAmount) {
    if (m_zoomParent->getContentHeight() * zoomAmount < this->getContentHeight()) zoomAmount = this->getContentHeight() / m_zoomParent->getContentHeight();
    if (m_zoomParent->getContentWidth() * zoomAmount < this->getContentWidth()) zoomAmount = this->getContentWidth() / m_zoomParent->getContentWidth();
    if (zoomAmount > m_minZoom) zoomAmount = m_minZoom;
    if (zoomAmount < m_maxZoom) zoomAmount = m_maxZoom;

    m_zoomParent->setScale(zoomAmount);

    moveTo(m_content->getPosition());
}

void AdvancedScrollLayer::zoomToMinimum() {
    CCPoint toUse;
    if (m_zoomParent->getContentWidth() > m_zoomParent->getContentHeight())
        toUse = ccp(this->getContentWidth(), m_zoomParent->getContentWidth());
    else
        toUse = ccp(this->getContentHeight(), m_zoomParent->getContentHeight());

    zoomTo(toUse.x / toUse.y);
}

void AdvancedScrollLayer::zoomToMaximum() {
    zoomTo(m_minZoom);
}

bool AdvancedScrollLayer::isAtMaxZoom() {
    float minAmount = m_minZoom;

    if (m_zoomParent->getContentHeight() * minAmount < this->getContentHeight()) minAmount = this->getContentHeight() / m_zoomParent->getContentHeight();
    if (m_zoomParent->getContentWidth() * minAmount < this->getContentWidth()) minAmount = this->getContentWidth() / m_zoomParent->getContentWidth();
    if (minAmount > m_minZoom) minAmount = m_minZoom;
    if (minAmount < m_maxZoom) minAmount = m_maxZoom;

    return m_zoomParent->getScale() == minAmount;
}

bool AdvancedScrollLayer::isAtMinZoom() {
    float maxAmount = m_maxZoom;

    if (m_zoomParent->getContentHeight() * maxAmount < this->getContentHeight()) maxAmount = this->getContentHeight() / m_zoomParent->getContentHeight();
    if (m_zoomParent->getContentWidth() * maxAmount < this->getContentWidth()) maxAmount = this->getContentWidth() / m_zoomParent->getContentWidth();
    if (maxAmount > m_minZoom) maxAmount = m_minZoom;
    if (maxAmount < m_maxZoom) maxAmount = m_maxZoom;

    return m_zoomParent->getScale() == maxAmount;
}


float AdvancedScrollLayer::getCurrentZoom() {
    return m_zoomParent->getScale();
}

void AdvancedScrollLayer::setEnabled(bool b) {
    m_isEnabled = b;
}

AdvancedScrollLayer::~AdvancedScrollLayer() {
    if (m_allTouches != nullptr)
        m_allTouches->release();
}