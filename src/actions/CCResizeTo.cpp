#include "CCResizeTo.hpp"

CCResizeTo* CCResizeTo::create(float duration, CCSize endSize) {
    auto ret = new CCResizeTo();
    
    if (ret && ret->initWithDuration(std::move(duration), std::move(endSize))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool CCResizeTo::initWithDuration(float duration, CCSize endSize) {
    if (!CCActionInterval::initWithDuration(std::move(duration))) return false;
    
    this->m_endSize = std::move(endSize);

    return true;
}

void CCResizeTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    m_startSize = target->getContentSize();
    m_deltaSize = CCSize(m_endSize.width - m_startSize.width, m_endSize.height - m_startSize.height);
}

void CCResizeTo::update(float time) {
    if (!m_pTarget) return;

    m_pTarget->setContentSize(CCSize(m_startSize.width + m_deltaSize.width * time, m_startSize.height + m_deltaSize.height * time));
}