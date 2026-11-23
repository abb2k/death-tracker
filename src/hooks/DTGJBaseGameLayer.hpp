#pragma once

#if !defined(GEODE_IS_MACOS)

#include <Geode/modify/GJBaseGameLayer.hpp>

class $modify(DTGJBaseGameLayer, GJBaseGameLayer) {
public:
    void checkpointActivated(CheckpointGameObject* checkpt);
};

#endif