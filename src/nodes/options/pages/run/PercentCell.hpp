#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class PercentCell : public CCMenu, public SliderDelegate {
    protected:
        bool init(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, Function<void(PercentCell*)> callback);
    public:
        static PercentCell* create(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, Function<void(PercentCell*)> callback = NULL);
        
        int getPercent() const {
            return m_percent;
        }
        int getMaxToHide() const {
            return m_maxToHide;
        }

        CCSprite* getSilderBG() const {
            return m_MTHScroll->m_sliderBar;
        }

        CCSprite* getSilderHandle() const {
            return m_MTHScroll->m_groove;
        }

        CCScale9Sprite* m_BGSprite;
        Function<void(PercentCell*)> m_onMaxToHideChanged = NULL;

        void hide();
        void hideInstant();
        void show();

        void setMaxToHide(float val);
        void sliderEnded(Slider* slider);
        
    private:
        void RunCallback(CCObject*);
        void mthScroll(CCObject*);

        Slider* m_MTHScroll;
        TextInput* m_MTHInput;

        int m_percent;
        int m_maxToHide;
        Function<void(PercentCell*)> m_callback;
};