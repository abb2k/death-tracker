#include "LabelEditLayer.hpp"

#include <types/DTTypes.hpp>
#include <nodes/SimpleToggler.hpp>

LabelEditLayer* LabelEditLayer::create() {
    auto ret = new LabelEditLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LabelEditLayer::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(winSize.width - 50, winSize.height - 50, "geode.loader/GE_square01.png"))
        return false;

    m_previewArea = CCClippingNode::create();
    m_previewArea->setZOrder(1);
    m_mainLayer->addChildAtPosition(m_previewArea, Anchor::TopRight, {-15, -15});

    CCPoint previewSize = {m_size.width / 1.5f, m_size.height / 1.75f};

    m_previewArea->setContentSize(previewSize);
    m_previewArea->setAnchorPoint({1, 1});

    auto stencil = CCLayerColor::create();
    stencil->setContentSize(m_previewArea->getContentSize());
    stencil->setAnchorPoint({0, 0});
    stencil->setPosition({0, 0});
    stencil->ignoreAnchorPointForPosition(false);

    m_previewArea->setStencil(stencil);
    m_mainLayer->addChild(stencil);

    m_previewAreaMenu = CCMenu::create();
    m_previewAreaMenu->setContentSize(m_previewArea->getContentSize());
    m_previewAreaMenu->setZOrder(1);
    m_previewArea->addChildAtPosition(m_previewAreaMenu, Anchor::Center);

    createAnchorButton(Anchor::BottomLeft, "BL");
    createAnchorButton(Anchor::TopLeft, "TL");
    createAnchorButton(Anchor::TopRight, "TR");
    createAnchorButton(Anchor::BottomRight, "BR");
    createAnchorButton(Anchor::Bottom, "B");
    createAnchorButton(Anchor::Top, "T");
    createAnchorButton(Anchor::Right, "R");
    createAnchorButton(Anchor::Left, "L");

    auto outline = CCLayerColor::create({0, 0, 0, 255});
    outline->setPosition(m_previewArea->getPosition() - m_previewArea->getContentSize() / 2);
    outline->setContentSize(m_previewArea->getContentSize() + ccp(4.5f, 4.5f));
    outline->ignoreAnchorPointForPosition(false);
    m_mainLayer->addChild(outline);

    ccColor3B bgColor = {40, 125, 255};

    auto bg = CCSprite::create("game_bg_01_001.png");
    bg->setPosition(m_previewArea->getContentSize() / 2);
    bg->setColor(bgColor);
    m_previewArea->addChild(bg);

    auto bg1 = CCSprite::create("game_bg_01_001.png");
    bg1->setAnchorPoint({1, .5f});
    bg1->setColor(bgColor);
    bg->addChildAtPosition(bg1, Anchor::Left);

    auto bg2 = CCSprite::create("game_bg_01_001.png");
    bg2->setAnchorPoint({0, .5f});
    bg2->setColor(bgColor);
    bg->addChildAtPosition(bg2, Anchor::Right);

    auto ground = GJGroundLayer::create(1, 0);
    ground->setPosition({0, 30});
    ground->setScale(0.64f);
    m_previewArea->addChild(ground);

    m_scroll = ScrollLayer::create({m_size.width / 4.f, m_size.height / 1.2f});
    m_scroll->setAnchorPoint({0, .5f});
    m_scroll->ignoreAnchorPointForPosition(false);
    m_scroll->setZOrder(1);
    m_mainLayer->addChildAtPosition(m_scroll, Anchor::Left, {15, 0});

    m_scroll->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(m_scroll->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisOverflow(false)
    );

    auto scrollBG = CCScale9Sprite::create("square02_small.png");
    scrollBG->setPosition(m_scroll->getPosition() + ccp(m_scroll->getScaledContentWidth() / 2, 0));
    scrollBG->setContentSize(m_scroll->getContentSize() + ccp(5, 5));
    scrollBG->setOpacity(150);
    m_mainLayer->addChild(scrollBG);

    m_noLabelsText = CCLabelBMFont::create("No Labels\nCreated!", "bigFont.fnt");
    m_noLabelsText->setScale(.5f);
    m_noLabelsText->setPosition(scrollBG->getPosition());
    m_noLabelsText->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    m_mainLayer->addChild(m_noLabelsText);

    auto sideMenu = CCMenu::create();
    sideMenu->setPosition({0, 0});
    sideMenu->setZOrder(1);
    m_mainLayer->addChild(sideMenu);

    auto addLabelButtonSpr = CCSprite::createWithSpriteFrameName("GJ_plus3Btn_001.png");
    addLabelButtonSpr->setScale(.75f);
    auto addLabelButton = CCMenuItemSpriteExtra::create(
        addLabelButtonSpr,
        this,
        menu_selector(LabelEditLayer::onLabelAdded)
    );
    addLabelButton->setPosition(scrollBG->getPosition() + ccp(
        scrollBG->getScaledContentWidth() / 2,
        scrollBG->getScaledContentHeight() / 2
    ));
    sideMenu->addChild(addLabelButton);

    auto editBG = CCScale9Sprite::create("square02_small.png");
    editBG->setContentSize({
        previewSize.x,
        m_size.height - previewSize.y - 35
    });
    editBG->setOpacity(150);
    editBG->setAnchorPoint({1, 0});
    m_mainLayer->addChildAtPosition(editBG, Anchor::BottomRight, {-15, 15});

    auto editMenu = CCMenu::create();
    editMenu->setPosition(editBG->getPosition());
    editMenu->setAnchorPoint(editBG->getAnchorPoint());
    editMenu->setContentSize(editBG->getContentSize());
    editMenu->ignoreAnchorPointForPosition(false);
    m_mainLayer->addChild(editMenu);

    auto freeModeToggle = SimpleToggler::createWithDefaults(
        .5f
    );
    editMenu->addChildAtPosition(freeModeToggle, Anchor::TopLeft, {editMenu->getContentWidth() / 4.5f, -freeModeToggle->getScaledContentHeight() / 2 - 5});

    auto freeModeToggleLabel = CCLabelBMFont::create("Free Mode", "bigFont.fnt");
    freeModeToggleLabel->setPosition(freeModeToggle->getPosition() + ccp(
        freeModeToggle->getScaledContentWidth() / 2 + 5,
        0
    ));
    freeModeToggleLabel->setAnchorPoint({0, .5f});
    freeModeToggleLabel->setScale(.5f);
    editMenu->addChild(freeModeToggleLabel);

    auto orderLabel = CCLabelBMFont::create("Order", "bigFont.fnt");
    orderLabel->setScale(.5f);
    editMenu->addChildAtPosition(orderLabel, Anchor::TopRight, {-editMenu->getContentWidth() / 6, -orderLabel->getScaledContentHeight() / 2 - 5});

    auto sliderHolder = CCNode::create();
    sliderHolder->setContentSize({editMenu->getContentWidth() / 1.6f, 20});
    sliderHolder->setAnchorPoint({.5f, .5f});
    editMenu->addChildAtPosition(sliderHolder, Anchor::TopLeft, {
        editMenu->getContentWidth() / 3.f,
        -sliderHolder->getScaledContentHeight() / 2 - freeModeToggleLabel->getScaledContentHeight() - 10
    });

    auto xLabel = CCLabelBMFont::create("X:", "bigFont.fnt");
    xLabel->setScale(.5f);
    xLabel->setAnchorPoint({0, .5f});
    sliderHolder->addChildAtPosition(xLabel, Anchor::Left);

    auto xSlider = Slider::create(this, menu_selector(LabelEditLayer::onSliderX));
    xSlider->setScale(.45f);
    xSlider->setPosition(xLabel->getPosition() + ccp(
        xLabel->getScaledContentWidth() + 15 + xSlider->m_sliderBar->getContentWidth() / xSlider->getScale() / 2,
        -1
    ));
    xSlider->setContentSize({0, 0});
    sliderHolder->addChild(xSlider);

    auto yLabel = CCLabelBMFont::create("Y:", "bigFont.fnt");
    yLabel->setScale(.5f);
    yLabel->setAnchorPoint({0, .5f});
    sliderHolder->addChildAtPosition(yLabel, Anchor::Left, {xLabel->getScaledContentWidth() + 15 + xSlider->m_sliderBar->getContentWidth() / xSlider->getScale() + 15, 0});

    auto ySlider = Slider::create(this, menu_selector(LabelEditLayer::onSliderX));
    ySlider->setScale(.45f);
    ySlider->setPosition(yLabel->getPosition() + ccp(
        yLabel->getScaledContentWidth() + 15 + ySlider->m_sliderBar->getContentWidth() / ySlider->getScale() / 2,
        -1
    ));
    ySlider->setContentSize({0, 0});
    sliderHolder->addChild(ySlider);

    auto orderInput = TextInput::create(50, "Ord");
    orderInput->setScale(.75f);
    orderInput->setCommonFilter(CommonFilter::Uint);
    editMenu->addChildAtPosition(orderInput, Anchor::TopRight, {-editMenu->getContentWidth() / 6, -orderInput->getScaledContentHeight() / 2 - orderLabel->getScaledContentHeight() - 10});


    auto nameLabel = CCLabelBMFont::create("Name:", "bigFont.fnt");
    nameLabel->setScale(.5f);
    nameLabel->setAnchorPoint({0, .5f});
    editMenu->addChildAtPosition(nameLabel, Anchor::BottomLeft, {10, 15});

    auto nameInput = TextInput::create(80, "Name");
    nameInput->setScale(.75f);
    nameInput->setCommonFilter(CommonFilter::Name);
    nameInput->setMaxCharCount(12);
    nameInput->setAnchorPoint({0, .5f});
    nameInput->setPosition(nameLabel->getPosition() + ccp(
        nameLabel->getScaledContentWidth() + 5,
        0
    ));
    editMenu->addChild(nameInput);

    auto textLabel = CCLabelBMFont::create("Text:", "bigFont.fnt");
    textLabel->setScale(.5f);
    textLabel->setAnchorPoint({0, .5f});
    textLabel->setPosition(nameInput->getPosition() + ccp(
        nameInput->getScaledContentWidth() + 5,
        0
    ));
    editMenu->addChild(textLabel);

    auto textInput = TextInput::create((editMenu->getScaledContentWidth() - (textLabel->getPositionX() + textLabel->getScaledContentWidth() - nameLabel->getPositionX()) - 20) / .75f, "Text");
    textInput->setScale(.75f);
    textInput->setAnchorPoint({0, .5f});
    textInput->setPosition(textLabel->getPosition() + ccp(
        textLabel->getScaledContentWidth() + 5,
        0
    ));
    editMenu->addChild(textInput);

    return true;
}

