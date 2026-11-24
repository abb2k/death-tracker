#include "OptionsNode.hpp"

bool OptionsNode::initWithSetup(CCSize size) {
    if (!CCMenu::init()) return false;
    this->m_size = std::move(size);

    this->setContentSize(m_size);

    if (!this->setup()) return false;

    return true;
}

void OptionsNode::setSettingsLayer(DTLevelSpecificSettingsLayer* settingsLayer) {
    this->m_settingsLayer = settingsLayer;
}