#if !defined(GEODE_IS_MACOS)

#include "DTGJBaseGameLayer.hpp"

#include <hooks/DTPlayLayer.hpp>

void DTGJBaseGameLayer::checkpointActivated(CheckpointGameObject* checkpt) {
    GJBaseGameLayer::checkpointActivated(checkpt);

    if (static_cast<GJBaseGameLayer*>(this) == PlayLayer::get()) {
        static_cast<DTPlayLayer*>(PlayLayer::get())->m_fields->m_currentRun.m_end++;
    }
}

#endif