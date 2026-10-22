#include "../hooks/DTPlayLayer.hpp"
#include <ctime>
using namespace geode::prelude;

bool DTPlayLayer::disableCompletedLevelTracking() {
    return Settings::isCompletedLevelTrackingDisabled()
        && m_level->m_newNormalPercent2.value() == 100
        && m_level->m_levelType == GJLevelType::Saved;
}

void DTPlayLayer::updateSessionLastPlayed() {
    auto session = StatsManager::getCurrentSession();

    if (!session) return;

    if (session == m_fields->prevSession && m_fields->isSessionExpired) return;
    StatsManager::updateCurrentSessionLastPlayed();
}

/* hooks
========== */
void DTPlayLayer::onModify(auto& self) {
    (void)self.setHookPriority("PlayLayer::levelComplete", -9999);
    (void)self.setHookPriorityPre("PlayLayer::destroyPlayer", Priority::First);
}

bool DTPlayLayer::init(GJGameLevel* level, bool p1, bool p2) {
    if (!PlayLayer::init(level, p1, p2)) return false;

    auto metaRes = StatsManager::getMetadata(level);
    if (metaRes.isErr() && metaRes.unwrapErr().code == 1) {
        LevelMetadeta newMeta{};
        metaRes = Ok(newMeta);
    }
    else if (metaRes.isErr()) {
        geode::Notification::create(fmt::format("Failed to load DT level data! {}", metaRes.unwrapErr().error), NotificationIcon::Error)->show();
    }

    if (metaRes.isOk()) {
        auto metadata = metaRes.unwrap();
        metadata.levelName = level->m_levelName;
        metadata.attempts = level->m_attempts;
        metadata.difficulty = StatsManager::getDifficulty(level);

        if (!metadata.hasGottenDataFromPT) {
            metadata.hasGottenDataFromPT = StatsManager::transferPlaytimeFromPT(level);
        }

        (void)StatsManager::setMetadata(metadata, level);
        
        metaRes = Ok(metadata);
    }

    StatsManager::setCurrentLevel(level);

    m_fields->saveDeleteHandle = SaveDeletionEvent().listen([&]() {

        m_fields->fzeroToSave.clear();
        m_fields->runsToSave.clear();

        m_fields->timePassedGeneral = PlaytimePair{};
        m_fields->timePassedDead = PlaytimePair{};
        m_fields->timePassedPaused = PlaytimePair{};

        StatsManager::currentFrom0 = GeneralData{};
        if (StatsManager::currentSession.has_value()) {
            StatsManager::currentSession.value().data = GeneralData{};
        }

        StatsManager::scheduleCreateNewSession(true);
        
        return ListenerResult::Propagate;
    });

    // log::info("PlayLayer::init()");

    auto session = StatsManager::getCurrentSession();
    auto levelKey = StatsManager::getLevelKey(level);
    auto sessionLength = Settings::getMaxSessionLength();

    if (session == nullptr) {
        StatsManager::scheduleCreateNewSession(true);
        DTPlayLayer::updateSessionLastPlayed();
        //log::info("first session created");
        return true;
    }

    // schedule create a new session
    // based on the session length setting
    // -2 means default session (no previous session)
    if (session->lastPlayed != -2) {
        switch (sessionLength) {
            // reset session per level play
            case -2: {
                m_fields->isSessionExpired = true;
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

                    m_fields->isSessionExpired = true;
                    StatsManager::scheduleCreateNewSession(true);
                    break;
                }
        }
    }

    DTPlayLayer::updateSessionLastPlayed();

    return true;
}

#if defined(GEODE_IS_MACOS)
void DTPlayLayer::checkpointActivated(CheckpointGameObject* checkpt) {
    PlayLayer::checkpointActivated(checkpt);

    m_fields->currentRun.end++;
}
#endif

