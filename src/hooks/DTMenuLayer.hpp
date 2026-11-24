#pragma once

#include <Geode/modify/MenuLayer.hpp>

class $modify(DTMenuLayer, MenuLayer) {
public:
    bool init();

private:
    arc::Future<> awaitOpening();
};