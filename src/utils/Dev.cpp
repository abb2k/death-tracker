#include "Dev.hpp"

void Dev::fadeTextInput(TextInput* input, bool fadeIn, float time, bool changeEnabled) {
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

void Dev::fadeSlider(Slider* slider, bool fadeIn, float time, bool changeEnabled) {
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

void Dev::setSliderEnabled(Slider* slider, bool enabled) {
    if (!slider || slider->m_enabled == enabled) return;

    slider->m_enabled = enabled;
    slider->m_touchLogic->setTouchEnabled(enabled);

    auto color = enabled ? cocos2d::ccColor3B{255, 255, 255} : cocos2d::ccColor3B{150, 150, 150};
    slider->m_sliderBar->setColor(color);
    slider->m_groove->setColor(color);

    if (auto normalImage = static_cast<cocos2d::CCSprite*>(slider->m_touchLogic->m_thumb->getNormalImage())) {
        normalImage->setColor(color);
    }
}