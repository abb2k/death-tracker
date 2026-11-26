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
    DTLabel* m_originLabel = nullptr;
    CCPoint m_originTopRight;
    CCPoint m_originBottomLeft;
    CCPoint m_originBottomRight;
    CCPoint m_originTopLeft;

    CCScale9Sprite* m_bg;
    CCMenuItemSpriteExtra* m_closeButton = nullptr;

    ScrollLayer* m_scroll;

    float m_maxPopupHeight = 0.f;
    float m_maxScrollHeight;

    void update(float dt) override;
    void positionToOrigin();

    void onEnter() override;
    void onExit() override;

    comm::ListenerHandle m_donerListenerHandle;

    void onClose(CCObject* sender);

    Scrollbar* m_scrollBargame;

    bool m_isHolding = false;
};