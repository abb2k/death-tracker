#include <managers/StatsManager.hpp>
#include <utils/Settings.hpp>
#include <utils/Dev.hpp>
#include <cvolton.level-id-api/EditorIDs.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <regex>
#include <asp/fs/fs.hpp>
#include <asp/fs/File.hpp>

using namespace geode::prelude;

std::vector<std::string> StatsManager::splitStr(const std::string& str, const std::string& delim) {
    if (delim.empty()) return {};

    size_t posStart = 0;
    size_t posEnd;
    size_t delimLen = delim.length();

    std::vector<std::string> res;

    while ((posEnd = str.find(delim, posStart)) != std::string::npos) {
        std::string token = str.substr(posStart, posEnd - posStart);
        posStart = posEnd + delimLen;
        res.push_back(token);
    }

    std::string lastToken = str.substr(posStart);
    if (!lastToken.empty() || res.empty())
        res.push_back(lastToken);

    return res;
}

/* static member variables
=========================== */
std::set<std::string> StatsManager::m_playedLevels{};
bool StatsManager::m_scheduleCreateNewSession = false;

GJGameLevel* StatsManager::currentLevel = nullptr;
std::optional<GeneralData> StatsManager::currentFrom0{};
std::optional<LevelMetadeta> StatsManager::currentMetadata{};
std::optional<Session> StatsManager::currentSession{};

const std::string StatsManager::METADATA_FILE_NAME = "metadata";
const std::string StatsManager::FROM0_FILE_NAME = "general.dt";
const std::string StatsManager::SESSIONS_DIR_NAME = "sessions";
const std::string StatsManager::BACKUPS_DIR_NAME = "backups";
arc::TaskHandle<void> StatsManager::backupHandler{};
bool StatsManager::isGameClosing = false;

asp::fs::Path StatsManager::getSavesFolderPath() {
    return Settings::getSavePath();
}

int StatsManager::MainLevelIDs[26]{
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 5001, 5002, 5003, 5004, 3001
};

std::array<std::string, 62> StatsManager::m_AllFontsMap{
    "bigFont.fnt",
    "chatFont.fnt",
    "goldFont.fnt",
    "gjFont01.fnt",
    "gjFont02.fnt",
    "gjFont03.fnt",
    "gjFont04.fnt",
    "gjFont05.fnt",
    "gjFont06.fnt",
    "gjFont07.fnt",
    "gjFont08.fnt",
    "gjFont09.fnt",
    "gjFont10.fnt",
    "gjFont11.fnt",
    "gjFont12.fnt",
    "gjFont13.fnt",
    "gjFont14.fnt",
    "gjFont15.fnt",
    "gjFont16.fnt",
    "gjFont17.fnt",
    "gjFont18.fnt",
    "gjFont19.fnt",
    "gjFont20.fnt",
    "gjFont21.fnt",
    "gjFont22.fnt",
    "gjFont23.fnt",
    "gjFont24.fnt",
    "gjFont25.fnt",
    "gjFont26.fnt",
    "gjFont27.fnt",
    "gjFont28.fnt",
    "gjFont29.fnt",
    "gjFont30.fnt",
    "gjFont31.fnt",
    "gjFont32.fnt",
    "gjFont33.fnt",
    "gjFont34.fnt",
    "gjFont35.fnt",
    "gjFont36.fnt",
    "gjFont37.fnt",
    "gjFont38.fnt",
    "gjFont39.fnt",
    "gjFont40.fnt",
    "gjFont41.fnt",
    "gjFont42.fnt",
    "gjFont43.fnt",
    "gjFont44.fnt",
    "gjFont45.fnt",
    "gjFont46.fnt",
    "gjFont47.fnt",
    "gjFont48.fnt",
    "gjFont49.fnt",
    "gjFont50.fnt",
    "gjFont51.fnt",
    "gjFont52.fnt",
    "gjFont53.fnt",
    "gjFont54.fnt",
    "gjFont55.fnt",
    "gjFont56.fnt",
    "gjFont57.fnt",
    "gjFont58.fnt",
    "gjFont59.fnt",
};

/* main functions
================== */

Result<LevelMetadeta, ErrorWithCode> StatsManager::getMetadata(GJGameLevel* const level) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return getMetadata(levelKey);
}
Result<LevelMetadeta, ErrorWithCode> StatsManager::getMetadata(const std::string& levelKey) {
    return getMetadata(getSavesFolderPath() / levelKey);
}

Result<LevelMetadeta, ErrorWithCode> StatsManager::getMetadata(const asp::fs::Path& path) {
    auto levelSaveFilePath = path / StatsManager::METADATA_FILE_NAME;

    if (!StatsManager::getSavesFolderPath().exists())
        return Err("Invalid save directory!");

    if (!levelSaveFilePath.exists())
        return Err(ErrorWithCode("no stats exist for level!", 1));

    auto readRes = file::readJson(levelSaveFilePath);
    if (readRes.isErr()) return Err("Metadata Corrupted");
    auto json = readRes.unwrap().as<LevelMetadeta>();
    if (json.isErr()) return Err("Metadata Corrupted");
        
    return Ok(json.unwrap());
}

Result<Session, ErrorWithCode> StatsManager::getSession(GJGameLevel* const level, long long sessionTime) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return getSession(levelKey, sessionTime);
}
Result<Session, ErrorWithCode> StatsManager::getSession(const std::string& levelKey, long long sessionTime) {
    return getSession(getSavesFolderPath() / levelKey, sessionTime);
}

Result<Session, ErrorWithCode> StatsManager::getSession(const asp::fs::Path& path, long long sessionTime) {
    auto levelSaveFilePath = path / StatsManager::SESSIONS_DIR_NAME / (std::to_string(sessionTime) + ".dt");

    if (!StatsManager::getSavesFolderPath().exists())
        return Err("Invalid save directory!");

    if (!levelSaveFilePath.exists())
        return Err(ErrorWithCode("no stats exist for level!", 1));

    auto sessionRes = file::readJson(levelSaveFilePath);
    if (sessionRes.isErr()) return Err("Session Corrupted");
    auto sessionResult = sessionRes.unwrap().as<Session>();
    if (sessionResult.isErr()) return Err("Session Corrupted");
    auto session = std::move(sessionResult).unwrap();

    if (!session.dataIsOverride && !session.timeRanges.empty()) {
        auto generalResult = getGeneral(path);
        if (generalResult.isErr()) {
            if (generalResult.unwrapErr().code != 1)
                return Err("Failed to load general data for session: {}", generalResult.unwrapErr().error);
        }
        else {
            auto general = std::move(generalResult).unwrap();
            auto sessionDeaths = general.deaths.filterOccurrences(session.timeRanges);
            auto sessionRuns = general.runs.filterOccurrences(session.timeRanges);

            auto deathsResult = session.data.deaths.merge(sessionDeaths);
            if (deathsResult.isErr())
                return Err("Failed to derive session deaths: {}", deathsResult.unwrapErr());
            auto runsResult = session.data.runs.merge(sessionRuns);
            if (runsResult.isErr())
                return Err("Failed to derive session runs: {}", runsResult.unwrapErr());
        }
    }

    return Ok(std::move(session));
}

