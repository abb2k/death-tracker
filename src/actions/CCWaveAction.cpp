#include "CCWaveAction.hpp"

CCWaveAction* CCWaveAction::create(float period, float distX, float distY, float phase) {
    auto ret = new CCWaveAction();

    if (ret && ret->init(std::move(period), std::move(distX), std::move(distY), std::move(phase))) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCWaveAction::init(float period, float distX, float distY, float phase) {
    if (!cocos2d::CCActionInterval::initWithDuration(std::move(period)))
        return false;

    m_distX = std::move(distX);
    m_distY = std::move(distY);
    m_phase = std::move(phase);
    m_baseX = NAN;
    m_baseY = NAN;

    return true;
}

void CCWaveAction::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    if (!std::isnan(m_baseX)) return;

    m_baseX = target->getPositionX();
    m_baseY = target->getPositionY();
}

void CCWaveAction::update(float t) {
    float angle = (t * 2.f * M_PI) + m_phase;

    float x = m_baseX + cosf(angle) * m_distX;
    float y = m_baseY + sinf(angle) * m_distY;

    m_pTarget->setPosition(x, y);
}

CCActionInterval* CCWaveAction::reverse() {
    return CCWaveAction::create(m_fDuration, m_distX, m_distY, -m_phase);
}