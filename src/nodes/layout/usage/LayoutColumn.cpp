#include "LayoutColumn.hpp"

#include <nodes/DTLayer.hpp>
#include <nodes/layout/usage/DTLabel.hpp>
#include <nodes/general/TouchSender.hpp>

float LayoutColumn::borderWidth = 2;
float LayoutColumn::addNewBtnOffset = 15;
float LayoutColumn::topHeight = 20;

LayoutColumn* LayoutColumn::create(DTColumnInfo info, bool startVisible, float minHeight) {
    auto ret = new LayoutColumn();
    if (ret && ret->init(std::move(info), std::move(startVisible), std::move(minHeight))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool LayoutColumn::init(DTColumnInfo info, bool startVisible, float minHeight) {
    if (!CCMenu::init()) return false;

    this->m_info = std::move(info);

    m_isVisible = std::move(startVisible);

    this->setZOrder(m_info.m_orderPos);

    this->m_minHeight = std::move(minHeight);

    m_topSpr = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_topSpr->setID("top-spr");
    m_topSpr->setAnchorPoint({0, 1});
    this->addChild(m_topSpr);

    m_bgSpr = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_bgSpr->setID("bg-spr");
    m_bgSpr->setAnchorPoint({0, 1});
    m_bgSpr->setOpacity(m_bgOpacity);
    m_bgSpr->setVisible(false);
    this->addChild(m_bgSpr);

    m_topBorder1 = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_topBorder1->setID("top-border-1");
    m_topBorder1->setAnchorPoint({0, 1});
    m_topBorder1->setColor({0,0,0});
    this->addChild(m_topBorder1);

    m_topBorder2 = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_topBorder2->setID("top-border-2");
    m_topBorder2->setAnchorPoint({1, 1});
    m_topBorder2->setColor({0,0,0});
    this->addChild(m_topBorder2);

    m_topBorder3 = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_topBorder3->setID("top-border-3");
    m_topBorder3->setAnchorPoint({0, 0});
    m_topBorder3->setColor({0,0,0});
    this->addChild(m_topBorder3);

    m_bgSideBorder = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_bgSideBorder->setID("bg-side-border");
    m_bgSideBorder->setAnchorPoint({1, 1});
    m_bgSideBorder->setColor({0,0,0});
    m_bgSideBorder->setOpacity(m_bgOpacity);
    this->addChild(m_bgSideBorder);

    m_menu = CCMenu::create();
    m_menu->setPosition({0, 0});
    m_menu->setEnabled(m_isVisible);
    this->addChild(m_menu);

    auto addLabelBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    //addLabelBtnSpr->setScale(.5f);
    auto addLabelBtn = CCMenuItemSpriteExtra::create(
        addLabelBtnSpr,
        this,
        menu_selector(LayoutColumn::onAddLabelBtnClicked)
    );
    m_menu->addChild(addLabelBtn);

    this->setAnchorPoint({0, 1});
    this->ignoreAnchorPointForPosition(false);
    this->setContentHeight(m_minHeight);
    this->setContentWidth(m_info.m_currentWidth);
    
    updateSizesByContent();

    onOrganized(0);

    DTLayer::get()->subscribeToOrganizationEvent(this, std::bind(&LayoutColumn::onOrganized, this, std::placeholders::_1));

    this->scheduleUpdate();

    setColor(m_info.m_color);

    auto touchSender = TouchSender::create(-502, false);
    touchSender->setID("touchSender");
    touchSender->m_beginCheck = [&](cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        if (!m_isVisible) return false;
        auto touchInParentSpace = m_topBorder2->getParent()->convertTouchToNodeSpace(touch);

        if (m_topSpr->boundingBox().containsPoint(touchInParentSpace)) {
            m_isTouchingTopSpr = true;
            m_originalTopSprTouchPos = touchInParentSpace;
            return true;
        }

        m_isTouchingTopSpr = false;

        return false;
    };

    touchSender->m_onTouch = [&](TouchSenderState state, cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        if (state == TouchSenderState::Moved) {
            if (m_isTouchingTopSpr) {
                auto touchInParentSpace = m_topBorder2->getParent()->convertTouchToNodeSpace(touch);
        
                auto distance = (m_originalTopSprTouchPos - touchInParentSpace).getLength();

                if (distance > m_maxMovementOffset)
                    m_isTouchingTopSpr = false;
            }
        }
        else if (state == TouchSenderState::Cancled) {
            m_isTouchingTopSpr = false;
        }
        else if (state == TouchSenderState::Ended) {
            if (m_isTouchingTopSpr) {
                DTLayer::get()->setOptionsLayerTo(this);
            }
            m_isTouchingTopSpr = false;
        }
    };

    this->addChild(touchSender);

    if (!m_isVisible) {
        for (const auto& child : CCArrayExt<CCNode*>(this->getChildren()))
        {
            if (auto rgbChild = typeinfo_cast<CCRGBAProtocol*>(child))
                rgbChild->setOpacity(0);
        }
    }

    return true;
}

void LayoutColumn::updateSizesByContent() {
    if (this->getContentHeight() < m_minHeight) this->setContentHeight(m_minHeight);
    
    m_topSpr->setContentSize({this->getContentWidth(), topHeight});
    m_topSpr->setPosition({0, this->getContentHeight()});

    m_topBorder1->setContentSize({m_topSpr->getContentWidth(), borderWidth});
    m_topBorder1->setPosition(m_topSpr->getPosition());

    m_topBorder2->setContentSize({borderWidth, m_topSpr->getContentHeight()});
    m_topBorder2->setPosition(m_topSpr->getPosition() + ccp(m_topSpr->getContentWidth(), 0));

    m_topBorder3->setContentSize({m_topSpr->getContentWidth(), borderWidth});
    m_topBorder3->setPosition(m_topSpr->getPosition() - ccp(0, m_topSpr->getContentHeight()));

    m_bgSpr->setContentSize({this->getContentWidth(), this->getContentHeight() - topHeight});
    m_bgSpr->setPosition({0, m_bgSpr->getContentHeight()});
    
    m_bgSideBorder->setContentSize({borderWidth, m_bgSpr->getContentHeight()});
    m_bgSideBorder->setPosition(m_topBorder2->getPosition() - ccp(0, m_topBorder2->getContentHeight()));
}

void LayoutColumn::update(float dt) {
    DTLabel* lowestLabel = nullptr;

    auto labelsTemp = m_labels;
    for (const auto& [labelLayer, label] : labelsTemp) {
        if (lowestLabel == nullptr) {
            lowestLabel = label;
            continue;
        }

        if (lowestLabel->getPositionY() > label->getPositionY()) lowestLabel = label;
    }

    CCPoint targetAddBtnPos;

    if (lowestLabel != nullptr) {
        auto lowestLabelPointWorldSpace = lowestLabel->convertToWorldSpace({0, 0});
        targetAddBtnPos = this->convertToNodeSpace(lowestLabelPointWorldSpace) - ccp(0, addNewBtnOffset);
    }
    else {
        targetAddBtnPos = m_bgSpr->getPosition() + ccp(0, -addNewBtnOffset);
    }

    targetAddBtnPos.x = this->getContentWidth() / 2;

    //log::info("plus button for column {}, {} | {}", orderPos, targetAddBtnPos, tempMenuPos);

    if (!targetAddBtnPos.equals(m_tempMenuPos)) {
        m_tempMenuPos = targetAddBtnPos;
        m_menu->stopActionByTag(1);
        auto btnMoveAction = CCEaseInOut::create(CCMoveTo::create(.5f, targetAddBtnPos), 2);
        btnMoveAction->setTag(1);
        m_menu->runAction(btnMoveAction);
    }
}

void LayoutColumn::setContentHeight(float height) {
    CCMenu::setContentHeight(height);
    updateSizesByContent();
}

void LayoutColumn::setContentWidth(float width) {
    CCMenu::setContentWidth(width);

    m_info.m_currentWidth = width;
}

void LayoutColumn::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -502, true);
}

