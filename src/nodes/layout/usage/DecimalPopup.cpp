#include "DecimalPopup.hpp"

#include <managers/StatsManager.hpp>
#include <nodes/layout/usage/DTLabel.hpp>
#include <managers/AuthManager.hpp>

DecimalPopup* DecimalPopup::create(CCSize size) {
    auto ret = new DecimalPopup();

    if (ret && ret->init(std::move(size))) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool DecimalPopup::init(CCSize size) {
    if (!CCNode::init()) return false;

    m_maxPopupHeight = size.height;
    this->setContentSize(size);

    m_bg = CCScale9Sprite::create("square02_small.png");
    m_bg->setAnchorPoint({0, 1});
    m_bg->setScale(.5f);
    m_bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / m_bg->getScale());
    m_bg->setPositionY(size.height - 2.5f);
    m_bg->setOpacity(200);
    this->addChild(m_bg);

    auto scrollOffset = ccp(2, 6.5f);
    m_maxScrollHeight = size.height - scrollOffset.y;
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
    info.m_scale = .5f;
    info.m_wrapping = WrappingMode::SPACE_WRAP;
    info.m_isExpanded = true;

    m_label = DTLabel::create(info);
    m_label->setAnchorPoint({0, 0});
    m_label->setPosition({0, 0});
    m_label->setExpandable(false);
    m_label->setContentWidth(m_scroll->getContentWidth());
    m_label->setDonerRichTextEnabled(AuthManager::isDoner());
    m_scroll->m_contentLayer->addChild(m_label);

    m_scroll->m_contentLayer->updateLayout();

    m_donerListenerHandle = OnDonerStateChangedEvent().listen([&](bool state) {
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
    m_closeButton = CCMenuItemSpriteExtra::create(
        closeBtnSpr,
        this,
        menu_selector(DecimalPopup::onClose)
    );
    m_closeButton->setPosition({2, size.height - 7});
    buttMenu->addChild(m_closeButton);

    m_scrollBargame = Scrollbar::create(m_scroll);
    m_scrollBargame->setPosition({size.width - 2, size.height / 2});
    m_scrollBargame->setAnchorPoint({1, 0.5f});
    m_scrollBargame->setVisible(false);
    this->addChild(m_scrollBargame);

    scheduleUpdate();

    return true;
}

void DecimalPopup::onEnter() {
    CCNode::onEnter();
    close();
}

void DecimalPopup::onExit() {
    close();
    CCNode::onExit();
}

void DecimalPopup::setDisplay(CCNode* origin, const Run& run, bool isSess, uint8_t decimalDepth, DTLabel* referenceLabel, std::optional<std::pair<long long, size_t>> occurrenceSelection) {
    if (m_originLabel != referenceLabel) {
        if (m_originLabel)
            m_originLabel->release();
        m_originLabel = referenceLabel;
        if (m_originLabel)
            m_originLabel->retain();
    }

    if (origin && m_originLabel) {
        auto toLabelSpace = [this, origin](CCPoint point) {
            return m_originLabel->convertToNodeSpace(origin->convertToWorldSpace(point));
        };
        m_originTopRight = toLabelSpace({origin->getContentWidth(), origin->getContentHeight()});
        m_originBottomLeft = toLabelSpace({0, 0});
        m_originBottomRight = toLabelSpace({origin->getContentWidth(), 0});
        m_originTopLeft = toLabelSpace({0, origin->getContentHeight()});
    }

    this->setVisible(true);
    m_label->setEnabled(true);

    this->positionToOrigin();

    m_label->setPayload("decimalDepth", decimalDepth);
    m_label->setPayload("decimalRun", run);
    m_label->setPayload("decimalIsSess", isSess);
    if (occurrenceSelection) {
        m_label->setPayload("decimalOccurrenceTimestamp", occurrenceSelection->first);
        m_label->setPayload("decimalOccurrenceIndex", occurrenceSelection->second);
    }
    else {
        m_label->removePayload("decimalOccurrenceTimestamp");
        m_label->removePayload("decimalOccurrenceIndex");
    }

    auto runStrRes = StatsManager::createRunKey(run, occurrenceSelection.has_value());

    if (referenceLabel) {
        m_label->setFontSize(referenceLabel->getInfo().m_scale);
        m_label->setFont(referenceLabel->getInfo().m_font);
        m_label->setLabelColor(referenceLabel->getInfo().m_labelColor);
        m_label->setTextColor(referenceLabel->getInfo().m_textColor);
        m_label->setTextWrapping(referenceLabel->getInfo().m_wrapping);
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

void DecimalPopup::positionToOrigin() {
    if (!m_originLabel || !m_originLabel->getParent()) {
        close();
        return;
    }
    if (!this->getParent() || !this->getParent()->getParent()) return;

    auto offset = ccp(2, 3);
    this->setAnchorPoint({0.f, 1});

    auto posWorld = m_originLabel->convertToWorldSpace(m_originTopRight);
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

        posWorld = m_originLabel->convertToWorldSpace(m_originBottomLeft);
        posNode = this->getParent()->convertToNodeSpace(posWorld);
        this->setPosition(posNode - offset);

        updateFarChecks();
    }
    else {
        if (tooFarBottom) {
            this->setAnchorPoint({this->getAnchorPoint().x, 0});

            posWorld = m_originLabel->convertToWorldSpace(m_originBottomRight);
            posNode = this->getParent()->convertToNodeSpace(posWorld);
            this->setPositionY(posNode.y - offset.y);

            updateFarChecks();
        }

        if (tooFarSide) {
            this->setAnchorPoint({1, this->getAnchorPoint().y});

            posWorld = m_originLabel->convertToWorldSpace(m_originTopLeft);
            posNode = this->getParent()->convertToNodeSpace(posWorld);
            this->setPositionX(posNode.x - offset.x);

            updateFarChecks();
        }

        auto posWorldTop = this->convertToWorldSpace({this->getContentWidth(), this->getContentHeight()});
        auto posNodeTop = this->getParent()->getParent()->convertToNodeSpace(posWorldTop);
        bool tooFarTop = !this->getParent()->boundingBox().containsPoint(posNodeTop);

        if (tooFarTop) {
            this->setAnchorPoint({this->getAnchorPoint().x, 1});

            posWorld = m_originLabel->convertToWorldSpace(m_originTopRight);
            posNode = this->getParent()->convertToNodeSpace(posWorld);
            this->setPositionY(posNode.y + offset.y);

            updateFarChecks();
        }

        auto posWorldRight = this->convertToWorldSpace({this->getContentWidth(), 0});
        auto posNodeRight = this->getParent()->getParent()->convertToNodeSpace(posWorldRight);
        bool tooFarRight = !this->getParent()->boundingBox().containsPoint(posNodeRight);

        if (tooFarRight) {
            this->setAnchorPoint({1, this->getAnchorPoint().y});

            posWorld = m_originLabel->convertToWorldSpace(m_originBottomLeft);
            posNode = this->getParent()->convertToNodeSpace(posWorld);
            this->setPositionX(posNode.x - offset.x);

            updateFarChecks();
        }

        if (tooFarBottom) {
            this->setAnchorPoint({this->getAnchorPoint().x, 0});

            posWorld = m_originLabel->convertToWorldSpace(m_originBottomRight);
            posNode = this->getParent()->convertToNodeSpace(posWorld);
            this->setPositionY(posNode.y - offset.y);

            updateFarChecks();
        }
    }
}

void DecimalPopup::update(float dt) {
    float prevHeight = m_scroll->m_contentLayer->getContentHeight();
    m_scroll->m_contentLayer->updateLayout();

    constexpr float bottomPadding = 3.f;
    auto labelHeight = m_label->getContentHeight();
    auto scrollHeight = std::min(labelHeight + bottomPadding, m_maxScrollHeight);
    auto popupHeight = scrollHeight + 6.5f;
    auto bgHeight = std::min(labelHeight + bottomPadding, m_maxPopupHeight - 2.5f);

    this->setContentHeight(popupHeight);
    m_bg->setPositionY(popupHeight - 2.5f);
    m_bg->setContentHeight(bgHeight / m_bg->getScale());
    m_closeButton->setPositionY(popupHeight - 7);
    auto scrollTop = popupHeight - 3.25f;

    if (m_scroll->getContentHeight() != scrollHeight) {
        m_scroll->setContentHeight(scrollHeight);
        m_scroll->setPositionY(scrollTop - scrollHeight);
        m_scrollBargame->setPositionY(scrollTop - scrollHeight / 2);
        m_scroll->m_contentLayer->setLayout(ColumnLayout::create()
            ->setAutoGrowAxis(scrollHeight)
            ->setAxisAlignment(AxisAlignment::End)
            ->setCrossAxisAlignment(AxisAlignment::Center)
            ->setCrossAxisOverflow(false)
        );
        m_scroll->m_contentLayer->updateLayout();
    }
    else {
        m_scroll->setPositionY(scrollTop - scrollHeight);
        m_scrollBargame->setPositionY(scrollTop - scrollHeight / 2);
    }

    if (m_scroll->m_contentLayer->getContentHeight() != prevHeight)
        m_scroll->moveToTop();

    if (this->isVisible())
        this->positionToOrigin();

    bool canScroll = this->isVisible() && m_scroll->m_contentLayer->getContentHeight() > m_scroll->getContentHeight();
    m_scroll->setMouseEnabled(canScroll);
    m_scroll->setTouchEnabled(canScroll);
    m_scrollBargame->setVisible(canScroll);
    m_scrollBargame->setTouchEnabled(canScroll);
}

bool DecimalPopup::isTouchOnMe(CCTouch *pTouch, bool begin) {
    if (begin) {
        bool canScroll = m_scroll->m_contentLayer->getContentHeight() > m_scroll->getContentHeight();
        m_isHolding = canScroll && m_bg->boundingBox().containsPoint(m_bg->getParent()->convertTouchToNodeSpace(pTouch)) && this->isVisible();
    }
    
    if (m_isHolding) {
        return true;
    }

    return false;
}

void DecimalPopup::close() {
    this->setVisible(false);
    auto originLabel = m_originLabel;
    m_originLabel = nullptr;
    if (originLabel)
        originLabel->release();

    m_scroll->setMouseEnabled(false);
    m_scroll->setTouchEnabled(false);
    m_label->setEnabled(false);
    m_scrollBargame->setVisible(false);
    m_scrollBargame->setTouchEnabled(false);
}

void DecimalPopup::onClose(CCObject* sender) {
    this->close();
}