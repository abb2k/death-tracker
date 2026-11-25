#include "DTPlayLayer.hpp"

#include <managers/StatsManager.hpp>
#include <nodes/overlayLabels/LabelEditLayer.hpp>
#include <managers/AuthManager.hpp>
#include <utils/Save.hpp>
#include <utils/Settings.hpp>
#include <utils/RichTextKeys.hpp>
#include <nodes/general/RichTextArea.hpp>

bool DTPlayLayer::disableCompletedLevelTracking() {
    return Settings::isCompletedLevelTrackingDisabled()
        && m_level->m_newNormalPercent2.value() == 100
        && m_level->m_levelType == GJLevelType::Saved;
}

void DTPlayLayer::updateSessionLastPlayed() {
    auto session = StatsManager::getCurrentSession();

    if (!session) return;

    if (session == m_fields->m_prevSession && m_fields->m_isSessionExpired) return;
    StatsManager::updateCurrentSessionLastPlayed();
}

bool DTPlayLayer::init(GJGameLevel* level, bool p1) {
    if (!PlayLayer::init(level, p1)) return false;

    auto metaRes = StatsManager::getMetadata(level);
    if (metaRes.isErr() && metaRes.unwrapErr().m_code == 1) {
        LevelMetadata newMeta{};
        metaRes = Ok(newMeta);
    }
    else if (metaRes.isErr()) {
        geode::Notification::create(fmt::format("Failed to load DT level data! {}", metaRes.unwrapErr().m_error), NotificationIcon::Error)->show();
    }

    if (metaRes.isOk()) {
        auto metadata = metaRes.unwrap();
        metadata.m_levelName = level->m_levelName;
        metadata.m_attempts = level->m_attempts;
        metadata.m_difficulty = StatsManager::getDifficulty(level);

        if (!metadata.m_hasGottenDataFromPT) {
            metadata.m_hasGottenDataFromPT = StatsManager::transferPlaytimeFromPT(level);
        }

        (void)StatsManager::setMetadata(metadata, level);
        
        metaRes = Ok(metadata);
    }

    StatsManager::setCurrentLevel(level);

    m_fields->m_overlayLabelsChangedListener = Save::OverlayLabelsChangedEvent().listen([this]() {
        refreshOverlayLabels();
        return ListenerResult::Propagate;
    });

    m_fields->m_saveDeleteHandle = SaveDeletionEvent().listen([&]() {

        m_fields->m_fzeroToSave.clear();
        m_fields->m_fzeroEndTimestamps.clear();
        m_fields->m_runsToSave.clear();

        m_fields->m_timePassedGeneral = PlaytimePair{};
        m_fields->m_timePassedDead = PlaytimePair{};
        m_fields->m_timePassedPaused = PlaytimePair{};

        StatsManager::currentFrom0 = GeneralData{};
        if (StatsManager::currentSession.has_value()) {
            StatsManager::currentSession.value().m_data = GeneralData{};
        }

        StatsManager::scheduleCreateNewSession(true);
        preloadOverlayData();
        refreshOverlayLabels();
        
        return ListenerResult::Propagate;
    });

    // log::info("PlayLayer::init()");

    auto session = StatsManager::getCurrentSession();
    auto levelKey = StatsManager::getLevelKey(level);
    auto sessionLength = Settings::getMaxSessionLength();

    if (session == nullptr) {
        StatsManager::scheduleCreateNewSession(true);
        DTPlayLayer::updateSessionLastPlayed();
        preloadOverlayData();
        refreshOverlayLabels();
        //log::info("first session created");
        return true;
    }

    // schedule create a new session
    // based on the session length setting
    // -2 means default session (no previous session)
    if (session->m_lastPlayed != -2) {
        switch (sessionLength) {
            // reset session per level play
            case -2: {
                m_fields->m_isSessionExpired = true;
                StatsManager::scheduleCreateNewSession(true);
                break;
            }

            // reset session per game launch
            case -1: {
                // returns true if a new session
                // was created during this game launch
                //log::info("has played lvl?");
                if (StatsManager::hasPlayedLevel()) break;
                //log::info("ya i no p[lay yet]");

                    m_fields->m_isSessionExpired = true;
                    StatsManager::scheduleCreateNewSession(true);
                    break;
                }
        }
    }

    DTPlayLayer::updateSessionLastPlayed();
    preloadOverlayData();
    refreshOverlayLabels();

    return true;
}

