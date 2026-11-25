#include "CCShowHide.hpp"

CCShowHide* CCShowHide::create(bool visible) {
    CCShowHide* ret = new CCShowHide();
    
    if (ret && ret->init(std::move(visible))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool CCShowHide::init(bool visible) {
    m_bVisible = std::move(visible);

    return true;
}

void CCShowHide::update(float) {
    if (!m_pTarget) return;

    m_pTarget->setVisible(m_bVisible);
}

CCObject* CCShowHide::copyWithZone(CCZone* pZone) {
    CCZone* pNewZone = nullptr;
    CCShowHide* pCopy = nullptr;

    if (pZone && pZone->m_pCopyObject) {
        pCopy = static_cast<CCShowHide*>(pZone->m_pCopyObject);
    }
    else {
        pCopy = new CCShowHide();
        pNewZone = new CCZone(pCopy);
    }

    CCActionInstant::copyWithZone(pZone);
    pCopy->init(m_bVisible);

    delete pNewZone;
    return pCopy;
}