Result<GeneralData, ErrorWithCode> StatsManager::getGeneral(GJGameLevel* const level) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return getGeneral(levelKey);
}
Result<GeneralData, ErrorWithCode> StatsManager::getGeneral(const std::string& levelKey) {
    return getGeneral(getSavesFolderPath() / levelKey);
}

Result<GeneralData, ErrorWithCode> StatsManager::getGeneral(const asp::fs::Path& path) {
    auto levelSaveFilePath = path / StatsManager::FROM0_FILE_NAME;

    if (!StatsManager::getSavesFolderPath().exists())
        return Err("Invalid save directory!");

    if (!levelSaveFilePath.exists() || levelSaveFilePath.empty())
        return Err(ErrorWithCode("no stats exist for level!", 1));

    auto jsonRes = file::readJson(levelSaveFilePath);
    if (jsonRes.isErr()) return Err("Save Corrupted");
    auto genDataRes = jsonRes.unwrap().as<GeneralData>();
    if (genDataRes.isErr()) return Err("Save Corrupted");
        
    return Ok(genDataRes.unwrap());
}

Result<LevelData, ErrorWithCode> StatsManager::getLevelData(GJGameLevel* const level) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));

    return getLevelData(levelKey);
}
Result<LevelData, ErrorWithCode> StatsManager::getLevelData(const std::string& levelKey) {
    LevelData data;

    GEODE_UNWRAP_INTO(data.metadata, StatsManager::getMetadata(levelKey));

    data.from0 = StatsManager::getGeneral(levelKey);
    data.sessionNames = StatsManager::getAllSessionTimesForLevel(levelKey);
    data.levelKey = levelKey;

    return Ok(data);
}

Result<LevelData, ErrorWithCode> StatsManager::getLevelData(const asp::fs::Path& path) {
    LevelData data;

    GEODE_UNWRAP_INTO(data.metadata, StatsManager::getMetadata(path));

    data.from0 = StatsManager::getGeneral(path);
    data.sessionNames = StatsManager::getAllSessionTimesForLevel(path);

    return Ok(data);
}

Result<BackupLevelData> StatsManager::getBackupData(const std::string& levelKey, long long backupName) {
    BackupLevelData data;

    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName);
    data.backupDate = backupName;

    if (!levelSaveFilePath.exists())
        return Err("1 (no stats exist for backup!)");

    if ((levelSaveFilePath / StatsManager::METADATA_FILE_NAME).exists()) {
        auto levelStatsJsonRes = file::readJson(levelSaveFilePath / StatsManager::METADATA_FILE_NAME);
        if (levelStatsJsonRes.isErr()) return Err("Failed to get backup level stats!");
        auto levelStatsJson = levelStatsJsonRes.unwrap();
        
        auto metaJsonObjRes = levelStatsJson.as<LevelMetadeta>();
        if (metaJsonObjRes.isErr()) return Err("Failed to get backup metadata!");
        data.meta = metaJsonObjRes.unwrap();
    }

    if ((levelSaveFilePath / StatsManager::FROM0_FILE_NAME).exists()) {
        auto levelStatsJsonRes = file::readJson(levelSaveFilePath / StatsManager::FROM0_FILE_NAME);
        if (levelStatsJsonRes.isErr()) return Err("Failed to get backup level stats!");
        auto levelStatsJson = levelStatsJsonRes.unwrap();
        
        auto metaJsonObjRes = levelStatsJson.as<GeneralData>();
        if (metaJsonObjRes.isErr()) return Err("Failed to get backup metadata!");
        data.from0 = metaJsonObjRes.unwrap();
    }

    if ((levelSaveFilePath / StatsManager::SESSIONS_DIR_NAME).exists()) {
        std::set<long long> sessionDates{};

        auto dirIteratorRes = asp::fs::iterDir(levelSaveFilePath / StatsManager::SESSIONS_DIR_NAME);
        if (dirIteratorRes.isErr()) return Err("Failed to iterate sessions directory!");

        for (const auto& entryRes : dirIteratorRes.unwrap()) {
            if (entryRes.isErr()) continue;

            auto entry = entryRes.unwrap();

            if (entry.isDir()) continue;

            auto numRes = geode::utils::numFromString<long long>(entry.path.string());
            if (numRes.isErr()) continue;

            sessionDates.insert(numRes.unwrap());
        }

        data.sessionNames = sessionDates;
    }

    return Ok(data);
}

Result<> StatsManager::setMetadata(const LevelMetadeta& stats, GJGameLevel* const level) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    GEODE_UNWRAP(setMetadata(stats, levelKey));

    return Ok();
}
Result<> StatsManager::setMetadata(const LevelMetadeta& stats, const std::string& levelKey) {
    createFilesIfNeeded(levelKey);

    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::METADATA_FILE_NAME;

    auto indentation = Dev::MINIFY_SAVE_FILE
        ? matjson::NO_INDENTATION
        : 4;

    auto jsonStr = matjson::Value(stats).dump(indentation);
    GEODE_UNWRAP(file::writeString(levelSaveFilePath, jsonStr));

    return Ok();
}

Result<> StatsManager::setSession(Session& stats, GJGameLevel* const level, long long sessionTime, bool updateLastPlayed) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    GEODE_UNWRAP(setSession(stats, levelKey, sessionTime, updateLastPlayed));

    return Ok();
}
Result<> StatsManager::setSession(Session& stats, const std::string& levelKey, long long sessionTime, bool updateLastPlayed) {
    createFilesIfNeeded(levelKey);

    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::SESSIONS_DIR_NAME / (std::to_string(sessionTime) + ".dt");

    createFile(levelSaveFilePath, Session{});

    auto indentation = Dev::MINIFY_SAVE_FILE
        ? matjson::NO_INDENTATION
        : 4;

    if (updateLastPlayed) {
        auto now = StatsManager::getNowSeconds();
        stats.lastPlayed = now;
        stats.recordTimestamp(now);
    }

    stats.ownerLevelKey = levelKey;

    auto jsonStr = matjson::Value(stats).dump(indentation);
    GEODE_UNWRAP(file::writeString(levelSaveFilePath, jsonStr));

    return Ok();
}

Result<> StatsManager::setGeneral(const GeneralData& stats, GJGameLevel* const level) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    GEODE_UNWRAP(setGeneral(stats, levelKey));

    return Ok();
}
Result<> StatsManager::setGeneral(const GeneralData& stats, const std::string& levelKey) {
    createFilesIfNeeded(levelKey);

    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::FROM0_FILE_NAME;

    auto indentation = Dev::MINIFY_SAVE_FILE
        ? matjson::NO_INDENTATION
        : 4;

    auto jsonStr = matjson::Value(stats).dump(indentation);
    GEODE_UNWRAP(file::writeString(levelSaveFilePath, jsonStr));

    return Ok();
}

