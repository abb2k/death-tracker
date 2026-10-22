#include "FontSelectionCell.hpp"

FontSelectionCell* FontSelectionCell::create(const std::string& font, geode::Function<void(FontSelectionCell* onSelected)> onFontChosen) {
    auto ret = new FontSelectionCell();
    if (ret && ret->init(font, std::move(onFontChosen))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool FontSelectionCell::init(const std::string& font, geode::Function<void(FontSelectionCell* onSelected)> onFontChosen) {
    if (!CCMenu::init()) return false;

    this->setContentSize({135, 60});
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setID("background");
    bg->setContentSize(this->getContentSize());
    bg->setAnchorPoint({0, 0});
    bg->setOpacity(150);
    this->addChild(bg);

    auto fontText = CCLabelBMFont::create(font.substr(0, font.length() - 4).c_str(), font.c_str());
    fontText->setPosition({this->getContentWidth() / 2, 40});
    fontText->setScale(.65f);
    this->addChild(fontText);
    
    auto useBtnSprUsed = ButtonSprite::create("Used", "goldFont.fnt", "GJ_button_05.png");
    useBtnSprUsed->setID("used");
    useBtnSprUsed->setVisible(false);
    useBtnSprUsed->setScale(.65f);
    auto useBtnSprUnused = ButtonSprite::create("Use", "bigFont.fnt", "GJ_button_03.png");
    useBtnSprUnused->setID("unused");
    useBtnSprUnused->setScale(.55f);
    useBtn = CCMenuItemSpriteExtra::create(
        useBtnSprUnused,
        this,
        menu_selector(FontSelectionCell::onUsedClicked)
    );
    useBtnSprUsed->setPosition(useBtnSprUnused->getPosition());
    useBtn->setPosition({this->getContentWidth() / 2, 17});
    useBtn->addChild(useBtnSprUsed);
    this->addChild(useBtn);

    this->font = font;
    this->onFontChosen = std::move(onFontChosen);

    return true;
}

void FontSelectionCell::deselect() {
    useBtn->setEnabled(true);
    useBtn->getChildByID("used")->setVisible(false);
    useBtn->getChildByID("unused")->setVisible(true);
}

void FontSelectionCell::select() {
    useBtn->setEnabled(false);
    useBtn->getChildByID("used")->setVisible(true);
    useBtn->getChildByID("unused")->setVisible(false);
}

void FontSelectionCell::onUsedClicked(CCObject*) {
    select();

    if (onFontChosen != NULL)
        onFontChosen(this);
}