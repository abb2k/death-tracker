#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ScrollableTextInput : public CCNode {
    protected:
        bool init(float width, const std::string& placeholder, const std::string& font);
        
    public:
        static ScrollableTextInput* create(float width, const std::string& placeholder, const std::string& font = "bigFont.fnt");

        void setCommonFilter(CommonFilter filter);
        void setCallback(geode::Function<void(std::string const&)> onInput);
        void setString(std::string const& str, bool triggerCallback = false);
        std::string getString() const { return textInput->getString(); }
        bool isSelected() const { return textInput->isFocused(); }
        TextInput* getTextInput() const { return textInput; }
        void setEnabled(bool enabled);
        bool isEnabled() const { return textInput->isEnabled(); }

    private:
        TextInput* textInput = nullptr;
        CommonFilter filter = CommonFilter::Any;

        geode::Function<void(std::string const&)> onInput = NULL;
};