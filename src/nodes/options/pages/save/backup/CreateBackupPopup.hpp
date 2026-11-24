#pragma once

using namespace geode::prelude;

class CreateBackupPopup : public Popup {
public:
    static CreateBackupPopup* create();

    void setCallback(Function<void(bool, std::optional<int>)> callback);

private:
    bool init() override;

    Function<void(bool, std::optional<int>)> m_callback = NULL;

    std::optional<int> m_sessions = -1;
    bool m_general = true;

    void createBackupClicked(cocos2d::CCObject* sender);
};