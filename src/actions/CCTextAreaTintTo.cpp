#include "CCTextAreaTintTo.hpp"
#include <nodes/general/RichTextArea.hpp>

CCTextAreaTintTo* CCTextAreaTintTo::create(float duration, ccColor3B toColor) {
    auto ret = new CCTextAreaTintTo();

    if (ret && ret->initWithDuration(std::move(duration), std::move(toColor))) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCTextAreaTintTo::initWithDuration(float duration, ccColor3B toColor) {
    if (!CCActionInterval::initWithDuration(std::move(duration))) return false;

    this->m_endColor = std::move(toColor);
    return true;
}

void CCTextAreaTintTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    ccColor4B color;
    if (!getTextAreaColor(target, color)) return;
    this->m_startColor = { color.r, color.g, color.b };
    this->m_deltaR = (int)m_endColor.r - (int)m_startColor.r;
    this->m_deltaG = (int)m_endColor.g - (int)m_startColor.g;
    this->m_deltaB = (int)m_endColor.b - (int)m_startColor.b;
}

void CCTextAreaTintTo::update(float time) {
    if (!m_pTarget) return;

    ccColor4B oldColor;
    if (!getTextAreaColor(m_pTarget, oldColor)) return;

    const int r = (int)m_startColor.r + (int)(m_deltaR * time);
    const int g = (int)m_startColor.g + (int)(m_deltaG * time);
    const int b = (int)m_startColor.b + (int)(m_deltaB * time);

    ccColor4B newColor = {
        static_cast<GLubyte>(r),
        static_cast<GLubyte>(g),
        static_cast<GLubyte>(b),
        oldColor.a
    };
    setTextAreaColor(m_pTarget, newColor);
}

bool CCTextAreaTintTo::getTextAreaColor(CCNode* target, ccColor4B& color) {
    if (auto textArea = typeinfo_cast<RichTextArea*>(target)) {
        color = textArea->getColor();
        return true;
    }
    if (auto textArea = typeinfo_cast<SimpleTextArea*>(target)) {
        color = textArea->getColor();
        return true;
    }
    return false;
}

void CCTextAreaTintTo::setTextAreaColor(CCNode* target, const ccColor4B& color) {
    if (auto textArea = typeinfo_cast<RichTextArea*>(target)) {
        textArea->setColor(color);
    }
    else if (auto textArea = typeinfo_cast<SimpleTextArea*>(target)) {
        textArea->setColor(color);
    }
}