#if !defined(GEODE_IS_MACOS)

#include "../hooks/DTGJBaseGameLayer.hpp"
#include "../hooks/DTPlayLayer.hpp"

void DTGJBaseGameLayer::checkpointActivated(CheckpointGameObject* checkpt) {
    GJBaseGameLayer::checkpointActivated(checkpt);

    if (static_cast<GJBaseGameLayer*>(this) == PlayLayer::get()) {
        static_cast<DTPlayLayer*>(PlayLayer::get())->m_fields->currentRun.end++;
    }
}

#endif