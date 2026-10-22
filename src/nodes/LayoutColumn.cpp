#include "LayoutColumn.hpp"

#include <nodes/layers/DTLayer.hpp>
#include <nodes/TouchSender.hpp>

float LayoutColumn::borderWidth = 2;
float LayoutColumn::addNewBtnOffset = 15;
float LayoutColumn::topHeight = 20;

LayoutColumn* LayoutColumn::create(const DTColumnInfo& info, bool startVisible, float minHeight) {
    auto ret = new LayoutColumn();
    if (ret && ret->init(info, startVisible, minHeight)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool LayoutColumn::init(const DTColumnInfo& info, bool startVisible, float minHeight) {
    if (!CCMenu::init()) return false;

    this->info = info;

    this->setZOrder(info.orderPos);

    this->minHeight = minHeight;

    topSpr = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    topSpr->setID("top-spr");
    topSpr->setAnchorPoint({0, 1});
    this->addChild(topSpr);

    bgSpr = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    bgSpr->setID("bg-spr");
    bgSpr->setAnchorPoint({0, 1});
    bgSpr->setOpacity(bgOpacity);
    bgSpr->setVisible(false);
    this->addChild(bgSpr);

    topBorder1 = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    topBorder1->setID("top-border-1");
    topBorder1->setAnchorPoint({0, 1});
    topBorder1->setColor({0,0,0});
    this->addChild(topBorder1);

    topBorder2 = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    topBorder2->setID("top-border-2");
    topBorder2->setAnchorPoint({1, 1});
    topBorder2->setColor({0,0,0});
    this->addChild(topBorder2);

    topBorder3 = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    topBorder3->setID("top-border-3");
    topBorder3->setAnchorPoint({0, 0});
    topBorder3->setColor({0,0,0});
    this->addChild(topBorder3);

    bgSideBorder = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    bgSideBorder->setID("bg-side-border");
    bgSideBorder->setAnchorPoint({1, 1});
    bgSideBorder->setColor({0,0,0});
    bgSideBorder->setOpacity(bgOpacity);
    this->addChild(bgSideBorder);

    menu = CCMenu::create();
    menu->setPosition({0, 0});
    menu->setEnabled(startVisible);
    this->addChild(menu);

    auto addLabelBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    //addLabelBtnSpr->setScale(.5f);
    auto addLabelBtn = CCMenuItemSpriteExtra::create(
        addLabelBtnSpr,
        this,
        menu_selector(LayoutColumn::onAddLabelBtnClicked)
    );
    menu->addChild(addLabelBtn);

    this->setAnchorPoint({0, 1});
    this->ignoreAnchorPointForPosition(false);
    this->setContentHeight(minHeight);
    this->setContentWidth(info.currentWidth);
    
    updateSizesByContent();

    onOrganized(0);

    DTLayer::get()->subscribeToOrganizationEvent(this, std::bind(&LayoutColumn::onOrganized, this, std::placeholders::_1));

    this->scheduleUpdate();

    setColor(info.color);

    auto touchSender = TouchSender::create(-502, false);
    touchSender->setID("touchSender");
    touchSender->beginCheck = [&](cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        if (!isVisible) return false;
        auto touchInParentSpace = topBorder2->getParent()->convertTouchToNodeSpace(touch);

        if (topSpr->boundingBox().containsPoint(touchInParentSpace)) {
            isTouchingTopSpr = true;
            originalTopSprTouchPos = touchInParentSpace;
            return true;
        }

        isTouchingTopSpr = false;

        return false;
    };
    touchSender->onTouch = [&](TouchSenderState state, cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
        if (state == TouchSenderState::Moved) {
            if (isTouchingTopSpr) {
                auto touchInParentSpace = topBorder2->getParent()->convertTouchToNodeSpace(touch);
        
                auto distance = (originalTopSprTouchPos - touchInParentSpace).getLength();

                if (distance > maxMovementOffset)
                    isTouchingTopSpr = false;
            }
        }
        else if (state == TouchSenderState::Cancled) {
            isTouchingTopSpr = false;
        }
        else if (state == TouchSenderState::Ended) {
            if (isTouchingTopSpr) {
                DTLayer::get()->setOptionsLayerTo(this);
            }
            isTouchingTopSpr = false;
        }
    };
    this->addChild(touchSender);

    if (!startVisible) {
        for (const auto& child : CCArrayExt<CCNode*>(this->getChildren()))
        {
            if (auto rgbChild = typeinfo_cast<CCRGBAProtocol*>(child))
                rgbChild->setOpacity(0);
        }
    }
    isVisible = startVisible;

    return true;
}

void LayoutColumn::updateSizesByContent() {
    if (this->getContentHeight() < minHeight) this->setContentHeight(minHeight);
    
    topSpr->setContentSize({this->getContentWidth(), topHeight});
    topSpr->setPosition({0, this->getContentHeight()});

    topBorder1->setContentSize({topSpr->getContentWidth(), borderWidth});
    topBorder1->setPosition(topSpr->getPosition());

    topBorder2->setContentSize({borderWidth, topSpr->getContentHeight()});
    topBorder2->setPosition(topSpr->getPosition() + ccp(topSpr->getContentWidth(), 0));

    topBorder3->setContentSize({topSpr->getContentWidth(), borderWidth});
    topBorder3->setPosition(topSpr->getPosition() - ccp(0, topSpr->getContentHeight()));

    bgSpr->setContentSize({this->getContentWidth(), this->getContentHeight() - topHeight});
    bgSpr->setPosition({0, bgSpr->getContentHeight()});
    
    bgSideBorder->setContentSize({borderWidth, bgSpr->getContentHeight()});
    bgSideBorder->setPosition(topBorder2->getPosition() - ccp(0, topBorder2->getContentHeight()));
}

void LayoutColumn::update(float dt) {
    DTLabel* lowestLabel = nullptr;

    auto labelsTemp = labels;
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
        targetAddBtnPos = bgSpr->getPosition() + ccp(0, -addNewBtnOffset);
    }

    targetAddBtnPos.x = this->getContentWidth() / 2;

    //log::info("plus button for column {}, {} | {}", orderPos, targetAddBtnPos, tempMenuPos);

    if (!targetAddBtnPos.equals(tempMenuPos)) {
        tempMenuPos = targetAddBtnPos;
        menu->stopActionByTag(1);
        auto btnMoveAction = CCEaseInOut::create(CCMoveTo::create(.5f, targetAddBtnPos), 2);
        btnMoveAction->setTag(1);
        menu->runAction(btnMoveAction);
    }
}

void LayoutColumn::setContentHeight(float height) {
    CCMenu::setContentHeight(height);
    updateSizesByContent();
}

void LayoutColumn::setContentWidth(float width) {
    CCMenu::setContentWidth(width);

    info.currentWidth = width;
}

void LayoutColumn::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -502, true);
}

