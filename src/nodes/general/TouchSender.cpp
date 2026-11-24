#include "TouchSender.hpp"

TouchSender* TouchSender::create(int prio, bool swallowTouches) {
    auto ret = new TouchSender();

    if (ret && ret->init(prio, swallowTouches)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool TouchSender::init(int prio, bool swallowTouches) {
    if (!CCMenu::init()) return false;
    this->m_prio = prio;
    this->m_swallowTouches = swallowTouches;

    return true;
}

void TouchSender::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, m_prio, m_swallowTouches);
}

bool TouchSender::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    bool toReturn = false;
    if (m_beginCheck != NULL) {
        toReturn = m_beginCheck(touch, event);
    }

    if (m_onTouch != NULL) {
        m_onTouch(toReturn ? TouchSenderState::BeganSuccessful : TouchSenderState::BeganFailed, touch, event);
    }

    return toReturn;
}

void TouchSender::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    if (m_onTouch != NULL) {
        m_onTouch(TouchSenderState::Moved, touch, event);
    }
}

void TouchSender::ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    if (m_onTouch != NULL) {
        m_onTouch(TouchSenderState::Ended, touch, event);
    }
}

void TouchSender::ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    if (m_onTouch != NULL) {
        m_onTouch(TouchSenderState::Cancled, touch, event);
    }
}

void TouchSender::setBeginCheckCallback(Function<bool(cocos2d::CCTouch* touch, cocos2d::CCEvent* event)> beginCheck) {
    this->m_beginCheck = std::move(beginCheck);
}

void TouchSender::setOnTouchCallback(Function<void(TouchSenderState, cocos2d::CCTouch* touch, cocos2d::CCEvent* event)> onTouch) {
    this->m_onTouch = std::move(onTouch);
}