#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ChangelogPopup : public Popup {
    protected:
        bool init() override;

    public:
        static ChangelogPopup* create();

    private:

};