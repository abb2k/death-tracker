#include <hooks/DTLevelInfoLayer.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Settings.hpp>

bool DTLevelInfoLayer::init(GJGameLevel* p0, bool p1) {
    if (!LevelInfoLayer::init(p0, p1)) return false;

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    m_fields->btn = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTLevelInfoLayer::openDTLayer)
    );
    m_fields->btn->setID("dt-skull-button");
    m_fields->btn->setZOrder(1);
    m_fields->btn->setVisible(false);

    auto otherMenu = getChildByID("other-menu");
    auto settingsMenu = getChildByID("settings-menu");

    otherMenu->addChild(m_fields->btn);
    if (otherMenu->getChildByID("favorite-button")->isVisible())
        m_fields->btn->setPosition({otherMenu->getChildByID("favorite-button")->getPositionX(), settingsMenu->getChildByID("settings-button")->getPositionY()});
    else
        m_fields->btn->setPosition({otherMenu->getChildByID("favorite-button")->getPosition()});
    otherMenu->updateLayout();

    schedule(schedule_selector(DTLevelInfoLayer::checkIfPlayVisible));

    return true;
}

void DTLevelInfoLayer::checkIfPlayVisible(float delta) {
    if (this->m_playBtnMenu->isVisible()) {
        m_fields->btn->setVisible(true);
        
        unschedule(schedule_selector(DTLevelInfoLayer::checkIfPlayVisible));
    }
}

void DTLevelInfoLayer::openDTLayer(CCObject*) {
    auto dtLayer = DTLayer::create(this->m_level);
    dtLayer->show();
}