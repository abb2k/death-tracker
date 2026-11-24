#include "FloatingList.hpp"

FloatingList* FloatingList::create(CCSize size, bool startOpen) {
    auto ret = new FloatingList();
    
    if (ret && ret->init(std::move(size), std::move(startOpen))) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool FloatingList::init(CCSize size, bool startOpen) {
    if (!CCNode::init()) return false;

    this->m_isOpen = std::move(startOpen);

    this->setContentSize(size);

    m_scrollLayer = ScrollLayer::create(size);
    m_scrollLayer->ignoreAnchorPointForPosition(true);
    this->addChild(m_scrollLayer);

    m_scrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
        ->setAutoGrowAxis(m_scrollLayer->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setAxisReverse(true)
        ->ignoreInvisibleChildren(false)
        ->setGap(m_elementOffset)
    );
    m_scrollLayer->setScaleY(m_isOpen ? 1 : 0);
    m_scrollLayer->setVisible(m_isOpen);

    auto listBG = CCScale9Sprite::create("square02_small.png");
    listBG->setContentSize(size);
    listBG->setOpacity(100);
    listBG->setPosition(size / 2);
    m_scrollLayer->addChild(listBG, -1);

    setOpenDirection(true);

    m_scrollLayer->setMouseEnabled(m_isOpen);
    m_scrollLayer->setStealingTouches(true);
    m_scrollLayer->setCancelTouchLimit(0.01f);
    m_scrollLayer->setKeypadEnabled(false);
    m_scrollLayer->setKeyboardEnabled(false);

    m_scrollbar = Scrollbar::create(m_scrollLayer);
    m_scrollbar->ignoreAnchorPointForPosition(false);
    m_scrollbar->setPositionX(m_scrollLayer->getContentWidth() + 4);
    m_scrollbar->setPositionY(m_scrollLayer->getContentHeight() / 2);
    m_scrollbar->setScaleY(0);
    this->addChild(m_scrollbar);

    m_clickArea = CCNode::create();
    m_clickArea->setPosition(m_scrollLayer->getContentSize() / 2);
    m_clickArea->setAnchorPoint({.5f, .5f});
    m_clickArea->setContentSize(size + ccp(
        40,
        40
    ));
    this->addChild(m_clickArea);
    
    m_scrollbar->setTouchEnabled(m_isOpen);

    return true;
}

void FloatingList::addItem(const FloatingListItem& text) {
    for (const auto& [_, item] : m_itemIds) {
        if (item.m_id == text.m_id)
            return;
    }

    float off = 5;

    CCMenuItemSpriteExtra* sideBtn = nullptr;
    if (text.m_sideButtonSprite != nullptr) {
        sideBtn = CCMenuItemSpriteExtra::create(
            text.m_sideButtonSprite,
            this,
            menu_selector(FloatingList::sideBtnClicked)
        );
        sideBtn->setTag(text.m_id);
    }

    auto buttonSpr = ButtonSprite::create(text.m_text.c_str(), text.m_font.c_str(), text.m_BGTexture.c_str());
    float wantedScaleX = (this->getContentWidth() - off - (sideBtn != nullptr ? sideBtn->getContentWidth() : 0)) / buttonSpr->getContentWidth();
    float wantedScaleY = (m_elementHeight - off) / buttonSpr->getContentHeight();
    buttonSpr->setScale(wantedScaleX > wantedScaleY ? wantedScaleY : wantedScaleX);
    auto buttonItem = CCMenuItemSpriteExtra::create(
        buttonSpr,
        this,
        menu_selector(FloatingList::itemClicked)
    );
    buttonItem->setTag(text.m_id);

    auto menu = CCMenu::createWithItem(buttonItem);
    menu->setContentSize({
        this->getContentWidth(),
        m_elementHeight
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
    
    buttonItem->setEnabled(m_isOpen);

    m_scrollLayer->m_contentLayer->addChild(menu);
    updateListLayout();
    m_itemIds[buttonItem] = text;
    if (sideBtn != nullptr) {
        buttonItem->setPositionX(buttonItem->getPositionX() - sideBtn->getContentWidth() / 2 - 1);
        sideBtn->setPosition(buttonItem->getPosition() + ccp(
            buttonItem->getContentWidth() / 2 + sideBtn->getContentWidth() / 2 + 2,
            0
        ));
        m_extraButtonsItemIds[sideBtn] = text;
    }
}
void FloatingList::addItems(const std::vector<FloatingListItem>& texts) {
    for (const auto& text : texts) {
        addItem(text);
    }
}

void FloatingList::removeItem(int id) {
    CCMenuItemSpriteExtra* element = nullptr;
    for (const auto& [_, item] : m_itemIds) {
        if (item.m_id == id) {
            element = _;
            break;
        }
    }

    if (element == nullptr) return;

    CCMenuItemSpriteExtra* extraToRemove = nullptr;

    for (const auto& [item, _] : m_extraButtonsItemIds) {
        if (_.m_id == id) {
            extraToRemove = item;
            break;
        }
    }

    if (extraToRemove != nullptr) {
        m_extraButtonsItemIds.erase(extraToRemove);
    }

    m_itemIds.erase(element);

    element->getParent()->removeMeAndCleanup();
    updateListLayout();
}

void FloatingList::clearAllItems() {
    std::set<int> ids{};

    for (const auto& [_, item] : m_itemIds) {
        ids.insert(item.m_id);
    }

    for (const auto& id : ids)
    {
        removeItem(id);
    }
}

void FloatingList::open() {
    m_isOpen = true;

    m_scrollLayer->moveToTop();

    m_scrollLayer->stopAllActions();
    m_scrollLayer->setVisible(true);
    m_scrollLayer->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 1), 2));
    
    m_scrollbar->stopAllActions();
    m_scrollbar->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 1), 2));

    setItemEnabled(true);

    m_scrollLayer->setMouseEnabled(m_isOpen);
    m_scrollbar->setTouchEnabled(m_isOpen);
}

