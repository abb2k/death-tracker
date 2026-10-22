#pragma once

#include <Geode/Geode.hpp>
#include "../../managers/StatsManager.hpp"
#include "../../utils/Save.hpp"
#include "AdvancedScrollLayer.hpp"
#include <nodes/SessionSelector.hpp>
#include <nodes/LayoutColumn.hpp>
#include <types/DTTypes.hpp>
#include <utils/SpecialKey.hpp>
#include <nodes/layers/LayoutOptionsLayer.hpp>
#include <nodes/TutorialButton.hpp>
#include <nodes/FloatingList.hpp>

using namespace geode::prelude;
template<class T>
using GetTFuture = arc::Future<Result<T, UpdateFutureError>>;

struct ColumnComperator {
    bool operator() (LayoutColumn* a, LayoutColumn* b) const;
};

struct organizationResult{
    std::vector<std::tuple<DTLabel*, CCPoint, float>> labelData{};
    float highestColumn;
};

using organizationFuture = arc::Future<organizationResult>;

class DTLayer : public Popup, public FLAlertLayerProtocol {
    protected:
        bool init(GJGameLevel* const& Level);

    public:
        static DTLayer* create(GJGameLevel* const& Level);

        void UpdateSharedStats();

        GJGameLevel* m_Level;

        Result<LevelData, ErrorWithCode> m_MyLevelStats = Err("");
        std::vector<LevelData> linkedLevelsData{};
        SessionCategory sessionsOrder{};
        SessionCategory daySGroup{};
        SessionCategory weekSGroup{};
        SessionCategory monthSGroup{};
        int currentGrouping = -4;
        SessionCategory& getCurrentGrouping();


        void show() override;

        virtual void keyDown(enumKeyCodes key, double d)  override;
        virtual void keyUp(enumKeyCodes key, double d)  override;

        static DTLayer* get();

        AdvancedScrollLayer* getScrollLayer();
        
        virtual void onClose(CCObject*) override;
        virtual void keyBackClicked() override;

        void UpdateDeathRelatedStrings();
        void updateStaticGroupings();

        std::map<std::string, std::shared_ptr<SpecialKey>> specialStrings{};
        void addSpecialString(const std::shared_ptr<SpecialKey>& key);
        void populateSpecialStrings();

        void specialKeyUpdateStarted(const std::shared_ptr<SpecialKey>& key);
        void specialKeyUpdateCompleted(const std::shared_ptr<SpecialKey>& key);

        void organizeLayout();

        bool doMoveScroll = true;

        std::pair<LayoutColumn*, int> getColumnLayerFromPosition(CCPoint posInWorldSpace);

        //points should be in world space!
        std::set<LayoutColumn*, ColumnComperator> getColumnsBetween(CCPoint a, CCPoint b);

        DTLabel* createNewLabel(DTLabelInfo info = DTLabelInfo{});

        void subscribeToOrganizationEvent(CCNode* target, geode::Function<void(float)> callback);
        void unsubscribeToOrganizationEvent(CCNode* target);

        void fixUpColumnPositions();

        static float transitionTime;

        void setOptionsLayerTo(DTLabel* label);
        void setOptionsLayerTo(LayoutColumn* column);
        void closeOptionsLayer();

        void removeColumn(LayoutColumn* column);
        bool cornerOnNextOrganization = true;
        void saveCurrentLayout();
        DTLayoutV3 currentLayout();
        bool isEditingLayout = false;

        void subscribeKeyListener(DTLabel* label);
        void unsubscribeKeyListener(DTLabel* label);

        int getCurrentSelectedSession();
        void onSessionSelected(int sessionNum, bool updateContent);

        void modifyRun(int startPer, int amount, std::optional<int> sessionNumber);
        void modifyRun(int startPer, int endPer, int amount, std::optional<int> sessionNumber);
        void modifyNewBest(int percent, bool makeTrue, std::optional<int> sessionNumber);

        bool DeleteSave();

        SessionSelector* sessionSelector = nullptr;

        Result<Session, UpdateFutureError> loadSessionFromSave(std::optional<int> sessionIndex = std::nullopt);

        void foreachLinkedLevel(geode::Function<void(LevelData&)> onLevelVisit);

        bool createDeathsString(const Deaths& deaths, const LevelMetadeta& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests = std::nullopt, const CustomizationData& newBestCustomizations = { .color = {255, 255, 255, 255} }, bool ignoreExtraSettings = false);

