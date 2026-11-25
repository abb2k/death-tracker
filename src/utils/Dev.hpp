#pragma once

using namespace geode::prelude;

class Dev {
public:
    static constexpr bool m_enabled = false;
    static constexpr bool m_minifySaveFile = !m_enabled;

    static void fadeTextInput(TextInput* input, bool fadeIn, float time, bool changeEnabled = true);
    static void fadeSlider(Slider* slider, bool fadeIn, float time, bool changeEnabled = true);
};