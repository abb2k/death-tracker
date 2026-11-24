#pragma once

using namespace geode::prelude;

class AdvancedScrollLayer : public CCLayer {
public:
    static AdvancedScrollLayer* create(CCSize size, CCSize limits);

    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchesMoved(CCSet* touches, CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

    virtual void keyDown(enumKeyCodes key, double d)  override;
    virtual void keyUp(enumKeyCodes key, double d)  override;
    
    CCNode* m_content;

    bool m_scrollMovement = true;

    void setLimits(CCSize newLimits);
    void setLimitsWidth(float width);
    void setLimitsHeight(float height);

    void setHorizontalScrollbarPosition(bool onTop);

    void drawGrid(float squaresAmount, float lineWidth, ccColor4B color);

    void addFromCenter(CCNode* child, CCPoint offset = {0, 0});

    void toggleHorizontalScrollbar(bool b);
    void toggleVerticalScrollbar(bool b);
    
    void setScrollSensetivity(float sense);

    void moveBy(CCPoint amount);
    void moveTo(CCPoint pos);
    
    void resetPositioning();

    void moveToCorner(bool left, bool bottom);

    void zoomBy(float zoomAmount);
    void zoomToAndMove(float zoomAmount, float betweenDelta = .5f);
    void zoomTo(float zoomAmount);

    void zoomToMinimum();
    void zoomToMaximum();
    bool isAtMaxZoom();
    bool isAtMinZoom();


    float getCurrentZoom();

    void setEnabled(bool b);

    Scrollbar* m_horizontalBar;
    Scrollbar* m_verticalBar;

    float m_minZoom = 2;
    float m_maxZoom = .5f;

    static constexpr float m_zoomSensetivity = 0.01f;

    float getMinimumPosition(bool horizontal);
    float getMaximumPosition(bool horizontal);

    ~AdvancedScrollLayer();
private:
    bool init(CCSize size, CCSize limits);

    CCNode* m_zoomParent;

    float m_scrollSense = 1;

    bool m_isEnabled = true;

    CCScrollLayerExt* m_widthScrollExt;
    CCScrollLayerExt* m_heightScrollExt;

    void updateProgressBars();
    void updatePositionByProgBars(float dt);

    bool m_touchDown = false;

    bool scrollWheel(float y, float x) override;
    void registerWithTouchDispatcher() override;

    CCDrawNode* m_gridNode = nullptr;
    float m_gridsquareSize;
    float m_gridLineWidth;
    ccColor4B m_gridColor;

    bool m_holdingShift;

    CCSet* m_allTouches = nullptr;
    float m_initialDistance;
    float m_initialScale;
    CCPoint m_initialMinPoint;
    CCPoint m_initialPoint1;
    CCPoint m_initialPoint2;
};