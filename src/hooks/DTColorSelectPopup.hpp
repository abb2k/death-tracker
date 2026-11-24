#pragma once

#include <Geode/modify/ColorSelectPopup.hpp>

using namespace geode::prelude;

class $modify(DTColorSelectPopup, ColorSelectPopup) {
public:
    struct Fields {
        SEL_CallFunc m_callback;
        CCObject* m_target = nullptr;
    };

    void setCallback(SEL_CallFunc _callback, CCObject* _target);


    void colorValueChanged(ccColor3B newColor);
};