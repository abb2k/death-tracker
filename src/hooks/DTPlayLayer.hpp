#include <Geode/modify/PlayLayer.hpp>
#include <managers/StatsManager.hpp>
#include <utils/Save.hpp>
#include <utils/Settings.hpp>
#include <nodes/DTLabel.hpp>

using namespace geode::prelude;

class $modify(DTPlayLayer, PlayLayer) {
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
        std::vector<RichTextArea*> overlayLabels;
        ListenerHandle overlayLabelsChangedListener;
        CCNode* overlayLabelsParent = nullptr;
        std::string overlayLevelKey;
        std::set<std::string> overlayLevelKeys;
        GeneralData overlayLinkedData;
        GeneralData overlaySessionData;
        long long overlaySessionDate = 0;
        bool overlayDataLoaded = false;
    };

    void refreshOverlayLabels();
    void preloadOverlayData();
    void updateOverlaySessionData();
    Result<GeneralData> getOverlayDeathsData(char scope);

    bool disableCompletedLevelTracking();

    void updateSessionLastPlayed();

    /* hooks
    ========== */
    static void onModify(auto& self);

    bool init(GJGameLevel* level, bool p1);

    void cutoutPlaytime();

    void discardPlaytime();

    void startupPlaytime();

    void resetLevel();

    void destroyPlayer(PlayerObject* player, GameObject* p1);

    void levelComplete();

    void removeAllCheckpoints();

    #if defined(GEODE_IS_MACOS)

    void checkpointActivated(CheckpointGameObject* checkpt);
    
    #endif

    void pauseGame(bool unfocused);

    void resume();

    float getActualProgress();

    void saveRun();

    void onExit();

    void startGame();

    void playEndAnimationToPos(cocos2d::CCPoint position);

    void playPlatformerEndAnimationToPos(cocos2d::CCPoint position, bool instant);

    bool isLegal();

    void postUpdate(float dt);

    void checkDelta(float delta);

    void updateTimeWarp(float timeWarp);

    void savePlaytime();

    ComplexDeathMap filterByEndTimestamp(const ComplexDeathMap& source, long long earliestTimestamp, long long latestTimestamp);
};
