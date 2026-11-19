#include <Geode/Geode.hpp>

#include <utils/Save.hpp>
#include <managers/StatsManager.hpp>
#include <utils/Settings.hpp>
#include <Geode/utils/web.hpp>

#include <managers/LinkedLevelsCacheManager.hpp>

using namespace geode::prelude;

$execute {
    (void)file::createDirectory(Mod::get()->getSaveDir() / "levels");

    GameEvent(GameEventType::Exiting).listen([]() {
        StatsManager::isGameClosing = true;
        return false;
    }).leak();

    // StatsManager::logGameOpening();

    if (!Save::getLayout().isEmpty())
        Save::setDidInitLayout(true);

    if (!Save::getDidInitLayout()) {
        Save::setLayout(Save::getDefaultLayout());

        Save::setGraphs(Save::getDefaultGraphs());

        Save::setNewBestCustomizations(Save::getDefaultNewBestCustomizations());
        Save::setSessionBestCustomizations(Save::getDefaultSessionBestCustomizations());

        Save::setOverlayLabels(Save::getDefaultOverlayLabels());

        Save::setDidInitLayout(true);
    }

    if (Settings::getPreloadLinked())
        LinkedLevelsCacheManager::getLinkedLevels();
};

$on_mod(Loaded) {
    //load fonts
    auto allFonts = StatsManager::getAllFonts();

    for (int i = 0; i < allFonts.size(); i++)
    {
        auto label = geode::Label::create("abcdefghijclmnopqrstuvwxyz{}-", allFonts[i].c_str());
        CCScene::get()->addChild(label);
        label->removeMeAndCleanup();
    }
};