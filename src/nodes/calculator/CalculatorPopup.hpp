#pragma once

#include <nodes/calculator/CalculatorMode.hpp>

using namespace geode::prelude;

class SwitcherButton;

class CalculatorPopup : public Popup {
public:
    static CalculatorPopup* create();

private:
    bool init() override;

    std::vector<CalculatorMode*> m_modes;

    SwitcherButton* m_modeButton;

    int m_currentMode;

    void onModeInfo(CCObject*);

    void varsChanged(std::string const& vars);
    void onCalc(std::string const& calculation);

    void addMode(CalculatorMode* mode);

    std::string m_currentVars;
    std::optional<std::string> m_currentCalc = std::nullopt;

    void updateText();
    TextInput* m_calcBG;

    void onCalcBtn(CCObject*);
    CCMenuItemSpriteExtra* m_calcBtn;
};