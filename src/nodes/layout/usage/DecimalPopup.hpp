#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class DTLabel;

class DecimalPopup : public CCNode {
public:
    static DecimalPopup* create(CCSize size);

    void setDisplay(CCNode* origin, const Run& run, bool isSess, uint8_t decimalDepth, DTLabel* referenceLabel, std::optional<std::pair<long long, size_t>> occurrenceSelection = std::nullopt);

    void close();

    bool isTouchOnMe(CCTouch *pTouch, bool begin);

private:
    bool init(CCSize size);

    DTLabel* m_label;

    CCScale9Sprite* m_bg;

    ScrollLayer* m_scroll;

    float m_maxScrollHeight;

    void update(float dt) override;

    void onEnter() override;
    void onExit() override;

    comm::ListenerHandle m_donerListenerHandle;

    void onClose(CCObject* sender);

    Scrollbar* m_scrollBargame;

    bool m_isHolding = false;
};