#include "Dev.hpp"

// TODO: set this to false before builing
const bool Dev::ENABLED = false;

/* global flags
================ */
const bool Dev::MINIFY_SAVE_FILE = Dev::ENABLED
    ? false
    : true;

void Dev::fadeTextInput(TextInput* const input, bool fadeIn, float time, bool changeEnabled) {
    if (changeEnabled)
        input->setEnabled(fadeIn);

    input->getBGSprite()->runAction(CCFadeTo::create(time, fadeIn ? 90 : 0));
    input->getContentLabel()->runAction(CCFadeTo::create(
        time,
        fadeIn ? (input->isEnabled() ? 255 : 150) : 0
    ));
    input->getPlaceholderLabel()->runAction(CCFadeTo::create(
        time,
        fadeIn ? 255 : 0
    ));
}

void Dev::fadeSlider(Slider* const slider, bool fadeIn, float time, bool changeEnabled) {
    slider->m_enabled = false;

    if (changeEnabled)
        slider->m_touchLogic->setTouchEnabled(fadeIn);

    auto createAction = [&fadeIn, &time]() -> CCAction* {return fadeIn ? static_cast<CCAction*>(CCFadeTo::create(time, 255)) : static_cast<CCAction*>(CCFadeTo::create(time, 0));};

    slider->m_sliderBar->runAction(createAction());

    slider->m_groove->runAction(createAction());

    if (auto normalImage = static_cast<cocos2d::CCSprite*>(slider->m_touchLogic->m_thumb->getNormalImage())) {
        normalImage->runAction(createAction());
    }
}