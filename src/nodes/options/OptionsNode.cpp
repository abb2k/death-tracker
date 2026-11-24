#include "OptionsNode.hpp"

bool OptionsNode::initWithSetup(const CCSize& size) {
    if (!CCMenu::init()) return false;
    this->m_size = size;

    this->setContentSize(size);

    if (!this->setup()) return false;

    return true;
}