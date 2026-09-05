#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class Settings {
public:
    static int64_t getMaxSessionLength();
    static bool isCompletedLevelTrackingDisabled();
    static bool getPauseMenuEnabled();
    static bool getCompleteMenuEnabled();
    static bool getQuickSaveEnabled();
    static float getGraphPointSize();
    static std::filesystem::path getSavePath();
    static std::optional<int> getMaxBackupAmount();
    static bool getAutoBackupEnabled();
    static bool getAutoBackupGeneral();
    static std::optional<int> getAutoBackupSessionAmount();
    static bool getAutoBackupAtLvlExit();
    static bool getAutoBackupAtDTExit();
    static bool getSafeMode();
    static bool getCheatDetect();
    static bool getDisablePractice();
    static bool is12HourClockFormat();
};