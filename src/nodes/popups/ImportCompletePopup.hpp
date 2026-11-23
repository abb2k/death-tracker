#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/general/SimpleToggler.hpp>

using namespace geode::prelude;

class ImportCompletePopup : public Popup, public FLAlertLayerProtocol {
    protected:
        bool init(LevelData&& data, std::vector<Session> const& sessions);

    public:
        static ImportCompletePopup* create(LevelData&& data, std::vector<Session> const& sessions);

    private:
        LevelData data;
        std::vector<Session> sessions;

        void update(float dt);

        ScrollLayer* generalView;
        ScrollLayer* sessionView;

        void selectAllSessions(CCObject*);
        void deselectAllSessions(CCObject*);

        void onCombine(CCObject*);
        void onOverride(CCObject*);

        void FLAlert_Clicked(FLAlertLayer* layer, bool btn2);

        FLAlertLayer* combineAlert;
        FLAlertLayer* overrideAlert;

        SimpleToggler* DoAddGeneralToggler;

};