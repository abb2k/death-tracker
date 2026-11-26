#include "CCGradientAction.hpp"

CCGradientAction* CCGradientAction::create(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset) {
    auto action = new CCGradientAction();
    
    if (action && action->init(std::move(transitionDuration), std::move(colorA), std::move(colorB), std::move(phaseOffset))) {
        action->autorelease();
        return action;
    }

    delete action;
    return nullptr;
}

CCGradientAction* CCGradientAction::create(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset) {
    auto action = new CCGradientAction();

    if (action && action->init(std::move(transitionDuration), std::move(colorA), std::move(colorB), std::move(phaseOffset))) {
        action->autorelease();
        return action;
    }

    delete action;
    return nullptr;
}

bool CCGradientAction::init(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset) {
    if (!CCActionInterval::initWithDuration(transitionDuration * 2.0f)) return false;

    m_transitionDuration = std::move(transitionDuration);
    m_cycleDuration = m_transitionDuration * 2.0f;
    m_colorA = to4B(std::move(colorA));
    m_colorB = to4B(std::move(colorB));
    m_phaseOffset = std::move(phaseOffset);
    m_animateOpacity = false;

    return true;
}

bool CCGradientAction::init(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset) {
    if (!CCActionInterval::initWithDuration(transitionDuration * 2.0f)) return false;

    m_transitionDuration = std::move(transitionDuration);
    m_cycleDuration = m_transitionDuration * 2.0f;
    m_colorA = std::move(colorA);
    m_colorB = std::move(colorB);
    m_phaseOffset = std::move(phaseOffset);
    m_animateOpacity = true;

    return true;
}

void CCGradientAction::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    if (auto rgba = typeinfo_cast<CCRGBAProtocol*>(target)) {
        m_baseOpacity = rgba->getOpacity();
    } else {
        m_baseOpacity = 255;
    }

    update(0.0f);
}

void CCGradientAction::update(float time) {
    auto rgba = typeinfo_cast<CCRGBAProtocol*>(m_pTarget);
    if (!rgba) return;

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

    rgba->setColor(ccColor3B{
        addOffset(interpolate(from.r, to.r)),
        addOffset(interpolate(from.g, to.g)),
        addOffset(interpolate(from.b, to.b))
    });
    if (m_animateOpacity) {
        auto gradientOpacity = interpolate(from.a, to.a);
        rgba->setOpacity(static_cast<GLubyte>(
            (static_cast<unsigned int>(m_baseOpacity) * gradientOpacity + 127) / 255
        ));
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
