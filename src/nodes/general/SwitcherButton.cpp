#include "SwitcherButton.hpp"

SwitcherButton* SwitcherButton::create(CCSize size, std::string_view spr, std::vector<std::string> options) {
    auto ret = new SwitcherButton();
    auto ownedSprite = std::string(spr);
    auto bgSpr = CCScale9Sprite::create(ownedSprite.c_str());

    if (ret && options.size() && bgSpr && ret->init(std::move(size), bgSpr, std::move(options))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

SwitcherButton* SwitcherButton::createWithSpriteFrameName(CCSize size, std::string_view spr, std::vector<std::string> options) {
    auto ret = new SwitcherButton();
    auto ownedSprite = std::string(spr);
    auto bgSpr = CCScale9Sprite::createWithSpriteFrameName(ownedSprite.c_str());

    if (ret && options.size() && bgSpr && ret->init(std::move(size), bgSpr, std::move(options))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool SwitcherButton::init(CCSize size, CCScale9Sprite* bgSpr, std::vector<std::string> options) {
    m_mySpr = bgSpr;
    m_mySpr->setContentSize(size);
    m_mySpr->setCascadeOpacityEnabled(true);

    this->m_options = std::move(options);

    if (!CCMenuItemSpriteExtra::init(m_mySpr, nullptr, this, menu_selector(SwitcherButton::onClicked))) return false;

    m_textLabel = geode::Label::create("", "goldFont.fnt");
    m_textLabel->setPosition(m_mySpr->getContentSize() / 2 + ccp(0, 1.5f));
    m_textLabel->setCascadeOpacityEnabled(true);
    m_mySpr->addChild(m_textLabel);

    setTextForCurrentOption();
    
    return true;
}

void SwitcherButton::onClicked(CCObject*) {
    m_currentOption++;

    if (m_currentOption == m_options.size())
        m_currentOption = 0;

    setTextForCurrentOption();

    if (m_onOptionChanged != NULL)
        m_onOptionChanged(m_currentOption);
}

void SwitcherButton::setTextForCurrentOption() {
    m_textLabel->setString(m_options[m_currentOption].c_str());

    auto calculatedSize = ccp(m_mySpr->getContentWidth() - 5, m_mySpr->getContentHeight() - 5) / ccp(m_textLabel->getContentWidth(), m_textLabel->getContentHeight());

    m_textLabel->setScale(calculatedSize.x > calculatedSize.y ? calculatedSize.y : calculatedSize.x);
}

void SwitcherButton::setOptionChangedCallback(Function<void(int)> callback) {
    m_onOptionChanged = std::move(callback);
}