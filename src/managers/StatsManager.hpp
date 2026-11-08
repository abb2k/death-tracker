#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class StatsManager {
private:
    static std::set<std::string> m_playedLevels;
    
    static bool m_scheduleCreateNewSession;

    // internal functions

    //gets new bests from level
    static Result<std::tuple<NewBests, int>> calcNewBests(GJGameLevel* const& level);
    static std::array<std::string, 62> m_AllFontsMap;

    static void createFilesIfNeeded(const std::string& levelKey);

    static const std::string METADATA_FILE_NAME;
    static const std::string FROM0_FILE_NAME;
    static const std::string SESSIONS_DIR_NAME; 
    static const std::string BACKUPS_DIR_NAME; 

    static arc::TaskHandle<void> backupHandler;

public:
    StatsManager() = delete;

    static int MainLevelIDs[26];

    static asp::fs::Path getSavesFolderPath();

    static GJGameLevel* currentLevel;
    static std::optional<GeneralData> currentFrom0;
    static std::optional<LevelMetadeta> currentMetadata;
    static std::optional<Session> currentSession;

    static bool isGameClosing;

    static void createFile(const asp::fs::Path& path, std::optional<matjson::Value> jsonToFill);

#pragma region level setters/getters

    static Result<LevelMetadeta, ErrorWithCode> getMetadata(GJGameLevel* const level);
    static Result<LevelMetadeta, ErrorWithCode> getMetadata(const std::string& levelKey);
    static Result<LevelMetadeta, ErrorWithCode> getMetadata(const asp::fs::Path& path);
    static Result<Session, ErrorWithCode> getSession(GJGameLevel* const level, long long sessionTime);
    static Result<Session, ErrorWithCode> getSession(const std::string& levelKey, long long sessionTime);
    static Result<Session, ErrorWithCode> getSession(const asp::fs::Path& path, long long sessionTime);
    static Result<GeneralData, ErrorWithCode> getGeneral(GJGameLevel* const level);
    static Result<GeneralData, ErrorWithCode> getGeneral(const std::string& levelKey);
    static Result<GeneralData, ErrorWithCode> getGeneral(const asp::fs::Path& path);
    static Result<LevelData, ErrorWithCode> getLevelData(GJGameLevel* const level);
    static Result<LevelData, ErrorWithCode> getLevelData(const std::string& levelKey);
    static Result<LevelData, ErrorWithCode> getLevelData(const asp::fs::Path& path);
    static Result<BackupLevelData> getBackupData(const std::string& levelKey, long long backupName);

    static Result<> setMetadata(const LevelMetadeta& stats, GJGameLevel* const level);
    static Result<> setMetadata(const LevelMetadeta& stats, const std::string& levelKey);
    static Result<> setSession(Session& stats, GJGameLevel* const level, long long sessionTime, bool updateLastPlayed);
    static Result<> setSession(Session& stats, const std::string& levelKey, long long sessionTime, bool updateLastPlayed);
    static Result<> setGeneral(const GeneralData& stats, GJGameLevel* const level);
    static Result<> setGeneral(const GeneralData& stats, const std::string& levelKey);
    // add a backup into the levels backups folder
    // sessionsToSave: how many sessions to save into the backup, -1 for all, nullopt for none
    static void addBackup(const std::string& levelKey, bool saveLevelStats, std::optional<int> sessionsToSave, bool showNotifications = false);

    static Result<std::set<long long>> getAllSessionTimesForLevel(GJGameLevel* const level);
    static std::set<long long> getAllSessionTimesForLevel(const std::string& levelKey);
    static std::set<long long> getAllSessionTimesForLevel(const asp::fs::Path& path);

    static std::set<long long> getBackupsCount(const std::string& levelKey);
    static uintmax_t getBackupFileSize(const std::string& levelKey, long long backupName);

    static Result<> reveretBackupSessions(const std::string& levelKey, long long backupName);

    static Result<> deleteLevelStats(const std::string& levelKey);
    static Result<> deleteBackup(const std::string& levelKey, long long backupName);
    static Result<> deleteAllSessions(const std::string& levelKey);

    static Result<> convertV2SaveToV3(const std::string& levelKey);
    static std::vector<std::string> allV2FileLevelKeys();

    static std::vector<std::string> allV3FileLevelKeys();
    
