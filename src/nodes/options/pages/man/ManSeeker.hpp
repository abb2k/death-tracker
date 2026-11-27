#pragma once

using namespace geode::prelude;

class ManSeeker : public CCNodeRGBA, public CCTouchDelegate {
public:
    static ManSeeker* create();

    void setMovementEnabled(bool enabled);
    void setMovementBounds(cocos2d::CCRect bounds);
    void setObstacleBounds(cocos2d::CCRect bounds);
    cocos2d::CCRect getCollisionBounds();
    void handleKeyDown(enumKeyCodes key);
    void handleKeyUp(enumKeyCodes key);

private:
    bool init() override;

    void update(float delta) override;

    void onEnter() override;
    void onExit() override;

    bool m_holdingUpArrow = false;
    bool m_holdingDownArrow = false;
    bool m_holdingLeftArrow = false;
    bool m_holdingRightArrow = false;

    void updateMovementVectorKeyboard();

    bool ccTouchBegan(CCTouch* pTouch, CCEvent* pEvent) override;
    void ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent) override;
    void ccTouchEnded(CCTouch* pTouch, CCEvent* pEvent) override;
    void ccTouchCancelled(CCTouch* pTouch, CCEvent* pEvent) override;

    void updateMovementVectorTouch(CCTouch* pTouch);
    
    CCPoint m_movementVector;
    CCPoint m_touchStartLocation;
    cocos2d::CCRect m_movementBounds;
    cocos2d::CCRect m_obstacleBounds;
    bool m_hasMovementBounds = false;
    bool m_hasObstacleBounds = false;

    CCNodeRGBA* m_visualsHolder = nullptr;

    static constexpr float m_movementSpeed = 50.f;
    static constexpr float m_touchMovementThreshold = 10.f;
    static constexpr int m_movementTickSpeed = 240;
    bool m_movementEnabled = false;
};