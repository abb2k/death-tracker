#include "FontSelectionCell.hpp"

FontSelectionCell* FontSelectionCell::create(std::string font, Function<void(FontSelectionCell* onSelected)> onFontChosen) {
    auto ret = new FontSelectionCell();
    
    if (ret && ret->init(std::move(font), std::move(onFontChosen))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool FontSelectionCell::init(std::string font, Function<void(FontSelectionCell* onSelected)> onFontChosen) {
    if (!CCMenu::init()) return false;

    this->m_font = std::move(font);
    this->m_onFontChosen = std::move(onFontChosen);

    this->setContentSize({135, 60});
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setID("background");
    bg->setContentSize(this->getContentSize());
    bg->setAnchorPoint({0, 0});
    bg->setOpacity(150);
    this->addChild(bg);

    auto fontText = geode::Label::create(m_font.substr(0, m_font.length() - 4).c_str(), m_font.c_str());
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
    m_useBtn = CCMenuItemSpriteExtra::create(
        useBtnSprUnused,
        this,
        menu_selector(FontSelectionCell::onUsedClicked)
    );
    useBtnSprUsed->setPosition(useBtnSprUnused->getPosition());
    m_useBtn->setPosition({this->getContentWidth() / 2, 17});
    m_useBtn->addChild(useBtnSprUsed);
    this->addChild(m_useBtn);

    return true;
}

void FontSelectionCell::deselect() {
    m_useBtn->setEnabled(true);
    m_useBtn->getChildByID("used")->setVisible(false);
    m_useBtn->getChildByID("unused")->setVisible(true);
}

void FontSelectionCell::select() {
    m_useBtn->setEnabled(false);
    m_useBtn->getChildByID("used")->setVisible(true);
    m_useBtn->getChildByID("unused")->setVisible(false);
}

void FontSelectionCell::onUsedClicked(CCObject*) {
    select();

    if (m_onFontChosen != NULL)
        m_onFontChosen(this);
}