void DTPlayLayer::cutoutPlaytime() {
    if (!m_fields->startTime.has_value()) return;

    auto now = std::chrono::steady_clock::now();
    uint64_t timeSpent = std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_fields->startTime.value()).count();
    m_fields->startTime = now;

    if (Settings::getSafeMode()) return;
    
    if (m_fields->currentRun.start == 0) {
        if (m_isPaused || m_fields->levelBeaten) {
            m_fields->timePassedPaused.playtimeF0 += timeSpent;
            // log::info("added playtime from 0 paused");
        }
        else if (m_fields->lastOneDied) {
            m_fields->timePassedDead.playtimeF0 += timeSpent;
            // log::info("added playtime from 0 dead");
        }
        else {
            m_fields->timePassedGeneral.playtimeF0 += timeSpent;
            // log::info("added playtime from 0 general");
        }
    }
    else {
        if (m_isPaused || m_fields->levelBeaten) {
            m_fields->timePassedPaused.playtimeRuns += timeSpent;
            // log::info("added playtime runs paused");
        }
        else if (m_fields->lastOneDied) {
            m_fields->timePassedDead.playtimeRuns += timeSpent;
            // log::info("added playtime runs dead");
        }
        else {
            m_fields->timePassedGeneral.playtimeRuns += timeSpent;
            // log::info("added playtime runs general");
        }
    }


    //log::info("playing for {}", m_fields->timePassed / 1000000000.0);
}

void DTPlayLayer::discardPlaytime() {
    if (!m_fields->startTime.has_value()) return;
    
    m_fields->startTime = std::chrono::steady_clock::now();

    //log::info("discarded playtime");
}

void DTPlayLayer::startupPlaytime() {
    m_fields->startTime = std::chrono::steady_clock::now();

    //log::info("startup playtime");
}

void DTPlayLayer::resetLevel() {
    m_fields->realTimeHistory.clear();
    m_fields->gameTimeHistory.clear();
    m_fields->rollingRealSum = 0;
    m_fields->rollingGameSum = 0;
    m_fields->speedhackCompare = std::nullopt;

    cutoutPlaytime();

    if (m_fields->lastOneDied) {
        m_fields->lastOneDied = false;
    }
    else {
        if (!(DTPlayLayer::disableCompletedLevelTracking() || (Settings::getDisablePractice() && m_isPracticeMode)) && isGameplayActive()) {
            auto metaRes = StatsManager::getMetadata(m_level);

            if (metaRes.isOk() && metaRes.unwrap().resetAsDeath) {
                if (!m_level->isPlatformer())
                    m_fields->currentRun.end = getActualProgress(this);

                saveRun();
            }
        }
    }
    PlayLayer::resetLevel();
    //log::info("PlayLayer::resetLevel()");

    m_fields->levelBeaten = false;

    m_fields->hasRespawned = true;

    if (!m_level->isPlatformer())
        m_fields->currentRun.start = getActualProgress(this);
    else
        m_fields->currentRun.start = m_fields->currentRun.end;

    m_fields->nolcip = false;
    m_fields->speedhack = false;
    m_fields->disabledCheat = nullptr;
}

void DTPlayLayer::destroyPlayer(PlayerObject* player, GameObject* p1) {
    PlayLayer::destroyPlayer(player, p1);

    m_fields->realTimeHistory.clear();
    m_fields->gameTimeHistory.clear();
    m_fields->rollingRealSum = 0;
    m_fields->rollingGameSum = 0;
    m_fields->speedhackCompare = std::nullopt;

    if (!m_fields->disabledCheat)
        m_fields->disabledCheat = p1;
    if (!m_fields->nolcip && m_fields->disabledCheat != static_cast<cocos2d::CCObject*>(p1) && !player->m_isDead && !m_levelEndAnimationStarted) {
        log::warn("Noclip Detected!");
        m_fields->nolcip = true;
    }

    if (!player->m_isDead) return;
 
    if (!m_fields->hasRespawned) return;
    m_fields->hasRespawned = false;

    cutoutPlaytime();

    if (DTPlayLayer::disableCompletedLevelTracking() || (Settings::getDisablePractice() && m_isPracticeMode)) return;

    if (!m_level->isPlatformer())
        m_fields->currentRun.end = getActualProgress(this);

    m_fields->lastOneDied = true;

    if (m_fields->currentRun.start < 0)
        return;

    saveRun();
}

