#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CCResizeWidthTo : public CCActionInterval {
public:
    static CCResizeWidthTo* create(float duration, float endWidth);

    bool initWithDuration(float duration, float endWidth);

    virtual void startWithTarget(CCNode* target) override;

    virtual void update(float time) override;

    float startWidth;
    float endWidth;
    float deltaWidth;
};
