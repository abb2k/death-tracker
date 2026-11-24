#pragma once

#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

class $modify(DTLevelInfoLayer, LevelInfoLayer) {
public:
    struct Fields {
        CCMenuItemSpriteExtra* m_btn = nullptr;
        CCPoint m_originalDiffPos;
        bool m_prevStatToggle;
        float m_prevProgressBarOpacity;
    };
    
    bool init(GJGameLevel* p0, bool p1);

private:
    void checkIfPlayVisible(float delta);

    void openDTLayer(CCObject*);
};