void StatsManager::addBackup(const std::string& levelKey, bool saveLevelStats, std::optional<int> sessionsToSave, bool showNotifications) {
    // log::info("adding backup for level {} | {} | {}", levelKey, saveLevelStats, sessionsToSave);

    auto progressFunc = [](float progress01) {
        // maybe ill use this later for somethn idk bruh
    };

    auto backupFunc = [levelKey, saveLevelStats, sessionsToSave, progressFunc]() -> arc::Future<Result<>> {
        progressFunc(0.0f);
        auto metaRes = getMetadata(levelKey);
        if (metaRes.isErr()) co_return Err("No level to back up! {}", metaRes.unwrapErr().error);
        auto metadata = metaRes.unwrap();
        
        createFilesIfNeeded(levelKey);
        progressFunc(0.1f);

        auto levelBackupsFilePath = getSavesFolderPath() / levelKey / StatsManager::BACKUPS_DIR_NAME;

        auto lvlBackupsDirRes = geode::utils::file::createDirectory(levelBackupsFilePath);
        if (lvlBackupsDirRes.isErr()) co_return Err("failed to create backups folder! {}", lvlBackupsDirRes.unwrapErr());

        auto currBackupName = StatsManager::getNowSeconds();

        levelBackupsFilePath /= std::to_string(currBackupName);

        auto lvlBackupDirRes = geode::utils::file::createDirectory(levelBackupsFilePath);
        if (lvlBackupDirRes.isErr()) co_return Err("failed to create backup folder! {}", lvlBackupDirRes.unwrapErr());
        progressFunc(0.2f);

        if (saveLevelStats) {

            if (getGeneral(levelKey).isErr()) {
                (void)deleteBackup(levelKey, currBackupName);

                co_return Err("backup failed! failed to read general stats");
            }

            auto copyRes = asp::fs::copy(getSavesFolderPath() / levelKey / StatsManager::FROM0_FILE_NAME, levelBackupsFilePath / StatsManager::FROM0_FILE_NAME);
            if (copyRes.isErr()) co_return Err("Failed to backup level stats: {}", copyRes.unwrapErr());
        }
        else if (sessionsToSave.has_value() && (getSavesFolderPath() / levelKey / StatsManager::FROM0_FILE_NAME).exists()) {
            auto copyRes = asp::fs::copy(
                getSavesFolderPath() / levelKey / StatsManager::FROM0_FILE_NAME,
                levelBackupsFilePath / StatsManager::FROM0_FILE_NAME
            );
            if (copyRes.isErr()) co_return Err("Failed to back up general data required by sessions: {}", copyRes.unwrapErr());
        }
        progressFunc(0.35f);

        auto backupsAmount = Settings::getMaxBackupAmount();

        if (backupsAmount != std::nullopt) {
            auto count = getBackupsCount(levelKey);

            if (count.size() > backupsAmount.value()) {
                int index = 0;
                for (const auto& backupName : count)
                {
                    if (index == count.size() - backupsAmount.value()) break;

                    (void)deleteBackup(levelKey, backupName);

                    index++;
                }
            }
        }
        progressFunc(0.5f);

        auto copyRes = asp::fs::copy(getSavesFolderPath() / levelKey / StatsManager::METADATA_FILE_NAME, levelBackupsFilePath / StatsManager::METADATA_FILE_NAME);
        if (copyRes.isErr()) co_return Err("Failed to backup level metadata: {}", copyRes.unwrapErr());
        progressFunc(0.7f);

        if (sessionsToSave == std::nullopt) {
            progressFunc(1.0f);
            co_return Ok();
        }

        if (sessionsToSave.value() >= -1) {
            levelBackupsFilePath /= StatsManager::SESSIONS_DIR_NAME;

            auto lvlBackupsDirRes = geode::utils::file::createDirectory(levelBackupsFilePath);
            if (lvlBackupsDirRes.isErr()) co_return Err("failed to create backup sessions folder! {}", lvlBackupsDirRes.unwrapErr());

            std::set<long long, std::greater<long long>> sessionTimes{};
            auto nonSorted = getAllSessionTimesForLevel(levelKey);
            sessionTimes.insert(nonSorted.begin(), nonSorted.end());

            int numToSave = sessionsToSave.value();
            if (numToSave == -1)
                numToSave = sessionTimes.size();

            int index = 0;
            for (const auto& sessionTime : sessionTimes)
            {
                if (index == numToSave) break;

                if (getSession(levelKey, sessionTime).isErr()) {
                    log::error("Failed to backup session {}", sessionTime);
                    index++;
                    continue;
                }

                auto copyRes = asp::fs::copy(
                    getSavesFolderPath() / levelKey / StatsManager::SESSIONS_DIR_NAME / (std::to_string(sessionTime) + ".dt"), 
                    levelBackupsFilePath / (std::to_string(sessionTime) + ".dt")
                );
                if (copyRes.isErr()) co_return Err("Failed to backup session {}: {}", sessionTime, copyRes.unwrapErr());

                index++;
                if (numToSave > 0) {
                    float sessionProgress = 0.7f + 0.3f * (static_cast<float>(index) / static_cast<float>(numToSave));
                    if (sessionProgress > 1.0f) sessionProgress = 1.0f;
                    progressFunc(sessionProgress);
                }
            }

            if (!sessionTimes.size())
                progressFunc(1.0f);
        }
        else progressFunc(1.0f);
        
        co_return Ok();
    };

    if (StatsManager::isGameClosing) {
        return;
    }

    if (backupHandler.isValid()) {
        backupHandler.abort();
    }

    backupHandler = async::spawn(
        backupFunc,
        [showNotifications](Result<> res) {
            if (!showNotifications) return;

            if (res.isErr()) {
                log::error("{}", res.unwrapErr());
                Notification::create("Failed to create backup!", NotificationIcon::Error)->show();
            }
            else Notification::create("Created backup successfully!", NotificationIcon::Success)->show();
        }
    );
    backupHandler.setName("DT-backup-task");
}

void StatsManager::createFilesIfNeeded(const std::string& levelKey) {
    (void)geode::utils::file::createDirectory(getSavesFolderPath());

    auto levelSaveFilePath = getSavesFolderPath() / levelKey;

    auto lvlSavesDirRes = geode::utils::file::createDirectory(levelSaveFilePath);
    if (lvlSavesDirRes.isErr() && !lvlSavesDirRes.unwrapErr().starts_with("AlreadyExists"))
        log::error("failed to create level folder: {}", lvlSavesDirRes.unwrapErr());

    createFile((levelSaveFilePath / METADATA_FILE_NAME), LevelMetadeta{});
    createFile((levelSaveFilePath / FROM0_FILE_NAME), GeneralData{});
    (void)geode::utils::file::createDirectory((levelSaveFilePath / SESSIONS_DIR_NAME));
}

