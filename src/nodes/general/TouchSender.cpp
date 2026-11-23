#include "TouchSender.hpp"

TouchSender* TouchSender::create(int prio, bool swallowTouches) {
    auto ret = new TouchSender();
    if (ret && ret->init(prio, swallowTouches)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool TouchSender::init(int prio, bool swallowTouches) {
    if (!CCMenu::init()) return false;
    this->prio = prio;
    this->swallowTouches = swallowTouches;

    return true;
}

void TouchSender::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, prio, swallowTouches);
}
bool TouchSender::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    bool toReturn = false;
    if (beginCheck != NULL) {
        toReturn = beginCheck(touch, event);
    }

    if (onTouch != NULL) {
        onTouch(toReturn ? TouchSenderState::BeganSuccessful : TouchSenderState::BeganFailed, touch, event);
    }

    return toReturn;
}
void TouchSender::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    if (onTouch != NULL) {
        onTouch(TouchSenderState::Moved, touch, event);
    }
}
void TouchSender::ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    if (onTouch != NULL) {
        onTouch(TouchSenderState::Ended, touch, event);
    }
}
void TouchSender::ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    if (onTouch != NULL) {
        onTouch(TouchSenderState::Cancled, touch, event);
    }
}