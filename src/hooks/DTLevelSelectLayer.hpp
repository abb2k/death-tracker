#pragma once

#include <Geode/modify/LevelSelectLayer.hpp>

using namespace geode::prelude;

class $modify(DTLevelSelectLayer, LevelSelectLayer) {
public:
    struct Fields {
        CCMenuItemSpriteExtra* m_dtButton;
        CCMenuItemSpriteExtra* m_robDTButton;
    };
    
    bool init(int page);

    void onNext(CCObject*);

    void onPrev(CCObject*);

private:
    void openDTLayer(CCObject*);

    std::optional<GJGameLevel*> getLevelForCurrentPage();
};
