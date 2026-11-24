#pragma once

using namespace geode::prelude;

class Dev {
public:
    static constexpr bool ENABLED = false;
    static constexpr bool MINIFY_SAVE_FILE = !ENABLED;

    static void fadeTextInput(TextInput* const input, bool fadeIn, float time, bool changeEnabled = true);
    static void fadeSlider(Slider* const slider, bool fadeIn, float time, bool changeEnabled = true);
};