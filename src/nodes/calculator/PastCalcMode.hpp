#pragma once

using namespace geode::prelude;

#include <nodes/calculator/CalculatorMode.hpp>

class PastCalcMode : public CalculatorMode {
protected:
    bool init(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate);

    TextInput* m_passPercentInput;

    bool m_isGeneral = true;

public:
    static PastCalcMode* create(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate);
    
    std::string getName() const override;
    std::string getDescription() const override;

    void sendVars();

    void calculate() override;
    void open() override;
    void close() override;
};