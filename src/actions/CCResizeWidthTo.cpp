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
    
    this->m_endWidth = std::move(endWidth);
    return true;
}

void CCResizeWidthTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    m_startWidth = target->getContentWidth();
    m_deltaWidth = m_endWidth - m_startWidth;
}

void CCResizeWidthTo::update(float time) {
    if (!m_pTarget) return;

    m_pTarget->setContentWidth(m_startWidth + m_deltaWidth * time);
}
