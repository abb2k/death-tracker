#include "../hooks/DTColorSelectPopup.hpp"

void DTColorSelectPopup::setCallback(const SEL_CallFunc& _callback, CCObject* const& _target) {
    m_fields->callback = _callback;
    m_fields->target = _target;
}

void DTColorSelectPopup::colorValueChanged(cocos2d::ccColor3B newColor) {
    ColorSelectPopup::colorValueChanged(newColor);

    if (m_fields->target)
        this->runAction(CCCallFunc::create(m_fields->target, m_fields->callback));
}