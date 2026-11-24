#pragma once

#include <Geode/modify/EditLevelLayer.hpp>

using namespace geode::prelude;

class $modify(DTEditLevelLayer, EditLevelLayer) {
public:
    bool init(GJGameLevel* level);

private:
    void openDTLayer(CCObject*);
};
