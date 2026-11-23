#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SimpleToggler : public CCMenuItemToggler {
    public:
        static SimpleToggler* create(CCNode* offSprite, CCNode* onSprite, float scale = 1, bool startState = false);
        static SimpleToggler* createWithDefaults(float scale = 1, bool startState = false);

        void setCallback(geode::Function<void(bool)> callback);

        void setOpacity(GLubyte opacity) override;
    private:
        bool init(CCNode* offSprite, CCNode* onSprite, float scale, bool startState);

        void onToggled(CCObject* sender);

        geode::Function<void(bool)> callback = NULL;
        CCNode* offSprite;
        CCNode* onSprite;
};