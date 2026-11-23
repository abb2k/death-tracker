#pragma once

using namespace geode::prelude;

class CCTextAreaFadeTo : public CCActionInterval {
public:
    static CCTextAreaFadeTo* create(float duration, GLubyte toOpacity);

    virtual void startWithTarget(CCNode* target) override;

private:
    bool initWithDuration(float duration, GLubyte toOpacity);

    void update(float time) override;

    bool getTextAreaColor(CCNode* target, ccColor4B& color);
    void setTextAreaColor(CCNode* target, const ccColor4B& color);

    GLubyte startOpacity;
    GLubyte endOpacity;
    int deltaOpacity;
};