void StatsManager::createFile(const asp::fs::Path& path, std::optional<matjson::Value> jsonToFill) {
    if (path.exists()) return;

    auto fileRes = asp::fs::File::create(path);

    if (fileRes.isErr()) return;
    auto file = std::move(fileRes).unwrap();
    
    if (jsonToFill.has_value())
        auto _ = file.write(jsonToFill.value().dump(matjson::NO_INDENTATION));
    file.close();
}

void StatsManager::setCurrentLevel(GJGameLevel* const& level) {
    currentLevel = level;

    if (level == nullptr) {
        currentFrom0 = GeneralData{};
        currentSession = Session{};
        currentMetadata = LevelMetadeta{};
        return;
    }

    auto from0Res = getGeneral(level);
    //auto sessionCount = getAllSessionTimesForLevel(level);
    //auto sessionRes = sessionCount.isErr() ? Err(sessionCount.unwrapErr()) : getSession(level, sessionCount.unwrap());
    auto metadataRes = getMetadata(level);

    if (metadataRes.isErr()) {
        currentLevel = nullptr;
        currentFrom0 = std::nullopt;
        currentSession = std::nullopt;
        currentMetadata = std::nullopt;
        log::error("Failed to apply current level stats as main stats ({})", metadataRes.unwrapErr().error);
        return;
    }

    if (from0Res.isErr()) {
        if (from0Res.unwrapErr().code == 1) {
            auto newF0 = GeneralData{};

            from0Res = Ok(newF0);
        }
        else {
            from0Res = Err(ErrorWithCode("", 3));
            currentFrom0 = std::nullopt;
        }
    }
    
    if (from0Res.isOk())
        currentFrom0 = from0Res.unwrap();

    currentMetadata = metadataRes.unwrap();
}

void StatsManager::logDeath(double percent, bool instantSave) {
    if (currentLevel == nullptr) {
        log::error("Failed to log death");
        return;
    }

    auto parsedPercent = ComplexPercent::fromPercent(percent);
    if (parsedPercent.isErr()) {
        log::error("Failed to log death at invalid percent {}: {}", percent, parsedPercent.unwrapErr());
        return;
    }

    if (instantSave)
        safeCheckCurrF0();

    auto exactPercent = parsedPercent.unwrap();
    const auto bestPercent = exactPercent.asPercent();
    double overallBest = -1;

    if (currentFrom0.has_value()) {
        auto& val = currentFrom0.value();
        ComplexDeathOccurrence occurrence;
        occurrence.runEndTimestamp = static_cast<time_t>(getNowSeconds());
        auto recordResult = val.deaths.recordDeath(exactPercent, std::nullopt, occurrence);
        if (recordResult.isErr())
            log::error("Failed to record death at {}%: {}", percent, recordResult.unwrapErr());

        if (val.newBests.size() != 0) overallBest = *val.newBests.rbegin();

        if (bestPercent > overallBest) {
            val.newBests.insert(bestPercent);
        }
    }

    auto session = StatsManager::getCurrentSession();
    if (session != nullptr) {
        // log::info("logging death {} to session {}", percentKey, session->sessionStartDate);

        int sessionBest = -1;
        if (session->data.newBests.size() != 0) sessionBest = *session->data.newBests.rbegin();

        if (bestPercent > sessionBest) {
            session->data.newBests.insert(bestPercent);
        }
    }

    if (instantSave) {
        if (currentFrom0.has_value()) {
            (void)StatsManager::setGeneral(currentFrom0.value(), currentLevel);
        }
        if (session) {
            (void)StatsManager::setSession(*session, currentLevel, session->sessionStartDate, true);
        }
    }
}

void StatsManager::logDeaths(const std::vector<double>& percents) {
    if (currentLevel == nullptr) {
        log::error("Failed to log deaths");
        return;
    }

    safeCheckCurrF0();

    for (int i = 0; i < percents.size(); i++)
    {
        logDeath(percents[i], false);
    }

    auto session = StatsManager::getCurrentSession();

    if (currentFrom0.has_value()) {
        (void)StatsManager::setGeneral(currentFrom0.value(), currentLevel);
    }
    if (session) {
        //log::info("session does exist");
        (void)StatsManager::setSession(*session, currentLevel, session->sessionStartDate, true);
    }
}

void StatsManager::logRun(const Run& run, bool instantSave) {
    if (currentLevel == nullptr || !run.start.has_value()) {
        log::error("Failed to log run");
        return;
    }

    auto startPercent = ComplexPercent::fromPercent(run.start.value());
    auto endPercent = ComplexPercent::fromPercent(run.end);
    if (startPercent.isErr() || endPercent.isErr()) {
        log::error("Failed to log run with invalid percentages");
        return;
    }

    if (instantSave)
        safeCheckCurrF0();

    const auto recordedAt = static_cast<time_t>(getNowSeconds());
    ComplexDeathOccurrence occurrence;
    occurrence.runStartTimestamp = run.startTimestamp;
    if (!occurrence.runStartTimestamp.has_value()) {
        log::warn("Run start timestamp is missing; using the logging time");
        occurrence.runStartTimestamp = recordedAt;
    }
    occurrence.runEndTimestamp = run.endTimestamp.value_or(recordedAt);

    auto session = StatsManager::getCurrentSession();
    if (session != nullptr) {
        // log::info("logging run {}-{} to session {}", run.start.value(), run.end, session->sessionStartDate);
    }

    if (currentFrom0.has_value()) {
        auto& val = currentFrom0.value();
        auto recordResult = val.runs.recordDeath(
            endPercent.unwrap(),
            startPercent.unwrap(),
            occurrence
        );
        if (recordResult.isErr())
            log::error("Failed to record run: {}", recordResult.unwrapErr());
    }
    
    if (instantSave) {
        if (currentFrom0.has_value()) {
            (void)StatsManager::setGeneral(currentFrom0.value(), currentLevel);
        }
        if (session) {
            (void)StatsManager::setSession(*session, currentLevel, session->sessionStartDate, true);
        }
    }
}

void StatsManager::logRuns(const std::vector<Run>& runs) {
    if (currentLevel == nullptr) {
        log::error("Failed to log runs");
        return;
    }

    safeCheckCurrF0();

    for (int i = 0; i < runs.size(); i++)
    {
        logRun(runs[i], false);
    }

    auto session = StatsManager::getCurrentSession();

    if (currentFrom0.has_value()) {
        (void)StatsManager::setGeneral(currentFrom0.value(), currentLevel);
    }
    if (session) {
        //log::info("session does exist r");
        (void)StatsManager::setSession(*session, currentLevel, session->sessionStartDate, true);
    }
}

