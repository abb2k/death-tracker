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

    ccColor3B startColor;
    ccColor3B endColor;
    int deltaR;
    int deltaG;
    int deltaB;
};
