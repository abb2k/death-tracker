#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CCResizeTo : public CCActionInterval {
public:
    static CCResizeTo* create(float duration, const CCSize& endSize);

    bool initWithDuration(float duration, const CCSize& endSize);

    virtual void startWithTarget(CCNode* target);

    virtual void update(float time);
    
    CCSize startSize;
    CCSize endSize;
    CCSize deltaSize;
};
