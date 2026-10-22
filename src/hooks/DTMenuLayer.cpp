#include <hooks/DTMenuLayer.hpp>

#include <nodes/layers/FileConversionLayer.hpp>
#include <arc/task/Yield.hpp>
#include <utils/Save.hpp>

bool DTMenuLayer::init() {
    if (!MenuLayer::init()) return false;

    if (Save::getLastOpenedVersion() != Mod::get()->getVersion().toNonVString() && !Save::getDidConvertToV3()) {
        arc::spawn(
            awaitOpening()
        );
    }
    
    return true;
}

arc::Future<> DTMenuLayer::awaitOpening() {
    co_await arc::yield();
    geode::queueInMainThread([&]() {
        auto fcLayer = FileConversionLayer::create(true);
        fcLayer->show();
    });
}