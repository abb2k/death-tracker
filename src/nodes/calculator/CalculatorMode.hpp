#pragma once

using namespace geode::prelude;

class CalculatorMode : public CCNode {
public:
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;

    virtual void calculate() = 0;
    virtual void open() = 0;
    virtual void close() = 0;
protected:
    bool init(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate);

    CCSize m_size;

    Function<void(std::string const&)> m_onVariablesChanged;
    Function<void(std::string const&)> m_onCalculate;
};