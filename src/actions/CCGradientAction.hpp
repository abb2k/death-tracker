#pragma once

using namespace geode::prelude;

class CCGradientAction : public CCActionInterval {
public:
    static CCGradientAction* create(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset);
    static CCGradientAction* create(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset);

    void addColorOffset(int offset);
    
    CCActionInterval* reverse() override;

    void startWithTarget(CCNode* target) override;

private:
    bool init(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset);
    bool init(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset);

    void update(float time) override;

    float m_transitionDuration = 1;
    float m_cycleDuration = 2;
    float m_phaseOffset = 0;
    std::int64_t m_colorOffset = 0;
    ccColor4B m_colorA = {255, 255, 255, 255};
    ccColor4B m_colorB = {255, 255, 255, 255};
    GLubyte m_baseOpacity = 255;
    bool m_animateOpacity = false;
};