#if defined(GEODE_IS_MACOS)
void DTPlayLayer::checkpointActivated(CheckpointGameObject* checkpt) {
    PlayLayer::checkpointActivated(checkpt);

    m_fields->m_currentRun.m_end++;
}
#endif

void DTPlayLayer::cutoutPlaytime() {
    if (!m_fields->m_startTime.has_value()) return;

    auto now = std::chrono::steady_clock::now();
    uint64_t timeSpent = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_fields->m_startTime.value()).count();
    m_fields->m_startTime = now;

    if (Settings::getSafeMode()) return;
    
    if (m_fields->m_currentRun.m_start == 0) {
        if (m_isPaused || m_fields->m_levelBeaten) {
            m_fields->m_timePassedPaused.m_playtimeF0 += timeSpent;
            // log::info("added playtime from 0 paused");
        }
        else if (m_fields->m_lastOneDied) {
            m_fields->m_timePassedDead.m_playtimeF0 += timeSpent;
            // log::info("added playtime from 0 dead");
        }
        else {
            m_fields->m_timePassedGeneral.m_playtimeF0 += timeSpent;
            // log::info("added playtime from 0 general");
        }
    }
    else {
        if (m_isPaused || m_fields->m_levelBeaten) {
            m_fields->m_timePassedPaused.m_playtimeRuns += timeSpent;
            // log::info("added playtime runs paused");
        }
        else if (m_fields->m_lastOneDied) {
            m_fields->m_timePassedDead.m_playtimeRuns += timeSpent;
            // log::info("added playtime runs dead");
        }
        else {
            m_fields->m_timePassedGeneral.m_playtimeRuns += timeSpent;
            // log::info("added playtime runs general");
        }
    }


    //log::info("playing for {}", m_fields->timePassed / 1000000000.0);
}

void DTPlayLayer::discardPlaytime() {
    if (!m_fields->m_startTime.has_value()) return;
    
    m_fields->m_startTime = std::chrono::steady_clock::now();

    //log::info("discarded playtime");
}

void DTPlayLayer::startupPlaytime() {
    m_fields->m_startTime = std::chrono::steady_clock::now();

    //log::info("startup playtime");
}

void DTPlayLayer::resetLevel() {
    m_fields->m_realTimeHistory.clear();
    m_fields->m_gameTimeHistory.clear();
    m_fields->m_rollingRealSum = 0;
    m_fields->m_rollingGameSum = 0;
    m_fields->m_speedhackCompare = std::nullopt;

    cutoutPlaytime();

    if (m_fields->m_lastOneDied) {
        m_fields->m_lastOneDied = false;
    }
    else {
        if (!(DTPlayLayer::disableCompletedLevelTracking() || (Settings::getDisablePractice() && m_isPracticeMode)) && isGameplayActive()) {
            auto metaRes = StatsManager::getMetadata(m_level);

            if (metaRes.isOk() && metaRes.unwrap().m_resetAsDeath) {
                if (!m_level->isPlatformer())
                    m_fields->m_currentRun.m_end = getActualProgress();

                saveRun();
            }
        }
    }
    PlayLayer::resetLevel();
    //log::info("PlayLayer::resetLevel()");

    m_fields->m_levelBeaten = false;

    m_fields->m_hasRespawned = true;

    if (!m_level->isPlatformer())
        m_fields->m_currentRun.m_start = getActualProgress();
    else
        m_fields->m_currentRun.m_start = m_fields->m_currentRun.m_end;
    m_fields->m_currentRun.m_startTimestamp = static_cast<time_t>(StatsManager::getNowSeconds());
    m_fields->m_currentRun.m_endTimestamp.reset();
    m_fields->m_currentRun.m_cheated = false;

    m_fields->m_nolcip = false;
    m_fields->m_speedhack = false;
    m_fields->m_disabledCheat = nullptr;
}

