#include <actions/CCResizeTo.hpp>

CCResizeTo* CCResizeTo::create(float duration, const CCSize& endSize)
{
    auto ret = new CCResizeTo();
    if (ret && ret->initWithDuration(duration, endSize)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CCResizeTo::initWithDuration(float duration, const CCSize& endSize)
{
    if (!CCActionInterval::initWithDuration(duration)) return false;
    
    this->endSize = endSize;
    return true;
}

void CCResizeTo::startWithTarget(CCNode* target)
{
    CCActionInterval::startWithTarget(target);
    startSize = target->getContentSize();
    deltaSize = CCSize(endSize.width - startSize.width, endSize.height - startSize.height);
}

void CCResizeTo::update(float time)
{
    if (!m_pTarget) return;

    m_pTarget->setContentSize(CCSize(startSize.width + deltaSize.width * time, startSize.height + deltaSize.height * time));
}