void FloatingList::close() {
    m_isOpen = false;

    m_scrollLayer->stopAllActions();
    m_scrollLayer->runAction(CCSequence::create(
        CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 0), 2),
        CCHide::create(),
        nullptr
    ));
    
    m_scrollbar->stopAllActions();
    m_scrollbar->runAction(CCEaseInOut::create(CCScaleTo::create(0.1f, 1, 0), 2));

    setItemEnabled(false);

    m_scrollLayer->setMouseEnabled(m_isOpen);
    m_scrollbar->setTouchEnabled(m_isOpen);
}

void FloatingList::setItemEnabled(bool isEnabled) {
    for (const auto& [item, _] : m_itemIds) {
        item->setEnabled(isEnabled);
    }
    for (const auto& [item, _] : m_extraButtonsItemIds) {
        item->setEnabled(isEnabled);
    }
}

void FloatingList::setCallback(Function<void(const int& id)> callback) {
    m_onItemClicked = std::move(callback);
}

void FloatingList::itemClicked(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!m_itemIds.contains(btn)) return;

    if (m_onItemClicked) m_onItemClicked(m_itemIds[btn].m_id);
}

void FloatingList::sideBtnClicked(CCObject* sender) {
    auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    if (!m_extraButtonsItemIds.contains(btn)) return;

    m_extraButtonsItemIds[btn].m_sideButtonCallback(m_extraButtonsItemIds[btn].m_id);
}

void FloatingList::setOpenDirection(bool openUpwards) {
    if (openUpwards) {
        m_scrollLayer->setAnchorPoint({0.5f, 0});
        m_scrollLayer->setPositionY(0);
    }
    else {
        m_scrollLayer->setAnchorPoint({0.5f, 1});
        m_scrollLayer->setPositionY(this->getContentHeight());
    }
}

std::optional<FloatingListItem> FloatingList::getItemForID(int id) {
    for (const auto& [btn, item] : m_itemIds)
    {
        if (btn->getTag() == id)
            return item;
    }
    
    return std::nullopt;
}

bool FloatingList::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    auto area = m_clickArea->convertTouchToNodeSpace(pTouch);

    if (m_isOpen) {
        if (!(area.x >= 0 && 
            area.y >= 0 && 
            area.x <= m_clickArea->getContentWidth() &&
            area.y <= m_clickArea->getContentHeight()
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
    for (const auto& [btn, _] : m_itemIds)
    {
        btn->setEnabled(b);
    }
    for (const auto& [btn, _] : m_extraButtonsItemIds)
    {
        btn->setEnabled(b);
    }
}

void FloatingList::updateListLayout() {
    std::set<CCNode*> invisNodes{};

    for (const auto& child : m_scrollLayer->m_contentLayer->getChildrenExt<CCNode*>())
    {
        if (!child->isVisible())
            invisNodes.insert(child);
        child->setVisible(true);
    }

    m_scrollLayer->m_contentLayer->updateLayout();

    for (const auto& invis : invisNodes)
    {
        invis->setVisible(false);
    }
}

void FloatingList::setListOpenAnchor(CCPoint const& anchor) {
    m_scrollLayer->setAnchorPoint(anchor);
}