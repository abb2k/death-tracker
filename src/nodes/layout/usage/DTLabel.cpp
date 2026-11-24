#include "DTLabel.hpp"

#include <nodes/layout/usage/LayoutColumn.hpp>
#include <nodes/DTLayer.hpp>

#include <actions/CCResizeHeightTo.hpp>
#include <actions/CCResizeWidthTo.hpp>
#include <actions/CCTextAreaTintTo.hpp>
#include <actions/CCTextAreaFadeTo.hpp>

#include <regex>
#include <typeinfo>
#include <arc/task/Yield.hpp>

#include <managers/AuthManager.hpp>

#include <nodes/layout/usage/DecimalPopup.hpp>
#include <utils/Settings.hpp>

#include <utils/RichTextKeys.hpp>

#include <nodes/layout/usage/SpecialKey.hpp>

#include <nodes/general/RichTextArea.hpp>

float DTLabel::labelTitleHeight = 15;
float DTLabel::moveThreshold = 5;
float DTLabel::labelLerpSpeed = 10;

bool DTLabel::ColumnComperator::operator()(LayoutColumn* a, LayoutColumn* b) const {
    return a->m_info.m_orderPos < b->m_info.m_orderPos;
}

DTLabel* DTLabel::create(const DTLabelInfo& info) {
    auto ret = new DTLabel();
    if (ret && ret->init(info)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

DTLabel::~DTLabel() {
    if (DTLayer::get() == nullptr) return;

    DTLayer::get()->unsubscribeToOrganizationEvent(this);
    DTLayer::get()->unsubscribeKeyListener(this);
}

bool DTLabel::init(const DTLabelInfo& info) {
    if (DTLayer::get() != nullptr) {
        DTLayer::get()->subscribeToOrganizationEvent(this, [&](auto _) {
            m_bg->stopAllActions();
            m_bg->runAction(CCEaseInOut::create(CCResizeHeightTo::create(DTLayer::transitionTime, this->getContentHeight() / m_bg->getScale()), 2));
        });
        DTLayer::get()->subscribeKeyListener(this);
    }
    if (!CCMenu::init()) return false;

    this->setCascadeOpacityEnabled(true);

    this->m_info = info;
    m_donerRichTextEnabled = AuthManager::isDoner();

    this->setAnchorPoint({0, 1});

    auto circleLayer = CCLayer::create();
    circleLayer->setID("circle-layer");
    circleLayer->setZOrder(5);
    this->addChild(circleLayer);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(circleLayer);
    m_loadingCircle->show();
    m_loadingCircle->ignoreAnchorPointForPosition(false);
    m_loadingCircle->setVisible(false);
    m_loadingCircle->setScale(.4f);

    m_labelTextContainer = CCNode::create();
    m_labelTextContainer->setID("text-container");
    m_labelTextContainer->setZOrder(1);
    this->addChild(m_labelTextContainer);

    m_labelText = RichTextArea::create("", info.m_font, info.m_scale);
    m_labelText->setID("text");
    m_labelText->setAnchorPoint({.5f, 1});
    m_labelText->setAlignment(info.m_horizontalAlignment);
    m_labelText->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    m_labelText->setWidth(this->getContentWidth() - m_textCornerOffset);
    setLabelText(info.m_text);
    m_labelTextContainer->addChild(m_labelText);
    
    m_labelText->registerRichTextKey(RichTextKeys::makeWaveKey([this](){ return m_donerRichTextEnabled; }));
    
    m_labelText->registerRichTextKey(RichTextKeys::makeGradientKey([this](){ return m_donerRichTextEnabled; }));
    
    m_labelText->registerRichTextKey(RichTextKeys::makeCPlusKey());

    m_labelText->registerRichTextKey(RichTextKeys::makeRunMoreKey(
        [this]() { return this->m_info.m_font; },
        [this]() { return m_runMoreButtonsVisible; },
        [this](CCMenuItemSpriteExtra* sender) { onRunMoreClicked(sender); }
    ));

    auto textHeight = 0.0f;
    if (!info.m_isExpanded) textHeight = 0;
    else {
        textHeight = m_labelText->getContentHeight() + m_textCornerOffset;
    }

    this->setContentHeight(textHeight + getSpecificTitleHeight());
    this->setContentWidth(0);
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    m_labelTextContainer->setScale(info.m_isExpanded ? 1 : 0);

    m_bg = CCScale9Sprite::create("GJ_squareB_01.png");
    m_bg->setID("text-bg");
    m_bg->setOpacity(m_normalBGOpacity * (info.m_labelColor.a / 255.0f));
    m_bg->setScale(.2f);
    m_bg->setAnchorPoint({0, 1});
    m_bg->setContentHeight(getSpecificTitleHeight() / m_bg->getScale());
    if (info.m_isExpanded) m_bg->setContentHeight(this->getContentHeight() / m_bg->getScale());
    this->addChild(m_bg);
    
    m_labelTitleBG = CCScale9Sprite::create("GJ_squareB_01.png");
    m_labelTitleBG->setID("title-bg");
    m_labelTitleBG->setScale(.15f);
    m_labelTitleBG->setAnchorPoint({0, 1});
    m_labelTitleBG->setContentHeight(labelTitleHeight / m_labelTitleBG->getScale());
    m_labelTitleBG->setOpacity(info.m_hideHeader ? 0 : 255);
    this->addChild(m_labelTitleBG);

    m_hideBGSpr = CCSprite::createWithSpriteFrameName("hideBtn_001.png");
    m_hideBGSpr->setPositionY(m_labelTitleBG->getContentHeight() / 2);
    m_hideBGSpr->setAnchorPoint({1, .5f});
    m_hideBGSpr->setScale(1 / m_labelTitleBG->getScale() / 2);
    m_hideBGSpr->setVisible(info.m_hideHeader);
    m_labelTitleBG->addChild(m_hideBGSpr);

    m_labelTitleArea = RichTextArea::create(info.m_labelName, "bigFont.fnt", .35f);
    m_labelTitleArea->setID("title-text");
    m_labelTitleArea->setAnchorPoint({.5f, 1});
    m_labelTitleArea->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_labelTitleArea->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    m_labelTitleArea->setMaxLines(1);
    m_labelTitleArea->setVisible(!info.m_hideHeader);
    
    m_labelTitleArea->registerRichTextKey(RichTextKeys::makeWaveKey([&](){ return m_donerRichTextEnabled; }));
    m_labelTitleArea->registerRichTextKey(RichTextKeys::makeGradientKey([&](){ return m_donerRichTextEnabled; }));
    m_labelTitleArea->registerRichTextKey(RichTextKeys::makeCPlusKey());

    m_labelTitleArea->updateContent();

    this->addChild(m_labelTitleArea);

    float expandLineWidth = 3;

    m_leftExpandLine = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_leftExpandLine->setID("expand-line-left");
    m_leftExpandLine->setAnchorPoint({0, 1});
    m_leftExpandLine->setContentWidth(expandLineWidth);
    m_leftExpandLine->setContentHeight(labelTitleHeight);
    m_leftExpandLine->setVisible(false);
    this->addChild(m_leftExpandLine);

    m_rightExpandLine = CCScale9Sprite::createWithSpriteFrameName("dtpixel.png"_spr);
    m_rightExpandLine->setID("expand-line-right");
    m_rightExpandLine->setAnchorPoint({1, 1});
    m_rightExpandLine->setContentWidth(expandLineWidth);
    m_rightExpandLine->setContentHeight(labelTitleHeight);
    m_rightExpandLine->setVisible(false);
    this->addChild(m_rightExpandLine);

    m_menu = CCMenu::create();
    m_menu->setID("btn-menu");
    m_menu->setPosition({0,0});
    m_menu->setTouchPriority(-503);
    this->addChild(m_menu);

    auto expandBtnSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    expandBtnSpr->setScale(.20f);
    m_expandBtn = CCMenuItemSpriteExtra::create(
        expandBtnSpr,
        this,
        menu_selector(DTLabel::toggleExpand)
    );
    m_expandBtn->setID("expand-btn");
    m_expandBtn->setPosition({5.0f, -labelTitleHeight / 2});
    m_menu->addChild(m_expandBtn);

    if (info.m_isExpanded) {
        m_expandBtn->setRotation(90);
        m_expandBtn->setPositionX(8);
    }

    m_menu->setEnabled(!info.m_hideHeader);
    m_menu->setOpacity(info.m_hideHeader ? 0 : 255);
    
    this->scheduleUpdate();

    setLabelColor(info.m_labelColor);
    setTextColor(info.m_textColor);
    setTextWrapping(info.m_wrapping);

    m_labelTitleArea->setAlignment(info.m_horizontalAlignment);

    m_glow = CCSprite::create("pixel.png");
    m_glow->setAnchorPoint({0, 0});
    ccBlendFunc bf = { GL_ONE, GL_ONE };
    m_glow->setBlendFunc(bf);
    m_glow->setColor({0, 0, 0});
    m_glow->setZOrder(15);
    this->addChild(m_glow);

    return true;
}

void DTLabel::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addPrioTargetedDelegate(this, -505, true);
}

void DTLabel::update(float dt) {

    m_labelTextContainer->setPosition(m_bg->getPosition() + ccp(this->getContentWidth() / 2, -getSpecificTitleHeight()));
    if (m_labelText->getWidth() != this->getContentWidth() - m_textCornerOffset && m_info.m_isExpanded) {
        m_labelText->setWidth(this->getContentWidth() - m_textCornerOffset);
        m_labelText->setWidth(m_labelText->getWidth());
    }
    if (m_labelText->getWidth() != this->getContentWidth() - m_textCornerOffset && m_info.m_isExpanded) {
        m_labelText->setWidth(this->getContentWidth() - m_textCornerOffset);
        m_labelText->setWidth(m_labelText->getWidth());
    }

    m_glow->setScaleX(this->getScaledContentWidth() / m_glow->getContentWidth());
    m_glow->setScaleY(this->getScaledContentHeight() / m_glow->getContentHeight());

    auto textHeight = 0.0f;
    if (!m_info.m_isExpanded) textHeight = 0;
    else textHeight = m_labelText->getContentHeight() + m_textCornerOffset;

    float targetHeight = textHeight + getSpecificTitleHeight();
    bool doUpdateLayout = false;

    if (targetHeight != this->getContentHeight()) doUpdateLayout = true;

    this->setContentHeight(targetHeight);

    if (doUpdateLayout && DTLayer::get() && !DTLayer::get()->m_cornerOnNextOrganization) {
        DTLayer::get()->organizeLayout();
    }

    m_labelTitleBG->setContentWidth(this->getContentWidth() / m_labelTitleBG->getScale());
    m_bg->setContentWidth(this->getContentWidth() / m_bg->getScale());
    m_labelTitleBG->setPositionY(this->getContentHeight());
    m_bg->setPositionY(this->getContentHeight());
    m_menu->setPositionY(this->getContentHeight());

    m_loadingCircle->setPosition({this->getContentWidth() / 2, this->getContentHeight() - getSpecificTitleHeight() / 2});

    auto titleWidth = this->getContentWidth() - m_expandBtn->getScaledContentHeight() * 2 - 5;
    if (std::abs(m_labelTitleArea->getWidth() - titleWidth) > 0.01f) {
        m_labelTitleArea->setWidth(titleWidth);
        m_labelTitleArea->setWidth(m_labelTitleArea->getWidth());
    }

    m_labelTitleArea->setPositionY(this->getContentHeight());
    m_labelTitleArea->setPositionX(this->getContentWidth() / 2 + (
        m_labelTitleArea->getAlignment() == CCTextAlignment::kCCTextAlignmentRight && !m_info.m_hideHeader ?
            m_expandBtn->getScaledContentHeight() :
            0
    ));

    m_leftExpandLine->setPositionY(this->getContentHeight());

    m_rightExpandLine->setPositionY(this->getContentHeight());
    m_rightExpandLine->setPositionX(this->getContentWidth());

    m_hideBGSpr->setPositionX(m_labelTitleBG->getContentWidth() - 5);
    m_hideBGSpr->setOpacity(m_labelTitleBG->getOpacity());

    if (m_isMovingLabel)
        onMoveUpdate(dt);

    auto mousePos = this->convertToNodeSpace(getMousePos());

    if (m_isEditable && !m_isBeingEdited && m_labelTitleBG->boundingBox().containsPoint(mousePos) && !m_isHovering) {
        m_isHovering = true;
        m_glow->stopAllActions();
        m_glow->runAction(CCTintTo::create(.1f, 20, 20, 20));
    }
    else if (m_isHovering && !m_labelTitleBG->boundingBox().containsPoint(mousePos) && !m_isBeingEdited) {
        m_isHovering = false;
        m_glow->stopAllActions();
        m_glow->runAction(CCTintTo::create(.1f, 0, 0, 0));
    }
    
    if (m_leftExpandLine->boundingBox().containsPoint(mousePos) && !m_currentlyExpandingLeft && m_isEditable) {
        m_leftExpandLine->setVisible(true);
        m_leftExpandLine->setOpacity(150);
    }
    else if (!m_currentlyExpandingLeft) {
        m_leftExpandLine->setVisible(false);
    }

    if (m_rightExpandLine->boundingBox().containsPoint(mousePos) && !m_currentlyExpandingRight && m_isEditable) {
        m_rightExpandLine->setVisible(true);
        m_rightExpandLine->setOpacity(150);
    }
    else if (!m_currentlyExpandingRight) {
        m_rightExpandLine->setVisible(false);
    }
}


void DTLabel::moveUpLayer() {
    for (const auto& column : m_holders)
        column->m_labels.erase(m_info.m_layer);
    m_info.m_layer++;
    for (const auto& column : m_holders)
    {
        column->updateLabelPosition(this);
    }
}

void DTLabel::toggleExpand(CCObject*) {
    if (!m_isExpandable) return;

    m_info.m_isExpanded = !m_info.m_isExpanded;

    m_expandBtn->stopAllActions();
    m_expandBtn->runAction(
        CCEaseInOut::create(
            CCRotateTo::create(
                DTLayer::transitionTime, 
                m_info.m_isExpanded ? 90 : 0
            ), 
            2
        )
    );
    m_expandBtn->runAction(
        CCEaseInOut::create(
            CCMoveTo::create(
                DTLayer::transitionTime, 
                m_info.m_isExpanded ? ccp(8, m_expandBtn->getPositionY()) : ccp(5, m_expandBtn->getPositionY())
            ),
            2
        )
    );

    m_labelTextContainer->stopAllActions();
    m_labelTextContainer->runAction(
        CCEaseInOut::create(
            CCScaleTo::create(
                DTLayer::transitionTime, 
                m_info.m_isExpanded ? 1 : 0
            ), 
            2
        )
    );

    if (!DTLayer::get()->m_isEditingLayout)
        DTLayer::get()->saveCurrentLayout();
}

bool DTLabel::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    auto touchInSpace = this->convertTouchToNodeSpace(touch);
    if (!m_isEditable) {
        if (m_labelTitleBG->boundingBox().containsPoint(touchInSpace)) {

            if (m_labelTitleBG->getActionByTag(1) != nullptr) {
                clipboard::write(m_labelText->getText());
                Notification::create(fmt::format("Copied text from {}", m_info.m_labelName), NotificationIcon::Success)->show();
                return false;
            }

            auto timerSeq = CCDelayTime::create(m_doubleClickTimer);
            timerSeq->setTag(1);
            
            m_labelTitleBG->runAction(timerSeq);
        }

        return false;
    }

    if (m_leftExpandLine->boundingBox().containsPoint(touchInSpace)) {
        m_leftExpandLine->setVisible(true);
        m_leftExpandLine->setOpacity(255);
        m_currentlyExpandingLeft = true;

        return true;
    }
    else if (m_rightExpandLine->boundingBox().containsPoint(touchInSpace)) {
        m_rightExpandLine->setVisible(true);
        m_rightExpandLine->setOpacity(255);
        m_currentlyExpandingRight = true;

        return true;
    }
    else if (m_labelTitleBG->boundingBox().containsPoint(touchInSpace)) {
        m_touchStartPoint = touch->getLocation();
        m_isBeingTouched = true;

        m_currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);

        return true;
    }

    return false;
}

