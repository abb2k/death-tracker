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

    this->setContentSize({width, 40});

    auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChildAtPosition(bg, Anchor::BottomLeft);

    auto labelName = SimpleTextArea::create(label.labelName, "bigFont.fnt", .5f);
    labelName->setAnchorPoint({0, 1});
    labelName->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    labelName->setWidth(width / 1.2f);
    this->addChildAtPosition(labelName, Anchor::TopLeft, {4, -4});

    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition({0, 0});
    buttonMenu->setContentSize({
        width / 1.5f,
        this->getContentHeight() / 2.75f
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