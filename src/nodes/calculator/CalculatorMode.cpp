#include "CalculatorMode.hpp"

bool CalculatorMode::init(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate) {
    if (!CCNode::init()) return false;
    
    this->m_size = std::move(size);

    this->m_onVariablesChanged = std::move(onVariablesChanged);
    this->m_onCalculate = std::move(onCalculate);

    this->setContentSize(m_size);

    return true;
}