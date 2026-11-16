#include <utils/CCTextAreaTintTo.hpp>
#include <nodes/RichTextArea.hpp>

namespace {
bool getTextAreaColor(CCNode* target, ccColor4B& color) {
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

void setTextAreaColor(CCNode* target, const ccColor4B& color) {
    if (auto textArea = dynamic_cast<RichTextArea*>(target)) {
        textArea->setColor(color);
    }
    else if (auto textArea = dynamic_cast<SimpleTextArea*>(target)) {
        textArea->setColor(color);
    }
}
}

CCTextAreaTintTo* CCTextAreaTintTo::create(float duration, const ccColor3B& toColor)
{
    auto ret = new CCTextAreaTintTo();
    if (ret && ret->initWithDuration(duration, toColor)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCTextAreaTintTo::initWithDuration(float duration, const ccColor3B& toColor)
{
    if (!CCActionInterval::initWithDuration(duration)) return false;

    this->endColor = toColor;
    return true;
}

void CCTextAreaTintTo::startWithTarget(CCNode* target)
{
    CCActionInterval::startWithTarget(target);

    ccColor4B color;
    if (!getTextAreaColor(target, color)) return;
    this->startColor = { color.r, color.g, color.b };
    this->deltaR = (int)endColor.r - (int)startColor.r;
    this->deltaG = (int)endColor.g - (int)startColor.g;
    this->deltaB = (int)endColor.b - (int)startColor.b;
}

void CCTextAreaTintTo::update(float time)
{
    if (!m_pTarget) return;

    ccColor4B oldColor;
    if (!getTextAreaColor(m_pTarget, oldColor)) return;

    const int r = (int)startColor.r + (int)(deltaR * time);
    const int g = (int)startColor.g + (int)(deltaG * time);
    const int b = (int)startColor.b + (int)(deltaB * time);

    ccColor4B newColor = { (GLubyte)r, (GLubyte)g, (GLubyte)b, oldColor.a };
    setTextAreaColor(m_pTarget, newColor);
}
