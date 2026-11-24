#include "DTEndLevelLayer.hpp"

#include <nodes/DTLayer.hpp>
#include <utils/Settings.hpp>

void DTEndLevelLayer::customSetup() {
    EndLevelLayer::customSetup();

    auto robBtnMenu = m_mainLayer->getChildByID("button-menu");

    if (robBtnMenu) {
        auto robDTButton = robBtnMenu->getChildByID("death-tracker-button");

        if (robDTButton)
            robDTButton->setVisible(false);

        robBtnMenu->updateLayout();
    }

    if (!Settings::getCompleteMenuEnabled()) return;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    s->setCascadeOpacityEnabled(true);
    s2->setCascadeOpacityEnabled(true);
    m_fields->m_skullBtn = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTEndLevelLayer::openDTLayer)
    );
    m_fields->m_skullBtn->setID("dt-skull-button");

    auto hideLMenu = this->getChildByID("hide-layer-menu");
    hideLMenu->addChild(m_fields->m_skullBtn);

    hideLMenu->updateLayout();
}
void DTEndLevelLayer::openDTLayer(CCObject*) {
    auto dtLayer = DTLayer::create(this->m_playLayer->m_level);
    dtLayer->show();
}

void DTEndLevelLayer::onHideLayer(CCObject* sender) {
    EndLevelLayer::onHideLayer(sender);

    if (m_fields->m_skullBtn == nullptr) return;

    m_fields->m_skullBtn->stopAllActions();
    m_fields->m_skullBtn->runAction(CCFadeTo::create(.25f, m_hidden ? 0 : 255));
    m_fields->m_skullBtn->setEnabled(!m_hidden);
}