void DTPlayLayer::destroyPlayer(PlayerObject* player, GameObject* p1) {
    PlayLayer::destroyPlayer(player, p1);

    m_fields->m_realTimeHistory.clear();
    m_fields->m_gameTimeHistory.clear();
    m_fields->m_rollingRealSum = 0;
    m_fields->m_rollingGameSum = 0;
    m_fields->m_speedhackCompare = std::nullopt;

    if (!m_fields->m_disabledCheat)
        m_fields->m_disabledCheat = p1;
    if (!m_fields->m_nolcip && m_fields->m_disabledCheat != static_cast<CCObject*>(p1) && !player->m_isDead && !m_levelEndAnimationStarted) {
        log::warn("Noclip Detected!");
        m_fields->m_nolcip = true;
    }

    if (!player->m_isDead) return;
 
    if (!m_fields->m_hasRespawned) return;
    m_fields->m_hasRespawned = false;

    cutoutPlaytime();

    if (DTPlayLayer::disableCompletedLevelTracking() || (Settings::getDisablePractice() && m_isPracticeMode)) return;

    if (!m_level->isPlatformer())
        m_fields->m_currentRun.m_end = getActualProgress();

    m_fields->m_lastOneDied = true;

    if (m_fields->m_currentRun.m_start < 0)
        return;

    saveRun();
}

void DTPlayLayer::saveRun() {
    if (Settings::getSafeMode()) {
        return;
    }
    m_fields->m_currentRun.m_cheated = !isLegal();

    m_fields->m_currentRun.m_endTimestamp = static_cast<time_t>(StatsManager::getNowSeconds());

    if (Settings::getQuickSaveEnabled()) {
        if (!m_fields->m_currentRun.m_cheated && m_fields->m_currentRun.m_start == 0 && !m_isPracticeMode) {
            StatsManager::logDeath(m_fields->m_currentRun.m_end);
        }
        else if (!m_fields->m_currentRun.m_cheated && !m_isPracticeMode && m_level->isPlatformer()) {
            StatsManager::logDeath(m_fields->m_currentRun.m_end);
        }
        else {
            StatsManager::logRun(m_fields->m_currentRun);
        }

        savePlaytime();
        updateOverlaySessionData();
        refreshOverlayLabels();
    }
    else {
        if (!m_fields->m_currentRun.m_cheated && m_fields->m_currentRun.m_start == 0 && !m_isPracticeMode) {
            m_fields->m_fzeroToSave.push_back(m_fields->m_currentRun.m_end);
            m_fields->m_fzeroEndTimestamps.push_back(m_fields->m_currentRun.m_endTimestamp.value());
        }
        else if (!m_fields->m_currentRun.m_cheated && !m_isPracticeMode && m_level->isPlatformer()) {
            m_fields->m_fzeroToSave.push_back(m_fields->m_currentRun.m_end);
            m_fields->m_fzeroEndTimestamps.push_back(m_fields->m_currentRun.m_endTimestamp.value());
        }
        else {
            m_fields->m_runsToSave.push_back(m_fields->m_currentRun);
        }

        refreshOverlayLabels();
    }
}

void DTPlayLayer::levelComplete() {
    PlayLayer::levelComplete();

    // same as PlayLayer::destroyPlayer()
    if (!m_fields->m_hasRespawned) return;
    m_fields->m_hasRespawned = false;

    // disable tracking deaths on completed levels
    if (DTPlayLayer::disableCompletedLevelTracking() || (Settings::getDisablePractice() && m_isPracticeMode)) return;

    if (!m_level->isPlatformer())
        m_fields->m_currentRun.m_end = 100;

    // log::info("PlayLayer::levelComplete()\ncurrentRun.start = {}\ncurrentRun.end = {}\nplatformer = {}",
    //     m_fields->currentRun.start,
    //     m_fields->currentRun.end,
    //     m_level->isPlatformer()
    // );

    m_fields->m_lastOneDied = true;

    if (m_fields->m_currentRun.m_start < 0)
        return;

    if (m_level->isPlatformer())
        m_fields->m_currentRun.m_end++;
        
    saveRun();

    m_fields->m_nolcip = false;
    m_fields->m_speedhack = false;
    m_fields->m_disabledCheat = nullptr;
}