void DTLabel::ccTouchEnded(CCTouch* touch, CCEvent*) {
    if (m_isBeingTouched) {
        if (!m_isMovingLabel) {
            auto touchInSpace = m_labelTitleBG->getParent()->convertTouchToNodeSpace(touch);

            if (m_labelTitleBG->boundingBox().containsPoint(touchInSpace)) {
                onSettings();
            }
        }
        else {
            m_currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);
            onMoveEnded();
        }
    }

    if (m_currentlyExpandingLeft) {
        m_leftExpandLine->setVisible(false);
    }
    if (m_currentlyExpandingRight) {
        m_rightExpandLine->setVisible(false);
    }

    m_isMovingLabel = false;
    m_isBeingTouched = false;
    m_currentlyExpandingLeft = false;
    m_currentlyExpandingRight = false;
}

void DTLabel::ccTouchCancelled(CCTouch* touch, CCEvent*) {
    if (m_isBeingTouched) {
        if (m_isMovingLabel) {
            m_currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);
            onMoveEnded();
        }
    }

    if (m_currentlyExpandingLeft) {
        m_leftExpandLine->setVisible(false);
    }
    if (m_currentlyExpandingRight) {
        m_rightExpandLine->setVisible(false);
    }

    m_isMovingLabel = false;
    m_isBeingTouched = false;
    m_currentlyExpandingLeft = false;
    m_currentlyExpandingRight = false;
}

