#include "CCTextAreaFadeTo.hpp"
#include <nodes/general/RichTextArea.hpp>

CCTextAreaFadeTo* CCTextAreaFadeTo::create(float duration, GLubyte toOpacity) {
    auto ret = new CCTextAreaFadeTo();

    if (ret && ret->initWithDuration(std::move(duration), std::move(toOpacity))) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCTextAreaFadeTo::initWithDuration(float duration, GLubyte toOpacity) {
    if (!CCActionInterval::initWithDuration(std::move(duration))) return false;

    this->m_endOpacity = std::move(toOpacity);
    return true;
}

void CCTextAreaFadeTo::startWithTarget(CCNode* target) {
    CCActionInterval::startWithTarget(target);

    ccColor4B color;
    if (!getTextAreaColor(target, color)) return;
    this->m_startOpacity = color.a;
    this->m_deltaOpacity = (int)m_endOpacity - (int)m_startOpacity;
}

void CCTextAreaFadeTo::update(float time) {
    if (!m_pTarget) return;

    ccColor4B oldColor;
    if (!getTextAreaColor(m_pTarget, oldColor)) return;

    const int a = (int)m_startOpacity + (int)(m_deltaOpacity * time);

    ccColor4B newColor = { oldColor.r, oldColor.g, oldColor.b, (GLubyte)a };
    setTextAreaColor(m_pTarget, newColor);
}

bool CCTextAreaFadeTo::getTextAreaColor(CCNode* target, ccColor4B& color) {
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

void CCTextAreaFadeTo::setTextAreaColor(CCNode* target, const ccColor4B& color) {
    if (auto textArea = typeinfo_cast<RichTextArea*>(target)) {
        textArea->setColor(color);
    }
    else if (auto textArea = typeinfo_cast<SimpleTextArea*>(target)) {
        textArea->setColor(color);
    }
}