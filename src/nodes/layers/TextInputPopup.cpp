#include "TextInputPopup.hpp"

TextInputPopup* TextInputPopup::create(const std::string& title, const std::string& placeholder, const std::string& bottomBtnText, const std::string& presetValue, geode::Function<void(const std::string&)> callback) {
    auto ret = new TextInputPopup();
    if (ret->init(title, placeholder, bottomBtnText, presetValue, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

// a little help from better edit on this one :)
bool TextInputPopup::init(const std::string& title, const std::string& placeholder, const std::string& bottomBtnText, const std::string& presetValue, geode::Function<void(const std::string&)> callback) {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(240, 110, "geode.loader/GE_square01.png"))
        return false;
    setTitle(title);

    this->m_callback = std::move(callback);
    
    input = geode::TextInput::create(m_size.width / 1.3f, placeholder);
    input->setString(presetValue);
    input->setPosition(m_size / 2);
    m_mainLayer->addChild(input);

    auto btnSpr = ButtonSprite::create(bottomBtnText.c_str());
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
        m_callback(input->getString());

    onClose(sender);
}