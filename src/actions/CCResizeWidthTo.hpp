#pragma once

using namespace geode::prelude;

class CCResizeWidthTo : public CCActionInterval {
public:
    static CCResizeWidthTo* create(float duration, float endWidth);

    virtual void startWithTarget(CCNode* target) override;

    float getStartWidth() const { return m_startWidth; }
    float getEndWidth() const { return m_endWidth; }

private:
    bool initWithDuration(float duration, float endWidth);

    void update(float time) override;

    float m_startWidth;
    float m_endWidth;
    float m_deltaWidth;
};
