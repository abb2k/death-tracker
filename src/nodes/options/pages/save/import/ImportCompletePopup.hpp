#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class SimpleToggler;

class ImportCompletePopup : public Popup, public FLAlertLayerProtocol {
public:
    static ImportCompletePopup* create(LevelData data, std::vector<Session> sessions);

private:
    bool init(LevelData data, std::vector<Session> sessions);
    
    LevelData m_data;
    std::vector<Session> m_sessions;

    void update(float dt);

    ScrollLayer* m_generalView;
    ScrollLayer* m_sessionView;

    void selectAllSessions(CCObject*);
    void deselectAllSessions(CCObject*);

    void onCombine(CCObject*);
    void onOverride(CCObject*);

    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2);

    FLAlertLayer* m_combineAlert;
    FLAlertLayer* m_overrideAlert;

    SimpleToggler* m_DoAddGeneralToggler;
};