void DTPlayLayer::removeAllCheckpoints() {
    PlayLayer::removeAllCheckpoints();

    m_fields->m_currentRun.m_end = 0;
}

void DTPlayLayer::resume() {
    // if remove pauses is on
    DTPlayLayer::cutoutPlaytime();
    PlayLayer::resume();
}

void DTPlayLayer::pauseGame(bool unfocused) {
    // if remove pauses
    if (!m_fields->m_levelBeaten)
        DTPlayLayer::cutoutPlaytime();

    m_fields->m_didJustPause = true;
    PlayLayer::pauseGame(unfocused);
    m_fields->m_didJustPause = false;

    if (m_fields->m_fzeroToSave.size()) {
        StatsManager::logDeaths(m_fields->m_fzeroToSave);
        m_fields->m_fzeroToSave.clear();
        m_fields->m_fzeroEndTimestamps.clear();
    }

    if (m_fields->m_runsToSave.size()) {
        StatsManager::logRuns(m_fields->m_runsToSave);
        m_fields->m_runsToSave.clear();
    }

    savePlaytime();
    updateOverlaySessionData();
    refreshOverlayLabels();

    auto lvlKey = StatsManager::getLevelKey(m_level);
    if (lvlKey.isOk() && Settings::getAutoBackupEnabled() && Settings::getAutoBackupAtLvlExit()) {
        auto metaRes = StatsManager::getMetadata(lvlKey.unwrap());
        if (metaRes.isOk() && metaRes.unwrap().m_autoBackup) {
            StatsManager::addBackup(
                lvlKey.unwrap(),
                Settings::getAutoBackupGeneral(),
                Settings::getAutoBackupSessionAmount()
            );
        }
    }

    // log::info("PlayLayer::onQuit()");

    // schedule session gets reset
    // this cancels creating a new session
    // if they back out before dying
    DTPlayLayer::updateSessionLastPlayed();
}

//tysm eclips menu ur awesome and mega goated :fire:
float DTPlayLayer::getActualProgress() {
    // float percent;
    // if (game->m_level->m_timestamp > 0) {
    //     percent = static_cast<float>(game->m_gameState.m_levelTime * 240.f) / game->m_level->m_timestamp * 100.f;
    // } else {
    //     percent = game->m_player1->getPositionX() / game->m_levelLength * 100.f;
    // }
    // log::info("my percent {} | gd percent {}", percent, );
    return std::clamp(getCurrentPercent(), 0.f, 100.f);
}

void DTPlayLayer::onExit() {
    PlayLayer::onExit();
    if (m_fields->m_didJustPause) return;

    if (m_fields->m_fzeroToSave.size()) {
        StatsManager::logDeaths(m_fields->m_fzeroToSave);
        m_fields->m_fzeroToSave.clear();
        m_fields->m_fzeroEndTimestamps.clear();
    }

    if (m_fields->m_runsToSave.size()) {
        StatsManager::logRuns(m_fields->m_runsToSave);
        m_fields->m_runsToSave.clear();
    }

    cutoutPlaytime();

    savePlaytime();

    auto lvlKey = StatsManager::getLevelKey(m_level);
    if (lvlKey.isOk() && Settings::getAutoBackupEnabled() && Settings::getAutoBackupAtLvlExit()) {
        auto metaRes = StatsManager::getMetadata(lvlKey.unwrap());
        if (metaRes.isOk() && metaRes.unwrap().m_autoBackup) {
            StatsManager::addBackup(
                lvlKey.unwrap(),
                Settings::getAutoBackupGeneral(),
                Settings::getAutoBackupSessionAmount()
            );
        }
    }


    // log::info("PlayLayer::onQuit()");

    // schedule session gets reset
    // this cancels creating a new session
    // if they back out before dying
    StatsManager::scheduleCreateNewSession(false);
    DTPlayLayer::updateSessionLastPlayed();

    StatsManager::setCurrentLevel(nullptr);
}

