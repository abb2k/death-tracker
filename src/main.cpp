#include <Geode/Geode.hpp>
using namespace geode::prelude;
#include "utils/Save.hpp"
#include "managers/StatsManager.hpp"
#include "utils/Settings.hpp"
#include <Geode/utils/web.hpp>

$execute {
    (void)file::createDirectory(Mod::get()->getSaveDir() / "levels");

    GameEvent(GameEventType::Exiting).listen([]() {
        StatsManager::isGameClosing = true;
        return false;
    }).leak();

    // StatsManager::logGameOpening();

    if (Save::getLayout().isEmpty()) {
        Save::setLayout(Save::getDefaultLayout());

        Save::setGraphs(Save::getDefaultGraphs());

        Save::setNewBestCustomizations(Save::getDefaultNewBestCustomizations());
        Save::setSessionBestCustomizations(Save::getDefaultSessionBestCustomizations());
    }
};

$on_mod(Loaded) {
    //load fonts
    auto allFonts = StatsManager::getAllFonts();

    for (int i = 0; i < allFonts.size(); i++)
    {
        auto label = CCLabelBMFont::create("abcdefghijclmnopqrstuvwxyz{}-", allFonts[i].c_str());
        CCScene::get()->addChild(label);
        label->removeMeAndCleanup();
    }
};