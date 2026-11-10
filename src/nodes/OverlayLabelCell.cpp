#include "OverlayLabelCell.hpp"

OverlayLabelCell* OverlayLabelCell::create(float width, OverlayLabel const& label) {
    auto ret = new OverlayLabelCell();
    if (ret && ret->init(width, label)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool OverlayLabelCell::init(float width, OverlayLabel const& label) {
    if (!CCMenu::init()) return false;

    m_myLabelData = label;

    this->setContentSize({width, 60});

    m_background = CCScale9Sprite::create("GJ_square01.png");
    m_background->setAnchorPoint({0, 0});
    m_background->setScale(.5f);
    m_background->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / m_background->getScale());
    this->addChildAtPosition(m_background, Anchor::BottomLeft);
    updateBackgroundColor();

    m_labelName = SimpleTextArea::create(label.labelName, "bigFont.fnt", .5f);
    m_labelName->setAnchorPoint({0, 1});
    m_labelName->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    m_labelName->setWidth(width / 1.2f);
    this->addChildAtPosition(m_labelName, Anchor::TopLeft, {4, -4});

    auto settingsMenu = CCMenu::create();
    settingsMenu->setContentSize({width, 20});
    settingsMenu->setAnchorPoint({0, 0});
    settingsMenu->ignoreAnchorPointForPosition(false);
    this->addChildAtPosition(settingsMenu, Anchor::BottomLeft);

    m_enabledToggle = SimpleToggler::createWithDefaults(.35f, label.enabled);
    m_enabledToggle->setPosition({10, 10});
    settingsMenu->addChild(m_enabledToggle);
    m_enabledToggle->setCallback([this](bool enabled) {
        if (m_updatingControls) return;

        m_myLabelData.enabled = enabled;
        updateBackgroundColor();
        if (m_onChanged != NULL) m_onChanged(this);
    });

    auto enabledLabel = CCLabelBMFont::create("Enabled", "bigFont.fnt");
    enabledLabel->setScale(.35f);
    enabledLabel->setAnchorPoint({0, .5f});
    enabledLabel->setPosition({m_enabledToggle->getPositionX() + m_enabledToggle->getScaledContentWidth() / 2 + 3, 10});
    settingsMenu->addChild(enabledLabel);

    auto orderLabel = CCLabelBMFont::create("Order:", "bigFont.fnt");
    orderLabel->setScale(.35f);
    orderLabel->setAnchorPoint({0, .5f});
    orderLabel->setPosition({5, this->getContentHeight() / 2});
    this->addChild(orderLabel);

    m_orderInput = TextInput::create(50, "Order");
    m_orderInput->setScale(.55f);
    m_orderInput->setCommonFilter(CommonFilter::Uint);
    m_orderInput->setString(std::to_string(label.order));
    m_orderInput->setAnchorPoint({0, .5f});
    m_orderInput->setPosition({orderLabel->getPositionX() + orderLabel->getScaledContentWidth() + 3, orderLabel->getPositionY()});
    m_orderInput->setCallback([this](const std::string& value) {
        if (m_updatingControls) return;

        auto order = geode::utils::numFromString<int>(value);
        if (order.isErr()) return;

        m_myLabelData.order = order.unwrap();
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
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSpr,
        this,
        menu_selector(OverlayLabelCell::onDelete)
    );
    buttonMenu->addChild(deleteBtn);
    
    auto editBtnSpr = CCSprite::createWithSpriteFrameName("GJ_optionsBtn02_001.png");
    auto editBtn = CCMenuItemSpriteExtra::create(
        editBtnSpr,
        this,
        menu_selector(OverlayLabelCell::onEdit)
    );
    buttonMenu->addChild(editBtn);

    buttonMenu->updateLayout();

    return true;
}

void OverlayLabelCell::setData(OverlayLabel const& label) {
    m_myLabelData = label;
    m_labelName->setText(label.labelName);
    updateBackgroundColor();
    m_updatingControls = true;
    m_enabledToggle->toggle(label.enabled);
    m_orderInput->setString(std::to_string(label.order));
    m_updatingControls = false;
}

void OverlayLabelCell::setEditing(bool isEditing) {
    m_isEditing = isEditing;
    updateBackgroundColor();
}

void OverlayLabelCell::updateBackgroundColor() {
    if (!m_myLabelData.enabled) {
        m_background->setColor({120, 120, 120});
    }
    else if (m_isEditing) {
        m_background->setColor({100, 255, 100});
    }
    else {
        m_background->setColor({255, 255, 255});
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

void OverlayLabelCell::onDeleteCallback(geode::Function<void(OverlayLabelCell*)> onDelete){
    m_onDelete = std::move(onDelete);
}

void OverlayLabelCell::onEditCallback(geode::Function<void(OverlayLabelCell*)> onEdit){
    m_onEdit = std::move(onEdit);
}

void OverlayLabelCell::onChangedCallback(geode::Function<void(OverlayLabelCell*)> onChanged){
    m_onChanged = std::move(onChanged);
}