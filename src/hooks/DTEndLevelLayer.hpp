#pragma once

#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;

class $modify(DTEndLevelLayer, EndLevelLayer) {
public:
    struct Fields {
        CCMenuItemSpriteExtra* skullBtn = nullptr;
    };
    
    void customSetup();

    void onHideLayer(CCObject* sender);

private:
    void openDTLayer(CCObject*);
};