#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CreateBackupPopup : public Popup {
    protected:
        bool init() override;

    public:
        static CreateBackupPopup* create();

        void setCallback(Function<void(bool, std::optional<int>)> callback) {
            this->m_callback = std::move(callback);
        }

    private:
        Function<void(bool, std::optional<int>)> m_callback = NULL;

        std::optional<int> m_sessions = -1;
        bool m_general = true;

        void createBackupClicked(cocos2d::CCObject* sender);
};