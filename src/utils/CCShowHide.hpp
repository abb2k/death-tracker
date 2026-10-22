#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CCShowHide : public cocos2d::CCActionInstant {
public:
    static CCShowHide* create(bool visible)
    {
        CCShowHide* ret = new CCShowHide();
        if (ret && ret->init(visible))
        {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(bool visible)
    {
        m_bVisible = visible;
        return true;
    }

    virtual void update(float time) override
    {
        if (m_pTarget)
            m_pTarget->setVisible(m_bVisible);
    }

    virtual CCObject* copyWithZone(CCZone* pZone) override
    {
        CCZone* pNewZone = nullptr;
        CCShowHide* pCopy = nullptr;
        if (pZone && pZone->m_pCopyObject)
        {
            pCopy = (CCShowHide*)(pZone->m_pCopyObject);
        }
        else
        {
            pCopy = new CCShowHide();
            pNewZone = new CCZone(pCopy);
        }

        CCActionInstant::copyWithZone(pZone);
        pCopy->init(m_bVisible);

        CC_SAFE_DELETE(pNewZone);
        return pCopy;
    }

protected:
    bool m_bVisible;
};