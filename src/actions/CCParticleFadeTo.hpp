#pragma once

using namespace geode::prelude;

class CCParticleFadeTo : public CCActionInterval {
public:
    static CCParticleFadeTo* create(float duration, GLubyte toOpacity);

    void startWithTarget(CCNode* target) override;

private:
    bool initWithDuration(float duration, GLubyte toOpacity);

    void update(float time) override;

    GLubyte m_startOpacity = 0;
    GLubyte m_endOpacity = 0;
    int m_deltaOpacity = 0;
};