void DTPlayLayer::startGame() {
    PlayLayer::startGame();

    m_fields->m_hasRespawned = true;
    if (!m_fields->m_currentRun.m_start.has_value())
        m_fields->m_currentRun.m_start = 0.0;
    m_fields->m_currentRun.m_startTimestamp = static_cast<time_t>(StatsManager::getNowSeconds());
    m_fields->m_currentRun.m_endTimestamp.reset();
    m_fields->m_currentRun.m_cheated = false;

    m_fields->m_didGameStartThisFrame = true;

    startupPlaytime();
}

void DTPlayLayer::playEndAnimationToPos(CCPoint position) {
    PlayLayer::playEndAnimationToPos(position);

    cutoutPlaytime();

    m_fields->m_levelBeaten = true;
}

void DTPlayLayer::playPlatformerEndAnimationToPos(CCPoint position, bool instant) {
    PlayLayer::playPlatformerEndAnimationToPos(position, instant);

    cutoutPlaytime();

    m_fields->m_levelBeaten = true;
}

bool DTPlayLayer::isLegal() {
    if (Settings::getCheatDetect() && (m_fields->m_nolcip || m_ignoreDamage || m_fields->m_speedhack)) return false;

    return true;
}

void DTPlayLayer::postUpdate(float dt) {
    checkDelta(dt);
    PlayLayer::postUpdate(dt);
}

void DTPlayLayer::checkDelta(float delta) {
    if (m_ignoreDamage && !m_fields->m_nolcip) {
        log::warn("Noclip Detected!");
        m_fields->m_nolcip = true;
    }

    if (m_player1->m_isDead || m_isPaused) return;

    auto now = std::chrono::steady_clock::now();

    if (!m_fields->m_speedhackCompare.has_value()) {
        m_fields->m_speedhackCompare = now;
        return;
    }

    std::chrono::duration<double> realElapsed = now - m_fields->m_speedhackCompare.value();
    m_fields->m_speedhackCompare = now;

    auto gameDt = static_cast<double>(delta);
    auto realDt = realElapsed.count();

    if (realDt > 0.2) return;

    m_fields->m_rollingRealSum += realDt;
    m_fields->m_rollingGameSum += gameDt;
    m_fields->m_realTimeHistory.push_back(realDt);
    m_fields->m_gameTimeHistory.push_back(gameDt);

    size_t maxSamples = 120; 
    if (m_fields->m_realTimeHistory.size() > maxSamples) {
        m_fields->m_rollingRealSum -= m_fields->m_realTimeHistory.front();
        m_fields->m_rollingGameSum -= m_fields->m_gameTimeHistory.front();
        m_fields->m_realTimeHistory.pop_front();
        m_fields->m_gameTimeHistory.pop_front();
    }

    if (m_fields->m_realTimeHistory.size() >= 30 && m_fields->m_rollingGameSum != 0) {
        auto currentRatio = m_fields->m_rollingGameSum / m_fields->m_rollingRealSum;
        auto expectedRatio = m_fields->m_currentTimeWarp;

        if (std::abs(currentRatio - expectedRatio) > 0.05) {
            if (!m_fields->m_speedhack && !m_fields->m_didGameStartThisFrame) {
                log::warn("Speedhack Detected!");
                m_fields->m_speedhack = true;
            }
        }

        m_fields->m_didGameStartThisFrame = false;
    }
}

void DTPlayLayer::updateTimeWarp(float timeWarp) {
    PlayLayer::updateTimeWarp(timeWarp);
    if (m_fields->m_levelBeaten) return;

    m_fields->m_currentTimeWarp = timeWarp;

    m_fields->m_realTimeHistory.clear();
    m_fields->m_gameTimeHistory.clear();
    m_fields->m_rollingRealSum = 0;
    m_fields->m_rollingGameSum = 0;
    m_fields->m_speedhackCompare.reset();
}