bool LayoutColumn::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!m_isVisible) return false;
    auto touchInParentSpace = m_topBorder2->getParent()->convertTouchToNodeSpace(touch);

    if (m_topBorder2->boundingBox().containsPoint(touchInParentSpace)) {
        m_isTouchingSide = true;
        
        return true;
    }

    m_isTouchingSide = false;
    return false;
}

void LayoutColumn::ccTouchEnded(CCTouch*, CCEvent*) {
    m_isTouchingSide = false;

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::ccTouchCancelled(CCTouch*, CCEvent*) {
    m_isTouchingSide = false;

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (m_isTouchingSide) {
        auto currentWidth = this->getContentWidth();

        auto spacedTouchLoc = this->getParent()->convertToNodeSpace(touch->getLocation());
        auto spacedPrevTouchLoc = this->getParent()->convertToNodeSpace(touch->m_prevPoint);

        currentWidth += (spacedTouchLoc - spacedPrevTouchLoc).x;

        this->setContentWidth(std::max(DTColumnInfo::minWidth, currentWidth));

        updateSizesByContent();

        DTLayer::get()->organizeLayout();
    }
}

void LayoutColumn::setColor(ccColor3B color) {
    m_topSpr->setColor(color);
    m_bgSpr->setColor(color);

    m_info.m_color = color;
}

void LayoutColumn::addLabel(DTLabel* label) {
    label->addColumnAsHolder(this);
    if (m_labels.contains(label->m_info.m_layer)) {
        m_labels[label->m_info.m_layer]->moveUpLayer();
    }
    
    m_labels.insert({label->m_info.m_layer, label});
}

void LayoutColumn::removeLabel(DTLabel* label) {
    if (!label->isPartOfColumn(this)) return;
    label->removeColumnAsHolder(this);

    this->m_labels.erase(label->m_info.m_layer);
}

void LayoutColumn::updateLabelPosition(DTLabel* label) {
    if (m_labels.contains(label->m_info.m_layer)) {
        m_labels[label->m_info.m_layer]->moveUpLayer();
    }

    m_labels.insert({label->m_info.m_layer, label});
}

void LayoutColumn::onAddLabelBtnClicked(CCObject*) {
    auto newLabel = DTLayer::get()->createNewLabel();

    int highestLayer = 0;

    for (const auto& [labelLayer, _] : m_labels)
    {
        highestLayer = std::max(highestLayer, labelLayer);
    }
    newLabel->m_info.m_layer = highestLayer + 1;

    addLabel(newLabel);

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::onOrganized(float deltaMove) {
    m_menu->setPositionY(m_menu->getPositionY() - deltaMove);
}

void LayoutColumn::destroyColumnAndCleanup() {
    auto labelsTemp = m_labels;
    std::set<DTLabel*> labelsToDelete{};
    for (const auto& [labelLayer, label] : labelsTemp)
    {
        removeLabel(label);

        if (label->isAlone() && !labelsToDelete.contains(label))
            labelsToDelete.insert(label);
    }

    for (const auto& label : labelsToDelete)
    {
        label->removeMeAndCleanup();
    }
    

    // Unsubscribe from organization events before destroying this column
    DTLayer::get()->unsubscribeToOrganizationEvent(this);

    DTLayer::get()->removeColumn(this);

    this->removeMeAndCleanup();
    DTLayer::get()->fixUpColumnPositions();
}

void LayoutColumn::refreshAllLabelsLayer() {
    auto labelsbackup = m_labels;

    for (const auto& [labelLayer, label] : m_labels)
    {
        if (!label->isPartOfColumn(this)) continue;
        label->removeColumnAsHolder(this);
    }

    m_labels.clear();

    for (const auto& [labelLayer, label] : labelsbackup)
    {
        addLabel(label);
    }
}

void LayoutColumn::setVisibility(bool visible) {
    m_isVisible = visible;
    m_menu->setEnabled(visible);

    for (const auto& child : CCArrayExt<CCNode*>(this->getChildren()))
    {
        child->stopActionByTag(5);

        int maxOpacity = 255;
        if (child == m_bgSpr || child == m_bgSideBorder) maxOpacity = m_bgOpacity;
        auto opacityAction = CCFadeTo::create(.15f, visible ? maxOpacity : 0);
        opacityAction->setTag(5);
        child->runAction(opacityAction);
    }
}