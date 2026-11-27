#pragma once

#include <nodes/options/OptionsNode.hpp>

using namespace geode::prelude;

class ManHider;
class ManSeeker;

class ManOptions : public OptionsNode, public DialogDelegate {
public:
    static ManOptions* create(CCSize size);

    void dialogClosed(DialogLayer* layer) override;

    void onOpened() override;
    void onClosed() override;
    void keyDown(enumKeyCodes key, double timestamp) override;
    void keyUp(enumKeyCodes key, double timestamp) override;

    bool isSeekerHidden();
    bool isSeekerWatchingStamp();
    void setHitboxesVisible(bool visible);
    void setHidingHitbox(cocos2d::CCRect hitbox);
    void setStumpHitbox(cocos2d::CCRect hitbox);
    void runDialogueSequence(
        std::vector<std::string> texts,
        std::optional<std::pair<std::string, std::string>> options = std::nullopt,
        Function<void(bool)> onOptionSelected = nullptr
    );

private:
    bool setup() override;

    void onEnter() override;

    void runDialogueSequenceStep(
        std::vector<std::string> texts,
        std::size_t index,
        std::optional<std::pair<std::string, std::string>> options,
        Function<void(bool)> onOptionSelected
    );
    void finishDialogueSequence();
    void update(float dt) override;
    void updateSeekerConstraints();
    void updateHitboxDebug();

    CCRect convertRect(CCRect rect, CCNode* source, CCNode* target);
    CCRect getNodeBounds(CCNode* node, CCNode* target);
    void drawRectOutline(CCDrawNode* drawNode, CCRect rect, ccColor4F color);

    ManHider* m_hider = nullptr;

    ManSeeker* m_seeker = nullptr;

    CCNodeRGBA* m_world = nullptr;
    CCSprite* m_seekerShadow = nullptr;
    CCDrawNode* m_hitboxDebug = nullptr;
    cocos2d::CCRect m_hidingHitbox{82.5f, 101.25f, 40.f, 42.5f};
    cocos2d::CCRect m_stampDialogueHitbox{72.5f, 40.25f, 40.f, 32.5f};
    cocos2d::CCRect m_stumpHitbox;
    cocos2d::CCRect m_screenBounds;
    cocos2d::CCRect m_stumpBounds;
    bool m_hasCustomStumpHitbox = false;
    bool m_showHitboxes = false;
    std::size_t m_activeDialogueSequences = 0;
    DialogLayer* m_dialogueLayer = nullptr;
    std::optional<std::pair<std::string, std::string>> m_pendingDialogueOptions;
    Function<void(bool)> m_pendingDialogueOptionCallback = nullptr;

    void runEggDialogue();
    void showDefaultMessage();
    void runMoreDialogue();

    bool m_interactedThisOpening = false;
};