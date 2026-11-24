#include "TextInputPopup.hpp"

TextInputPopup* TextInputPopup::create(std::string_view title, std::string_view placeholder, std::string_view bottomBtnText, std::string_view presetValue, Function<void(const std::string&)> callback) {
    auto ret = new TextInputPopup();
    if (ret->init(title, placeholder, bottomBtnText, presetValue, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool TextInputPopup::init(std::string_view title, std::string_view placeholder, std::string_view bottomBtnText, std::string_view presetValue, Function<void(const std::string&)> callback) {
    if (!Popup::init(240, 110, "geode.loader/GE_square01.png"))
        return false;
    setTitle(std::string(title));

    this->m_callback = std::move(callback);
    
    m_input = geode::TextInput::create(m_size.width / 1.3f, std::string(placeholder));
    m_input->setString(std::string(presetValue));
    m_input->setPosition(m_size / 2);
    m_mainLayer->addChild(m_input);

    auto ownedButtonText = std::string(bottomBtnText);
    auto btnSpr = ButtonSprite::create(ownedButtonText.c_str());
    auto btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(TextInputPopup::onOk));
    btn->setPosition({
        m_size.width / 2,
        btn->getContentHeight() / 2 + 5
    });
    m_buttonMenu->addChild(btn);

    return true;
}

void TextInputPopup::onOk(CCObject* sender) {
    if (m_callback != NULL)
        m_callback(m_input->getString());

    onClose(sender);
}