#pragma endregion

#pragma region logging

    static void setCurrentLevel(GJGameLevel* const& level);

    //save a normal mode death to the loaded levels save file
    static void logDeath(double percent, bool instantSave = true);
    //save an array of normal mode deaths to the loaded levels save file
    static void logDeaths(const std::vector<double>& percents);
    //save run to the loaded levels save file
    static void logRun(const Run& run, bool instantSave = true);
    //save an array of run to the loaded levels save file
    static void logRuns(const std::vector<Run>& runs);

    static bool safeCheckCurrF0();

#pragma endregion

    // utility functions

    static void updateCurrentSessionLastPlayed();
    static GJGameLevel* getCurrentLevel();

    //gets the epoch time
    static long long getNowSeconds();
    //get the a levels level key
    static Result<std::string> getLevelKey(GJGameLevel* const& level);
    //convert a run string into a run struct
    static Result<Run> splitRunKey(const std::string& runKey);
    static Result<std::string> createRunKey(const Run& runKey, bool roundToInt = false);
    //get the current ongoing session
    static Session* getCurrentSession();
    //schedule the creation of a new session
    static void scheduleCreateNewSession(const bool& scheduled);
    //if youve played the loaded level before, returns true
    static bool hasPlayedLevel();
    //get a font by a fontID
    static std::string getFont(const int& fontID);
    //get a fonts name by a fontID
    static std::string getFontName(const int& fontID);
    //get an array of all fonts
    static std::array<std::string, 62> getAllFonts();
    //gets an array of all levels you have progress on
    static Result<std::vector<std::pair<std::string, LevelMetadeta>>> getAllLevels();
    //seperate a level key to [levelID, levelType]
    static std::pair<std::string, std::string> splitLevelKey(const std::string& levelKey);
    //get the difficulty of a level
    //
    // -1 = auto, 0 = NA, 1 = Easy, 2 = Normal
    // 3 = Hard, 4 = Harder, 5 = Insane, 6 = Hard Demon
    // 7 = Easy Demon, 8 = Medium Demon, 9 = Insane Demon, 10 = Extreme Demon
    static int getDifficulty(GJGameLevel* const& level);

    // return a splitted version of the string provided, devided by the delim
    static std::vector<std::string> splitStr(const std::string& str, const std::string& delim);
    
    //gets the index in which the CCLabelBMFonts cursor is located on the string
    static int getCursorPosition(CCLabelBMFont* const& text, CCLabelBMFont* const& cursor);

    //get if a key is in an index of a string
    //@param s the string to look for the key in
    //@param index the index where to look for the key in
    //@param key the key to look for in the string
    static bool isKeyInIndex(const std::string& s, const int& Index, const std::string& key);

    //better info calc :)
    //converts time to a working time string
    static std::string workingTime(long long value);
    static std::string workingTime(uint64_t nanoseconds);

    template<typename K, typename V>
    static void mergeMapsAdd(std::map<K, V>& target, const std::map<K, V>& source);
    static void mergeMapsAdd(Deaths& target, const ComplexDeathMap& source);
    static void mergeMapsAdd(ComplexDeathMap& target, const ComplexDeathMap& source);

    static bool transferPlaytimeFromPT(GJGameLevel* level);
    static bool transferPlaytimeFromPT(geode::Result<LevelData, ErrorWithCode>& data, GJGameLevel* level);

    // static void logGameOpening();
};

template<typename K, typename V>
void StatsManager::mergeMapsAdd(std::map<K, V>& target, const std::map<K, V>& source)
{
    for (const auto& [key, value] : source)
    {
        target[key] += value;
    }
}