void DTLabel::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (m_isBeingTouched) {
        if (!m_isMovingLabel) {
            auto touchCurrentLocation = touch->getLocation();
        
            if (std::abs((m_touchStartPoint - touchCurrentLocation).getLength()) >= moveThreshold) {
                m_isMovingLabel = true;
                onMoveBegan();
            }
        }
        else m_currentTouchPosition = this->getParent()->convertTouchToNodeSpace(touch);
    }

    if (m_currentlyExpandingLeft) {
        auto worldPos = this->convertToWorldSpace(ccp(this->getContentWidth(), 0));
        auto touchPos = touch->getLocation();

        if (touchPos.x >= worldPos.x) return;

        auto columns = DTLayer::get()->getColumnsBetween(worldPos, touchPos);

        // log::info("found {} columns", columns.size());

        int oldColumnsSize = m_holders.size();

        removeFromColumns();

        for (const auto& column : columns)
        {
            column->addLabel(this);
        }

        if (m_holders.size() != oldColumnsSize)
            DTLayer::get()->organizeLayout();
    }

    if (m_currentlyExpandingRight) {
        auto worldPos = this->convertToWorldSpace(ccp(5, 0));
        auto touchPos = touch->getLocation();

        if (touchPos.x <= worldPos.x) return;

        auto columns = DTLayer::get()->getColumnsBetween(worldPos, touchPos);

        // log::info("found {} columns", columns.size());

        int oldColumnsSize = m_holders.size();

        removeFromColumns();

        for (const auto& column : columns)
        {
            column->addLabel(this);
        }

        if (m_holders.size() != oldColumnsSize)
            DTLayer::get()->organizeLayout();
    }
}

