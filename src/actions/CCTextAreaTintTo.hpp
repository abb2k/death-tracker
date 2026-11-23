#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextArea.hpp>

using namespace geode::prelude;

class CCTextAreaTintTo : public CCActionInterval
{
public:
    static CCTextAreaTintTo* create(float duration, const ccColor3B& toColor);

    bool initWithDuration(float duration, const ccColor3B& toColor);

    virtual void startWithTarget(CCNode* target) override;

    virtual void update(float time) override;

    ccColor3B startColor;
    ccColor3B endColor;
    int deltaR;
    int deltaG;
    int deltaB;
    
private:
    bool getTextAreaColor(CCNode* target, ccColor4B& color);
    void setTextAreaColor(CCNode* target, const ccColor4B& color);
};
