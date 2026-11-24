#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class DTLevelSpecificSettingsLayer;

class OptionsNode : public CCMenu {
    protected:
        bool initWithSetup(const CCSize& size);

        virtual bool setup() = 0;

        DTLevelSpecificSettingsLayer* m_settingsLayer;
    public:
        virtual void onOpened() = 0;
        virtual void onClosed() = 0;

        void setSettingsLayer(DTLevelSpecificSettingsLayer* settingsLayer) {
            this->m_settingsLayer = settingsLayer;
        }
    protected:
        CCSize m_size;
};