        void CleanGetStats();

        template<class T>
        GetTFuture<T> getTFor(geode::Function<T(GeneralData const&)> dataGetter, geode::Function<T(T const&, T const&)> combineFunc, bool session, bool localOnly = false) {
            if (!session) {
                if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate playtime");
                auto myStats = m_MyLevelStats.unwrap();
                if (myStats.from0.isErr()) {
                    auto error = myStats.from0.unwrapErr();
                    if (error.code == 1)
                        co_return Err(UpdateFutureError("No Deaths Saved!", false));
                    else
                        co_return Err(error.error);
                }
                auto myFrom0Stats = myStats.from0.unwrap();
                auto linkedLevelsCopy = linkedLevelsData;

                T all = dataGetter(myFrom0Stats);

                if (!localOnly) {
                    for (const auto& levelData : linkedLevelsCopy)
                    {
                        co_await arc::yield();
                        if (levelData.from0.isErr() || levelData.levelKey == myStats.levelKey) continue;
                        auto levelFrom0Stats = levelData.from0.unwrap();
                        all = combineFunc(all, dataGetter(levelFrom0Stats));
                    }
                }

                co_return Ok(all);
            }
            else {
                auto sessionRes = loadSessionFromSave();
                if (sessionRes.isErr()) co_return Err(sessionRes.unwrapErr());
                auto session = sessionRes.unwrap();

                co_await arc::yield();

                co_return Ok(dataGetter(session.data));
            }
        }

    private:
        
        std::mutex dataMutex;
        LayoutOptionsLayer* layoutOptionsLayer = nullptr;
        organizationFuture organizeLayoutTask();
        async::TaskHolder<organizationFuture::Output> organizationListener;

        bool canOrganize = true;
        int firstTime = 0;
        LoadingCircle* lc;
        
        std::set<LayoutColumn*, ColumnComperator> columns;

        std::set<DTLabel*> keyListeners{};

