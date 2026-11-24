#include "GraphPoint.hpp"

GraphPoint* GraphPoint::create(std::string_view run, float rate, ccColor4B color, DTGraphNode* relatedGraph) {
    auto ret = new GraphPoint();

    if (ret && ret->init(run, std::move(rate), std::move(color), relatedGraph)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool GraphPoint::init(std::string_view run, float rate, ccColor4B color, DTGraphNode* relatedGraph) {

    m_Run = run;
    m_Rate = std::move(rate);
    m_relatedGraph = relatedGraph;

    m_c = CCSprite::createWithSpriteFrameName("d_circle_02_001.png");
    m_c->setColor({color.r, color.g, color.b});
    m_c->setOpacity(color.a);
    m_c->setPosition(m_c->getContentSize());
    this->addChild(m_c);

    this->setContentSize(m_c->getContentSize() * 2);
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
        m_c->setScale(2);
        if (!m_oneTimeCall) {
            m_oneTimeCall = true;
            if (m_Delegate == nullptr) return;
            m_Delegate->OnPointSelected(this);
        }
    }
    else {
        m_c->setScale(1);
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