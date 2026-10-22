#include "DTPauseLayer.hpp"

void DTPauseLayer::customSetup() {
    PauseLayer::customSetup();

    if (!Settings::getPauseMenuEnabled()) return;

    auto sideMenu = this->getChildByID("left-button-menu");

    if (!sideMenu) return;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    auto button = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTPauseLayer::onDTMiniMenuOpened)
    );

    button->setID("dt-skull-button");
    sideMenu->addChild(button);
    sideMenu->updateLayout();
}

void DTPauseLayer::onDTMiniMenuOpened(CCObject*) {
    auto level = StatsManager::getCurrentLevel();
    if (level == nullptr) {
        Notification::create("Error! No level data found!", NotificationIcon::Error)->show();
        return;
    }

    auto dtLayer = DTLayer::create(level);
    dtLayer->show();
}

#if defined(GEODE_IS_MACOS)

void DTPauseLayer::onNormalMode(cocos2d::CCObject* sender) {
    PauseLayer::onNormalMode(sender);

    static_cast<DTPlayLayer*>(GameManager::get()->getPlayLayer())->m_fields->currentRun.end = 0;

    //log::info("PauseLayer::onNormalMode()");
    auto playLayer = GameManager::get()->getPlayLayer();
    if (playLayer) {
        static_cast<DTPlayLayer*>(playLayer)->m_fields->currentRun.end = 0;
    }
}

#endif