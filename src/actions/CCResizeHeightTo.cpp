#include "CCResizeHeightTo.hpp"

CCResizeHeightTo* CCResizeHeightTo::create(float duration, float m_endHeight) {
    auto ret = new CCResizeHeightTo();

    if (ret && ret->initWithDuration(std::move(duration), std::move(m_endHeight))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool CCResizeHeightTo::initWithDuration(float duration, float m_endHeight) {
    if (!CCActionInterval::initWithDuration(std::move(duration))) return false;
    
    this->m_endHeight = std::move(m_endHeight);
    
    return true;
}

void CCResizeHeightTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    m_startHeight = target->getContentHeight();
    m_deltaHeight = m_endHeight - m_startHeight;
}

void CCResizeHeightTo::update(float time) {
    if (!m_pTarget) return;

    m_pTarget->setContentHeight(m_startHeight + m_deltaHeight * time);
}
