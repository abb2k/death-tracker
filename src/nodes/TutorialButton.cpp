#include "TutorialButton.hpp"
#include <utils/Save.hpp>

TutorialButton* TutorialButton::create(float size, const std::string& id, geode::Function<void(DTTutorialLayer*)> initilizeTutorial) {
    auto ret = new TutorialButton();
    if (ret && ret->init(size, id, std::move(initilizeTutorial))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool TutorialButton::init(float size, const std::string& id, geode::Function<void(DTTutorialLayer*)> initilizeTutorial) {
    this->size = size;

    spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    spr->setScale(size);

    if (!CCMenuItemSpriteExtra::init(spr, nullptr, this, menu_selector(TutorialButton::onClicked))) return false;

    this->initilizeTutorial = std::move(initilizeTutorial);

    this->id = id;

    if (!Save::wasTutorialSeen(id)) {
        auto action = CCRepeatForever::create(CCSequence::create(
            CCEaseInOut::create(CCScaleTo::create(.5f, size + .1f), 2),
            CCEaseInOut::create(CCScaleTo::create(.5f, size), 2),
            nullptr
        ));
        action->setTag(1);

        spr->runAction(action);
    }

    return true;
}

void TutorialButton::onClicked(CCObject*) {
    auto layer = DTTutorialLayer::create();

    initilizeTutorial(layer);

    layer->show();

    if (!Save::wasTutorialSeen(id)) {
        Save::setTutorialSeen(id);

        spr->stopActionByTag(1);
        spr->runAction(CCEaseInOut::create(CCScaleTo::create(.5f, size), 2));
    }
}
