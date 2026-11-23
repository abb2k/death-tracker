#pragma once

#include <Geode/Geode.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>

using namespace geode::prelude;

class TutorialButton : public CCMenuItemSpriteExtra {
    protected:
        bool init(float size, const std::string& id, geode::Function<void(DTTutorialLayer*)> initilizeTutorial);
        
    public:
        static TutorialButton* create(float size, const std::string& id, geode::Function<void(DTTutorialLayer*)> initilizeTutorial);

    private:

        geode::Function<void(DTTutorialLayer*)> initilizeTutorial = NULL;

        void onClicked(CCObject*);

        std::string id;
        CCSprite* spr;
        float size;
};