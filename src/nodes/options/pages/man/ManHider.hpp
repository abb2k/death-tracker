#pragma once

#include <random>

class ManHider : public cocos2d::CCNode {
protected:
    std::mt19937 mt{std::random_device{}()};

    cocos2d::CCPoint const m_backPos{101.f, 113.f};
    cocos2d::CCPoint const m_frontPos{103.f, 116.f};
    cocos2d::CCPoint const m_particlePos{105.f, 125.f};

    cocos2d::CCSprite* m_backLeaves = nullptr;
    cocos2d::CCSprite* m_frontLeaves = nullptr;
    cocos2d::CCSprite* m_particle = nullptr;
    cocos2d::CCSprite* m_stump = nullptr;
    cocos2d::CCNode* m_tree = nullptr;

    float m_animTimer{};
    float m_particleTimer{};

    bool init() override;
    void update(float dt) override;
    void resetParticle();
    void updateParticle();

public:
    static ManHider* create();

    cocos2d::CCNode* getTree() const { return m_tree; }
    cocos2d::CCSprite* getStump() const { return m_stump; }
    void runFade(bool fadeIn, float duration);
};