bool StatsManager::safeCheckCurrF0() {
    auto from0Res = getGeneral(currentLevel);
    if (from0Res.isErr() && from0Res.unwrapErr().code != 1) {
        currentFrom0 = std::nullopt;
        Notification::create(fmt::format("Death Tracker cant save! {}", from0Res.unwrapErr().error), NotificationIcon::Error)->show();
        return false;
    }
    else if (from0Res.isErr()) {
        return true;
    }
    
    if (!currentFrom0.has_value()) {
        currentFrom0 = from0Res.unwrap();
    }
    return true;
}

/* utility functions
===================== */
long long StatsManager::getNowSeconds() {
    using namespace std::chrono;
    auto now = system_clock::now();
    return time_point_cast<seconds>(now).time_since_epoch().count();
}

Result<std::string> StatsManager::getLevelKey(GJGameLevel* const& level) {
	if (!level) return Err("invalid level!");

	std::string levelId;

    if (level->m_levelType == GJLevelType::Editor) {
        auto idRes = EditorIDs::getID(level);
        if (idRes.isErr()) return Err("Failed to get editor ID!");
        levelId += std::to_string(idRes.unwrap());
    }
    else {
        levelId += std::to_string(level->m_levelID.value());
    }

	// local level postfix
	if (level->m_levelType == GJLevelType::Main)
		levelId += "-local";

    if (level->m_levelType == GJLevelType::Editor)
		levelId += "-editor";

	// daily/weekly postfix
	if (level->m_dailyID > 0)
		levelId += "-daily";

	// gauntlet level postfix
	if (level->m_gauntletLevel)
		levelId += "-gauntlet";

	return Ok(levelId);
}

Result<Run> StatsManager::splitRunKey(const std::string& runKey) {
    if (runKey.empty())
        return Err("Invalid run key!");

    auto separator = runKey.find('-', runKey.starts_with('-') ? 1 : 0);
    std::optional<double> start = std::nullopt;
    double end = 0;

    if (separator == std::string::npos) {
        GEODE_UNWRAP_INTO(end, geode::utils::numFromString<double>(runKey));
    }
    else {
        GEODE_UNWRAP_INTO(double startRes, geode::utils::numFromString<double>(runKey.substr(0, separator)));
        start = startRes;
        GEODE_UNWRAP_INTO(end, geode::utils::numFromString<double>(runKey.substr(separator + 1)));
    }

    Run r;
    r.start = start;
    r.end = end;

    return Ok(r);
}

Result<std::string> StatsManager::createRunKey(const Run& runKey) {
    if (runKey.start == std::nullopt) {
        return Ok(fmt::format("{}", runKey.end));
    }
    else if (runKey.start.value() >= 0) {
        return Ok(fmt::format("{}-{}", runKey.start.value(), runKey.end));
    }

    return Err("Bad percentages");
}

Session* StatsManager::getCurrentSession() {
    if (currentLevel == nullptr) {
        log::error("Failed to get session");
        return nullptr;
    }

    auto currentSession = StatsManager::currentSession.has_value() ? &StatsManager::currentSession.value() : nullptr;
    // log::info("getting curr session {}", currentSession == nullptr ? "session is null" : std::to_string(currentSession->sessionStartDate));

    auto levelKeyRes = StatsManager::getLevelKey(currentLevel);

    //log::info("scheduled new session ? {}", m_scheduleCreateNewSession);

    if (levelKeyRes.isErr()) return currentSession;

    //log::info("a");
    
    auto sessionsCount = getAllSessionTimesForLevel(levelKeyRes.unwrap());

    // new sessions can be scheduled
    // and are created when the player dies
    if (!sessionsCount.size() && !m_scheduleCreateNewSession) return nullptr;

    if (!m_scheduleCreateNewSession && (currentSession == nullptr || currentSession->sessionStartDate == 0)) {
        auto latestSession = StatsManager::getSession(currentLevel, *std::prev(sessionsCount.end()));
        if (latestSession.isErr()) {
            log::error("failed to load latest session! current session is dysfunctional");
            StatsManager::currentSession = std::nullopt;
            currentSession = nullptr;
        }
        else {
            StatsManager::currentSession = latestSession.unwrap();
        }
    }

    if (!m_scheduleCreateNewSession) return currentSession;
    //log::info("c");
    m_scheduleCreateNewSession = false;

    // the user has played the level
    // if a new session is created
    m_playedLevels.insert(levelKeyRes.unwrap());

    // create the new session
    auto session = Session {
        .lastPlayed = -1,
        .sessionStartDate = StatsManager::getNowSeconds(),
        .data = GeneralData{
            .deaths = {},
            .runs = {},
            .newBests = {}
        }
    };

    if (setSession(session, currentLevel, session.sessionStartDate, true).isErr()) {
        log::error("Failed to create new session");
        return currentSession;
    }

    StatsManager::currentSession = session;

    auto meta = getMetadata(currentLevel).unwrapOrDefault();

    for (const auto& linked : meta.linkedLevels)
    {
        if (linked == levelKeyRes.unwrap() || m_playedLevels.contains(linked)) continue;

        m_playedLevels.insert(linked);

        if (setSession(session, linked, session.sessionStartDate, true).isErr()) {

        }
    }
    
    return currentSession;
}

void StatsManager::scheduleCreateNewSession(const bool& scheduled) {
    if (currentLevel == nullptr) {
        log::error("Failed to get session");
        return;
    }

    m_scheduleCreateNewSession = scheduled;
}

bool StatsManager::hasPlayedLevel() {
    if (currentLevel == nullptr) {
        log::error("Failed to check if level has been played");
        return false;
    }
    
    auto levelKeyRes = StatsManager::getLevelKey(currentLevel);

    if (levelKeyRes.isErr()) return false;

    return m_playedLevels.contains(levelKeyRes.unwrap());
}

/* internal functions
======================= */

Result<> StatsManager::deleteLevelStats(const std::string& levelKey) {
    auto levelSaveFilePath = getSavesFolderPath() / levelKey;

    if (levelSaveFilePath.isDir()) {
        auto res = asp::fs::removeDirAll(levelSaveFilePath);
        if (res.isErr()) return Err("Failed to delete stats folder: {}", res.unwrapErr());
        return Ok();
    }

    return Err("Cant delete stats because level save file does not exist: " + levelSaveFilePath.string());
}

Result<> StatsManager::deleteBackup(const std::string& levelKey, long long backupName) {
    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName);

    if (levelSaveFilePath.isDir()) {
        auto res = asp::fs::removeDirAll(levelSaveFilePath);
        if (res.isErr()) return Err("Failed to delete backup {}: {}", backupName, res.unwrapErr());
        return Ok();
    }

    return Err("Cant delete backup because level save file does not exist: " + levelSaveFilePath.string());
}

