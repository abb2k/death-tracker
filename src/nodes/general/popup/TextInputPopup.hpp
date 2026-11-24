#pragma once

using namespace geode::prelude;

class TextInputPopup : public Popup {
public:
    static TextInputPopup* create(std::string_view title, std::string_view placeholder, std::string_view bottomBtnText, std::string_view presetValue, Function<void(const std::string&)> callback);

private:
    bool init(std::string_view title, std::string_view placeholder, std::string_view bottomBtnText, std::string_view presetValue, Function<void(const std::string&)> callback);
    Function<void(const std::string&)> m_callback;

    void onOk(CCObject* sender);

    TextInput* m_input;
};