void DTLabel::onSettings() {
    DTLayer::get()->setOptionsLayerTo(this);
}

void DTLabel::onMoveBegan() {
    m_holdersSave = m_holders;

    removeFromColumns();

    this->runAction(
        CCEaseInOut::create(
            CCResizeWidthTo::create(
                DTLayer::transitionTime, 
                DTColumnInfo::minWidth
            ), 
            2
        )
    );
    if (m_info.m_isExpanded) toggleExpand(nullptr);
    else DTLayer::get()->organizeLayout();
}

void DTLabel::onMoveUpdate(float dt) {
    auto targetPosition = m_currentTouchPosition + ccp(-this->getContentWidth(), this->getContentHeight()) / 2;
    CCPoint currentLerp;
    currentLerp.x = std::lerp(this->getPositionX(), targetPosition.x, dt * DTLabel::labelLerpSpeed);
    currentLerp.y = std::lerp(this->getPositionY(), targetPosition.y, dt * DTLabel::labelLerpSpeed);
    this->setPosition(currentLerp);
}

void DTLabel::onMoveEnded() {
    auto res = DTLayer::get()->getColumnLayerFromPosition(this->getParent()->convertToWorldSpace(m_currentTouchPosition + ccp(-this->getContentWidth(), this->getContentHeight()) / 2));

    if (res.first == nullptr) {
        for (const auto& container : m_holdersSave) {
            container->addLabel(this);
        }

        DTLayer::get()->organizeLayout();
    }
    else {
        this->m_info.m_layer = res.second;
        res.first->addLabel(this);

        DTLayer::get()->organizeLayout();
    }
}

