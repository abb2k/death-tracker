#pragma once

using namespace geode::prelude;

class CCWaveAction : public CCActionInterval {
public:
    static CCWaveAction* create(float period, float distX, float distY, float phase);

    void startWithTarget(CCNode* target) override;

    CCActionInterval* reverse() override;

private:
    bool init(float period, float distX, float distY, float phase);

    void update(float t) override;

    float m_distX;
    float m_distY;
    float m_phase;
    float m_baseX;
    float m_baseY;
};