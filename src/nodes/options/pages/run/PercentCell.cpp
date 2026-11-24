#include "PercentCell.hpp"

#include <utils/Dev.hpp>

PercentCell* PercentCell::create(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, Function<void(PercentCell*)> callback) {
    auto ret = new PercentCell();

    if (ret && ret->init(width, Percent, maxToHide, sideButtonSprite, std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool PercentCell::init(float width, int percent, int maxToHide, CCNode* sideButtonSprite, Function<void(PercentCell*)> callback) {
    if (!CCMenu::init()) return false;

    this->m_maxToHide = maxToHide;

    this->setContentSize({width, 40});
    this->ignoreAnchorPointForPosition(false);
    this->setPosition({0,0});

    m_BGSprite = CCScale9Sprite::create("GJ_squareB_01.png");
    m_BGSprite->setScale(.25f);
    m_BGSprite->setContentSize(this->getContentSize() / m_BGSprite->getScale());
    m_BGSprite->setAnchorPoint({0, 0});
    this->addChild(m_BGSprite);

    auto button = CCMenuItemSpriteExtra::create(
        sideButtonSprite,
        this,
        menu_selector(PercentCell::RunCallback)
    );
    button->setPosition({width - button->getContentWidth() / 2 - 5, this->getContentHeight() / 2});
    button->setID("btn");
    this->addChild(button);

    auto label = geode::Label::create(fmt::format("{}%", percent).c_str(), "gjFont17.fnt");
    label->setLimitLabelWidth(width - button->getContentWidth() / 2, .6f);
    label->setAlignment(geode::Label::Alignment::Left);
    label->setAnchorPoint({0, .5f});
    label->setPosition({5, this->getContentHeight() / 2});
    label->setID("percent-label");
    this->addChild(label);

    this->m_percent = percent;
    this->m_callback = std::move(callback);

    m_MTHScroll = Slider::create(this, menu_selector(PercentCell::mthScroll));
    m_MTHScroll->ignoreAnchorPointForPosition(false);
    m_MTHScroll->setContentSize({0,0});
    m_MTHScroll->setPosition({width / 2,10});
    m_MTHScroll->setScale(1 - m_MTHScroll->m_sliderBar->getContentWidth() / (width - 140));
    m_MTHScroll->setValue((maxToHide - percent) / (100.0f - percent));
    m_MTHScroll->m_delegate = this;
    m_MTHScroll->setID("mth-slider");
    this->addChild(m_MTHScroll);

    auto MTHLabel = geode::Label::create("Max to\nHide:", "bigFont.fnt");
    MTHLabel->setPosition({width / 2, this->getContentHeight() - 2});
    MTHLabel->setAnchorPoint({1, 1});
    MTHLabel->setScale(.3f);
    MTHLabel->setAlignment(geode::Label::Alignment::Right);
    MTHLabel->setID("mth-label");
    this->addChild(MTHLabel);

    m_MTHInput = TextInput::create(60, "MTH");
    m_MTHInput->setPosition(MTHLabel->getPosition() + ccp(0, (MTHLabel->getScaledContentHeight() - m_MTHInput->getScaledContentHeight()) / 4));
    m_MTHInput->setAnchorPoint({0, 1});
    m_MTHInput->setScale(.6f);
    m_MTHInput->setString(std::to_string(maxToHide));
    m_MTHInput->setCommonFilter(CommonFilter::Uint);
    m_MTHInput->setID("mth-input");
    m_MTHInput->setCallback([&](const auto& newStr) {
        auto numRes = utils::numFromString<int>(newStr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num < this->m_percent) {
            num = this->m_percent;
        }
        if (num > 100) {
            num = 100;
        }
        
        this->m_maxToHide = num;

        m_MTHScroll->setValue((this->m_maxToHide - this->m_percent) / (100.0f - this->m_percent));
        if (this->m_percent == 100) {
            m_MTHScroll->setValue(1);
        }

        if (m_onMaxToHideChanged != NULL)
            m_onMaxToHideChanged(this);
    });
    this->addChild(m_MTHInput);

    if (this->m_percent == 100) {
        m_MTHScroll->setValue(1);
        m_MTHInput->setString("100");
    }

    return true;
}

void PercentCell::RunCallback(CCObject*) {
    if (m_callback != NULL)
        m_callback(this);
}

void PercentCell::hide() {
    this->runAction(CCFadeTo::create(0.2f, 0));
    Dev::fadeTextInput(m_MTHInput, false, .2f);
    Dev::fadeSlider(m_MTHScroll, false, .2f);
    this->setEnabled(false);
}
void PercentCell::hideInstant() {
    this->setOpacity(0);
    this->setEnabled(false);
    Dev::fadeTextInput(m_MTHInput, false, 0);
    Dev::fadeSlider(m_MTHScroll, false, 0);
}

void PercentCell::show() {
    this->runAction(CCFadeTo::create(0.2f, 255));
    Dev::fadeTextInput(m_MTHInput, true, .2f);
    Dev::fadeSlider(m_MTHScroll, true, .2f);
    this->setEnabled(true);
}

void PercentCell::mthScroll(CCObject*) {
    int remappedValue = std::lerp(m_percent, 100, m_MTHScroll->getValue());

    this->m_maxToHide = remappedValue;

    m_MTHInput->setString(std::to_string(remappedValue));
    if (this->m_percent == 100) {
        m_MTHInput->setString("100");
    }
}

void PercentCell::sliderEnded(Slider* slider) {
    if (m_onMaxToHideChanged != NULL)
        m_onMaxToHideChanged(this);
}

void PercentCell::setMaxToHide(float val) {
    m_MTHScroll->setValue(val);
    mthScroll(nullptr);
}

void PercentCell::setOnMaxToHideChangedCallback(Function<void(PercentCell*)> callback) {
    m_onMaxToHideChanged = std::move(callback);
}