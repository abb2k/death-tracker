#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class TextInputPopup : public Popup {
    protected:
        bool init(const std::string& title, const std::string& placeholder, const std::string& bottomBtnText, const std::string& presetValue, Function<void(const std::string&)> callback);
        Function<void(const std::string&)> m_callback;

        void onOk(CCObject* sender);

        TextInput* input;

    public:
        static TextInputPopup* create(const std::string& title, const std::string& placeholder, const std::string& bottomBtnText, const std::string& presetValue, Function<void(const std::string&)> callback);
};