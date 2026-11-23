#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

enum TouchSenderState{
    BeganSuccessful,
    BeganFailed,
    Ended,
    Cancled,
    Moved
};

class TouchSender : public CCMenu {
    public:
        static TouchSender* create(int prio, bool swallowTouches);

        geode::Function<bool(cocos2d::CCTouch* touch, cocos2d::CCEvent* event)> beginCheck = NULL;
        geode::Function<void(TouchSenderState, cocos2d::CCTouch* touch, cocos2d::CCEvent* event)> onTouch = NULL;

    private:
        bool init(int prio, bool swallowTouches);

        int prio;
        bool swallowTouches;

        void registerWithTouchDispatcher() override;
        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
};