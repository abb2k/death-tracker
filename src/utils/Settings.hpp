#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class Settings {
public:
    static int64_t getMaxSessionLength();
    static int64_t getDateFormat(); 
    static bool isCompletedLevelTrackingDisabled();
    static bool isPracticeZeroDeathsEnabled();
    static bool getPauseMenuEnabled();
    static bool getLeftMenuEnabled();
    static bool getLateSaveEnabled();
    static float getGraphPointSize();
    static std::filesystem::path getSavePath();
};