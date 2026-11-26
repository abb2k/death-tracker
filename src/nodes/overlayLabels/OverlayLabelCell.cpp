#include "OverlayLabelCell.hpp"

#include <nodes/DTLayer.hpp>
#include <nodes/general/SimpleToggler.hpp>
#include <managers/StatsManager.hpp>

OverlayLabelCell* OverlayLabelCell::create(float width, OverlayLabel label) {
    auto ret = new OverlayLabelCell();

    if (ret && ret->init(std::move(width), std::move(label))) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr; 
}

bool OverlayLabelCell::init(float width, OverlayLabel label) {
    if (!CCMenu::init()) return false;

    m_myLabelData = std::move(label);
    updateLevelVisibility();

    this->setContentSize({width, 80});

    m_background = CCScale9Sprite::create("GJ_square01.png");
    m_background->setAnchorPoint({0, 0});
    m_background->setScale(.5f);
    m_background->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / m_background->getScale());
    this->addChildAtPosition(m_background, Anchor::BottomLeft);
    updateBackgroundColor();

    m_labelName = SimpleTextArea::create(m_myLabelData.m_labelName, "bigFont.fnt", .5f);
    m_labelName->setAnchorPoint({0, 1});
    m_labelName->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_labelName->setWidth(width / 1.2f);
    this->addChildAtPosition(m_labelName, Anchor::TopLeft, {4, -4});

    auto settingsMenu = CCMenu::create();
    settingsMenu->setContentSize({width, 20});
    settingsMenu->setAnchorPoint({0, 0});
    settingsMenu->ignoreAnchorPointForPosition(false);
    this->addChildAtPosition(settingsMenu, Anchor::BottomLeft);

    m_enabledToggle = SimpleToggler::createWithDefaults(.35f, m_myLabelData.m_enabled);
    m_enabledToggle->setPosition({10, 10});
    settingsMenu->addChild(m_enabledToggle);
    m_enabledToggle->setCallback([this](bool enabled) {
        if (m_updatingControls) return;

        m_myLabelData.m_enabled = enabled;
        updateBackgroundColor();
        if (m_onChanged != NULL) m_onChanged(this);
    });

    m_enableLabel = geode::Label::create("Enabled", "bigFont.fnt");
    m_enableLabel->setScale(.35f);
    m_enableLabel->setAnchorPoint({0, .5f});
    m_enableLabel->setPosition({m_enabledToggle->getPositionX() + m_enabledToggle->getScaledContentWidth() / 2 + 3, 10});
    settingsMenu->addChild(m_enableLabel);

    m_levelSpecificToggle = SimpleToggler::createWithDefaults(.35f, m_myLabelData.m_levelSpecific.has_value());
    m_levelSpecificToggle->setPosition({10, 18 + m_enabledToggle->getContentHeight()});
    settingsMenu->addChild(m_levelSpecificToggle);
    m_levelSpecificToggle->setCallback([this](bool enabled) {
        if (m_updatingControls) return;

        if (enabled) {
            auto levelKeyRes = StatsManager::getLevelKey(DTLayer::get()->m_Level);
            m_myLabelData.m_levelSpecific = levelKeyRes.isOk() ? std::optional<std::string>(levelKeyRes.unwrap()) : std::nullopt;
        } else {
            m_myLabelData.m_levelSpecific = std::nullopt;
        }
        auto wasVisible = isVisible();
        updateLevelVisibility();
        if (wasVisible != isVisible() && m_onVisibilityChanged != NULL)
            m_onVisibilityChanged(this);
        updateBackgroundColor();
        if (m_onChanged != NULL) m_onChanged(this);
    });

    m_levelSpecificLabel = geode::Label::create("Level\nSpecific", "bigFont.fnt");
    m_levelSpecificLabel->setScale(.35f);
    m_levelSpecificLabel->setAnchorPoint({0, .5f});
    m_levelSpecificLabel->setPosition({m_levelSpecificToggle->getPositionX() + m_levelSpecificToggle->getScaledContentWidth() / 2 + 3, 18 + m_enabledToggle->getContentHeight()});
    settingsMenu->addChild(m_levelSpecificLabel);

    m_orderLabel = geode::Label::create("Order:", "bigFont.fnt");
    m_orderLabel->setScale(.35f);
    m_orderLabel->setAnchorPoint({0, .5f});
    m_orderLabel->setPosition({5, this->getContentHeight() / 2 + 9});
    this->addChild(m_orderLabel);

    m_orderInput = TextInput::create(50, "Order");
    m_orderInput->setScale(.55f);
    m_orderInput->setCommonFilter(CommonFilter::Uint);
    m_orderInput->setString(std::to_string(m_myLabelData.m_order));
    m_orderInput->setAnchorPoint({0, .5f});
    m_orderInput->setPosition({m_orderLabel->getPositionX() + m_orderLabel->getScaledContentWidth() + 3, m_orderLabel->getPositionY()});
    m_orderInput->setCallback([this](const std::string& value) {
        if (m_updatingControls) return;

        auto order = geode::utils::numFromString<int>(value);
        if (order.isErr()) return;

        m_myLabelData.m_order = order.unwrap();
        if (m_onChanged != NULL) m_onChanged(this);
    });
    this->addChild(m_orderInput);

    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition({0, 0});
    buttonMenu->setContentSize({
        width / 2.5f,
        this->getContentHeight() / 2.5f
    });
    buttonMenu->setAnchorPoint({1, 0});
    buttonMenu->ignoreAnchorPointForPosition(false);
    buttonMenu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setGap(5)
        ->setCrossAxisScaling(AxisScaling::ScaleDownGaps)
        ->setMainAxisAlignment(MainAxisAlignment::Start)
        ->setMinRelativeScale(std::nullopt)
        ->setMainAxisDirection(AxisDirection::RightToLeft)
    );
    this->addChildAtPosition(buttonMenu, Anchor::BottomRight, {-4, 4});
    
    auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    m_deleteButton = CCMenuItemSpriteExtra::create(
        deleteBtnSpr,
        this,
        menu_selector(OverlayLabelCell::onDelete)
    );
    buttonMenu->addChild(m_deleteButton);
    
    auto editBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
    m_editButton = CCMenuItemSpriteExtra::create(
        editBtnSpr,
        this,
        menu_selector(OverlayLabelCell::onEdit)
    );
    buttonMenu->addChild(m_editButton);

    buttonMenu->updateLayout();

    return true;
}

