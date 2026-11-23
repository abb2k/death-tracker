#pragma once

using namespace geode::prelude;

class CCResizeHeightTo : public CCActionInterval {
public:
    static CCResizeHeightTo* create(float duration, float endHeight);

    virtual void startWithTarget(CCNode* target) override;

private:
    bool initWithDuration(float duration, float endHeight);

    void update(float time) override;

    float startHeight;
    float endHeight;
    float deltaHeight;
};