void LabelEditLayer::onLabelAdded(CCObject*){
    auto newLabel = OverlayLabel{};
    newLabel.labelName = "New Label";

    auto newLabelCell = OverlayLabelCell::create(m_scroll->getContentWidth(), newLabel);
    newLabelCell->onEditCallback([&](auto cell){ onCellEdit(cell); });
    newLabelCell->onDeleteCallback([&](auto cell){ onCellDelete(cell); });

    m_scroll->m_contentLayer->addChild(newLabelCell);
    m_scroll->m_contentLayer->updateLayout();

    m_labels.insert(newLabelCell);

    m_noLabelsText->setVisible(!m_labels.size());
}


void LabelEditLayer::onCellDelete(OverlayLabelCell* cell){
    if (!m_labels.contains(cell)) return;

    m_labels.erase(cell);

    cell->removeMeAndCleanup();

    m_scroll->m_contentLayer->updateLayout();

    m_noLabelsText->setVisible(!m_labels.size());
}

void LabelEditLayer::onCellEdit(OverlayLabelCell* cell){
    m_currentEdit = cell;



    // edit stuff
}

void LabelEditLayer::onSliderX(CCObject*){

}

void LabelEditLayer::onSliderY(CCObject*){

}

CCMenuItemSpriteExtra* LabelEditLayer::createAnchorButton(Anchor anchor, std::string const& text){

    // @geode-ignore(unknown-resource)
    auto bg = CCScale9Sprite::create("geode.loader/white-square.png");

    auto btnText = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    btnText->setScale(.35f);

    Anchor invertedAnchor;

    switch (anchor) {
        default:
        case Anchor::Center: invertedAnchor = Anchor::Center; break;
        case Anchor::TopLeft: invertedAnchor = Anchor::BottomRight; break;
        case Anchor::Top: invertedAnchor = Anchor::Bottom; break;
        case Anchor::TopRight: invertedAnchor = Anchor::BottomLeft; break;
        case Anchor::Right: invertedAnchor = Anchor::Left; break;
        case Anchor::BottomRight: invertedAnchor = Anchor::TopLeft; break;
        case Anchor::Bottom: invertedAnchor = Anchor::Top; break;
        case Anchor::BottomLeft: invertedAnchor = Anchor::TopRight; break;
        case Anchor::Left: invertedAnchor = Anchor::Right; break;
    }

    CCPoint anchorPoint;

    switch (invertedAnchor) {
        default:
        case Anchor::Center: anchorPoint = ccp(0.5f, 0.5f); break;
        case Anchor::TopLeft: anchorPoint = ccp(0, 1); break;
        case Anchor::Top: anchorPoint = ccp(.5f, 1); break;
        case Anchor::TopRight: anchorPoint = ccp(1, 1); break;
        case Anchor::Right: anchorPoint = ccp(1, .5f); break;
        case Anchor::BottomRight: anchorPoint = ccp(1, 0); break;
        case Anchor::Bottom: anchorPoint = ccp(.5f, 0); break;
        case Anchor::BottomLeft: anchorPoint = ccp(0, 0); break;
        case Anchor::Left: anchorPoint = ccp(0, .5f); break;
    }

    CCPoint directionNormalized;

    switch (invertedAnchor) {
        default:
        case Anchor::Center: directionNormalized = ccp(0, 0); break;
        case Anchor::TopLeft: directionNormalized = ccp(1, -1); break;
        case Anchor::Top: directionNormalized = ccp(0, -1); break;
        case Anchor::TopRight: directionNormalized = ccp(-1, -1); break;
        case Anchor::Right: directionNormalized = ccp(-1, 0); break;
        case Anchor::BottomRight: directionNormalized = ccp(-1, 1); break;
        case Anchor::Bottom: directionNormalized = ccp(0, 1); break;
        case Anchor::BottomLeft: directionNormalized = ccp(1, 1); break;
        case Anchor::Left: directionNormalized = ccp(1, 0); break;
    }

    bg->addChildAtPosition(btnText, invertedAnchor, directionNormalized * 3.5f);
    btnText->setAnchorPoint(anchorPoint);

    auto btn = CCMenuItemSpriteExtra::create(
        bg,
        this,
        menu_selector(LabelEditLayer::onAnchorClicked)
    );

    m_previewAreaMenu->addChildAtPosition(btn, anchor);

    m_anchorButtons.insert({btn, anchor});

    return btn;
}

void LabelEditLayer::onAnchorClicked(CCObject*){

}