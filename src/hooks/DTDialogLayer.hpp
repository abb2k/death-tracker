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
    };

    static DialogLayer* createWithTaggedSprites(CCArray* objects, int bg, std::vector<std::tuple<std::string, int, float>> icons);

    void setProgressCallback(Function<void(DialogObject*)> callback);


    bool init(DialogObject* object, CCArray* objects, int background);
    
    void addToMainScene();
    
    void displayDialogObject(DialogObject* dialogue);

private:
    void useTaggedSprites(std::vector<std::tuple<std::string, int, float>> icons);
};