#pragma once

#include <Geode/modify/DialogLayer.hpp>

using namespace geode::prelude;

class $modify(DTDialogLayer, DialogLayer) {
public:
    struct Fields {
        std::vector<std::tuple<std::string, int, float>> m_icons;
        Function<void(DialogObject*)> m_callback = NULL;
        DialogObject* m_firstDialogue = nullptr;
        bool m_doCallCreationCallback = false;
        bool m_progressOnZ = false;
        bool m_hideCharacter = false;
        bool m_disableFastForward = false;
    };

    static DialogLayer* createWithTaggedSprites(CCArray* objects, int bg, std::vector<std::tuple<std::string, int, float>> icons);
    static void addStoredEggBadge(
        CCNode* dialogueMainLayer,
        CCScale9Sprite* dialogueBackground,
        bool isDeltaruneTextboxes = false
    );

    void setProgressCallback(Function<void(DialogObject*)> callback);


    bool init(DialogObject* object, CCArray* objects, int background);
    
    void addToMainScene();
    
    void displayDialogObject(DialogObject* dialogue);
    void handleDialogTap();
    void keyDown(enumKeyCodes key, double timestamp) override;
    void setProgressOnZ(bool enabled);
    void setCharacterHidden(bool hidden);
    void setFastForwardDisabled(bool disabled);

private:
    void useTaggedSprites(std::vector<std::tuple<std::string, int, float>> icons);
};