Result<> StatsManager::deleteAllSessions(const std::string& levelKey) {
    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::SESSIONS_DIR_NAME;

    if (levelSaveFilePath.isDir()) {
        auto res = asp::fs::removeDirAll(levelSaveFilePath);
        if (res.isErr()) return Err("Failed to delete sessions folder: {}", res.unwrapErr());
        return Ok();
    }

    return Err("Cant delete sessions because level save file does not exist: " + levelSaveFilePath.string());
}

Result<std::tuple<NewBests, int>> StatsManager::calcNewBests(GJGameLevel* const& level) {
    NewBests newBests{};
    std::stringstream bestsStream(level->m_personalBests);
    std::string currentBest;
    int currentPercent = 0;

    while (std::getline(bestsStream, currentBest, ',')) {
        GEODE_UNWRAP_INTO(auto tempPercent, geode::utils::numFromString<int>(currentBest));
        
        currentPercent += tempPercent;
        newBests.insert(currentPercent);
    }

    return Ok(std::make_tuple(newBests, currentPercent));
}

std::string StatsManager::getFont(const int& fontID) {
    for (int i = 0; i < m_AllFontsMap.size(); i++)
    {
        if (i == fontID)
            return m_AllFontsMap[i];
    }
    return m_AllFontsMap[1];
}

std::string StatsManager::getFontName(const int& fontID) {
    for (int i = 0; i < m_AllFontsMap.size(); i++)
    {
        if (i == fontID) {
            std::string fontName;

            if (i == 0)
                fontName = "Big Font";
            else if (i == 1)
                fontName = "Chat Font";
            else if (i == 2)
                fontName = "Gold Font";
            else {
                fontName = "Font " + std::to_string(i - 2);
            }

            return fontName;
        }
    }
    return "Chat Font";
}

std::array<std::string, 62> StatsManager::getAllFonts() {
    return m_AllFontsMap;
}

Result<std::vector<std::pair<std::string, LevelMetadeta>>> StatsManager::getAllLevels() {
    GEODE_UNWRAP_INTO(auto allLevels, file::readDirectory(getSavesFolderPath()));

    std::vector<std::pair<std::string, LevelMetadeta>> toReturn{};

    std::vector<std::string> failedKeys{};

    for (int i = 0; i < allLevels.size(); i++)
    {
        if (allLevels[i].isDir()) {
            auto currentDirNameRes = allLevels[i].filename();
            if (!currentDirNameRes.has_value()) continue;
            auto currentDirName = std::string(currentDirNameRes.value());
            // log::info("Getting level stats for level: {}", currentDirName);

            auto currentLevel = StatsManager::getMetadata(currentDirName);
            if (currentLevel.isErr()) {
                Notification::create(fmt::format("failed getting some levels, errors send in logs.", currentDirName, currentLevel.unwrapErr().error), nullptr)->show();
                failedKeys.push_back(currentDirName);
                continue;
            }

            auto stats = currentLevel.unwrap();

            toReturn.push_back(std::make_pair(currentDirName, stats));
        }
    }

    if (failedKeys.size()) {
        log::warn("Failed loading some level! failed level keys are:");
        log::warn("------------");

        for (const auto& failedKey : failedKeys)
            log::warn("- {}", failedKey);
        
        log::warn("------------");
        log::warn("Make sure to check these levels and revert to their backups if possible!");
    }
    
    return Ok(toReturn);
}

std::pair<std::string, std::string> StatsManager::splitLevelKey(const std::string& levelKey) {
    auto levelKeySplit = splitStr(levelKey, "-");

    std::pair<std::string, std::string> toReturn;

    toReturn.first = levelKeySplit[0];

    if (levelKeySplit.size() == 2) {
        toReturn.second = levelKeySplit[1];
    }
    else {
        toReturn.second = "online";
    }

    return toReturn;
}

int StatsManager::getDifficulty(GJGameLevel* const& level) {
    if (level->m_autoLevel)
        return -1;

    if (level->m_ratingsSum != 0)
        if (level->m_demon == 1) {
            int fixedNum = level->m_demonDifficulty;
            // Demon difficulty values in GD are offset by 2 (e.g., 3 = Easy, 4 = Medium)
            if (fixedNum >= 3)
                fixedNum -= 2;

            return 6 + fixedNum;
        }
        else {
            return level->m_ratingsSum / level->m_ratings;
        }
    else 
        return 0;
}

int StatsManager::getCursorPosition(CCLabelBMFont* const& text, CCLabelBMFont* const& cursor) {
    if (text->getString() == nullptr) return -1;
    if (text->getString()[0] == '\0') return -1;

    std::string tempS = text->getString();
    if (tempS == " ") return 0;

    int index = -1;

    for (const auto& node : CCArrayExt<CCNode*>(text->getChildren()))
    {
        if (node->isVisible()) {
            index++;

            if (node->getParent()->convertToWorldSpace(node->getPosition()).x > cursor->getParent()->convertToWorldSpace(cursor->getPosition()).x)
                return index;
        }
    }
    
    return index + 1;
}

bool StatsManager::isKeyInIndex(const std::string& s, const int& index, const std::string& key) {
    if (index + key.length() > s.length()) return false;

    return s.substr(index, key.length()) == key;
}

std::string StatsManager::workingTime(long long value) {
    if(value < 0) return fmt::format("NA ({})", value);
    if(value == 0) return "NA";

    int hours = value / 3600;
    int minutes = (value % 3600) / 60;
    int seconds = value % 60;

    std::ostringstream stream;
    if(hours > 0) stream << hours << "h ";
    if(minutes > 0) stream << minutes << "m ";
    stream << seconds << "s";

    return stream.str();
}

std::string StatsManager::workingTime(uint64_t nanoseconds) {
    if (nanoseconds == 0) return "NA";

    uint64_t totalSeconds = nanoseconds / 1'000'000'000ULL;

    uint64_t hours = totalSeconds / 3600;
    uint64_t minutes = (totalSeconds % 3600) / 60;
    uint64_t seconds = totalSeconds % 60;

    std::ostringstream stream;
    if (hours > 0) stream << hours << "h ";
    if (minutes > 0) stream << minutes << "m ";
    stream << seconds << "s";

    return stream.str();
}

void StatsManager::updateCurrentSessionLastPlayed() {
    if (currentLevel == nullptr) {
        log::error("Failed to update current session last played");
        return;
    }

    auto session = StatsManager::getCurrentSession();
    if (!session) return;

    auto now = StatsManager::getNowSeconds();
    session->lastPlayed = now;

    (void)StatsManager::setSession(*session, currentLevel, session->sessionStartDate, true);
}

GJGameLevel* StatsManager::getCurrentLevel() {
    return StatsManager::currentLevel;
}

Result<std::set<long long>> StatsManager::getAllSessionTimesForLevel(GJGameLevel* const level) {
    GEODE_UNWRAP_INTO(auto levelKey, StatsManager::getLevelKey(level));
    return Ok(getAllSessionTimesForLevel(levelKey));
}

