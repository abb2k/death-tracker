#pragma once

using namespace geode::prelude;

enum TutorialBoxPlacement {
    TBPTop,
    TBPBottom,
    TBPLeft,
    TBPRight,
    TBPCenter,
    TBPBottomRight,
    TBPTopRight,
    TBPTopLeft,
    TBPBottomLeft,
};
enum TutorialTextPlacement {
    TTTop,
    TTBottom,
    TTLeft,
    TTRight,
    TTCenter
};

enum TutorialCharacterFace {
    TCFNormal = -204,
    TCFNormalTilted = -205,
    TCFHappy = -206,
    TCFAngry = -207,
    TCFWorried = -208,
    TCFSleeping = -209,
    TCFBored = -210
};

struct HighlightText {
public:
    std::string m_text;
    TutorialTextPlacement m_placement;
    float m_size;
};

struct TutorialSegment {
public:
    Function<void()> m_beforeCallback = NULL;
    Function<void()> m_afterCallback = NULL;

    DialogObject* m_dialogue = nullptr;
    std::optional<TutorialBoxPlacement> m_alignment = std::nullopt;
    std::optional<float> m_boxScale = std::nullopt;

    CCNode* m_lastAddedHighlight;

    std::map<CCNode*, std::tuple<float, bool, bool>> m_targetObjects{};
    std::map<CCNode*, HighlightText> m_textForTargets{};
};

class DTTutorialLayer : public CCLayer, public DialogDelegate, public FLAlertLayerProtocol {
public:
    static DTTutorialLayer* create();

    DTTutorialLayer* appendDialogue(
        const std::string& text, 
        TutorialCharacterFace face, 
        const ccColor3B& textColor = {255,255,255}, 
        float textSize = 1
    );
    DTTutorialLayer* appendDialogue(DialogObject* dialogue);
    DTTutorialLayer* joinTransform(TutorialBoxPlacement alignment, float boxScale = 1);
    DTTutorialLayer* joinHighlight(CCNode* targetObject, float delayTime = 0, bool allowTouches = false, bool ignoreDisabled = true);
    DTTutorialLayer* joinPreviousHighlight();
    DTTutorialLayer* joinTextToHighlight(const std::string& text, float size = 1, TutorialTextPlacement alignment = TutorialTextPlacement::TTTop);
    DTTutorialLayer* joinCallback(Function<void()> callback, bool beforeDialogue);

    DTTutorialLayer* insertHighlight(int dialogueIndex, CCNode* targetObject, float delayTime = 0, bool allowTouches = false, bool ignoreDisabled = true);
    DTTutorialLayer* insertJoinTextToHighlight(int dialogueIndex, const std::string& text, float size = 1, TutorialTextPlacement alignment = TutorialTextPlacement::TTTop);
    
    void show();
    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

    ~DTTutorialLayer();
    
private:
    bool init() override;
    using TutorialPortrait = std::tuple<std::string, int, float>;
    static std::vector<TutorialPortrait> const& tutorialPortraits();
    
    std::vector<TutorialSegment> m_allSegments{};
    std::map<DialogObject*, int> m_dialogueSegmentIndexes{};
    CCLayerColor* m_shadow;

    CCRenderTexture* m_lightRT;

    void onProgress(DialogObject* dObject);
    void showDeltaruneDialogueStep(std::size_t index);
    void appendEggTutorialExtension();
    void setEggDialogueSkipKeybindDisabled(bool disabled);
    CCPoint dialogueMainLayerPositionForCenter(CCPoint center, float scale);
    void moveSkipMenuTo(CCNode* parent, int zOrder);

    DialogLayer* m_dialogueLayer = nullptr;
    FLAlertLayer* m_deltaruneAlert = nullptr;
    CCNode* m_dialogueMainLayer = nullptr;
    CCScale9Sprite* m_dialogueBackground = nullptr;
    std::size_t m_deltaruneDialogueIndex = 0;
    CCPoint m_dialogueCenter;
    float m_dialogueBoxScale = 1.f;
    bool m_hasDialoguePlacement = false;
    bool m_isEggTutorial = false;
    bool m_eggTutorialCompleted = false;
    std::optional<std::size_t> m_eggTutorialDialogueStartIndex;
    std::optional<std::vector<Keybind>> m_previousDeltaruneSkipKeybinds;

    void dialogClosed(DialogLayer* layer) override;

    void close();

    bool m_firstDialogue = false;

    std::set<CCScale9Sprite*> m_retainedSprites{};

    void update(float dt) override;
    std::map<CCScale9Sprite*, CCNode*> m_prevHighlights{};
    std::map<CCScale9Sprite*, geode::Label*> m_textsForHighlights{};

    std::map<CCScale9Sprite*, CCNode*> m_touchAllowedNodes{};

    bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
    void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
    void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
    void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;

    void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent) override;
    void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent) override;
    void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent) override;
    void ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent) override;

    void keyBackClicked() override;

    CCTouchDelegate* m_touchedHighlight = nullptr;

    void keyDown(enumKeyCodes key, double) override;
    void keyUp(enumKeyCodes key, double) override;

    void rightKeyDown() override;

    void rightKeyUp() override;

    bool scrollWheel(float x, float y) override;

    DialogObject* m_prevDialogue = nullptr;
    CCMenuItemSpriteExtra* m_skipButton;
    CCMenu* m_skipMenu = nullptr;

    void skipTutorial(CCObject*);

    bool m_isClosing = false;
};