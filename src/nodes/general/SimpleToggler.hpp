#pragma once

using namespace geode::prelude;

class SimpleToggler : public CCMenuItemToggler {
public:
    static SimpleToggler* create(CCNode* offSprite, CCNode* onSprite, float scale = 1, bool startState = false);
    static SimpleToggler* createWithDefaults(float scale = 1, bool startState = false);

    void setCallback(Function<void(bool)> callback);

    void setOpacity(GLubyte opacity) override;
private:
    bool init(CCNode* offSprite, CCNode* onSprite, float scale, bool startState);

    void onToggled(CCObject* sender);

    Function<void(bool)> m_callback = NULL;
    CCNode* m_offSprite;
    CCNode* m_onSprite;
};