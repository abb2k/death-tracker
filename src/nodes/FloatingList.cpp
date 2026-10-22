#include "FloatingList.hpp"

FloatingList* FloatingList::create(CCSize const& size, bool startOpen) {
    auto ret = new FloatingList();
    if (ret && ret->init(size, startOpen)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool FloatingList::init(CCSize const& size, bool startOpen) {
    if (!CCNode::init()) return false;

    this->isOpen = startOpen;

    this->setContentSize(size);

    scrollLayer = ScrollLayer::create(size);
    this->addChild(scrollLayer);

    scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(scrollLayer->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
        ->ignoreInvisibleChildren(false)
        ->setGap(elementOffset)
    );
    scrollLayer->setScaleY(startOpen ? 1 : 0);
    scrollLayer->setVisible(startOpen);

    auto listBG = CCScale9Sprite::create("square02_small.png");
    listBG->setContentSize(size);
    listBG->setOpacity(100);
    listBG->setPosition(size / 2);
    scrollLayer->addChild(listBG, -1);

    setOpenDirection(true);

    scrollLayer->setMouseEnabled(startOpen);
    scrollLayer->setStealingTouches(true);
    scrollLayer->setCancelTouchLimit(0.01f);
    scrollLayer->setKeypadEnabled(false);
    scrollLayer->setKeyboardEnabled(false);

    scrollbar = Scrollbar::create(scrollLayer);
    scrollbar->ignoreAnchorPointForPosition(false);
    scrollbar->setPositionX(scrollLayer->getContentWidth() + 4);
    scrollbar->setPositionY(scrollLayer->getContentHeight() / 2);
    scrollbar->setScaleY(0);
    this->addChild(scrollbar);

    clickArea = CCNode::create();
    clickArea->setPosition(scrollLayer->getContentSize() / 2);
    clickArea->setAnchorPoint({.5f, .5f});
    clickArea->setContentSize(size + ccp(
        40,
        40
    ));
    this->addChild(clickArea);
    
    scrollbar->setTouchEnabled(startOpen);

    return true;
}

void FloatingList::addItem(const FloatingListItem& text) {
    for (const auto& [_, item] : itemIds) {
        if (item.id == text.id)
            return;
    }

    float off = 5;

    CCMenuItemSpriteExtra* sideBtn = nullptr;
    if (text.sideButtonSprite != nullptr) {
        sideBtn = CCMenuItemSpriteExtra::create(
            text.sideButtonSprite,
            this,
            menu_selector(FloatingList::sideBtnClicked)
        );
        sideBtn->setTag(text.id);
    }

    auto buttonSpr = ButtonSprite::create(text.text.c_str(), text.font.c_str(), text.BGTexture.c_str());
    float wantedScaleX = (this->getContentWidth() - off - (sideBtn != nullptr ? sideBtn->getContentWidth() : 0)) / buttonSpr->getContentWidth();
    float wantedScaleY = (elementHeight - off) / buttonSpr->getContentHeight();
    buttonSpr->setScale(wantedScaleX > wantedScaleY ? wantedScaleY : wantedScaleX);
    auto buttonItem = CCMenuItemSpriteExtra::create(
        buttonSpr,
        this,
        menu_selector(FloatingList::itemClicked)
    );
    buttonItem->setTag(text.id);

    auto menu = CCMenu::createWithItem(buttonItem);
    menu->setContentSize({
        this->getContentWidth(),
        elementHeight
    });
    buttonItem->setPosition(menu->getContentSize() / 2);

    if (sideBtn != nullptr) {
        menu->addChild(sideBtn);
    }

    auto itemBG = CCScale9Sprite::create("square02_small.png");
    itemBG->setScale(.75f);
    itemBG->setContentSize(menu->getContentSize() / itemBG->getScale());
    itemBG->setOpacity(150);
    itemBG->setPosition(menu->getContentSize() / 2);
    menu->addChild(itemBG, -1);
    
    buttonItem->setEnabled(isOpen);

    scrollLayer->m_contentLayer->addChild(menu);
    updateListLayout();
    itemIds[buttonItem] = text;
    if (sideBtn != nullptr) {
        buttonItem->setPositionX(buttonItem->getPositionX() - sideBtn->getContentWidth() / 2 - 1);
        sideBtn->setPosition(buttonItem->getPosition() + ccp(
            buttonItem->getContentWidth() / 2 + sideBtn->getContentWidth() / 2 + 2,
            0
        ));
        extraButtonsItemIds[sideBtn] = text;
    }
}
void FloatingList::addItems(const std::vector<FloatingListItem>& texts) {
    for (const auto& text : texts) {
        addItem(text);
    }
}

void FloatingList::removeItem(int id) {
    CCMenuItemSpriteExtra* element = nullptr;
    for (const auto& [_, item] : itemIds) {
        if (item.id == id) {
            element = _;
            break;
        }
    }

    if (element == nullptr) return;

    CCMenuItemSpriteExtra* extraToRemove = nullptr;

    for (const auto& [item, _] : extraButtonsItemIds) {
        if (_.id == id) {
            extraToRemove = item;
            break;
        }
    }

    if (extraToRemove != nullptr) {
        extraButtonsItemIds.erase(extraToRemove);
    }

    itemIds.erase(element);

    element->getParent()->removeMeAndCleanup();
    updateListLayout();
}

void FloatingList::clearAllItems() {
    std::set<int> ids{};

    for (const auto& [_, item] : itemIds) {
        ids.insert(item.id);
    }

    for (const auto& id : ids)
    {
        removeItem(id);
    }
    
}

void FloatingList::open() {
    isOpen = true;

    scrollLayer->moveToTop();

    scrollLayer->stopAllActions();
    scrollLayer->setVisible(true);
    scrollLayer->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 1), 2));
    
    scrollbar->stopAllActions();
    scrollbar->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 1), 2));

    setItemEnabled(true);

    scrollLayer->setMouseEnabled(isOpen);
    scrollbar->setTouchEnabled(isOpen);
}

