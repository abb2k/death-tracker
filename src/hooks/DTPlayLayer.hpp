#pragma once

#include <Geode/modify/PlayLayer.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class $modify(DTPlayLayer, PlayLayer) {
public:
    struct Fields {
        Session* prevSession = nullptr;
        bool isSessionExpired = false;
        bool hasRespawned = false;
        Run currentRun;

        bool lastOneDied = false;
        
        bool nolcip = false;
        bool speedhack = false;
        CCObject* disabledCheat = nullptr;
        
        std::optional<std::chrono::steady_clock::time_point> speedhackCompare;
        std::deque<double> realTimeHistory;
        std::deque<double> gameTimeHistory;
        double rollingRealSum = 0;
        double rollingGameSum = 0;
        double currentTimeWarp = 1;
        

        bool didJustPause = false;
        PlaytimePair timePassedGeneral;
        PlaytimePair timePassedDead;
        PlaytimePair timePassedPaused;
        std::optional<std::chrono::steady_clock::time_point> startTime;

        std::vector<double> fzeroToSave{};
        std::vector<long long> fzeroEndTimestamps{};
        std::vector<Run> runsToSave{};
        
        bool levelBeaten = false;

        geode::comm::ListenerHandle gameExitListener;

        bool didGameStartThisFrame;

        ListenerHandle saveDeleteHandle;
        std::vector<CCNode*> overlayLabels;
        ListenerHandle overlayLabelsChangedListener;
        CCNode* overlayLabelsParent = nullptr;
        std::string overlayLevelKey;
        std::set<std::string> overlayLevelKeys;
        GeneralData overlayLinkedData;
        GeneralData overlaySessionData;
        std::optional<LevelMetadata> preloadedMetadata = std::nullopt;
        long long overlaySessionDate = 0;
        bool overlayDataLoaded = false;
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
