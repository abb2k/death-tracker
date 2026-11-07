#include "DecimalPopup.hpp"

#include <managers/StatsManager.hpp>

#include <managers/AuthManager.hpp>

DecimalPopup* DecimalPopup::create(CCSize size) {
    auto ret = new DecimalPopup();
    if (ret && ret->init(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool DecimalPopup::init(CCSize size) {
    if (!CCNode::init()) return false;

    this->setContentSize(size);

    auto bg = CCScale9Sprite::create("square02_small.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    bg->setOpacity(200);
    this->addChild(bg);

    auto scrollOffset = ccp(2, 6.5f);
    m_scroll = ScrollLayer::create(size - scrollOffset);
    m_scroll->setAnchorPoint({0, 0});
    m_scroll->setPosition(scrollOffset / 2);
    m_scroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_scroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisOverflow(false)
    );
    this->addChild(m_scroll);

    DTLabelInfo info;
    info.scale = .5f;
    info.wrapping = WrappingMode::SPACE_WRAP;
    info.isExpanded = true;

    m_label = DTLabel::create(info);
    m_label->setAnchorPoint({0, 0});
    m_label->setPosition({0, 0});
    m_label->setExpandable(false);
    m_label->setContentWidth(m_scroll->getContentWidth());
    m_label->setDonerRichTextEnabled(AuthManager::isDoner());
    m_scroll->m_contentLayer->addChild(m_label);

    m_scroll->m_contentLayer->updateLayout();

    donerListenerHandle = OnDonerStateChangedEvent().listen([&](bool state) {
        if (m_label) {
            m_label->setDonerRichTextEnabled(state);
        }
        
        return ListenerResult::Propagate;
    });

    auto buttMenu = CCMenu::create();
    buttMenu->setPosition({0, 0});
    this->addChild(buttMenu);

    auto closeBtnSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    closeBtnSpr->setScale(.4f);
    auto closeBtn = CCMenuItemSpriteExtra::create(
        closeBtnSpr,
        this,
        menu_selector(DecimalPopup::onClose)
    );
    closeBtn->setPosition({2, size.height - 7});
    buttMenu->addChild(closeBtn);

    scrollBargame = Scrollbar::create(m_scroll);
    scrollBargame->setPosition({size.width - 2, size.height / 2});
    scrollBargame->setAnchorPoint({1, 0.5f});
    this->addChild(scrollBargame);

    scheduleUpdate();

    return true;
}

void DecimalPopup::onEnter() {
    CCNode::onEnter();
    close();
}

void DecimalPopup::onExit() {
    CCNode::onExit();
}

void DecimalPopup::setDisplay(CCNode* origin, const Run& run, bool isSess, uint8_t decimalDepth, DTLabel* referenceLabel) {    
    this->setVisible(true);
    m_scroll->setMouseEnabled(true);
    m_scroll->setTouchEnabled(true);
    m_label->setEnabled(true);
    scrollBargame->setTouchEnabled(true);

    auto offset = ccp(2, 3);

    this->setAnchorPoint({0.f, 1});

    auto posWorld = origin->convertToWorldSpace({origin->getContentWidth(), origin->getContentHeight()});
    auto posNode = this->getParent()->convertToNodeSpace(posWorld);

    this->setPosition(posNode + offset);

    bool tooFarSide;
    bool tooFarBottom;

    auto updateFarChecks = [&]() {
        auto posWorldBottom = this->convertToWorldSpace({this->getContentWidth(), 0});
        auto posNodeBottom = this->getParent()->getParent()->convertToNodeSpace(posWorldBottom);

        auto posWorldSide = this->convertToWorldSpace({0, this->getContentHeight()});
        auto posNodeSide = this->getParent()->getParent()->convertToNodeSpace(posWorldSide);

        tooFarSide = !this->getParent()->boundingBox().containsPoint(posNodeSide);
        tooFarBottom = !this->getParent()->boundingBox().containsPoint(posNodeBottom);
    };
    updateFarChecks();

    if (tooFarSide && tooFarBottom) {
        this->setAnchorPoint({1, 0});

        auto posWorld = origin->convertToWorldSpace({0, 0});
        auto posNode = this->getParent()->convertToNodeSpace(posWorld);

        this->setPosition(posNode - offset);

        updateFarChecks();
    }
    else {
        if (tooFarBottom) {
            this->setAnchorPoint({this->getAnchorPoint().x, 0});

            auto posWorld = origin->convertToWorldSpace({origin->getContentWidth(), 0});
            auto posNode = this->getParent()->convertToNodeSpace(posWorld);

            this->setPositionY(posNode.y - offset.y);

            updateFarChecks();
        }

        if (tooFarSide) {
            this->setAnchorPoint({1, this->getAnchorPoint().y});

            auto posWorld = origin->convertToWorldSpace({0, origin->getContentHeight()});
            auto posNode = this->getParent()->convertToNodeSpace(posWorld);

            this->setPositionX(posNode.x - offset.x);

            updateFarChecks();
        }

        auto posWorldTop = this->convertToWorldSpace({this->getContentWidth(), this->getContentHeight()});
        auto posNodeTop = this->getParent()->getParent()->convertToNodeSpace(posWorldTop);

        bool tooFarTop = !this->getParent()->boundingBox().containsPoint(posNodeTop);

        if (tooFarTop) {
            this->setAnchorPoint({this->getAnchorPoint().x, 1});

            auto posWorld = origin->convertToWorldSpace({origin->getContentWidth(), origin->getContentHeight()});
            auto posNode = this->getParent()->convertToNodeSpace(posWorld);

            this->setPositionY(posNode.y + offset.y);

            updateFarChecks();
        }

        auto posWorldRight = this->convertToWorldSpace({this->getContentWidth(), 0});
        auto posNodeRight = this->getParent()->getParent()->convertToNodeSpace(posWorldRight);

        bool tooFarRight = !this->getParent()->boundingBox().containsPoint(posNodeRight);

        if (tooFarRight) {
            this->setAnchorPoint({1, this->getAnchorPoint().y});

            auto posWorld = origin->convertToWorldSpace({0, 0});
            auto posNode = this->getParent()->convertToNodeSpace(posWorld);

            this->setPositionX(posNode.x - offset.x);

            updateFarChecks();
        }

        if (tooFarBottom) {
            this->setAnchorPoint({this->getAnchorPoint().x, 0});

            auto posWorld = origin->convertToWorldSpace({origin->getContentWidth(), 0});
            auto posNode = this->getParent()->convertToNodeSpace(posWorld);

            this->setPositionY(posNode.y - offset.y);

            updateFarChecks();
        }
    }

    m_label->payload["decimalDepth"] = decimalDepth;
    m_label->payload["decimalRun"] = run;
    m_label->payload["decimalIsSess"] = isSess;

    auto runStrRes = StatsManager::createRunKey(run);

    if (referenceLabel) {
        m_label->setFontSize(referenceLabel->info.scale);
        m_label->setFont(referenceLabel->info.font);
        m_label->setLabelColor(referenceLabel->info.labelColor);
        m_label->setTextColor(referenceLabel->info.textColor);
        m_label->setTextWrapping(referenceLabel->info.wrapping);
    }
    else {
        m_label->setFontSize(.5f);
        m_label->setFont("bigFont.fnt");
        m_label->setLabelColor({255, 255, 255, 255});
        m_label->setTextColor({255, 255, 255, 255});
        m_label->setTextWrapping(WrappingMode::SPACE_WRAP);
    }

    if (runStrRes.isOk())
        m_label->setLabelName(fmt::format("{}%, x.X{}", runStrRes.unwrap(), decimalDepth));
    else
        m_label->setLabelName("NA run");

    m_label->setLabelText("{dynamicDecimal}");
}

void DecimalPopup::update(float dt) {
    float prevHeight = m_scroll->m_contentLayer->getContentHeight();
    m_scroll->m_contentLayer->updateLayout();
    if (m_scroll->m_contentLayer->getContentHeight() != prevHeight)
        m_scroll->moveToTop();
}

bool DecimalPopup::isTouchOnMe(CCTouch *pTouch, bool begin) {
    if (begin) {
        isHolding = this->boundingBox().containsPoint(this->getParent()->convertTouchToNodeSpace(pTouch)) && this->isVisible();
    }
    
    if (isHolding) {
        return true;
    }

    return false;
}

void DecimalPopup::close() {
    this->setVisible(false);

    m_scroll->setMouseEnabled(false);
    m_scroll->setTouchEnabled(false);
    m_label->setEnabled(false);
    scrollBargame->setTouchEnabled(false);
}

void DecimalPopup::onClose(CCObject* sender) {
    this->close();
}