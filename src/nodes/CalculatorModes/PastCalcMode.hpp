#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;
#include <nodes/CalculatorModes/CalculatorMode.hpp>

class PastCalcMode : public CalculatorMode {
    protected:
        bool init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate);

        TextInput* passPercentInput;

        bool isGeneral = true;

    public:
        static PastCalcMode* create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate);
        
        std::string getName() const override;
        std::string getDescription() const override;

        void sendVars();

        void calculate() override;
        void open() override;
        void close() override;
};