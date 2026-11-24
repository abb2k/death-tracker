#pragma once

using namespace geode::prelude;

#include <nodes/calculator/CalculatorMode.hpp>

class BeforeCalcMode : public CalculatorMode {
public:
    static BeforeCalcMode* create(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate);

    std::string getName() const override;
    std::string getDescription() const override;

    void sendVars();

    void calculate() override;
    void open() override;
    void close() override;
    
protected:
    bool init(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate);

    TextInput* m_beforePercentInput;

    bool m_isGeneral;
};