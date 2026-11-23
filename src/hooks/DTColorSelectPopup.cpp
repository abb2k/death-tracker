#include "DTColorSelectPopup.hpp"

void DTColorSelectPopup::setCallback(SEL_CallFunc _callback, CCObject* _target) {
    m_fields->callback = std::move(_callback);
    m_fields->target = _target;
}

void DTColorSelectPopup::colorValueChanged(ccColor3B newColor) {
    ColorSelectPopup::colorValueChanged(newColor);

    if (m_fields->target)
        this->runAction(CCCallFunc::create(m_fields->target, m_fields->callback));
}