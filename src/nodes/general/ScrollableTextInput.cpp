#include "ScrollableTextInput.hpp"

ScrollableTextInput* ScrollableTextInput::create(float width, const std::string& placeholder, const std::string& font) {
    auto ret = new ScrollableTextInput();
    if (ret && ret->init(width, placeholder, font)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ScrollableTextInput::init(float width, const std::string& placeholder, const std::string& font) {
    if (!CCNode::init()) return false;

    this->setContentSize({width, 38});
    this->setAnchorPoint({.5f, .5f});

    textInput = TextInput::create(width, placeholder, font);
    if (!textInput) return false;
    textInput->setOverflowMode(TextInput::OverflowMode::Clip);
    textInput->setCommonFilter(filter);
    textInput->setCallback([this](std::string const& value) {
        if (onInput)
            onInput(value);
    });
    this->addChild(textInput);
    return true;
}

void ScrollableTextInput::setCommonFilter(CommonFilter filter) {
    this->filter = filter;
    textInput->setCommonFilter(filter);
}

void ScrollableTextInput::setCallback(geode::Function<void(std::string const&)> onInput) {
    this->onInput = std::move(onInput);
}

void ScrollableTextInput::setString(std::string const& str, bool triggerCallback) {
    textInput->setString(str);
    if (triggerCallback && onInput)
        onInput(str);
}

void ScrollableTextInput::setEnabled(bool enabled) {
    textInput->setEnabled(enabled);
}
