#include "CalculatorMode.hpp"

CalculatorMode* CalculatorMode::create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    auto ret = new CalculatorMode();
    if (ret && ret->init(size, std::move(onVariablesChanged), std::move(onCalculate))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool CalculatorMode::init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    if (!CCNode::init()) return false;

    this->onVariablesChanged = std::move(onVariablesChanged);
    this->onCalculate = std::move(onCalculate);

    this->setContentSize(size);

    return true;
}