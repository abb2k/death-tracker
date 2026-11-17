#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextArea.hpp>

using namespace geode::prelude;

class CCTextAreaFadeTo : public CCActionInterval
{
public:
    static CCTextAreaFadeTo* create(float duration, GLubyte toOpacity);

    bool initWithDuration(float duration, GLubyte toOpacity);

    virtual void startWithTarget(CCNode* target) override;

    virtual void update(float time) override;

    GLubyte startOpacity;
    GLubyte endOpacity;
    int deltaOpacity;

private:
    bool getTextAreaColor(CCNode* target, ccColor4B& color);
    void setTextAreaColor(CCNode* target, const ccColor4B& color);
};
