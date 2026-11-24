#pragma once

using namespace geode::prelude;

class ImportPopup : public Popup {
public:
    static ImportPopup* create();

private:
    bool init() override;

    void onFromDT(CCObject*);
    void onFromText(CCObject*);

    async::TaskHolder<file::PickResult> m_fileListener;
};