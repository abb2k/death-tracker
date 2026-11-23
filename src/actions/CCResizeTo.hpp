#pragma once

using namespace geode::prelude;

class CCResizeTo : public CCActionInterval {
public:
    static CCResizeTo* create(float duration, CCSize endSize);

    bool initWithDuration(float duration, CCSize endSize);

    virtual void startWithTarget(CCNode* target) override;

private:

    void update(float time) override;

    CCSize m_startSize;
    CCSize m_endSize;
    CCSize m_deltaSize;
};
