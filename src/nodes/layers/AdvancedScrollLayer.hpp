#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class AdvancedScrollLayer : public CCLayer {
    private:
    
        bool init(CCSize size, CCSize limits);

        CCNode* zoomParent;

        float scrollSense = 1;

        bool isEnabled = true;

        CCScrollLayerExt* widthScrollExt;
        CCScrollLayerExt* heightScrollExt;

        void updateProgressBars();
        void updatePositionByProgBars(float dt);

        bool touchDown = false;

        bool scrollWheel(float y, float x) override;
        void registerWithTouchDispatcher() override;

        CCDrawNode* gridNode = nullptr;
        float gridsquareSize;
        float gridLineWidth;
        ccColor4B gridColor;

        bool holdingShift;

        CCSet* allTouches = nullptr;
        float initialDistance;
        float initialScale;
        CCPoint initialMinPoint;
        CCPoint initialPoint1;
        CCPoint initialPoint2;
    public:
        static AdvancedScrollLayer* create(CCSize size, CCSize limits);

        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchesMoved(CCSet* touches, CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

        virtual void keyDown(enumKeyCodes key, double d)  override;
        virtual void keyUp(enumKeyCodes key, double d)  override;
        
        CCNode* content;

        bool scrollMovement = true;

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

        Scrollbar* horizontalBar;
        Scrollbar* verticalBar;

        float minZoom = 2;
        float maxZoom = .5f;

        float zoomSensetivity = 0.01f;

        float getMinimumPosition(bool horizontal);
        float getMaximumPosition(bool horizontal);

        ~AdvancedScrollLayer();
};