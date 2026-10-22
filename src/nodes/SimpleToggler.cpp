#include <nodes/SimpleToggler.hpp>

SimpleToggler* SimpleToggler::create(CCNode* offSprite, CCNode* onSprite, float scale, bool startState) {
    auto ret = new SimpleToggler();
    if (ret && ret->init(offSprite, onSprite, scale, startState)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

SimpleToggler* SimpleToggler::createWithDefaults(float scale, bool startState) {
    return create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        scale,
        startState
    );
}

bool SimpleToggler::init(CCNode* offSprite, CCNode* onSprite, float scale, bool startState) {
    this->offSprite = offSprite;
    this->onSprite = onSprite;

    offSprite->setScale(scale);
    onSprite->setScale(scale);

    this->setCascadeColorEnabled(true);
    this->setCascadeOpacityEnabled(true);

    if (!CCMenuItemToggler::init(offSprite, onSprite, this, menu_selector(SimpleToggler::onToggled))) return false;

    this->toggle(startState);

    return true;
}

void SimpleToggler::setCallback(geode::Function<void(bool)> callback) {
    this->callback = std::move(callback);
}

void SimpleToggler::onToggled(CCObject* sender) {
    if (this->callback != NULL)
        this->callback(!static_cast<CCMenuItemToggler*>(sender)->isToggled());
}

void SimpleToggler::setOpacity(GLubyte opacity) {
    if (auto rgba = typeinfo_cast<CCRGBAProtocol*>(offSprite))
        rgba->setOpacity(opacity);
    
    if (auto rgba = typeinfo_cast<CCRGBAProtocol*>(onSprite))
        rgba->setOpacity(opacity);
}