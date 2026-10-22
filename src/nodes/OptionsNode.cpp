#include <nodes/OptionsNode.hpp>

bool OptionsNode::initWithSetup(const CCSize& size) {
    if (!CCMenu::init()) return false;
    this->size = size;

    this->setContentSize(size);

    if (!this->setup()) return false;

    return true;
}