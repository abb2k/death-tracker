#include "CCResizeWidthTo.hpp"

CCResizeWidthTo* CCResizeWidthTo::create(float duration, float endWidth) {
    auto ret = new CCResizeWidthTo();
    
    if (ret && ret->initWithDuration(std::move(duration), std::move(endWidth))) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCResizeWidthTo::initWithDuration(float duration, float endWidth) {
    if (!CCActionInterval::initWithDuration(std::move(duration))) return false;
    
    this->endWidth = std::move(endWidth);
    return true;
}

void CCResizeWidthTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    startWidth = target->getContentWidth();
    deltaWidth = endWidth - startWidth;
}

void CCResizeWidthTo::update(float time) {
    if (!m_pTarget) return;

    m_pTarget->setContentWidth(startWidth + deltaWidth * time);
}
