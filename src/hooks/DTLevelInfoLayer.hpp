#pragma once

#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

class $modify(DTLevelInfoLayer, LevelInfoLayer) {
public:
    struct Fields {
        CCMenuItemSpriteExtra* btn = nullptr;
        CCPoint originalDiffPos;
        bool prevStatToggle;
        float prevProgressBarOpacity;
    };
    
    bool init(GJGameLevel* p0, bool p1);

private:
    void checkIfPlayVisible(float delta);

    void openDTLayer(CCObject*);
};