void DTLabel::removeFromColumns() {
    auto holdersTemp = m_holders;
    for (const auto& container : holdersTemp) {
        container->removeLabel(this);
    }
}

void DTLabel::addColumnAsHolder(LayoutColumn* column) {
    m_holders.insert(column);

    updateInfoWithColumnData();
}
void DTLabel::removeColumnAsHolder(LayoutColumn* column) {
    m_holders.erase(column);

    updateInfoWithColumnData();
}

bool DTLabel::isPartOfColumn(LayoutColumn* column) {
    return m_holders.contains(column);
}

bool DTLabel::isAlone() {
    return !m_holders.size();
}

void DTLabel::updateInfoWithColumnData() {
    int min = std::numeric_limits<int>::max();
    int max = std::numeric_limits<int>::min();

    for (const auto& column : m_holders)
    {
        min = std::min(min, column->m_info.m_orderPos);
        max = std::max(max, column->m_info.m_orderPos);
    }

    m_info.m_minPlacementRange = min;
    m_info.m_maxPlacementRange = max;
}

std::set<LayoutColumn*, DTLabel::ColumnComperator> DTLabel::getHolders() {
    return m_holders;
}

void DTLabel::setDonerRichTextEnabled(bool enabled) {
    if (m_donerRichTextEnabled == enabled) return;

    m_donerRichTextEnabled = enabled;
    setLabelText(m_info.m_text);
    m_labelTitleArea->updateContent();
}

