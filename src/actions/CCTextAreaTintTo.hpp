#pragma once

using namespace geode::prelude;

class CCTextAreaTintTo : public CCActionInterval {
public:
    static CCTextAreaTintTo* create(float duration, ccColor3B toColor);

    virtual void startWithTarget(CCNode* target) override;

private:
    bool initWithDuration(float duration, ccColor3B toColor);

    void update(float time) override;

    bool getTextAreaColor(CCNode* target, ccColor4B& color);
    void setTextAreaColor(CCNode* target, const ccColor4B& color);

    ccColor3B m_startColor;
    ccColor3B m_endColor;
    int m_deltaR;
    int m_deltaG;
    int m_deltaB;
};
