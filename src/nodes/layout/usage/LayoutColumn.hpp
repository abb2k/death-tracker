#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class DTLabel;

class LayoutColumn : public CCMenu {
public:
    static LayoutColumn* create(DTColumnInfo info, bool startVisible, float minHeight);

    DTColumnInfo m_info;

    void setColor(ccColor3B color);

    void addLabel(DTLabel* label);
    void removeLabel(DTLabel* label);

    std::map<int, DTLabel*> m_labels{};

    void updateLabelPosition(DTLabel* label);

    static float borderWidth;

    void setContentHeight(float height);
    void setContentWidth(float width);

    static float addNewBtnOffset;

    void destroyColumnAndCleanup();

    void refreshAllLabelsLayer();
    CCScale9Sprite* m_bgSpr;
    CCScale9Sprite* m_topSpr;
    CCScale9Sprite* m_topBorder2;

    static float topHeight;

    void updateSizesByContent();

    void setVisibility(bool visible);
    
private:
    bool init(DTColumnInfo info, bool startVisible, float minHeight);

    float m_minHeight;

    bool m_isVisible;

    int m_bgOpacity = 125;

    CCScale9Sprite* m_topBorder1;
    
    CCScale9Sprite* m_topBorder3;
    CCScale9Sprite* m_bgSideBorder;

    CCMenu* m_menu;
    CCPoint m_tempMenuPos;

    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

    bool m_isTouchingSide;
    bool m_isTouchingTopSpr;
    CCPoint m_originalTopSprTouchPos;
    float m_maxMovementOffset = 3;

    void onAddLabelBtnClicked(CCObject*);

    void update(float dt) override;

    void onOrganized(float deltaMove);
};