#include <nodes/GraphPoint.hpp>

GraphPoint* GraphPoint::create(const std::string& run, const float& rate, const ccColor4B& color) {
    auto ret = new GraphPoint();
    if (ret && ret->init(run, rate, color)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool GraphPoint::init(const std::string& run, const float& rate, const ccColor4B& color) {

    m_Run = run;
    m_Rate = rate;

    c = CCSprite::createWithSpriteFrameName("d_circle_02_001.png");
    c->setColor({color.r, color.g, color.b});
    c->setOpacity(color.a);
    c->setPosition(c->getContentSize());
    this->addChild(c);

    this->setContentSize(c->getContentSize() * 2);
    this->setAnchorPoint({0.5f, 0.5f});

    this->setEnabled(true);

    scheduleUpdate();

    return true;
}

void GraphPoint::update(float delta) {

    bool isHovered = false;

    #if defined(GEODE_IS_ANDROID) || defined(GEODE_IS_IOS)
    isHovered = m_bSelected;
    #else

    auto mousePos = this->getParent()->convertToNodeSpace(getMousePos());
    isHovered = this->boundingBox().containsPoint(mousePos);
    
    #endif


    if (isHovered && geode::cocos::nodeIsVisible(this)) {
        c->setScale(2);
        if (!m_oneTimeCall) {
            m_oneTimeCall = true;
            if (m_Delegate == nullptr) return;
            m_Delegate->OnPointSelected(this);
        }
    }
    else {
        c->setScale(1);
        if (m_oneTimeCall) {
            m_oneTimeCall = false;
            if (m_Delegate == nullptr) return;
            m_Delegate->OnPointDeselected(this);
        }
    }
}

void GraphPoint::setDelegate(GraphPointDelegate* Delegate) {
    m_Delegate = Delegate;
}

GraphPoint::~GraphPoint() {
    if (m_oneTimeCall) {
        m_oneTimeCall = false;
        if (m_Delegate == nullptr) return;
        m_Delegate->OnPointDeselected(this);
    }
}