std::set<long long> StatsManager::getAllSessionTimesForLevel(const std::string& levelKey) {
    return getAllSessionTimesForLevel(getSavesFolderPath() / levelKey);
}

std::set<long long> StatsManager::getAllSessionTimesForLevel(const asp::fs::Path& path) {
    auto levelSaveFilePath = path / StatsManager::SESSIONS_DIR_NAME;

    if (!levelSaveFilePath.exists()) return {};

    std::set<long long> toReturn{};

    auto dirIteratorRes = asp::fs::iterDir(levelSaveFilePath);

    if (dirIteratorRes.isErr()) return {};

    for (const auto& entryRes : dirIteratorRes.unwrap()) {
        if (entryRes.isErr()) continue;
        auto entry = entryRes.unwrap();

        if (!entry.isFile() || entry.path.extension() != "dt") continue;

        auto filenameRes = entry.path.filename();

        if (!filenameRes.has_value()) continue;

        auto numRes = geode::utils::numFromString<long long>(filenameRes.value());
        if (numRes.isErr()) continue;

        toReturn.insert(numRes.unwrap());
    }

    return toReturn;
}

std::set<long long> StatsManager::getBackupsCount(const std::string& levelKey) {
    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::BACKUPS_DIR_NAME;

    if (!levelSaveFilePath.exists()) return {};

    std::set<long long> toReturn{};

    auto dirIteratorRes = asp::fs::iterDir(levelSaveFilePath);

    if (dirIteratorRes.isErr()) return {};

    for (const auto& entryRes : dirIteratorRes.unwrap()) {
        if (entryRes.isErr()) continue;
        auto entry = entryRes.unwrap();

        if (!entry.isDir()) continue;

        auto dirnameRes = entry.path.filename();
        if (!dirnameRes.has_value()) continue;

        auto numRes = geode::utils::numFromString<long long>(dirnameRes.value());
        if (numRes.isErr()) continue;

        toReturn.insert(numRes.unwrap());
    }

    return toReturn;
}

uintmax_t StatsManager::getBackupFileSize(const std::string& levelKey, long long backupName) {
    auto levelSaveFilePath = getSavesFolderPath() / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName);

    if (!levelSaveFilePath.exists()) return 0;

    auto dirIteratorRes = asp::fs::iterDirRecursive(levelSaveFilePath);

    if (dirIteratorRes.isErr()) return 0;

    uintmax_t totalSize = 0;

    for (const auto& entryRes : dirIteratorRes.unwrap()) {
        if (entryRes.isErr()) continue;
        auto entry = entryRes.unwrap();
        
        if (!entry.isFile()) continue;

        auto fileSizeRes = asp::fs::fileSize(entry.path);
        if (fileSizeRes.isErr()) continue;

        totalSize += fileSizeRes.unwrap();
    }

    return totalSize;
}

Result<> StatsManager::reveretBackupSessions(const std::string& levelKey, long long backupName) {
    auto bakupSessionDirPath = getSavesFolderPath() / levelKey / StatsManager::BACKUPS_DIR_NAME / std::to_string(backupName) / StatsManager::SESSIONS_DIR_NAME;
    auto sessionDirPath = getSavesFolderPath() / levelKey / StatsManager::SESSIONS_DIR_NAME;

    if (!bakupSessionDirPath.exists()) return Err("No sessions backup found!");

    if (sessionDirPath.exists()) {
        auto removeRes = asp::fs::removeDirAll(sessionDirPath);
        if (removeRes.isErr()) return Err("Failed to delete current sessions folder: {}", removeRes.unwrapErr());
    }

    auto copyRes = asp::fs::copy(bakupSessionDirPath, sessionDirPath);
    if (copyRes.isErr()) return Err("Failed to revert sessions: {}", copyRes.unwrapErr());

    return Ok();
}

Result<> StatsManager::convertV2SaveToV3(const std::string& levelKey) {
    log::info("converting V2 save to V3...");

    if (!getSavesFolderPath().exists()) return Err("No levels directory found");

    auto v2Path = getSavesFolderPath() / (levelKey + ".json");

    if (!v2Path.exists()) return Err("V2 level file doesnt exist!");
    
    auto res = file::readJson(v2Path);
    GEODE_UNWRAP_INTO(auto json, res);
    
    auto statsRes = json.as<V2LevelStats>();

    if (statsRes.isErr()) return Err("Error parsing V2 json!");

    auto stats = statsRes.unwrap();

    auto v3Meta = LevelMetadeta{};
    std::map<int, int> runsMap{};
    bool showAnyRun = false;
    for (const auto& percent : stats.RunsToSave)
    {
        if (percent == -1) {
            showAnyRun = true;
            continue;
        }

        runsMap.insert({percent, percent});
    }
    v3Meta.runsToShow = runsMap;
    v3Meta.showAnyRun = showAnyRun;
    std::set<std::string> linkedLevelsSet(stats.LinkedLevels.begin(), stats.LinkedLevels.end());
    v3Meta.linkedLevels = linkedLevelsSet;
    v3Meta.levelName = stats.levelName;
    v3Meta.attempts = stats.attempts;
    v3Meta.difficulty = stats.difficulty;
    v3Meta.hideUpto = stats.hideUpto;

    if (setMetadata(v3Meta, levelKey).isErr()) return Err("Failed to write V3 meta!");

    auto v3General = GeneralData{};
    auto deathsResult = ComplexDeathMap::fromLegacy(stats.deaths, false);
    if (deathsResult.isErr())
        return Err("Failed converting V2 deaths: {}", deathsResult.unwrapErr());
    auto runsResult = ComplexDeathMap::fromLegacy(stats.runs, true);
    if (runsResult.isErr())
        return Err("Failed converting V2 runs: {}", runsResult.unwrapErr());
    v3General.deaths = std::move(deathsResult).unwrap();
    v3General.runs = std::move(runsResult).unwrap();
    v3General.newBests = stats.newBests;

    if (setGeneral(v3General, levelKey).isErr()) return Err("Failed to write V3 general!");

    for (const auto& v2session : stats.sessions) {
        auto v3Session = Session{};

        v3Session.ownerLevelKey = levelKey;
        v3Session.lastPlayed = v2session.lastPlayed;
        auto sessionDeathsResult = ComplexDeathMap::fromLegacy(v2session.deaths, false);
        if (sessionDeathsResult.isErr())
            return Err("Failed converting V2 session deaths: {}", sessionDeathsResult.unwrapErr());
        auto sessionRunsResult = ComplexDeathMap::fromLegacy(v2session.runs, true);
        if (sessionRunsResult.isErr())
            return Err("Failed converting V2 session runs: {}", sessionRunsResult.unwrapErr());
        v3Session.data.deaths = std::move(sessionDeathsResult).unwrap();
        v3Session.data.runs = std::move(sessionRunsResult).unwrap();
        v3Session.data.newBests = v2session.newBests;
        v3Session.sessionStartDate = v2session.sessionStartDate;
        v3Session.legacyData = v3Session.data;

        if (setSession(v3Session, levelKey, v3Session.sessionStartDate, false).isErr()) return Err("Failed to write V3 session! session sd: {}", v3Session.sessionStartDate);
    }

    auto removeV2PathRes = asp::fs::remove(v2Path);
    if (removeV2PathRes.isErr()) return Err("Failed to erase old data: {}", removeV2PathRes.unwrapErr());

    auto backupPath = getSavesFolderPath() / (levelKey + ".deathsBackup");

    if (backupPath.exists()) {
        auto removeBackupRes = asp::fs::remove(backupPath);
        if (removeBackupRes.isErr()) return Err("Failed to delete backup old data: {}", removeBackupRes.unwrapErr());
    }

    return Ok();
}

