#pragma once

using namespace geode::prelude;

class PercentCell : public CCMenu, public SliderDelegate {
public:
    static PercentCell* create(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, Function<void(PercentCell*)> callback = NULL);
    
    int getPercent() const { return m_percent; }
    int getMaxToHide() const { return m_maxToHide; }

    CCSprite* getSilderBG() const { return m_MTHScroll->m_sliderBar; }

    CCSprite* getSilderHandle() const { return m_MTHScroll->m_groove; }

    CCScale9Sprite* getBGSprite() const { return m_BGSprite; }

    void hide();
    void hideInstant();
    void show();

    void setMaxToHide(float val);
    void sliderEnded(Slider* slider);

    void setOnMaxToHideChangedCallback(Function<void(PercentCell*)> callback);
    
private:
    bool init(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, Function<void(PercentCell*)> callback);

    void RunCallback(CCObject*);
    void mthScroll(CCObject*);

    Slider* m_MTHScroll;
    TextInput* m_MTHInput;

    int m_percent;
    int m_maxToHide;
    Function<void(PercentCell*)> m_callback;

    CCScale9Sprite* m_BGSprite;
    Function<void(PercentCell*)> m_onMaxToHideChanged = NULL;
};