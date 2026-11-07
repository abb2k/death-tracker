#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

#include <nodes/DTLabel.hpp>

using namespace geode::prelude;

class DecimalPopup : public CCNode {
    public:
        static DecimalPopup* create(CCSize size);

        void setDisplay(CCNode* origin, const Run& run, bool isSess, uint8_t decimalDepth, DTLabel* referenceLabel);

        void close();

        bool isTouchOnMe(CCTouch *pTouch, bool begin);

    private:
        bool init(CCSize size);

        DTLabel* m_label;

        ScrollLayer* m_scroll;

        void update(float dt) override;

        void onEnter() override;
        void onExit() override;

        comm::ListenerHandle donerListenerHandle;

        void onClose(CCObject* sender);

        Scrollbar* scrollBargame;

        bool isHolding;
};