bool LayoutColumn::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    if (!isVisible) return false;
    auto touchInParentSpace = topBorder2->getParent()->convertTouchToNodeSpace(touch);

    if (topBorder2->boundingBox().containsPoint(touchInParentSpace)) {
        isTouchingSide = true;
        
        return true;
    }

    isTouchingSide = false;
    return false;
}

void LayoutColumn::ccTouchEnded(CCTouch*, CCEvent*) {
    isTouchingSide = false;

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::ccTouchCancelled(CCTouch*, CCEvent*) {
    isTouchingSide = false;

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (isTouchingSide) {
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
    topSpr->setColor(color);
    bgSpr->setColor(color);

    info.color = color;
}

void LayoutColumn::addLabel(DTLabel* label) {
    label->addColumnAsHolder(this);
    if (labels.contains(label->info.layer)) {
        labels[label->info.layer]->moveUpLayer();
    }
    
    labels.insert({label->info.layer, label});
}

void LayoutColumn::removeLabel(DTLabel* label) {
    if (!label->isPartOfColumn(this)) return;
    label->removeColumnAsHolder(this);

    this->labels.erase(label->info.layer);
}

void LayoutColumn::updateLabelPosition(DTLabel* label) {
    if (labels.contains(label->info.layer)) {
        labels[label->info.layer]->moveUpLayer();
    }

    labels.insert({label->info.layer, label});
}

void LayoutColumn::onAddLabelBtnClicked(CCObject*) {
    auto newLabel = DTLayer::get()->createNewLabel();

    int highestLayer = 0;

    for (const auto& [labelLayer, _] : labels)
    {
        highestLayer = std::max(highestLayer, labelLayer);
    }
    newLabel->info.layer = highestLayer + 1;

    addLabel(newLabel);

    DTLayer::get()->organizeLayout();
}

void LayoutColumn::onOrganized(float deltaMove) {
    menu->setPositionY(menu->getPositionY() - deltaMove);
}

void LayoutColumn::destroyColumnAndCleanup() {
    auto labelsTemp = labels;
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
    auto labelsbackup = labels;

    for (const auto& [labelLayer, label] : labels)
    {
        if (!label->isPartOfColumn(this)) continue;
        label->removeColumnAsHolder(this);
    }

    labels.clear();

    for (const auto& [labelLayer, label] : labelsbackup)
    {
        addLabel(label);
    }
}

void LayoutColumn::setVisibility(bool visible) {
    isVisible = visible;
    menu->setEnabled(visible);

    for (const auto& child : CCArrayExt<CCNode*>(this->getChildren()))
    {
        child->stopActionByTag(5);

        int maxOpacity = 255;
        if (child == bgSpr || child == bgSideBorder) maxOpacity = bgOpacity;
        auto opacityAction = CCFadeTo::create(.15f, visible ? maxOpacity : 0);
        opacityAction->setTag(5);
        child->runAction(opacityAction);
    }
}