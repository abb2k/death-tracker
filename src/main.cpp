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

        auto graphs = std::vector<DTGraphInfo>{
            DTGraphInfo{
                .isEnabled = false,
                .coverage = static_cast<DTGraphCoverage>(3),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 0,
                .thickness = 1.0f,
                .outlineThickness = 0.75f,
#if defined(GEODE_IS_MOBILE)
                .pointScale = 0.175,
#else
                .pointScale = 0.1,
#endif
                .color = {122, 74, 0, 255},
                .outlineColor = {101, 50, 0, 255},
                .pointColor = {164, 76, 0, 255},
                .name = "session runs"
            },
            DTGraphInfo{
                .isEnabled = false,
                .coverage = static_cast<DTGraphCoverage>(2),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 1,
                .thickness = 1.0f,
                .outlineThickness = 0.75f,
#if defined(GEODE_IS_MOBILE)
                .pointScale = 0.175,
#else
                .pointScale = 0.1,
#endif
                .color = {255, 145, 0, 255},
                .outlineColor = {170, 104, 16, 255},
                .pointColor = {114, 66, 3, 255},
                .name = "sess from0"
            },
            DTGraphInfo{
                .isEnabled = false,
                .coverage = static_cast<DTGraphCoverage>(1),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 2,
                .thickness = 0.75f,
                .outlineThickness = 0.5f,
#if defined(GEODE_IS_MOBILE)
                .pointScale = 0.175,
#else
                .pointScale = 0.1,
#endif
                .color = {14, 138, 37, 253},
                .outlineColor = {11, 96, 32, 255},
                .pointColor = {5, 181, 48, 255},
                .name = "general runs"
            },
            DTGraphInfo{
                .isEnabled = true,
                .coverage = static_cast<DTGraphCoverage>(0),
                .type = static_cast<DTGraphType>(0),
                .orderPos = 3,
                .thickness = 1.0f,
                .outlineThickness = 0.75f,
#if defined(GEODE_IS_MOBILE)
                .pointScale = 0.175,
#else
                .pointScale = 0.1,
#endif
                .color = {0, 255, 55, 255},
                .outlineColor = {18, 162, 53, 255},
                .pointColor = {15, 105, 41, 255},
                .name = "general from0"
            }
        };

        Save::setGraphs(graphs);

        CustomizationData newBestDefaultCustomization {
            .color = {255, 255, 0, 255},
            .gradientEnabled = true,
            .gradient = {
                .colorA = {255, 255, 40, 255},
                .colorB = {255, 255, 255, 255},
                .duration = 1.0f,
                .stagger = 0.2f
            }
        };

        Save::setNewBestCustomizations(newBestDefaultCustomization);
        Save::setSessionBestCustomizations(CustomizationData{.color = {255, 136, 0, 255}});
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