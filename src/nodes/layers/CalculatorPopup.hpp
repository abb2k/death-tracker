#pragma once

#include <Geode/Geode.hpp>
#include <nodes/SwitcherButton.hpp>

#include <nodes/CalculatorModes/CalculatorMode.hpp>

using namespace geode::prelude;

class CalculatorPopup : public Popup {
    protected:
        bool init() override;

    public:
        static CalculatorPopup* create();

    private:

        std::vector<CalculatorMode*> modes;

        SwitcherButton* modeButton;

        int currentMode;

        void onModeInfo(CCObject*);

        void varsChanged(std::string const& vars);
        void onCalc(std::string const& calculation);

        void addMode(CalculatorMode* mode);

        std::string currentVars;
        std::optional<std::string> currentCalc = std::nullopt;

        void updateText();
        TextInput* calcBG;

        void onCalcBtn(CCObject*);
        CCMenuItemSpriteExtra* calcBtn;
};