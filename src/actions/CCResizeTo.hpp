#pragma once

using namespace geode::prelude;

class CCResizeTo : public CCActionInterval {
public:
    static CCResizeTo* create(float duration, const CCSize& endSize);

    bool initWithDuration(float duration, const CCSize& endSize);

    virtual void startWithTarget(CCNode* target) override;

private:

    void update(float time) override;

    CCSize startSize;
    CCSize endSize;
    CCSize deltaSize;
};
