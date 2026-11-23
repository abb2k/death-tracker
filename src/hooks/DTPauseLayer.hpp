#include <Geode/modify/PauseLayer.hpp>

#include <nodes/DTLayer.hpp>
#include <utils/Settings.hpp>
#include <hooks/DTPlayLayer.hpp>

using namespace geode::prelude;

class $modify(DTPauseLayer, PauseLayer) {

    void customSetup();

    void onDTMiniMenuOpened(CCObject*);

    #if defined(GEODE_IS_MACOS)

    void onNormalMode(cocos2d::CCObject* sender);

    #endif
};