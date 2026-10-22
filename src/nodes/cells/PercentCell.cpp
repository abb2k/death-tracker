#include <nodes/cells/PercentCell.hpp>
#include <utils/Dev.hpp>

PercentCell* PercentCell::create(float width, int Percent, int maxToHide, CCNode* sideButtonSprite, geode::Function<void(PercentCell*)> callback) {
    auto ret = new PercentCell();
    if (ret && ret->init(width, Percent, maxToHide, sideButtonSprite, std::move(callback))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool PercentCell::init(float width, int percent, int maxToHide, CCNode* sideButtonSprite, geode::Function<void(PercentCell*)> callback) {
    if (!CCMenu::init()) return false;

    this->maxToHide = maxToHide;

    this->setContentSize({width, 40});
    this->ignoreAnchorPointForPosition(false);
    this->setPosition({0,0});

    BGSprite = CCScale9Sprite::create("GJ_squareB_01.png");
    BGSprite->setScale(.25f);
    BGSprite->setContentSize(this->getContentSize() / BGSprite->getScale());
    BGSprite->setAnchorPoint({0, 0});
    this->addChild(BGSprite);

    auto button = CCMenuItemSpriteExtra::create(
        sideButtonSprite,
        this,
        menu_selector(PercentCell::RunCallback)
    );
    button->setPosition({width - button->getContentWidth() / 2 - 5, this->getContentHeight() / 2});
    button->setID("btn");
    this->addChild(button);

    auto label = CCLabelBMFont::create(fmt::format("{}%", percent).c_str(), "gjFont17.fnt");
    label->setWidth(width - button->getContentWidth() / 2);
    label->setScale(.6f);
    label->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
    label->setAnchorPoint({0, .5f});
    label->setPosition({5, this->getContentHeight() / 2});
    label->setID("percent-label");
    this->addChild(label);

    this->percent = percent;
    this->callback = std::move(callback);

    MTHScroll = Slider::create(this, menu_selector(PercentCell::mthScroll));
    MTHScroll->ignoreAnchorPointForPosition(false);
    MTHScroll->setContentSize({0,0});
    MTHScroll->setPosition({width / 2,10});
    MTHScroll->setScale(1 - MTHScroll->m_sliderBar->getContentWidth() / (width - 140));
    MTHScroll->setValue((maxToHide - percent) / (100.0f - percent));
    MTHScroll->m_delegate = this;
    MTHScroll->setID("mth-slider");
    this->addChild(MTHScroll);

    auto MTHLabel = CCLabelBMFont::create("Max to\nHide:", "bigFont.fnt");
    MTHLabel->setPosition({width / 2, this->getContentHeight() - 2});
    MTHLabel->setAnchorPoint({1, 1});
    MTHLabel->setScale(.3f);
    MTHLabel->setAlignment(CCTextAlignment::kCCTextAlignmentRight);
    MTHLabel->setID("mth-label");
    this->addChild(MTHLabel);

    MTHInput = TextInput::create(60, "MTH");
    MTHInput->setPosition(MTHLabel->getPosition() + ccp(0, (MTHLabel->getScaledContentHeight() - MTHInput->getScaledContentHeight()) / 4));
    MTHInput->setAnchorPoint({0, 1});
    MTHInput->setScale(.6f);
    MTHInput->setString(std::to_string(maxToHide));
    MTHInput->setCommonFilter(CommonFilter::Uint);
    MTHInput->setID("mth-input");
    MTHInput->setCallback([&](const auto& newStr) {
        auto numRes = utils::numFromString<int>(newStr);
        if (numRes.isErr()) return;
        int num = numRes.unwrap();

        if (num < this->percent) {
            num = this->percent;
        }
        if (num > 100) {
            num = 100;
        }
        
        this->maxToHide = num;

        MTHScroll->setValue((this->maxToHide - this->percent) / (100.0f - this->percent));
        if (this->percent == 100) {
            MTHScroll->setValue(1);
        }

        if (onMaxToHideChanged != NULL)
            onMaxToHideChanged(this);
    });
    this->addChild(MTHInput);

    if (this->percent == 100) {
        MTHScroll->setValue(1);
        MTHInput->setString("100");
    }

    return true;
}

void PercentCell::RunCallback(CCObject*) {
    if (callback != NULL)
        callback(this);
}

void PercentCell::hide() {
    this->runAction(CCFadeTo::create(0.2f, 0));
    Dev::fadeTextInput(MTHInput, false, .2f);
    Dev::fadeSlider(MTHScroll, false, .2f);
    this->setEnabled(false);
}
void PercentCell::hideInstant() {
    this->setOpacity(0);
    this->setEnabled(false);
    Dev::fadeTextInput(MTHInput, false, 0);
    Dev::fadeSlider(MTHScroll, false, 0);
}

void PercentCell::show() {
    this->runAction(CCFadeTo::create(0.2f, 255));
    Dev::fadeTextInput(MTHInput, true, .2f);
    Dev::fadeSlider(MTHScroll, true, .2f);
    this->setEnabled(true);
}

void PercentCell::mthScroll(CCObject*) {
    int remappedValue = std::lerp(percent, 100, MTHScroll->getValue());

    this->maxToHide = remappedValue;

    MTHInput->setString(std::to_string(remappedValue));
    if (this->percent == 100) {
        MTHInput->setString("100");
    }
}

void PercentCell::sliderEnded(Slider* slider) {
    if (onMaxToHideChanged != NULL)
        onMaxToHideChanged(this);
}

void PercentCell::setMaxToHide(float val) {
    MTHScroll->setValue(val);
    mthScroll(nullptr);
}