void DTPlayLayer::saveRun() {
    if (Settings::getSafeMode()) return;
    if (!isLegal()) return;

    if (Settings::getQuickSaveEnabled()) {
        if (m_fields->currentRun.start == 0 && !m_isPracticeMode) {
            StatsManager::logDeath(m_fields->currentRun.end);
        }
        else if (!m_isPracticeMode && m_level->isPlatformer()) {
            StatsManager::logDeath(m_fields->currentRun.end);
        }

        else if (m_level->isPlatformer() && m_isPracticeMode) {
            StatsManager::logRun(m_fields->currentRun);
        }
        else {
            StatsManager::logRun(m_fields->currentRun);
        }

        savePlaytime();
    }
    else {
        if (m_fields->currentRun.start == 0 && !m_isPracticeMode) {
            m_fields->fzeroToSave.push_back(m_fields->currentRun.end);
        }
        else if (!m_isPracticeMode && m_level->isPlatformer()) {
            m_fields->fzeroToSave.push_back(m_fields->currentRun.end);
        }

        else if (m_level->isPlatformer() && m_isPracticeMode) {
            m_fields->runsToSave.push_back(m_fields->currentRun);
        }
        else {
            m_fields->runsToSave.push_back(m_fields->currentRun);
        }
    }
}

void DTPlayLayer::levelComplete() {
    PlayLayer::levelComplete();

    // same as PlayLayer::destroyPlayer()
    if (!m_fields->hasRespawned) return;
    m_fields->hasRespawned = false;

    // disable tracking deaths on completed levels
    if (DTPlayLayer::disableCompletedLevelTracking() || (Settings::getDisablePractice() && m_isPracticeMode)) return;

    if (!m_level->isPlatformer())
        m_fields->currentRun.end = 100;

    // log::info("PlayLayer::levelComplete()\ncurrentRun.start = {}\ncurrentRun.end = {}\nplatformer = {}",
    //     m_fields->currentRun.start,
    //     m_fields->currentRun.end,
    //     m_level->isPlatformer()
    // );

    m_fields->lastOneDied = true;

    if (m_fields->currentRun.start < 0)
        return;

    if (m_level->isPlatformer())
        m_fields->currentRun.end++;
        
    saveRun();

    m_fields->nolcip = false;
    m_fields->speedhack = false;
    m_fields->disabledCheat = nullptr;
}

void DTPlayLayer::removeAllCheckpoints() {
    PlayLayer::removeAllCheckpoints();

    m_fields->currentRun.end = 0;
}

void DTPlayLayer::resume() {
    // if remove pauses is on
    DTPlayLayer::cutoutPlaytime();
    PlayLayer::resume();
}

void DTPlayLayer::pauseGame(bool unfocused) {
    // if remove pauses
    if (!m_fields->levelBeaten)
        DTPlayLayer::cutoutPlaytime();

    m_fields->didJustPause = true;
    PlayLayer::pauseGame(unfocused);
    m_fields->didJustPause = false;
}

//tysm eclips menu ur awesome and mega goated :fire:
float DTPlayLayer::getActualProgress(GJBaseGameLayer* game) {
    float percent;
    if (game->m_level->m_timestamp > 0) {
        percent = static_cast<float>(game->m_gameState.m_levelTime * 240.f) / game->m_level->m_timestamp * 100.f;
    } else {
        percent = game->m_player1->getPositionX() / game->m_levelLength * 100.f;
    }
    return std::clamp(percent, 0.f, 100.f);
}

