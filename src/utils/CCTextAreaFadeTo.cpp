#include <utils/CCTextAreaFadeTo.hpp>
#include <nodes/RichTextArea.hpp>

CCTextAreaFadeTo* CCTextAreaFadeTo::create(float duration, GLubyte toOpacity)
{
    auto ret = new CCTextAreaFadeTo();
    if (ret && ret->initWithDuration(duration, toOpacity)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCTextAreaFadeTo::initWithDuration(float duration, GLubyte toOpacity)
{
    if (!CCActionInterval::initWithDuration(duration)) return false;

    this->endOpacity = toOpacity;
    return true;
}

void CCTextAreaFadeTo::startWithTarget(CCNode* target)
{
    CCActionInterval::startWithTarget(target);

    ccColor4B color;
    if (!getTextAreaColor(target, color)) return;
    this->startOpacity = color.a;
    this->deltaOpacity = (int)endOpacity - (int)startOpacity;
}

void CCTextAreaFadeTo::update(float time)
{
    if (!m_pTarget) return;

    ccColor4B oldColor;
    if (!getTextAreaColor(m_pTarget, oldColor)) return;

    const int a = (int)startOpacity + (int)(deltaOpacity * time);

    ccColor4B newColor = { oldColor.r, oldColor.g, oldColor.b, (GLubyte)a };
    setTextAreaColor(m_pTarget, newColor);
}

bool CCTextAreaFadeTo::getTextAreaColor(CCNode* target, ccColor4B& color) {
    if (auto textArea = dynamic_cast<RichTextArea*>(target)) {
        color = textArea->getColor();
        return true;
    }
    if (auto textArea = dynamic_cast<SimpleTextArea*>(target)) {
        color = textArea->getColor();
        return true;
    }
    return false;
}

void CCTextAreaFadeTo::setTextAreaColor(CCNode* target, const ccColor4B& color) {
    if (auto textArea = dynamic_cast<RichTextArea*>(target)) {
        textArea->setColor(color);
    }
    else if (auto textArea = dynamic_cast<SimpleTextArea*>(target)) {
        textArea->setColor(color);
    }
}