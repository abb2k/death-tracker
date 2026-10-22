#include <hooks/DTEditLevelLayer.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Settings.hpp>

bool DTEditLevelLayer::init(GJGameLevel* level) {
    if (!EditLevelLayer::init(level)) return false;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    s->setScale(0.8f);
    auto btn = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTEditLevelLayer::openDTLayer)
    );
    btn->setID("dt-skull-button");
    btn->setZOrder(1);

    auto IBMenu = getChildByID("info-button-menu");
    IBMenu->addChild(btn);
    btn->setPosition({IBMenu->getChildByID("info-button")->getPosition() + ccp(btn->getScaledContentSize().width, 0)});
    IBMenu->updateLayout();

    return true;
}

void DTEditLevelLayer::openDTLayer(CCObject*) {
    auto dtLayer = DTLayer::create(this->m_level);
    dtLayer->show();
}

