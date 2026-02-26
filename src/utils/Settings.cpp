#include "Settings.hpp"

int64_t Settings::getMaxSessionLength() {
    auto sessionMethod = Mod::get()->getSettingValue<std::string>("session-method");
    if (sessionMethod == "Exit level")
        return -2;
    else if (sessionMethod == "Exit game")
        return -1;
    else
        return Mod::get()->getSettingValue<int64_t>("session-length");
}

int64_t Settings::getDateFormat() {
    auto dateFormat = Mod::get()->getSettingValue<std::string>("date-format");
    
    if (dateFormat == "MM/DD/YYYY") {
        return -2;
    } else if (dateFormat == "DD/MM/YYYY") {
        return -1;
    }
    //exception
    return 1;
}

bool Settings::isCompletedLevelTrackingDisabled() {
    return Mod::get()->getSettingValue<bool>("disable-tracking-completed-levels");
}

bool Settings::isPracticeZeroDeathsEnabled() {
    return Mod::get()->getSettingValue<bool>("practice-zero-deaths");
}

bool Settings::getPauseMenuEnabled(){
    return Mod::get()->getSettingValue<bool>("pause-menu");
}

bool Settings::getLeftMenuEnabled(){
    return Mod::get()->getSettingValue<bool>("left-menu");
}

bool Settings::getLateSaveEnabled(){
    return Mod::get()->getSettingValue<bool>("late-save");
}

float Settings::getGraphPointSize(){
    return static_cast<float>(Mod::get()->getSettingValue<double>("graph-point-size"));
}

std::filesystem::path Settings::getSavePath(){
    return Mod::get()->getSettingValue<std::filesystem::path>("save-path-new");
}