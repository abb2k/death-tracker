#pragma once

using namespace geode::prelude;

class CCResizeWidthTo : public CCActionInterval {
public:
    static CCResizeWidthTo* create(float duration, float endWidth);

    virtual void startWithTarget(CCNode* target) override;

    float getStartWidth() const { return startWidth; }
    float getEndWidth() const { return endWidth; }

private:
    bool initWithDuration(float duration, float endWidth);

    void update(float time) override;

    float startWidth;
    float endWidth;
    float deltaWidth;
};