void DTPlayLayer::onExit() {
    PlayLayer::onExit();
    if (m_fields->didJustPause) return;

    

    if (m_fields->fzeroToSave.size()) {
        StatsManager::logDeaths(m_fields->fzeroToSave);
    }

    if (m_fields->runsToSave.size()) {
        StatsManager::logRuns(m_fields->runsToSave);
    }

    cutoutPlaytime();

    savePlaytime();

    auto lvlKey = StatsManager::getLevelKey(m_level);
    if (lvlKey.isOk() && Settings::getAutoBackupEnabled() && Settings::getAutoBackupAtLvlExit()) {
        auto metaRes = StatsManager::getMetadata(lvlKey.unwrap());
        if (metaRes.isOk() && metaRes.unwrap().autoBackup) {
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

    m_fields->didGameStartThisFrame = true;

    startupPlaytime();
}

void DTPlayLayer::playEndAnimationToPos(cocos2d::CCPoint position) {
    PlayLayer::playEndAnimationToPos(position);

    cutoutPlaytime();

    m_fields->levelBeaten = true;
}

void DTPlayLayer::playPlatformerEndAnimationToPos(cocos2d::CCPoint position, bool instant) {
    PlayLayer::playPlatformerEndAnimationToPos(position, instant);

    cutoutPlaytime();

    m_fields->levelBeaten = true;
}

bool DTPlayLayer::isLegal() {
    if (Settings::getCheatDetect() && (m_fields->nolcip || m_isIgnoreDamageEnabled || m_ignoreDamage || m_fields->speedhack)) return false;

    return true;
}

void DTPlayLayer::postUpdate(float dt) {
    checkDelta(dt);
    PlayLayer::postUpdate(dt);
}

void DTPlayLayer::checkDelta(float delta) {
    if (m_player1->m_isDead || m_isPaused) return;

    auto now = std::chrono::steady_clock::now();

    if (!m_fields->speedhackCompare.has_value()) {
        m_fields->speedhackCompare = now;
        return;
    }

    std::chrono::duration<double> realElapsed = now - m_fields->speedhackCompare.value();
    m_fields->speedhackCompare = now;

    auto gameDt = static_cast<double>(delta);
    auto realDt = realElapsed.count();

    if (realDt > 0.2) return;

    m_fields->rollingRealSum += realDt;
    m_fields->rollingGameSum += gameDt;
    m_fields->realTimeHistory.push_back(realDt);
    m_fields->gameTimeHistory.push_back(gameDt);

    size_t maxSamples = 120; 
    if (m_fields->realTimeHistory.size() > maxSamples) {
        m_fields->rollingRealSum -= m_fields->realTimeHistory.front();
        m_fields->rollingGameSum -= m_fields->gameTimeHistory.front();
        m_fields->realTimeHistory.pop_front();
        m_fields->gameTimeHistory.pop_front();
    }

    if (m_fields->realTimeHistory.size() >= 30 && m_fields->rollingGameSum != 0) {
        auto currentRatio = m_fields->rollingGameSum / m_fields->rollingRealSum;
        auto expectedRatio = m_fields->currentTimeWarp;

        if (std::abs(currentRatio - expectedRatio) > 0.05) {
            if (!m_fields->speedhack && !m_fields->didGameStartThisFrame) {
                log::warn("Speedhack Detected!");
                m_fields->speedhack = true;
            }
        }

        m_fields->didGameStartThisFrame = false;
    }
}

void DTPlayLayer::updateTimeWarp(float timeWarp) {
    PlayLayer::updateTimeWarp(timeWarp);
    if (m_fields->levelBeaten) return;

    m_fields->currentTimeWarp = timeWarp;

    m_fields->realTimeHistory.clear();
    m_fields->gameTimeHistory.clear();
    m_fields->rollingRealSum = 0;
    m_fields->rollingGameSum = 0;
}

void DTPlayLayer::savePlaytime() {
    auto session = StatsManager::getCurrentSession();
    if (session != nullptr) {
        session->data.playtimeGeneral += m_fields->timePassedGeneral;
        session->data.playtimeDead += m_fields->timePassedDead;
        session->data.playtimePaused += m_fields->timePassedPaused;
        (void)StatsManager::setSession(*session, m_level, session->sessionStartDate, false);
    }

    StatsManager::safeCheckCurrF0();

    if (StatsManager::currentFrom0.has_value()) {
        auto& val = StatsManager::currentFrom0.value();
        val.playtimeGeneral += m_fields->timePassedGeneral;
        val.playtimeDead += m_fields->timePassedDead;
        val.playtimePaused += m_fields->timePassedPaused;
        (void)StatsManager::setGeneral(val, m_level);
    }
    

    m_fields->timePassedGeneral.reset();
    m_fields->timePassedDead.reset();
    m_fields->timePassedPaused.reset();
}