std::vector<std::string> StatsManager::allV2FileLevelKeys() {
    if (!getSavesFolderPath().exists()) return {};
    
    std::vector<std::string> toReturn{};

    auto dirIterRes = asp::fs::iterDir(getSavesFolderPath());
    if (dirIterRes.isErr()) return {};

    for (const auto& entryRes : dirIterRes.unwrap()) {
        if (entryRes.isErr()) continue;
        auto entry = entryRes.unwrap();

        if (!entry.isFile() || entry.path.extension() != "json") continue;

        auto stemRes = entry.path.stem();
        if (!stemRes.has_value()) continue;

        toReturn.push_back(std::string(stemRes.value()));
    }

    return toReturn;
}

std::vector<std::string> StatsManager::allV3FileLevelKeys() {
    std::vector<std::string> toReturn{};

    auto dirIterRes = asp::fs::iterDir(getSavesFolderPath());
    if (dirIterRes.isErr()) return {};

    for (const auto& entryRes : dirIterRes.unwrap()) {
        if (entryRes.isErr()) continue;
        auto entry = entryRes.unwrap();

        if (!entry.isDir()) continue;

        auto stemRes = entry.path.stem();
        if (!stemRes.has_value()) continue;

        toReturn.push_back(std::string(stemRes.value()));
    }

    return toReturn;
}

bool StatsManager::transferPlaytimeFromPT(GJGameLevel* level) {
    auto data = getLevelData(level);

    return transferPlaytimeFromPT(data, level);
}

bool StatsManager::transferPlaytimeFromPT(geode::Result<LevelData, ErrorWithCode>& data, GJGameLevel* level) {
    std::string lvlID = level->m_levelType == GJLevelType::Editor ? fmt::format("Editor-{}", EditorIDs::getID(level)) : std::to_string(level->m_levelID.value());

    auto parentPathRes = Mod::get()->getSaveDir().parent();
    if (!parentPathRes.has_value()) return false;

    auto ptPath = parentPathRes.value() / "nanew.playtime-tracker" / "data" / (lvlID + ".json");

    if (!ptPath.exists()) return false;

    auto lvlKeyRes = StatsManager::getLevelKey(level);
    if (lvlKeyRes.isErr()) return false;
    auto lvlKey = lvlKeyRes.unwrap();

    if (data.isErr()) {
        LevelData newData;
        newData.levelKey = lvlKey;
        data = Ok(newData);
    }

    auto& stats = data.unwrap();

    if (stats.metadata.hasGottenDataFromPT) {
        Save::setLevelWasPTTransferred(lvlKey);
        return false;
    }

    if (Save::getLevelWasPTTransferred(lvlKey)) return false;

    Save::setLevelWasPTTransferred(lvlKey);
    (void)StatsManager::setMetadata(stats.metadata, stats.levelKey);

    auto ptObj = file::readFromJson<matjson::Value>(ptPath).unwrapOrDefault();

    if (ptObj.isNull()) return true;

    uint64_t overallPT = 0;

    for (auto session : ptObj["sessions"]) {
        if (!session[0][0].isNumber()) continue;

        for (const auto& dtSession : stats.sessionNames) {
            if (dtSession != session[0][0].as<long long>().unwrap()) continue;

            auto realSessRes = StatsManager::getSession(stats.levelKey, dtSession);
            if (realSessRes.isErr()) continue;
            auto realSess = realSessRes.unwrap();

            for (const auto& ptPair : session) {

                auto endT = ptPair[1].as<long long>();
                auto startT = ptPair[0].as<long long>();
                if (endT.isErr() || startT.isErr()) continue;

                realSess.data.playtimeGeneral.playtimeF0 += (endT.unwrap() - startT.unwrap()) * 1000000000LL;

                break;
            }

            (void)StatsManager::setSession(realSess, stats.levelKey, dtSession, false);
        }

        //log::info("found playtime for level");

        for (auto ptPair : session) {

            //log::info("adding session");

            auto endT = ptPair[1].as<long long>();
            auto startT = ptPair[0].as<long long>();
            if (endT.isErr() || startT.isErr()) continue;

            overallPT += (endT.unwrap() - startT.unwrap());
        }
    }

    if (overallPT != 0) {
        if (stats.from0.isErr()) {
            GeneralData gData{};
            stats.from0 = Ok(gData);
        }

        if (stats.from0.isOk()) {
            auto& f0 = stats.from0.unwrap();
            f0.playtimeGeneral.playtimeF0 += overallPT * 1000000000LL;

            (void)StatsManager::setGeneral(f0, stats.levelKey);
        }
    }

    return true;
}

// data collection for user count idea idk if i can add this yet :(
// void StatsManager::logGameOpening() {
//     std::string platform = "Unknown";
    
//     #ifdef GEODE_IS_WINDOWS
//         platform = "Windows";
//     #elif defined(GEODE_IS_ANDROID)
//         platform = "Android";
//     #elif defined(GEODE_IS_MACOS)
//         platform = "MacOS";
//     #elif defined(GEODE_IS_IOS)
//         platform = "iOS";
//     #endif

//     matjson::Value payload{};
//     payload["platform"] = platform;
    
//     auto req = web::WebRequest();
//     req.bodyJSON(payload);
//     async::spawn(req.post("https://api.abb2k.me/v1/log-opening"), [](web::WebResponse response) {
//         if (response.ok()) {
//             log::info("Request Good");

//             auto res = response.string();
//             if (res.isOk()) {
//                 log::info("aa {}", res.unwrap());
//             }
//         }
//         else {
//             log::info("Request Bad");
//         }
//     });
// }

void StatsManager::mergeMapsAdd(Deaths& target, const ComplexDeathMap& source) {
    mergeMapsAdd(target, source.toLegacyDeaths());
}

void StatsManager::mergeMapsAdd(ComplexDeathMap& target, const ComplexDeathMap& source) {
    auto result = target.merge(source);
    if (result.isErr())
        log::error("Failed to merge death maps: {}", result.unwrapErr());
}