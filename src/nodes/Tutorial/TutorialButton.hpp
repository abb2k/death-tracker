#pragma once

using namespace geode::prelude;

class DTTutorialLayer;

class TutorialButton : public CCMenuItemSpriteExtra {
protected:
    bool init(float size, std::string_view id, Function<void(DTTutorialLayer*)> initilizeTutorial);
    
public:
    static TutorialButton* create(float size, std::string_view id, Function<void(DTTutorialLayer*)> initilizeTutorial);

private:

    Function<void(DTTutorialLayer*)> m_initilizeTutorial = NULL;

    void onClicked(CCObject*);

    std::string m_id;
    CCSprite* m_spr;
    float m_size;
};