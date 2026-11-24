#include "DTPauseLayer.hpp"

#include <nodes/DTLayer.hpp>
#include <utils/Settings.hpp>

#if defined(GEODE_IS_MACOS)
#include <hooks/DTPlayLayer.hpp>
#endif

void DTPauseLayer::customSetup() {
    PauseLayer::customSetup();

    auto sideMenu = m_layer->getChildByID("left-button-menu");

    if (!sideMenu) return;

    auto robDTButton = sideMenu->getChildByID("death-tracker-button");

    if (robDTButton) robDTButton->setVisible(false);

    if (!Settings::getPauseMenuEnabled()) return;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.575f);
    auto button = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTPauseLayer::onDTMiniMenuOpened)
    );

    button->setID("dt-skull-button");
    button->setZOrder(-1);
    sideMenu->addChild(button);
    sideMenu->updateLayout();
}

void DTPauseLayer::onDTMiniMenuOpened(CCObject*) {
    auto level = StatsManager::getCurrentLevel();
    
    if (!level) {
        Notification::create("Error! No level data found!", NotificationIcon::Error)->show();
        return;
    }

    auto dtLayer = DTLayer::create(level);
    dtLayer->show();
}

#if defined(GEODE_IS_MACOS)

void DTPauseLayer::onNormalMode(cocos2d::CCObject* sender) {
    PauseLayer::onNormalMode(sender);

    auto playLayer = GameManager::get()->getPlayLayer();
    if (!playLayer) return;

    auto* dtPlayLayer = static_cast<DTPlayLayer*>(playLayer);
    dtPlayLayer->m_fields->currentRun.start = 0.0;
    dtPlayLayer->m_fields->currentRun.end = 0.0;
    dtPlayLayer->m_fields->currentRun.m_startTimestamp = static_cast<time_t>(StatsManager::getNowSeconds());
    dtPlayLayer->m_fields->currentRun.m_endTimestamp.reset();
}

#endif