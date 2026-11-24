#include "DTColorSelectPopup.hpp"

void DTColorSelectPopup::setCallback(SEL_CallFunc _callback, CCObject* _target) {
    m_fields->m_callback = std::move(_callback);
    m_fields->m_target = _target;
}

void DTColorSelectPopup::colorValueChanged(ccColor3B newColor) {
    ColorSelectPopup::colorValueChanged(newColor);

    if (m_fields->m_target)
        this->runAction(CCCallFunc::create(m_fields->m_target, m_fields->m_callback));
}