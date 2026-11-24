#include "LevelLinkCell.hpp"

LevelLinkCell* LevelLinkCell::create(float width, const std::string& levelKey, const LevelMetadata& data) {
    auto ret = new LevelLinkCell();
    if (ret && ret->init(width, levelKey, data)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool LevelLinkCell::init(float width, const std::string& levelKey, const LevelMetadata& data) {
    if (!CCMenu::init()) return false;

    this->m_levelKey = levelKey;

    this->setContentSize({width, 40});

    auto bg = CCScale9Sprite::create("square01_001.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize(this->getContentSize() / bg->getScale());
    bg->setCascadeOpacityEnabled(true);
    this->addChild(bg);

    auto arrowBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    arrowBtnSpr->setScale(.6f);
    m_arrowBtn = CCMenuItemSpriteExtra::create(
        arrowBtnSpr,
        this,
        menu_selector(LevelLinkCell::onArrow)
    );
    m_arrowBtn->setID("arrow");
    this->addChild(m_arrowBtn);

    m_diffSpr = GJDifficultySprite::create(data.m_difficulty, GJDifficultyName::Long);
    m_diffSpr->setScale(.5f);
    m_diffSpr->setCascadeOpacityEnabled(true);
    this->addChild(m_diffSpr);

    m_title = geode::Label::create(data.m_levelName.c_str(), "bigFont.fnt");
    m_titleWidth = width - m_diffSpr->getScaledContentWidth() - m_arrowBtn->getContentWidth() - 20;
    m_title->setLimitLabelWidth(m_titleWidth, .4f);
    m_title->setCascadeOpacityEnabled(true);
    this->addChild(m_title);

    m_att = geode::Label::create(fmt::format("{} attempts | levelID: {}", data.m_attempts, levelKey).c_str(), "chatFont.fnt");
    m_attWidth = width - m_diffSpr->getScaledContentWidth() - m_arrowBtn->getContentWidth() - 20;
    m_att->setLimitLabelWidth(m_attWidth, .5f);
    m_att->setCascadeOpacityEnabled(true);
    this->addChild(m_att);

    setAlignment(false);

    return true;
}

void LevelLinkCell::setAlignment(bool left) {
    m_isAlignedLeft = left;

    if (left) {
        m_diffSpr->setPosition({m_diffSpr->getScaledContentWidth() / 2 + 5, this->getContentHeight() / 2});
        m_title->setPosition({this->getContentWidth() / 2 - m_titleWidth / 2, this->getContentHeight() / 1.6f});
        m_title->setAlignment(geode::Label::Alignment::Left);
        m_title->setAnchorPoint({0, 0.5f});
        m_att->setPosition({this->getContentWidth() / 2 - m_attWidth / 2, this->getContentHeight() / 3.5f});
        m_att->setAlignment(geode::Label::Alignment::Left);
        m_att->setAnchorPoint({0, 0.5f});
        m_arrowBtn->setPosition({this->getContentWidth() - m_arrowBtn->getScaledContentWidth() / 2 - 5, this->getContentHeight() / 2});
        m_arrowBtn->setRotation(180);
    }
    else {
        m_arrowBtn->setPosition({m_arrowBtn->getScaledContentWidth() / 2 + 5, this->getContentHeight() / 2});
        m_title->setPosition({this->getContentWidth() / 2 + m_titleWidth / 2, this->getContentHeight() / 1.6f});
        m_title->setAlignment(geode::Label::Alignment::Right);
        m_title->setAnchorPoint({1, 0.5f});
        m_att->setPosition({this->getContentWidth() / 2 + m_attWidth / 2, this->getContentHeight() / 3.5f});
        m_att->setAlignment(geode::Label::Alignment::Right);
        m_att->setAnchorPoint({1, 0.5f});
        m_diffSpr->setPosition({this->getContentWidth() - m_diffSpr->getScaledContentWidth() / 2 - 5, this->getContentHeight() / 2});
        m_arrowBtn->setRotation(0);
    }
}

void LevelLinkCell::setCallback(Function<void(const std::string& levelKey, bool isLeftAligned)> callback) {
    this->m_callback = std::move(callback);
}

void LevelLinkCell::onArrow(CCObject*) {
    if (m_callback == NULL) return;

    m_callback(m_levelKey, m_isAlignedLeft);
}

void LevelLinkCell::setEnabledAndFade(bool b) {
    this->setOpacity(b ? 255 : 120);
    this->setEnabled(b);

    m_enabledAndFade = b;
}