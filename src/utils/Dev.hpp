#pragma once

using namespace geode::prelude;

class Dev {
public:
    static const bool ENABLED;
    static const bool MINIFY_SAVE_FILE;

    static void fadeTextInput(TextInput* const input, bool fadeIn, float time, bool changeEnabled = true);
    static void fadeSlider(Slider* const slider, bool fadeIn, float time, bool changeEnabled = true);
};