void DTPlayLayer::savePlaytime() {
    auto session = StatsManager::getCurrentSession();
    if (session != nullptr) {
        session->m_data.m_playtimeGeneral += m_fields->m_timePassedGeneral;
        session->m_data.m_playtimeDead += m_fields->m_timePassedDead;
        session->m_data.m_playtimePaused += m_fields->m_timePassedPaused;
        session->recordTimestamp(StatsManager::getNowSeconds());
        (void)StatsManager::setSession(*session, m_level, session->m_sessionStartDate, false);
    }

    StatsManager::safeCheckCurrF0();

    if (StatsManager::currentFrom0.has_value()) {
        auto& val = StatsManager::currentFrom0.value();
        val.m_playtimeGeneral += m_fields->m_timePassedGeneral;
        val.m_playtimeDead += m_fields->m_timePassedDead;
        val.m_playtimePaused += m_fields->m_timePassedPaused;
        (void)StatsManager::setGeneral(val, m_level);
    }
    

    m_fields->m_timePassedGeneral.reset();
    m_fields->m_timePassedDead.reset();
    m_fields->m_timePassedPaused.reset();
}

void DTPlayLayer::preloadOverlayData() {
    m_fields->m_overlayDataLoaded = false;
    m_fields->m_overlayLevelKeys.clear();
    m_fields->m_overlayLinkedData = GeneralData{};
    m_fields->m_overlaySessionData = GeneralData{};

    auto levelKeyResult = StatsManager::getLevelKey(m_level);
    if (levelKeyResult.isErr()) {
        log::error("Failed to preload overlay label data: {}", levelKeyResult.unwrapErr());
        return;
    }

    m_fields->m_overlayLevelKey = levelKeyResult.unwrap();
    m_fields->m_overlayLevelKeys.insert(m_fields->m_overlayLevelKey);

    auto currentSession = StatsManager::getCurrentSession();
    m_fields->m_overlaySessionDate = currentSession ? currentSession->m_sessionStartDate : 0;

    if (currentSession && m_fields->m_overlaySessionDate) {
        auto sessionResult = StatsManager::getSession(m_fields->m_overlayLevelKey, m_fields->m_overlaySessionDate);
        if (sessionResult.isOk()) {
            m_fields->m_overlaySessionData = sessionResult.unwrap().m_data;
        }
        else if (sessionResult.unwrapErr().m_code == 1) {
            m_fields->m_overlaySessionData = currentSession->m_data;
        }
        else {
            log::error("Failed to preload current session for overlay labels: {}", sessionResult.unwrapErr().m_error);
        }
    }

    std::vector<std::string> levelsToVisit;
    auto metadataResult = StatsManager::getMetadata(m_fields->m_overlayLevelKey);
    if (metadataResult.isOk()) {
        auto linkedLevels = metadataResult.unwrap().m_linkedLevels;
        levelsToVisit.assign(linkedLevels.begin(), linkedLevels.end());
        m_fields->m_preloadedMetadata = metadataResult.unwrap();
    }
    else if (metadataResult.unwrapErr().m_code != 1) {
        log::error("Failed to preload linked overlay label data: {}", metadataResult.unwrapErr().m_error);
    }

    while (!levelsToVisit.empty()) {
        auto levelKey = std::move(levelsToVisit.back());
        levelsToVisit.pop_back();
        if (!m_fields->m_overlayLevelKeys.insert(levelKey).second) continue;

        auto levelDataResult = StatsManager::getLevelData(levelKey);
        if (levelDataResult.isErr()) {
            log::warn("Failed to preload linked level {} for overlay labels: {}", levelKey, levelDataResult.unwrapErr().m_error);
            continue;
        }

        auto levelData = levelDataResult.unwrap();
        if (levelData.m_from0.isOk()) {
            m_fields->m_overlayLinkedData += levelData.m_from0.unwrap();
        }
        else if (levelData.m_from0.unwrapErr().m_code != 1) {
            log::warn("Failed to preload general data for linked level {}: {}", levelKey, levelData.m_from0.unwrapErr().m_error);
        }
        levelsToVisit.insert(
            levelsToVisit.end(),
            levelData.m_metadata.m_linkedLevels.begin(),
            levelData.m_metadata.m_linkedLevels.end()
        );
    }

    m_fields->m_overlayDataLoaded = true;
}

