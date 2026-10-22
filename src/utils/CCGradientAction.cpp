#include "CCGradientAction.hpp"

CCGradientAction* CCGradientAction::create(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset) {
    auto action = new CCGradientAction();
    if (action && action->init(transitionDuration, colorA, colorB, phaseOffset)) {
        action->autorelease();
        return action;
    }

    CC_SAFE_DELETE(action);
    return nullptr;
}

CCGradientAction* CCGradientAction::create(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset) {
    auto action = new CCGradientAction();
    if (action && action->init(transitionDuration, colorA, colorB, phaseOffset)) {
        action->autorelease();
        return action;
    }

    CC_SAFE_DELETE(action);
    return nullptr;
}

bool CCGradientAction::init(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset) {
    if (!CCActionInterval::initWithDuration(transitionDuration * 2.0f)) return false;

    m_transitionDuration = transitionDuration;
    m_cycleDuration = transitionDuration * 2.0f;
    m_colorA = {colorA.r, colorA.g, colorA.b, 255};
    m_colorB = {colorB.r, colorB.g, colorB.b, 255};
    m_phaseOffset = phaseOffset;
    m_animateOpacity = false;
    return true;
}

bool CCGradientAction::init(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset) {
    if (!CCActionInterval::initWithDuration(transitionDuration * 2.0f)) return false;

    m_transitionDuration = transitionDuration;
    m_cycleDuration = transitionDuration * 2.0f;
    m_colorA = colorA;
    m_colorB = colorB;
    m_phaseOffset = phaseOffset;
    m_animateOpacity = true;
    return true;
}

void CCGradientAction::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);
    update(0.0f);
}

void CCGradientAction::update(float time) {
    auto sprite = typeinfo_cast<CCSprite*>(m_pTarget);
    if (!sprite) return;

    auto phase = std::fmod(time * m_cycleDuration + m_phaseOffset, m_cycleDuration);

    if (phase < 0.0f) phase += m_cycleDuration;

    auto transitioningToB = phase < m_transitionDuration;
    auto transitionTime = transitioningToB ? phase : phase - m_transitionDuration;
    auto progress = transitionTime / m_transitionDuration;

    auto from = transitioningToB ? m_colorA : m_colorB;
    auto to = transitioningToB ? m_colorB : m_colorA;

    auto interpolate = [progress](GLubyte start, GLubyte end) {
        return static_cast<GLubyte>(std::lround(start + (static_cast<int>(end) - start) * progress));
    };
    auto addOffset = [this](GLubyte color) {
        return static_cast<GLubyte>(std::clamp<std::int64_t>(
            static_cast<std::int64_t>(color) + m_colorOffset,
            0,
            255
        ));
    };

    sprite->setColor({
        addOffset(interpolate(from.r, to.r)),
        addOffset(interpolate(from.g, to.g)),
        addOffset(interpolate(from.b, to.b))
    });
    if (m_animateOpacity) {
        sprite->setOpacity(interpolate(from.a, to.a));
    }
}

void CCGradientAction::addColorOffset(int offset) {
    m_colorOffset += offset;
}

CCActionInterval* CCGradientAction::reverse() {
    CCGradientAction* reversed = nullptr;
    if (m_animateOpacity) {
        reversed = CCGradientAction::create(m_transitionDuration, m_colorB, m_colorA, m_phaseOffset);
    } else {
        reversed = CCGradientAction::create(
            m_transitionDuration,
            ccColor3B{m_colorB.r, m_colorB.g, m_colorB.b},
            ccColor3B{m_colorA.r, m_colorA.g, m_colorA.b},
            m_phaseOffset
        );
    }

    if (reversed) reversed->m_colorOffset = m_colorOffset;
    return reversed;
}
