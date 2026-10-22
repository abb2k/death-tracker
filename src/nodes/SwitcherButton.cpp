#include "SwitcherButton.hpp"

SwitcherButton* SwitcherButton::create(CCSize size, const std::string& spr, const std::vector<std::string>& options) {
    auto ret = new SwitcherButton();
    auto bgSpr = CCScale9Sprite::create(spr.c_str());

    if (ret && options.size() && bgSpr && ret->init(size, bgSpr, options)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

SwitcherButton* SwitcherButton::createWithSpriteFrameName(CCSize size, const std::string& spr, const std::vector<std::string>& options) {
    auto ret = new SwitcherButton();
    auto bgSpr = CCScale9Sprite::createWithSpriteFrameName(spr.c_str());

    if (ret && options.size() && bgSpr && ret->init(size, bgSpr, options)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SwitcherButton::init(CCSize size, CCScale9Sprite* bgSpr, const std::vector<std::string>& options) {
    mySpr = bgSpr;
    mySpr->setContentSize(size);
    mySpr->setCascadeOpacityEnabled(true);

    this->options = options;

    if (!CCMenuItemSpriteExtra::init(mySpr, nullptr, this, menu_selector(SwitcherButton::onClicked))) return false;

    textLabel = CCLabelBMFont::create("", "goldFont.fnt");
    textLabel->setPosition(mySpr->getContentSize() / 2 + ccp(0, 1.5f));
    textLabel->setCascadeOpacityEnabled(true);
    mySpr->addChild(textLabel);

    setTextForCurrentOption();
    
    return true;
}

void SwitcherButton::onClicked(CCObject*) {
    currentOption++;

    if (currentOption == options.size())
        currentOption = 0;

    setTextForCurrentOption();

    if (onOptionChanged != NULL)
        onOptionChanged(currentOption);
}

void SwitcherButton::setTextForCurrentOption() {
    textLabel->setString(options[currentOption].c_str());

    auto calculatedSize = ccp(mySpr->getContentWidth() - 5, mySpr->getContentHeight() - 5) / ccp(textLabel->getContentWidth(), textLabel->getContentHeight());

    textLabel->setScale(calculatedSize.x > calculatedSize.y ? calculatedSize.y : calculatedSize.x);
}

void SwitcherButton::setOptionChangedCallback(geode::Function<void(int)> callback) {
    onOptionChanged = std::move(callback);
}