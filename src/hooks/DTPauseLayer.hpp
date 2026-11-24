#pragma once

#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

class $modify(DTPauseLayer, PauseLayer) {

    void customSetup();

#if defined(GEODE_IS_MACOS)
    void onNormalMode(cocos2d::CCObject* sender);
#endif

private:
    void onDTMiniMenuOpened(CCObject*);
};