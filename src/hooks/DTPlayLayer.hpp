#pragma once

#include <Geode/modify/PlayLayer.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class $modify(DTPlayLayer, PlayLayer) {
public:
    struct Fields {
        Session* m_prevSession = nullptr;
        bool m_isSessionExpired = false;
        bool m_hasRespawned = false;
        Run m_currentRun;

        bool m_lastOneDied = false;
        
        bool m_nolcip = false;
        bool m_speedhack = false;
        CCObject* m_disabledCheat = nullptr;
        
        std::optional<std::chrono::steady_clock::time_point> m_speedhackCompare;
        std::deque<double> m_realTimeHistory;
        std::deque<double> m_gameTimeHistory;
        double m_rollingRealSum = 0;
        double m_rollingGameSum = 0;
        double m_currentTimeWarp = 1;
        

        bool m_didJustPause = false;
        PlaytimePair m_timePassedGeneral;
        PlaytimePair m_timePassedDead;
        PlaytimePair m_timePassedPaused;
        std::optional<std::chrono::steady_clock::time_point> m_startTime;

        std::vector<double> m_fzeroToSave{};
        std::vector<long long> m_fzeroEndTimestamps{};
        std::vector<Run> m_runsToSave{};
        
        bool m_levelBeaten = false;

        geode::comm::ListenerHandle m_gameExitListener;

        bool m_didGameStartThisFrame;

        ListenerHandle m_saveDeleteHandle;
        std::vector<CCNode*> m_overlayLabels;
        ListenerHandle m_overlayLabelsChangedListener;
        CCNode* m_overlayLabelsParent = nullptr;
        std::string m_overlayLevelKey;
        std::set<std::string> m_overlayLevelKeys;
        GeneralData m_overlayLinkedData;
        GeneralData m_overlaySessionData;
        std::optional<LevelMetadata> m_preloadedMetadata = std::nullopt;
        long long m_overlaySessionDate = 0;
        bool m_overlayDataLoaded = false;
    };


    static void onModify(auto& self) {
        (void)self.setHookPriority("PlayLayer::levelComplete", -9999);
        (void)self.setHookPriorityPre("PlayLayer::destroyPlayer", Priority::First);
    }

    bool init(GJGameLevel* level, bool p1);

    void resetLevel();

    void destroyPlayer(PlayerObject* player, GameObject* p1);

    void levelComplete();

    void removeAllCheckpoints();

#if defined(GEODE_IS_MACOS)
    void checkpointActivated(CheckpointGameObject* checkpt);
#endif

    void pauseGame(bool unfocused);

    void resume();

    void onExit();

    void startGame();

    void playEndAnimationToPos(CCPoint position);

    void playPlatformerEndAnimationToPos(CCPoint position, bool instant);

    void postUpdate(float dt);

    void updateTimeWarp(float timeWarp);

private:
    void refreshOverlayLabels();
    void preloadOverlayData();
    void updateOverlaySessionData();
    Result<GeneralData> getOverlayDeathsData(char scope);

    bool disableCompletedLevelTracking();
    void updateSessionLastPlayed();

    void cutoutPlaytime();
    void discardPlaytime();
    void startupPlaytime();
    void savePlaytime();

    float getActualProgress();
    void saveRun();

    bool isLegal();
    void checkDelta(float delta);

    ComplexDeathMap filterByEndTimestamp(const ComplexDeathMap& source, long long earliestTimestamp, long long latestTimestamp);
};