void OverlayLabelCell::setData(OverlayLabel label) {
    m_myLabelData = std::move(label);
    m_labelName->setText(m_myLabelData.m_labelName);
    updateLevelVisibility();
    updateBackgroundColor();
    m_updatingControls = true;
    m_enabledToggle->toggle(m_myLabelData.m_enabled);
    m_orderInput->setString(std::to_string(m_myLabelData.m_order));
    m_updatingControls = false;
}

void OverlayLabelCell::setEditing(bool isEditing) {
    m_isEditing = isEditing;
    updateBackgroundColor();
}

void OverlayLabelCell::updateLevelVisibility() {
    auto dtLayer = DTLayer::get();

    auto isVisible = m_myLabelData.isOverlayLabelVisibleOnLevel(
        dtLayer ? dtLayer->m_Level : nullptr
    );

    this->setVisible(isVisible);
}

void OverlayLabelCell::updateBackgroundColor() {
    if (!m_myLabelData.m_enabled) {
        m_background->setColor({120, 120, 120});
    }
    else if (m_isEditing) {
        m_background->setColor({100, 255, 100});
    }
    else {
        if (m_myLabelData.m_levelSpecific.has_value())
            m_background->setColor({ 76, 86, 228 });
        else
            m_background->setColor({ 255, 255, 255 });
    }
}

void OverlayLabelCell::onDelete(CCObject*){
    if (m_onDelete != NULL)
        m_onDelete(this);
}
void OverlayLabelCell::onEdit(CCObject*){
    if (m_onEdit != NULL)
        m_onEdit(this);
}

void OverlayLabelCell::onDeleteCallback(Function<void(OverlayLabelCell*)> onDelete){
    m_onDelete = std::move(onDelete);
}

void OverlayLabelCell::onEditCallback(Function<void(OverlayLabelCell*)> onEdit){
    m_onEdit = std::move(onEdit);
}

void OverlayLabelCell::onChangedCallback(Function<void(OverlayLabelCell*)> onChanged){
    m_onChanged = std::move(onChanged);
}

void OverlayLabelCell::onVisibilityChangedCallback(Function<void(OverlayLabelCell*)> onVisibilityChanged) {
    m_onVisibilityChanged = std::move(onVisibilityChanged);
}