void DTLabel::setLabelText(const std::string& text) {
    m_info.m_text = text;
    m_labelText->setText(text);

    auto self = this;
    self->retain();

    m_modifyListener.spawn("DT-text-apply-keys-recursive-task", modifyKeys(), [self](std::optional<std::string> opt) {
        if (!self) return;

        self->m_loadingCircle->setVisible(false);
        if (!opt) {
            self->release();
            return;
        }

        self->m_labelText->setText(opt.value());
        self->release();
    });
}

void DTLabel::setRunMoreButtonsVisible(bool visible, float fadeTime) {
    m_runMoreButtonsVisible = visible;

    auto applyVisibility = [&](auto&& self, CCNode* node) -> void {
        if (auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(node)) {
            button->setEnabled(visible);

            if (auto image = typeinfo_cast<CCScale9Sprite*>(button->getNormalImage())) {
                image->stopActionByTag(m_runMoreButtonFadeActionTag);

                if (fadeTime <= 0) {
                    image->setOpacity(visible ? m_runMoreButtonOpacity : 0);
                }
                else {
                    auto fade = CCFadeTo::create(
                        fadeTime,
                        visible ? m_runMoreButtonOpacity : 0
                    );
                    fade->setTag(m_runMoreButtonFadeActionTag);
                    image->runAction(fade);
                }
            }
        }

        auto children = node->getChildren();
        if (!children) return;

        for (const auto& child : CCArrayExt<CCNode*>(children)) {
            self(self, child);
        }
    };

    applyVisibility(applyVisibility, m_labelText);
}

arc::Future<std::optional<std::string>> DTLabel::modifyKeys() {
    if (m_currentlyLoadingFor.size() != 0) co_return std::nullopt;

    auto text = co_await async::waitForMainThread<std::string>([self = this]() -> std::string {
        if (!self) return "";
        self->m_loadingCircle->setVisible(true);
        return self->m_labelText->getRawText();
    });
    if (!text.has_value()) {
        co_return std::nullopt;
    }
    
    m_keysUsed.clear();
    
    co_await arc::yield();
    auto modifiedText = co_await modifyStrRecursive(text.value());
    co_await arc::yield();
    if (modifiedText.length() != 0 && modifiedText[modifiedText.length() - 1] == '\n') modifiedText.push_back(' ');
    
    co_return std::make_optional(modifiedText);
}

