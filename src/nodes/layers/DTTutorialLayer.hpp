#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

enum TutorialBoxPlacement{
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
enum TutorialTextPlacement{
    TTTop,
    TTBottom,
    TTLeft,
    TTRight,
    TTCenter
};

enum TutorialCharacterFace{
    TCFNormal = -204,
    TCFNormalTilted = -205,
    TCFHappy = -206,
    TCFAngry = -207,
    TCFWorried = -208,
    TCFSleeping = -209,
    TCFBored = -210
};

struct HighlightText{
    std::string text;
    TutorialTextPlacement placement;
    float size;
};

struct TutorialSegment{
    geode::Function<void()> beforeCallback = NULL;
    geode::Function<void()> afterCallback = NULL;

    DialogObject* dialogue = nullptr;
    std::optional<TutorialBoxPlacement> alignment = std::nullopt;
    std::optional<float> boxScale = std::nullopt;

    CCNode* lastAddedHighlight;

    std::map<CCNode*, std::tuple<float, bool, bool>> targetObjects{};
    std::map<CCNode*, HighlightText> textForTargets{};
};

class DTTutorialLayer : public CCLayer, public DialogDelegate {
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
        DTTutorialLayer* joinCallback(geode::Function<void()> callback, bool beforeDialogue);

        DTTutorialLayer* insertHighlight(int dialogueIndex, CCNode* targetObject, float delayTime = 0, bool allowTouches = false, bool ignoreDisabled = true);
        DTTutorialLayer* insertJoinTextToHighlight(int dialogueIndex, const std::string& text, float size = 1, TutorialTextPlacement alignment = TutorialTextPlacement::TTTop);
        
        void show();

        ~DTTutorialLayer();
        
    private:
        bool init() override;
        
        std::vector<TutorialSegment> allSegments{};
        std::map<DialogObject*, int> dialogueSegmentIndexes{};
        CCLayerColor* shadow;

        CCRenderTexture* lightRT;

        void onProgress(DialogObject* dObject);

        DialogLayer* dialogueLayer;

        void dialogClosed(DialogLayer* layer) override;

        void close();

        bool firstDialogue = false;

        std::set<CCScale9Sprite*> retainedSprites{};

        void update(float dt) override;
        std::map<CCScale9Sprite*, CCNode*> prevHighlights{};
        std::map<CCScale9Sprite*, CCLabelBMFont*> textsForHighlights{};

        std::map<CCScale9Sprite*, CCNode*> touchAllowedNodes{};

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;

        void ccTouchesBegan(CCSet *pTouches, CCEvent *pEvent) override;
        void ccTouchesMoved(CCSet *pTouches, CCEvent *pEvent) override;
        void ccTouchesEnded(CCSet *pTouches, CCEvent *pEvent) override;
        void ccTouchesCancelled(CCSet *pTouches, CCEvent *pEvent) override;

        void keyBackClicked() override;

        CCTouchDelegate* touchedHighlight = nullptr;

        void keyDown(enumKeyCodes key, double) override;
        void keyUp(enumKeyCodes key, double) override;

        void rightKeyDown() override;

	    void rightKeyUp() override;

	    bool scrollWheel(float x, float y) override;

        DialogObject* prevDialogue = nullptr;
        CCMenuItemSpriteExtra* skipButton;

        void skipTutorial(CCObject*);

        bool isClosing;
};