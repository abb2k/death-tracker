#pragma once

#include <types/DTTypes.hpp>
#include <nodes/layout/usage/SpecialKey.hpp>

using namespace geode::prelude;

class LayoutColumn;
class AdvancedScrollLayer;
class SpecialKey;
class DTLabel;
class SessionSelector;
class LayoutOptionsLayer;
class FloatingList;
class DecimalPopup;
class TutorialButton;

struct ColumnComperator {
    bool operator() (LayoutColumn* a, LayoutColumn* b) const;
};

struct organizationResult{
    std::vector<std::tuple<DTLabel*, CCPoint, float>> m_labelData{};
    float m_highestColumn;
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
        std::vector<LevelData> m_linkedLevelsData{};
        SessionCategory m_sessionsOrder{};
        SessionCategory m_daySGroup{};
        SessionCategory m_weekSGroup{};
        SessionCategory m_monthSGroup{};
        int m_currentGrouping = -4;
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

        std::map<std::string, std::shared_ptr<SpecialKey>> m_specialStrings{};
        void addSpecialString(const std::shared_ptr<SpecialKey>& key);
        void populateSpecialStrings();

        void specialKeyUpdateStarted(const std::shared_ptr<SpecialKey>& key);
        void specialKeyUpdateCompleted(const std::shared_ptr<SpecialKey>& key);

        void organizeLayout();

        bool m_doMoveScroll = true;

        std::pair<LayoutColumn*, int> getColumnLayerFromPosition(CCPoint posInWorldSpace);

        //points should be in world space!
        std::set<LayoutColumn*, ColumnComperator> getColumnsBetween(CCPoint a, CCPoint b);

        DTLabel* createNewLabel(DTLabelInfo info = DTLabelInfo{});

        void subscribeToOrganizationEvent(CCNode* target, Function<void(float)> callback);
        void unsubscribeToOrganizationEvent(CCNode* target);

        void fixUpColumnPositions();

        static float transitionTime;

        void setOptionsLayerTo(DTLabel* label);
        void setOptionsLayerTo(LayoutColumn* column);
        void closeOptionsLayer();

        void removeColumn(LayoutColumn* column);
        bool m_cornerOnNextOrganization = true;
        void saveCurrentLayout();
        DTLayoutV3 currentLayout();
        bool m_isEditingLayout = false;

        void subscribeKeyListener(DTLabel* label);
        void unsubscribeKeyListener(DTLabel* label);

        int getCurrentSelectedSession();
        void onSessionSelected(int sessionNum, bool updateContent);

        void modifyRun(double startPer, int amount, std::optional<int> sessionNumber);
        void modifyRun(double startPer, double endPer, int amount, std::optional<int> sessionNumber);
        void modifyNewBest(double percent, bool makeTrue, std::optional<int> sessionNumber);

        bool DeleteSave();

        SessionSelector* m_sessionSelector = nullptr;

        Result<Session, UpdateFutureError> loadSessionFromSave(std::optional<int> sessionIndex = std::nullopt);

        void foreachLinkedLevel(Function<void(LevelData&)> onLevelVisit);

        bool createDeathsString(const Deaths& deaths, const LevelMetadata& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests = std::nullopt, const CustomizationData& newBestCustomizations = { .m_color = {255, 255, 255, 255} }, bool ignoreExtraSettings = false, bool isSession = false);
        bool createDecimalDeathsString(const ComplexDeathMap& deaths, const Run& selectedRun, const LevelMetadata& meta, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests = std::nullopt, const CustomizationData& newBestCustomizations = { .m_color = {255, 255, 255, 255} }, bool ignoreExtraSettings = false, uint8_t decimalDepth = 0);

        void CleanGetStats();