void FloatingList::close() {
    isOpen = false;

    scrollLayer->stopAllActions();
    scrollLayer->runAction(CCSequence::create(
        CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 0), 2),
        CCHide::create(),
        nullptr
    ));
    
    scrollbar->stopAllActions();
    scrollbar->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 0), 2));

    setItemEnabled(false);

    scrollLayer->setMouseEnabled(isOpen);
    scrollbar->setTouchEnabled(isOpen);
}

void FloatingList::setItemEnabled(bool isEnabled) {
    for (const auto& [item, _] : itemIds) {
        item->setEnabled(isEnabled);
    }
    for (const auto& [item, _] : extraButtonsItemIds) {
        item->setEnabled(isEnabled);
    }
}

void FloatingList::setCallback(geode::Function<void(const int& id)> callback) {
    onItemClicked = std::move(callback);
}

void FloatingList::itemClicked(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!itemIds.contains(btn)) return;

    if (onItemClicked) onItemClicked(itemIds[btn].id);
}

void FloatingList::sideBtnClicked(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!extraButtonsItemIds.contains(btn)) return;

    extraButtonsItemIds[btn].sideButtonCallback(extraButtonsItemIds[btn].id);
}

void FloatingList::setOpenDirection(bool openUpwards) {
    if (openUpwards) {
        scrollLayer->setAnchorPoint({0.5f, 0});
        scrollLayer->setPositionY(0);
    }
    else {
        scrollLayer->setAnchorPoint({0.5f, 1});
        scrollLayer->setPositionY(this->getContentHeight());
    }
}

std::optional<FloatingListItem> FloatingList::getItemForID(int id) {
    for (const auto& [btn, item] : itemIds)
    {
        if (btn->getTag() == id)
            return item;
    }
    
    return std::nullopt;
}

bool FloatingList::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    auto area = clickArea->convertTouchToNodeSpace(pTouch);

    if (isOpen) {
        if (!(area.x >= 0 && 
            area.y >= 0 && 
            area.x <= clickArea->getContentWidth() && 
            area.y <= clickArea->getContentHeight()
        )) {
            close();
        }
    }
    
    return false;
}

void FloatingList::onEnter() {
    CCNode::onEnter();
    CCTouchDispatcher::get()->addPrioTargetedDelegate(this, 0, false);
}

void FloatingList::onExit() {
    CCNode::onExit();
    CCTouchDispatcher::get()->removeDelegate(this);
}

void FloatingList::setEnabled(bool b) {
    for (const auto& [btn, _] : itemIds)
    {
        btn->setEnabled(b);
    }
    for (const auto& [btn, _] : extraButtonsItemIds)
    {
        btn->setEnabled(b);
    }
}

void FloatingList::updateListLayout() {
    std::set<CCNode*> invisNodes{};

    for (const auto& child : scrollLayer->m_contentLayer->getChildrenExt<CCNode*>())
    {
        if (!child->isVisible())
            invisNodes.insert(child);
        child->setVisible(true);
    }
    

    scrollLayer->m_contentLayer->updateLayout();

    for (const auto& invis : invisNodes)
    {
        invis->setVisible(false);
    }
    
}

void FloatingList::setListOpenAnchor(CCPoint const& anchor) {
    scrollLayer->setAnchorPoint(anchor);
}