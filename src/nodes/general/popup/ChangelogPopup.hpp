#pragma once

using namespace geode::prelude;

class ChangelogPopup : public Popup {
public:
    static ChangelogPopup* create();

private:
    bool init() override;
};