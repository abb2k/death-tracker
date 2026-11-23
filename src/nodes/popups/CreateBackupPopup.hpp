#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CreateBackupPopup : public Popup {
    protected:
        bool init() override;

    public:
        static CreateBackupPopup* create();

        void setCallback(geode::Function<void(bool, std::optional<int>)> callback) {
            this->callback = std::move(callback);
        }

    private:
        geode::Function<void(bool, std::optional<int>)> callback = NULL;

        std::optional<int> sessions = -1;
        bool general = true;

        void createBackupClicked(cocos2d::CCObject* sender);
};