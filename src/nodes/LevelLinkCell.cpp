#include "LevelLinkCell.hpp"

LevelLinkCell* LevelLinkCell::create(float width, const std::string& levelKey, const LevelMetadeta& data) {
    auto ret = new LevelLinkCell();
    if (ret && ret->init(width, levelKey, data)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool LevelLinkCell::init(float width, const std::string& levelKey, const LevelMetadeta& data) {
    if (!CCMenu::init()) return false;

    this->levelKey = levelKey;

    this->setContentSize({width, 40});

    auto bg = CCScale9Sprite::create("square01_001.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize(this->getContentSize() / bg->getScale());
    bg->setCascadeOpacityEnabled(true);
    this->addChild(bg);

    auto arrowBtnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    arrowBtnSpr->setScale(.6f);
    arrowBtn = CCMenuItemSpriteExtra::create(
        arrowBtnSpr,
        this,
        menu_selector(LevelLinkCell::onArrow)
    );
    arrowBtn->setID("arrow");
    this->addChild(arrowBtn);

    diffSpr = GJDifficultySprite::create(data.difficulty, GJDifficultyName::Long);
    diffSpr->setScale(.5f);
    diffSpr->setCascadeOpacityEnabled(true);
    this->addChild(diffSpr);

    title = CCLabelBMFont::create(data.levelName.c_str(), "bigFont.fnt");
    title->setScale(.4f);
    titleWidth = width - diffSpr->getScaledContentWidth() - arrowBtn->getContentWidth() - 20;
    title->setWidth(titleWidth);
    title->setCascadeOpacityEnabled(true);
    this->addChild(title);

    att = CCLabelBMFont::create(fmt::format("{} attempts | levelID: {}", data.attempts, levelKey).c_str(), "chatFont.fnt");
    att->setScale(.5f);
    attWidth = width - diffSpr->getScaledContentWidth() - arrowBtn->getContentWidth() - 20;
    att->setWidth(attWidth);
    att->setCascadeOpacityEnabled(true);
    this->addChild(att);

    setAlignment(false);

    return true;
}

void LevelLinkCell::setAlignment(bool left) {
    isAlignedLeft = left;

    if (left) {
        diffSpr->setPosition({diffSpr->getScaledContentWidth() / 2 + 5, this->getContentHeight() / 2});
        title->setPosition({this->getContentWidth() / 2 - titleWidth / 2, this->getContentHeight() / 1.6f});
        title->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        title->setAnchorPoint({0, 0.5f});
        att->setPosition({this->getContentWidth() / 2 - attWidth / 2, this->getContentHeight() / 3.5f});
        att->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        att->setAnchorPoint({0, 0.5f});
        arrowBtn->setPosition({this->getContentWidth() - arrowBtn->getScaledContentWidth() / 2 - 5, this->getContentHeight() / 2});
        arrowBtn->setRotation(180);
    }
    else {
        arrowBtn->setPosition({arrowBtn->getScaledContentWidth() / 2 + 5, this->getContentHeight() / 2});
        title->setPosition({this->getContentWidth() / 2 + titleWidth / 2, this->getContentHeight() / 1.6f});
        title->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        title->setAnchorPoint({1, 0.5f});
        att->setPosition({this->getContentWidth() / 2 + attWidth / 2, this->getContentHeight() / 3.5f});
        att->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
        att->setAnchorPoint({1, 0.5f});
        diffSpr->setPosition({this->getContentWidth() - diffSpr->getScaledContentWidth() / 2 - 5, this->getContentHeight() / 2});
        arrowBtn->setRotation(0);
    }
}

void LevelLinkCell::setCallback(geode::Function<void(const std::string& levelKey, bool isLeftAligned)> callback) {
    this->callback = std::move(callback);
}

void LevelLinkCell::onArrow(CCObject*) {
    if (callback == NULL) return;

    callback(levelKey, isAlignedLeft);
}

void LevelLinkCell::setEnabledAndFade(bool b) {
    this->setOpacity(b ? 255 : 120);
    this->setEnabled(b);

    enabledAndFade = b;
}