void DTLabel::setLoading(const std::shared_ptr<SpecialKey>& key) {
    if (m_currentlyLoadingFor.contains(key) || (!m_keysUsed.contains(key->getKey()) && m_keysUsed.size())) return;
    m_currentlyLoadingFor.insert(key);

    m_loadingCircle->setVisible(true);
}
void DTLabel::completeLoading(const std::shared_ptr<SpecialKey>& key) {
    if (!m_currentlyLoadingFor.contains(key)) return;
    m_currentlyLoadingFor.erase(key);

    if (m_currentlyLoadingFor.size() == 0) {
        m_loadingCircle->setVisible(false);
        m_labelText->setText(m_info.m_text);
        auto self = this;
        self->retain();

        m_modifyListener.spawn("DT-load-complete-apply-keys-recursive-task", modifyKeys(), [self](std::optional<std::string> opt) {
            if (!self) return;

            self->m_loadingCircle->setVisible(false);
            if (!opt) {
                self->release();
                return;
            }

            self->m_labelText->setText(opt.value());
            self->release();
        });
    }
}

void DTLabel::setFontSize(float newSize) {
    newSize = std::clamp(newSize, DTLabelInfo::MIN_MAX_SCALE.x, DTLabelInfo::MIN_MAX_SCALE.y);
    m_info.m_scale = newSize;

    m_labelText->setScale(newSize);
}

void DTLabel::setFont(const std::string& fnt) {
    m_info.m_font = fnt;
    
    m_labelText->setFont(fnt);
}

void DTLabel::setLabelName(const std::string& newName) {
    m_info.m_labelName = newName;

    m_labelTitleArea->setText(newName);
    m_labelTitleArea->updateContent();
}

void DTLabel::setLabelColor(const ccColor4B& newColor) {
    m_info.m_labelColor = newColor;

    m_bg->setColor({m_info.m_labelColor.r, m_info.m_labelColor.g, m_info.m_labelColor.b});
    m_bg->setOpacity(m_normalBGOpacity * (m_info.m_labelColor.a / 255.0f));
    m_labelTitleBG->setColor({m_info.m_labelColor.r, m_info.m_labelColor.g, m_info.m_labelColor.b});
    m_labelTitleBG->setOpacity(m_info.m_hideHeader && !m_isEditable ? 0 : m_info.m_labelColor.a);
}

void DTLabel::setTextColor(const ccColor4B& newColor) {
    m_labelText->stopActionByTag(10);
    m_labelText->stopActionByTag(11);
    m_info.m_textColor = newColor;

    m_labelText->setColor(newColor);
}

void DTLabel::setTextAlignment(CCTextAlignment alignment) {
    m_info.m_horizontalAlignment = alignment;

    m_labelText->setAlignment(alignment);
    m_labelTitleArea->setAlignment(alignment);
}

void DTLabel::setTextWrapping(WrappingMode wrapMode) {
    if (wrapMode == WrappingMode::NO_WRAP) wrapMode = WrappingMode::CUTOFF_WRAP;
    m_info.m_wrapping = wrapMode;

    m_labelText->setWrappingMode(wrapMode);
}

void DTLabel::setHideHeader(bool hideHeader) {
    m_info.m_hideHeader = hideHeader;

    m_hideBGSpr->setVisible(m_info.m_hideHeader);
}

void DTLabel::onBeingEdited() {
    m_glow->stopAllActions();
    m_glow->runAction(CCTintTo::create(.1f, 35, 35, 35));
    m_isBeingEdited = true;
}
void DTLabel::onBeingEditedEnded() {
    m_glow->stopAllActions();
    m_glow->runAction(CCTintTo::create(.1f, 0, 0, 0));
    m_isBeingEdited = false;
}

void DTLabel::setEditable(bool editable) {
    m_isEditable = editable;

    if (m_isEditable) {
        m_labelTitleBG->setOpacity(m_info.m_labelColor.a);
        m_menu->setEnabled(true);
        m_menu->setOpacity(255);
        m_labelTitleArea->setVisible(true);
    }
    else {
        m_labelTitleBG->setOpacity(m_info.m_hideHeader ? 0 : m_info.m_labelColor.a);
        m_menu->setEnabled(!m_info.m_hideHeader);
        m_menu->setOpacity(m_info.m_hideHeader ? 0 : 255);
        if (m_info.m_hideHeader && !m_info.m_isExpanded)
            toggleExpand(nullptr);

        m_labelTitleArea->setVisible(!m_info.m_hideHeader);
    }
}