        template<class T>
        GetTFuture<T> getTFor(Function<T(GeneralData const&)> dataGetter, Function<T(T const&, T const&)> combineFunc, bool session, bool localOnly = false) {
            if (!session) {
                if (m_MyLevelStats.isErr()) co_return Err("Failed to calculate playtime");
                auto myStats = m_MyLevelStats.unwrap();
                if (myStats.m_from0.isErr()) {
                    auto error = myStats.m_from0.unwrapErr();
                    if (error.m_code == 1)
                        co_return Err(UpdateFutureError("No Deaths Saved!", false));
                    else
                        co_return Err(error.m_error);
                }
                auto myFrom0Stats = myStats.m_from0.unwrap();
                auto linkedLevelsCopy = m_linkedLevelsData;

                T all = dataGetter(myFrom0Stats);

                if (!localOnly) {
                    for (const auto& levelData : linkedLevelsCopy)
                    {
                        co_await arc::yield();
                        if (levelData.m_from0.isErr() || levelData.m_levelKey == myStats.m_levelKey) continue;
                        auto levelFrom0Stats = levelData.m_from0.unwrap();
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

                co_return Ok(dataGetter(session.m_data));
            }
        }

        LayoutColumn* addColumn(std::optional<DTColumnInfo> info = std::nullopt);

    private:
        
        std::mutex m_dataMutex;
        LayoutOptionsLayer* m_layoutOptionsLayer = nullptr;
        organizationFuture organizeLayoutTask();
        async::TaskHolder<organizationFuture::Output> m_organizationListener;

        bool m_canOrganize = true;
        int m_firstTime = 0;
        LoadingCircle* m_lc;
        
        std::set<LayoutColumn*, ColumnComperator> m_columns;

        std::set<DTLabel*> m_keyListeners{};

        bool createDeathsString(const Deaths& deaths, const stringCustomazations& custom, std::string& out, std::optional<NewBests> const newBests = std::nullopt, const CustomizationData& newBestCustomizations = { .m_color = {255, 255, 255, 255} }, bool ignoreExtraSettings = false, bool isSession = false);

        int m_currentSession;

        void update(float dt) override;

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchesMoved(CCSet* touches, CCEvent* event) override;

        static DTLayer* instance;

        AdvancedScrollLayer* m_scrollLayer = nullptr;
        CCMenu* m_columnHolder;
        CCSize m_ogLimits;

        CCNode* m_labelsHolder;

        void onLSOClicked(CCObject*);

        void onEditLayout(CCObject*);

        void graphBtnClicked(CCObject*);

        void onSettings(CCObject*);
        void onLabels(CCObject*);

        //better info time calc :)
        uint64_t timeInMs();
        std::string decodeBase64Gzip(std::string_view input);
        float timeForLevelString(std::string_view levelString);
        int stoi(std::string_view str);
        float stof(std::string_view str);

        void addColumnBtnClicked(CCObject*);

        std::map<CCNode*, Function<void(float)>> m_onOrganizationCompleteEvent{};

        void setLayoutBy(const DTLayoutV3& layout);

        CCMenuItemSpriteExtra* m_addColumnButton;
        CCSprite* m_editLayoutBtnSpr;
        CCSprite* m_labelsBtnSpr;

        CCMenu* m_bottomLeftMenu;
        CCMenu* m_bottomRightMenu;
        CCMenu* m_editLayoutMenu;
        TutorialButton* m_layoutInfo;
        ButtonSprite* m_discardChangesButtonSpr;
        ButtonSprite* m_applyChangesButtonSpr;
        CCScale9Sprite* m_colorChangeBG;
        CCMenu* m_colorMenu;
        void onApplyLayoutChanges(CCObject*);
        void onDiscardLayoutChanges(CCObject*);

        void exitLayoutEditing();

        UpdateFuture onNLKey(std::map<std::string, std::any> payload);
        UpdateFuture onATTKey(std::map<std::string, std::any> payload);
        UpdateFuture onLVLNKey(std::map<std::string, std::any> payload);
        UpdateFuture onGeneralKey(std::map<std::string, std::any> payload);
        UpdateFuture onRUNSKey(std::map<std::string, std::any> payload);
        UpdateFuture onCheatedGeneralKey(std::map<std::string, std::any> payload);
        UpdateFuture onCheatedRunsKey(std::map<std::string, std::any> payload);
        UpdateFuture onCheatedAttemptsKey(std::map<std::string, std::any> payload);
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

        UpdateFuture onRunAttemptsKey(std::map<std::string, std::any> payload);

        UpdateFuture onDynamicDecimalKey(std::map<std::string, std::any> payload);

        UpdateFuture onRecentDeathsKey(std::map<std::string, std::any> payload);

        UpdateFuture getPlaytimeFor(Function<uint64_t(GeneralData const&)>&& dataGetter, bool session);
        UpdateFuture getAttemptsFor(Function<unsigned long long(GeneralData const&, geode::FunctionRef<unsigned long long(const Deaths&)> const&)>&& dataGetter, bool session);

        std::optional<float> m_cachedLevelLength;

        void onCalculator(CCObject*);
        void onGroups(CCObject*);
        void onGroupSelected(int const& id);

        FloatingList* m_groupsList;
        CCMenuItemSpriteExtra* m_groupsBtn;
        CCMenu* m_groupsHolder;

        void onNewBestColor(CCObject*);
        void onSessionBestColor(CCObject*);

        CCMenu* m_resetLayoutMenu;
        void onResetLayout(CCObject*);

        void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

        CCSprite* m_newBestColorBtnSpr;
        CCSprite* m_sessionBestColorBtnSpr;

        CustomizationData m_originalNewBestCustomization;
        CustomizationData m_originalSessionBestCustomization;


        void onLayoutPresets(CCObject*);
        void onAddPreset(CCObject*);
        void onImportPreset(CCObject*);
        void onExportPreset(CCObject*);

        void savePreset(DTLayoutPreset preset);

        FloatingList* m_presetList;
        std::map<int, DTLayoutPreset> m_presets{};

        void deletePreset(int id);

        void resetLayoutTo(DTLayoutPreset const& preset);

        static geode::Label* verText;
        static CCParticleSystemQuad* verStarsParticle;
        static CCParticleSystemQuad* verSmokeParticle;

        comm::ListenerHandle m_donerListenerHandle;

        ListenerResult onDonerStateChanged(bool state);

        DecimalPopup* m_decimalPopup;
};