        bool createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests = std::nullopt, const CustomizationData& newBestCustomizations = { .color = {255, 255, 255, 255} }, bool ignoreExtraSettings = false);

        int currentSession;

        void update(float dt) override;

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchesMoved(CCSet* touches, CCEvent* event) override;

        static DTLayer* instance;

        AdvancedScrollLayer* scrollLayer = nullptr;
        CCMenu* columnHolder;
        CCSize ogLimits;

        CCNode* labelsHolder;

        void onLSOClicked(CCObject*);

        void onEditLayout(CCObject*);

        void graphBtnClicked(CCObject*);

        void onSettings(CCObject*);

        //better info time calc :)
        uint64_t timeInMs();
        std::string decodeBase64Gzip(const std::string& input);
        float timeForLevelString(const std::string& levelString);
        int stoi(std::string_view str);
        float stof(std::string_view str);

        LayoutColumn* addColumn(std::optional<DTColumnInfo> info = std::nullopt);
        void addColumnBtnClicked(CCObject*);

        std::map<CCNode*, geode::Function<void(float)>> onOrganizationCompleteEvent{};

        void setLayoutBy(const DTLayoutV3& layout);

        CCMenuItemSpriteExtra* addColumnButton;
        CCSprite* editLayoutBtnSpr;

        CCMenu* bottomLeftMenu;
        CCMenu* bottomRightMenu;
        CCMenu* editLayoutMenu;
        TutorialButton* layoutInfo;
        ButtonSprite* discardChangesButtonSpr;
        ButtonSprite* applyChangesButtonSpr;
        CCScale9Sprite* colorChangeBG;
        CCMenu* colorMenu;
        void onApplyLayoutChanges(CCObject*);
        void onDiscardLayoutChanges(CCObject*);

        void exitLayoutEditing();

        UpdateFuture onNLKey(std::map<std::string, std::any> payload);
        UpdateFuture onATTKey(std::map<std::string, std::any> payload);
        UpdateFuture onLVLNKey(std::map<std::string, std::any> payload);
        UpdateFuture onGeneralKey(std::map<std::string, std::any> payload);
        UpdateFuture onRUNSKey(std::map<std::string, std::any> payload);
        UpdateFuture onS0Key(std::map<std::string, std::any> payload);
        UpdateFuture onSRUNSKey(std::map<std::string, std::any> payload);

        UpdateFuture onDTATTKey(std::map<std::string, std::any> payload);
        UpdateFuture onDTF0ATTKey(std::map<std::string, std::any> payload);
        UpdateFuture onDTRunsATTKey(std::map<std::string, std::any> payload);

        UpdateFuture onSAttKey(std::map<std::string, std::any> payload);
        UpdateFuture onSF0AttKey(std::map<std::string, std::any> payload);
        UpdateFuture onSRunsAttKey(std::map<std::string, std::any> payload);
        
        long long calcPlaytime(const Deaths& deaths);
        
        UpdateFuture onAPTALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onAPTF0SKey(std::map<std::string, std::any> payload);
        UpdateFuture onAPTRUNSKey(std::map<std::string, std::any> payload);
        UpdateFuture onAPTSALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onAPTSF0Key(std::map<std::string, std::any> payload);
        UpdateFuture onAPTSRUNSKey(std::map<std::string, std::any> payload);

        //playtime
        UpdateFuture onPTALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onPTF0SKey(std::map<std::string, std::any> payload);
        UpdateFuture onPTRUNSKey(std::map<std::string, std::any> payload);
        UpdateFuture onPTSALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onPTSF0Key(std::map<std::string, std::any> payload);
        UpdateFuture onPTSRUNSKey(std::map<std::string, std::any> payload);

        //dead playtime
        UpdateFuture onDeadPTALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onDeadPTF0SKey(std::map<std::string, std::any> payload);
        UpdateFuture onDeadPTRUNSKey(std::map<std::string, std::any> payload);
        UpdateFuture onDeadPTSALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onDeadPTSF0Key(std::map<std::string, std::any> payload);
        UpdateFuture onDeadPTSRUNSKey(std::map<std::string, std::any> payload);
        
        //paused playtime
        UpdateFuture onPausedPTALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onPausedPTF0SKey(std::map<std::string, std::any> payload);
        UpdateFuture onPausedPTRUNSKey(std::map<std::string, std::any> payload);
        UpdateFuture onPausedPTSALLSKey(std::map<std::string, std::any> payload);
        UpdateFuture onPausedPTSF0Key(std::map<std::string, std::any> payload);
        UpdateFuture onPausedPTSRUNSKey(std::map<std::string, std::any> payload);

        UpdateFuture onRunsTo100Key(std::map<std::string, std::any> payload);
        UpdateFuture onBestRunsKey(std::map<std::string, std::any> payload);

        UpdateFuture onSessionRunsTo100Key(std::map<std::string, std::any> payload);
        UpdateFuture onSessionBestRunsKey(std::map<std::string, std::any> payload);
        
        UpdateFuture onSectionKey(std::map<std::string, std::any> payload);

        UpdateFuture onSessionDateKey(std::map<std::string, std::any> payload);

        UpdateFuture onLevelRunsKey(std::map<std::string, std::any> payload);

        UpdateFuture getPlaytimeFor(geode::Function<uint64_t(GeneralData const&)>&& dataGetter, bool session);
        UpdateFuture getAttemptsFor(geode::Function<unsigned long long(GeneralData const&, geode::FunctionRef<unsigned long long(const Deaths&)> const&)>&& dataGetter, bool session);

        std::optional<float> cachedLevelLength;

        void onCalculator(CCObject*);
        void onGroups(CCObject*);
        void onGroupSelected(int const& id);

        FloatingList* groupsList;
        CCMenuItemSpriteExtra* groupsBtn;
        CCMenu* groupsHolder;

        void onNewBestColor(CCObject*);
        void onSessionBestColor(CCObject*);

        CCMenu* resetLayoutMenu;
        void onResetLayout(CCObject*);

        void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

        CCSprite* newBestColorBtnSpr;
        CCSprite* sessionBestColorBtnSpr;

        ccColor3B originalNewBestColor = {255, 255, 255};
        ccColor3B originalSessionBestColor = {255, 255, 255};


        void onLayoutPresets(CCObject*);
        void onAddPreset(CCObject*);
        void onImportPreset(CCObject*);
        void onExportPreset(CCObject*);

        void savePreset(DTLayoutPreset preset);

        FloatingList* presetList;
        std::map<int, DTLayoutPreset> presets{};

        void deletePreset(int id);

        void resetLayoutTo(DTLayoutPreset const& preset);

        static CCLabelBMFont* verText;
        static CCParticleSystemQuad* verStarsParticle;
        static CCParticleSystemQuad* verSmokeParticle;

        comm::ListenerHandle donerListenerHandle;

        ListenerResult onDonerStateChanged(bool state);
};