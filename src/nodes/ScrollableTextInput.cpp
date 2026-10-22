#include "ScrollableTextInput.hpp"

ScrollableTextInput* ScrollableTextInput::create(float width, const std::string& placeholder, const std::string& font) {
    auto ret = new ScrollableTextInput();
    if (ret && ret->init(width, placeholder, font)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool ScrollableTextInput::init(float width, const std::string& placeholder, const std::string& font) {
    if (!CCNode::init()) return false;

    this->placeholder = placeholder;
    this->font = font;
    this->width = width;

    this->setContentSize({width, 38});
    this->setAnchorPoint({.5f, .5f});

    auto scrollLayerHolder = CCNode::create();
    scrollLayerHolder->setPositionY(10);
    this->addChild(scrollLayerHolder);

    scrollLayer = new CCScrollLayerExt({0, 0, width, width});
    scrollLayerHolder->addChild(scrollLayer);
    scrollLayer->setTouchEnabled(false);
    scrollLayer->autorelease();

    bar = Scrollbar::create(scrollLayer);
    bar->setRotation(90);
    bar->setAnchorPoint({.5f, 0});
    bar->setPositionY(4);
    this->addChild(bar);

    updateTextInput();

    scheduleUpdate();
    
    return true;
}

void ScrollableTextInput::updateTextInput() {
    if (textInput == nullptr) {
        recreateTextInput(0);
    }
    else {
        recreateTextInput(
            (
                textInput->getInputNode()->m_textLabel->getContentWidth() * 
                .6f - 
                (width - 10)
            )
        );
    }
}

void ScrollableTextInput::recreateTextInput(float extraWidth) {
    std::string oldText;
    int oldCursorPos = 0;

    bool doFocus = isFocused;

    if (textInput != nullptr) {
        oldText = textInput->getString();
        oldCursorPos = textInput->getInputNode()->m_textField->m_uCursorPos;
        textInput->removeMeAndCleanup();
    }

    float newWidth = width + extraWidth;

    textInput = TextInput::create(newWidth <= width ? width : newWidth, placeholder.c_str(), font.c_str());
    textInput->setString(oldText);
    textInput->setDelegate(this);
    #if !defined(GEODE_IS_MOBILE)
    if (doFocus) {
        textInput->focus();
    }
    #endif
    textInput->getInputNode()->m_textField->m_uCursorPos = oldCursorPos;
    textInput->getInputNode()->updateBlinkLabelToChar(oldCursorPos);
    textInput->setAnchorPoint({0, 0});
    scrollLayer->addChild(textInput);

    scrollLayer->m_contentLayer->setContentHeight(textInput->getContentWidth());

    textInput->setCommonFilter(filter);
    textInput->setEnabled(enabled);
}

void ScrollableTextInput::textChanged(CCTextInputNode* node) {
    #if defined(GEODE_IS_MOBILE)
    if (androidTextChangedCounter == -1) {
        androidTextChangedCounter = textInput->getString().size();
        return;
    }
    else if (androidTextChangedCounter > 0) {
        androidTextChangedCounter--;
        return;
    }

    androidTextChangedCounter = -1;
    #endif

    updateTextInput();

    if (onInput != NULL)
        onInput(textInput->getString());
}

void ScrollableTextInput::textInputOpened(CCTextInputNode* node) {
    isFocused = true;
}

void ScrollableTextInput::textInputClosed(CCTextInputNode* node) {
    isFocused = false;
}

void ScrollableTextInput::update(float dt) {
    if (textInput->getInputNode()->m_cursor->isVisible()) {
        if (!prevXursorPos.has_value()) {
            prevXursorPos = textInput->getInputNode()->m_cursor->getPositionX();
        }
        else if (prevXursorPos.has_value() && prevXursorPos.value() != textInput->getInputNode()->m_cursor->getPositionX()) {
            auto nextPos = textInput->getInputNode()->m_cursor->getPositionX() + (textInput->getInputNode()->getContentWidth() - width - 10) / 2;

            if (nextPos < 0) 
                nextPos = 0;
            else if (nextPos > scrollLayer->m_contentLayer->getContentHeight() - scrollLayer->getContentHeight())
                nextPos = scrollLayer->m_contentLayer->getContentHeight() - scrollLayer->getContentHeight();

            scrollLayer->m_contentLayer->setPositionY(-nextPos);

            prevXursorPos = textInput->getInputNode()->m_cursor->getPositionX();
        }
    }

    textInput->setPositionX(scrollLayer->m_contentLayer->getPositionY());
}

void ScrollableTextInput::setCommonFilter(CommonFilter filter) {
    this->filter = filter;
    textInput->setCommonFilter(filter);
}

void ScrollableTextInput::setCallback(geode::Function<void(std::string const&)> onInput) {
    this->onInput = std::move(onInput);
}

void ScrollableTextInput::setString(std::string const& str, bool triggerCallback) {
    textInput->setString(str);

    updateTextInput();

    if (triggerCallback && onInput != NULL)
        onInput(str);
}

void ScrollableTextInput::setEnabled(bool enabled) {
    textInput->setEnabled(enabled);
    bar->setTouchEnabled(enabled);
    if (!enabled) {
        scrollLayer->m_contentLayer->setPositionY(0);
    }
    this->enabled = enabled;
}