#include "SpecialKeyCell.hpp"

SpecialKeyCell* SpecialKeyCell::create(std::shared_ptr<SpecialKey> key, geode::Function<void(const std::string&)> onSelected) {
    auto ret = new SpecialKeyCell();
    if (ret && ret->init(key, std::move(onSelected))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SpecialKeyCell::init(std::shared_ptr<SpecialKey> key, geode::Function<void(const std::string&)> onSelected) {
    if (!CCMenu::init()) return false;

    this->setContentSize({135, 30});
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setID("background");
    bg->setScale(.5f);
    bg->setContentSize(this->getContentSize() / bg->getScale());
    bg->setAnchorPoint({0, 0});
    bg->setOpacity(150);
    this->addChild(bg);

    auto keyText = CCLabelBMFont::create(key->getKey().c_str(), "bigFont.fnt");
    keyText->setPosition({2, this->getContentHeight() / 2 + 3});
    keyText->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    keyText->setAnchorPoint({0, .45f});
    float wantedScale = 41 / keyText->getContentWidth();
    keyText->setScale(.45f > wantedScale ? wantedScale : .45f);
    this->addChild(keyText);

    auto descriptionText = SimpleTextArea::create(key->getDescription().c_str(), "chatFont.fnt", .45f);
    descriptionText->setPosition({this->getContentWidth() - 2, this->getContentHeight() / 2});
    descriptionText->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    descriptionText->setWidth(85);
    descriptionText->setAnchorPoint({1, .5f});
    descriptionText->setWrappingMode(WrappingMode::SPACE_WRAP);
    this->addChild(descriptionText);
    
    auto addBtnSpr = ButtonSprite::create("Add", "goldFont.fnt", "GJ_button_05.png");
    addBtnSpr->setScale(.45f);
    auto addBtn = CCMenuItemSpriteExtra::create(
        addBtnSpr,
        this,
        menu_selector(SpecialKeyCell::onAddClicked)
    );
    addBtn->setPosition({15, 2});
    this->addChild(addBtn);

    this->key = key;
    this->onSelected = std::move(onSelected);

    return true;
}

void SpecialKeyCell::onAddClicked(CCObject*) {
    if (onSelected != NULL)
        onSelected(key->getKey());
}