void DTPlayLayer::updateOverlaySessionData() {
    if (!m_fields->m_overlayDataLoaded) return;

    auto currentSession = StatsManager::getCurrentSession();
    auto sessionDate = currentSession ? currentSession->m_sessionStartDate : 0;
    if (sessionDate != m_fields->m_overlaySessionDate) {
        preloadOverlayData();
        return;
    }

    if (!currentSession || !sessionDate) {
        m_fields->m_overlaySessionData = GeneralData{};
        return;
    }

    auto sessionResult = StatsManager::getSession(m_fields->m_overlayLevelKey, sessionDate);
    if (sessionResult.isOk()) {
        m_fields->m_overlaySessionData = sessionResult.unwrap().m_data;
    }
    else if (sessionResult.unwrapErr().m_code == 1) {
        m_fields->m_overlaySessionData = currentSession->m_data;
    }
    else {
        log::error("Failed to refresh current session for overlay labels: {}", sessionResult.unwrapErr().m_error);
    }
}

Result<GeneralData> DTPlayLayer::getOverlayDeathsData(char scope) {
    if (!m_fields->m_overlayDataLoaded) return Err("Overlay label data is not loaded");

    GeneralData currentLevelData{};
    if (StatsManager::currentFrom0.has_value())
        currentLevelData = StatsManager::currentFrom0.value();

    for (size_t i = 0; i < m_fields->m_fzeroToSave.size(); i++) {
        ComplexDeathOccurrence occurrence;
        occurrence.m_runEndTimestamp = m_fields->m_fzeroEndTimestamps[i];
        (void)currentLevelData.m_deaths.recordDeath(
            m_fields->m_fzeroToSave[i],
            std::nullopt,
            occurrence
        );
    }
    for (const auto& run : m_fields->m_runsToSave) {
        ComplexDeathOccurrence occurrence;
        occurrence.m_runEndTimestamp = run.m_endTimestamp.value_or(StatsManager::getNowSeconds());
        occurrence.m_cheated = run.m_cheated;
        (void)currentLevelData.m_runs.recordDeath(run.m_end, run.m_start, occurrence);
    }

    if (scope == 'g') {
        currentLevelData += m_fields->m_overlayLinkedData;
        return Ok(currentLevelData);
    }
    if (scope == 'd') {
        currentLevelData += m_fields->m_overlayLinkedData;
        auto now = StatsManager::getNowSeconds();
        auto earliest = now - 24 * 60 * 60;

        return Ok(GeneralData{
            .m_deaths = filterByEndTimestamp(currentLevelData.m_deaths, earliest, now),
            .m_runs = filterByEndTimestamp(currentLevelData.m_runs, earliest, now)
        });
    }

    GeneralData currentSessionData = m_fields->m_overlaySessionData;
    for (size_t i = 0; i < m_fields->m_fzeroToSave.size(); i++) {
        ComplexDeathOccurrence occurrence;
        occurrence.m_runEndTimestamp = m_fields->m_fzeroEndTimestamps[i];
        (void)currentSessionData.m_deaths.recordDeath(
            m_fields->m_fzeroToSave[i],
            std::nullopt,
            occurrence
        );
    }
    for (const auto& run : m_fields->m_runsToSave) {
        ComplexDeathOccurrence occurrence;
        occurrence.m_runEndTimestamp = run.m_endTimestamp.value_or(StatsManager::getNowSeconds());
        occurrence.m_cheated = run.m_cheated;
        (void)currentSessionData.m_runs.recordDeath(run.m_end, run.m_start, occurrence);
    }

    if (scope == 's')
        return Ok(currentSessionData);

    return Err("Unknown overlay label data scope");
}

