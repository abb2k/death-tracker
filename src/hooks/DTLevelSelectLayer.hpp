#include <Geode/modify/LevelSelectLayer.hpp>

using namespace geode::prelude;

class $modify(DTLevelSelectLayer, LevelSelectLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* dtButton;
        CCMenuItemSpriteExtra* robDTButton;
    };
    
    bool init(int page);

    void openDTLayer(CCObject*);

    void onNext(CCObject*);

    void onPrev(CCObject*);

    std::optional<GJGameLevel*> getLevelForCurrentPage();
};
