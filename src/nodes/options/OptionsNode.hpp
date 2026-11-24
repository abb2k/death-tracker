#pragma once

using namespace geode::prelude;

class DTLevelSpecificSettingsLayer;

class OptionsNode : public CCMenu {
public:
    virtual void onOpened() = 0;
    virtual void onClosed() = 0;

    void setSettingsLayer(DTLevelSpecificSettingsLayer* settingsLayer);

protected:
    bool initWithSetup(CCSize size);

    virtual bool setup() = 0;

    DTLevelSpecificSettingsLayer* m_settingsLayer;

    CCSize m_size;
};