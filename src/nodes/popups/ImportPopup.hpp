#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ImportPopup : public Popup {
    protected:
        bool init() override;

    public:
        static ImportPopup* create();

    private:
        void onFromDT(CCObject*);
        void onFromText(CCObject*);

        async::TaskHolder<file::PickResult> fileListener;
};