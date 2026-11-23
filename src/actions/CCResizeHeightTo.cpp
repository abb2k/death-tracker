#include "CCResizeHeightTo.hpp"

CCResizeHeightTo* CCResizeHeightTo::create(float duration, float endHeight) {
    auto ret = new CCResizeHeightTo();
    
    if (ret && ret->initWithDuration(std::move(duration), std::move(endHeight))) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCResizeHeightTo::initWithDuration(float duration, float endHeight) {
    if (!CCActionInterval::initWithDuration(std::move(duration))) return false;
    
    this->endHeight = std::move(endHeight);
    return true;
}

void CCResizeHeightTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    startHeight = target->getContentHeight();
    deltaHeight = endHeight - startHeight;
}

void CCResizeHeightTo::update(float time) {
    if (!m_pTarget) return;

    m_pTarget->setContentHeight(startHeight + deltaHeight * time);
}
