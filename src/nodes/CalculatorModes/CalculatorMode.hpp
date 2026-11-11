#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;


class CalculatorMode : public CCNode {
    protected:
        bool init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate);

        geode::Function<void(std::string const&)> onVariablesChanged;
        geode::Function<void(std::string const&)> onCalculate;
    public:
        static CalculatorMode* create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate);

        virtual std::string getName() const {return "";}
        virtual std::string getDescription() const {return "";}

        virtual void calculate() {}
        virtual void open() {}
        virtual void close() {}
};