arc::Future<std::string> DTLabel::modifyStrRecursive(const std::string& str) {
    std::regex specialKeyRegex(R"(\{([A-Za-z0-9_\\]+)(?:-([^{}]*))?\})");

    std::sregex_iterator begin(str.begin(), str.end(), specialKeyRegex);
    std::sregex_iterator end;

    std::string result = "";

    int lastPos = 0;
    for (auto it = begin; it != end; ++it) {
        co_await arc::yield();
        const auto& match = *it;

        int matchStart = match.position();
        int matchEnd = matchStart + match.length();

        result += str.substr(lastPos, matchStart - lastPos);

        std::string key = match.str(1);
        if (!m_keysUsed.contains(key)) {
            m_keysUsed.insert(key);
        }
        
        bool isCancellation = false;
        if (key.size() && key[0] == '\\') {
            isCancellation = true;
            key = key.erase(0, 1);
        }

        std::string value = match.size() > 2 ? match.str(2) : "";

        auto dtLayer = DTLayer::get();
        if (dtLayer != nullptr && dtLayer->m_specialStrings.contains(key)) {
            std::string content = dtLayer->m_specialStrings.at(key)->getContent();
            
            if (m_ignoreExtraSettings || m_payload.size()) {

                auto payloadCopy = m_payload;
                if (m_ignoreExtraSettings)
                    payloadCopy["ignoreExtraSettings"] = true;

                auto res = co_await dtLayer->m_specialStrings.at(key)->getUpdatedContentAsync(payloadCopy);
                if (res.isOk())
                    content = res.unwrap();
            }
            
            result += co_await modifyStrRecursive(content);
        }
        else {
            result += match.str();
        }

        lastPos = matchEnd;

        co_await arc::yield();
    }

    result += str.substr(lastPos);

    co_await arc::yield();
    co_return result;
}
void DTLabel::setExpandable(bool enabled) {
    m_isExpandable = enabled;
    m_expandBtn->setVisible(enabled);
}

void DTLabel::fadeTextColorTo(ccColor4B newColor, float time) {
    m_info.m_textColor = newColor;

    m_labelText->stopActionByTag(10);
    m_labelText->stopActionByTag(11);

    auto faceAction = CCTextAreaFadeTo::create(time, newColor.a);
    faceAction->setTag(10);

    auto colorAction = CCTextAreaTintTo::create(time, {newColor.r, newColor.g, newColor.b});
    colorAction->setTag(11);

    m_labelText->runAction(faceAction);
    m_labelText->runAction(colorAction);
}

void DTLabel::fadeTitleColorTo(ccColor4B newColor, float time) {
    m_labelTitleArea->stopActionByTag(10);
    m_labelTitleArea->stopActionByTag(11);

    auto faceAction = CCTextAreaFadeTo::create(time, newColor.a);
    faceAction->setTag(10);

    auto colorAction = CCTextAreaTintTo::create(time, {newColor.r, newColor.g, newColor.b});
    colorAction->setTag(11);

    m_labelTitleArea->runAction(faceAction);
    m_labelTitleArea->runAction(colorAction);
}

float DTLabel::getSpecificTitleHeight() {
    return !m_info.m_hideHeader || m_isEditable ? labelTitleHeight : 2.5f;
}

void DTLabel::onRunMoreClicked(CCObject* sender) {
    auto moreBtn = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto buttonIDParts = utils::string::split(moreBtn->getID(), ",");
    if (buttonIDParts.size() != 1 && buttonIDParts.size() != 3) return;

    auto runKeyRes = StatsManager::splitRunKey(buttonIDParts[0]);

    if (runKeyRes.isErr()) return;

    std::optional<std::pair<long long, size_t>> occurrenceSelection;
    if (buttonIDParts.size() == 3) {
        auto timestampRes = utils::numFromString<long long>(buttonIDParts[1]);
        auto indexRes = utils::numFromString<size_t>(buttonIDParts[2]);
        if (timestampRes.isErr() || indexRes.isErr()) {
            log::error("Invalid recent-death decimal button selection");
            return;
        }
        occurrenceSelection = std::make_pair(timestampRes.unwrap(), indexRes.unwrap());
    }
    
    if (m_decimalPopup && !m_isEditable)
        m_decimalPopup->setDisplay(moreBtn, runKeyRes.unwrap(), moreBtn->getTag() == 1, Settings::getDecimalPrecision(), this, occurrenceSelection);
}

void DTLabel::attachedDecimalPopup(DecimalPopup* popup) {
    m_decimalPopup = popup;
}