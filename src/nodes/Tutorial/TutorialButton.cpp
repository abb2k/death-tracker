#include "TutorialButton.hpp"

#include <utils/Save.hpp>
#include <nodes/Tutorial/DTTutorialLayer.hpp>

TutorialButton* TutorialButton::create(float size, std::string_view id, Function<void(DTTutorialLayer*)> initilizeTutorial) {
    auto ret = new TutorialButton();
    
    if (ret && ret->init(std::move(size), id, std::move(initilizeTutorial))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool TutorialButton::init(float size, std::string_view id, Function<void(DTTutorialLayer*)> initilizeTutorial) {
    this->m_size = std::move(size);

    m_spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    m_spr->setScale(m_size);

    if (!CCMenuItemSpriteExtra::init(m_spr, nullptr, this, menu_selector(TutorialButton::onClicked))) return false;

    this->m_initilizeTutorial = std::move(initilizeTutorial);

    this->m_id = std::string(id);

    if (!Save::wasTutorialSeen(m_id)) {
        auto action = CCRepeatForever::create(CCSequence::create(
            CCEaseInOut::create(CCScaleTo::create(.5f, m_size + .1f), 2),
            CCEaseInOut::create(CCScaleTo::create(.5f, m_size), 2),
            nullptr
        ));
        action->setTag(1);

        m_spr->runAction(action);
    }

    return true;
}

void TutorialButton::onClicked(CCObject*) {
    auto layer = DTTutorialLayer::create();

    m_initilizeTutorial(layer);

    layer->show();

    if (!Save::wasTutorialSeen(m_id)) {
        Save::setTutorialSeen(m_id);

        m_spr->stopActionByTag(1);
        m_spr->runAction(CCEaseInOut::create(CCScaleTo::create(.5f, m_size), 2));
    }
}
