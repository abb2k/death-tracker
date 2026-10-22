#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CCResizeHeightTo : public CCActionInterval {
public:
    static CCResizeHeightTo* create(float duration, float endHeight);

    bool initWithDuration(float duration, float endHeight);

    virtual void startWithTarget(CCNode* target) override;

    virtual void update(float time) override;
    
    float startHeight;
    float endHeight;
    float deltaHeight;
};
