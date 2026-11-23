#pragma once

using namespace geode::prelude;

class CCShowHide : public cocos2d::CCActionInstant {
public:
    static CCShowHide* create(bool visible);

    virtual CCObject* copyWithZone(CCZone* pZone) override;

private:
    bool init(bool visible);

    void update(float time) override;

    bool m_bVisible;
};