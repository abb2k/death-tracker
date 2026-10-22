#include "CCParticleFadeTo.hpp"

CCParticleFadeTo* CCParticleFadeTo::create(float duration, GLubyte toOpacity) {
    auto action = new CCParticleFadeTo();

    if (action && action->initWithDuration(duration, toOpacity)) {
        action->autorelease();
        return action;
    }

    CC_SAFE_DELETE(action);

    return nullptr;
}

bool CCParticleFadeTo::initWithDuration(float duration, GLubyte toOpacity) {
    if (!CCActionInterval::initWithDuration(duration)) return false;

    m_endOpacity = toOpacity;
    
    return true;
}

void CCParticleFadeTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    auto particle = typeinfo_cast<CCParticleSystemQuad*>(target);
    if (!particle) return;

    m_startOpacity = particle->getOpacity();
    m_deltaOpacity = static_cast<int>(m_endOpacity) - static_cast<int>(m_startOpacity);
}

void CCParticleFadeTo::update(float time) {
    auto particle = typeinfo_cast<CCParticleSystemQuad*>(m_pTarget);
    
    if (!particle) return;

    const int opacity = static_cast<int>(m_startOpacity) + static_cast<int>(m_deltaOpacity * time);
    particle->setOpacity(static_cast<GLubyte>(opacity));
}