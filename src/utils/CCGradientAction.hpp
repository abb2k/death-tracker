#pragma once

#include <Geode/Geode.hpp>
#include <cstdint>

using namespace geode::prelude;

class CCGradientAction : public CCActionInterval {
    public:
        static CCGradientAction* create(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset);
        static CCGradientAction* create(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset);

        bool init(float transitionDuration, ccColor3B colorA, ccColor3B colorB, float phaseOffset);
        bool init(float transitionDuration, ccColor4B colorA, ccColor4B colorB, float phaseOffset);

        void startWithTarget(CCNode* target) override;
        void update(float time) override;
        CCActionInterval* reverse() override;
        void addColorOffset(int offset);

    private:
        float m_transitionDuration = 1;
        float m_cycleDuration = 2;
        float m_phaseOffset = 0;
        std::int64_t m_colorOffset = 0;
        ccColor4B m_colorA = {255, 255, 255, 255};
        ccColor4B m_colorB = {255, 255, 255, 255};
        bool m_animateOpacity = false;
};
