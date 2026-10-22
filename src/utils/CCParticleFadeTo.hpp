#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CCParticleFadeTo : public CCActionInterval {
    public:
        static CCParticleFadeTo* create(float duration, GLubyte toOpacity);

        bool initWithDuration(float duration, GLubyte toOpacity);

        void startWithTarget(CCNode* target) override;

        void update(float time) override;

    private:
        GLubyte m_startOpacity = 0;
        GLubyte m_endOpacity = 0;
        int m_deltaOpacity = 0;
};