void DTPlayLayer::refreshOverlayLabels() {
    for (auto label : m_fields->m_overlayLabels)
        label->removeMeAndCleanup();
    m_fields->m_overlayLabels.clear();

    if (!m_fields->m_overlayLabelsParent) {
        auto parent = CCNode::create();
        parent->setContentSize(CCDirector::sharedDirector()->getWinSize());
        parent->setAnchorPoint({0, 0});
        parent->ignoreAnchorPointForPosition(false);
        parent->setPosition({0, 0});
        parent->setID("death-tracker-overlay-labels");
        this->addChild(parent, 10000);
        m_fields->m_overlayLabelsParent = parent;
    }

    auto labelSettings = Save::getOverlayLabels();
    auto orderedLabels = labelSettings.m_labels;

    std::stable_sort(orderedLabels.begin(), orderedLabels.end(), [](const auto& a, const auto& b) {
        return a.m_order > b.m_order;
    });

    std::map<Anchor, float> anchorOffsets;

    for (const auto& label : orderedLabels) {
        if (!label.m_enabled || !label.isOverlayLabelVisibleOnLevel(m_level, m_fields->m_preloadedMetadata.has_value() ? m_fields->m_preloadedMetadata.value() : LevelMetadata{})) continue;

        auto overlayLabel = RichTextArea::create(
            LabelEditLayer::expandDeaths(label.m_text, [this](char scope) {
                return getOverlayDeathsData(scope);
            }),
            labelSettings.m_font,
            labelSettings.m_size
        );
        m_fields->m_overlayLabels.push_back(overlayLabel);

        auto isDoner = []() { return AuthManager::isDoner(); };
        overlayLabel->registerRichTextKey(RichTextKeys::makeGradientKey(isDoner));
        overlayLabel->registerRichTextKey(RichTextKeys::makeWaveKey(isDoner));
        overlayLabel->registerRichTextKey(RichTextKeys::makeCPlusKey());

        float widest = 0;
        for (const auto& line : overlayLabel->getLines()) {
            widest = std::max(widest, line->getScaledContentWidth());
        }
        overlayLabel->setWidth(widest);
        overlayLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);

        auto anchor = label.m_freeMode ? ccp(label.m_posX, label.m_posY) : LabelEditLayer::anchorPoint(label.m_anchor);
        auto direction = LabelEditLayer::anchorDirection(anchor);
        overlayLabel->setAnchorPoint(anchor);

        if (!label.m_freeMode) {
            auto& offset = anchorOffsets[label.m_anchor];
            auto stackDirection = LabelEditLayer::anchorDirection(LabelEditLayer::anchorPoint(label.m_anchor)).y;
            if (stackDirection == 0) stackDirection = -1;
            m_fields->m_overlayLabelsParent->addChildAtPosition(
                overlayLabel,
                label.m_anchor,
                ccp(0, stackDirection * offset) + labelSettings.m_offset * direction,
                1000
            );
            offset += overlayLabel->getScaledContentHeight();
        }
        else {
            m_fields->m_overlayLabelsParent->addChild(overlayLabel, 1000);
            overlayLabel->setPosition(
                m_fields->m_overlayLabelsParent->getContentSize() * anchor + labelSettings.m_offset * direction
            );
        }

        for (const auto& line : overlayLabel->getLines())
            line->setOpacity(labelSettings.m_opacity);
    }
}

ComplexDeathMap DTPlayLayer::filterByEndTimestamp(const ComplexDeathMap& source, long long earliestTimestamp, long long latestTimestamp) {
    ComplexDeathMap filtered;

    for (const auto& [key, death] : source.m_records) {
        auto matching = death;
        matching.m_deaths = 0;
        matching.m_occurrences.clear();

        for (const auto& occurrence : death.m_occurrences) {
            if (!occurrence.m_runEndTimestamp) continue;

            auto timestamp = occurrence.m_runEndTimestamp.value();
            if (timestamp < earliestTimestamp || timestamp > latestTimestamp) continue;
            matching.m_occurrences.push_back(occurrence);
        }

        matching.m_deaths = matching.m_occurrences.size();
        if (matching.m_deaths != 0)
            filtered.m_records.emplace(key, std::move(matching));
    }

    return filtered;
}