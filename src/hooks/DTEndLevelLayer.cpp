#include <hooks/DTEndLevelLayer.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Settings.hpp>

void DTEndLevelLayer::customSetup() {
    EndLevelLayer::customSetup();

    if (!Settings::getCompleteMenuEnabled()) return;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    s->setCascadeOpacityEnabled(true);
    s2->setCascadeOpacityEnabled(true);
    m_fields->skullBtn = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTEndLevelLayer::openDTLayer)
    );
    m_fields->skullBtn->setID("dt-skull-button");

    auto hideLMenu = this->getChildByID("hide-layer-menu");
    hideLMenu->addChild(m_fields->skullBtn);

    hideLMenu->updateLayout();
}
void DTEndLevelLayer::openDTLayer(CCObject*) {
    auto dtLayer = DTLayer::create(this->m_playLayer->m_level);
    dtLayer->show();
}

void DTEndLevelLayer::onHideLayer(cocos2d::CCObject* sender) {
    EndLevelLayer::onHideLayer(sender);

    if (m_fields->skullBtn == nullptr) return;

    m_fields->skullBtn->stopAllActions();
    m_fields->skullBtn->runAction(CCFadeTo::create(.25f, m_hidden ? 0 : 255));
    m_fields->skullBtn->setEnabled(!m_hidden);
}