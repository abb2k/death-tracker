#include "SessionSelector.hpp"

SessionSelector* SessionSelector::create(int count) {
    auto ret = new SessionSelector();

    if (ret && ret->init(count)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool SessionSelector::init(int count) {
    if (!CCMenu::init()) return false;

    this->setContentSize({ 180, 30 });
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    m_inputNode = geode::TextInput::create(120, "Session", "gjFont17.fnt");
    m_inputNode->setPosition(this->getContentSize() / 2);
    m_inputNode->setCallback([this](std::string const& str) {
        this->textChanged(str);
    });
    m_inputNode->setFocusCallback([this](TextInput::FocusState focused) {
        if (focused == TextInput::FocusState::Focused) {
            this->textInputOpened();
        } else {
            this->textInputClosed();
        }
    });
    this->addChild(m_inputNode);

    auto leftArrowSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    leftArrowSpr->setScale(.4f);
    auto leftArrowBtn = CCMenuItemSpriteExtra::create(
        leftArrowSpr,
        nullptr,
        this,
        menu_selector(SessionSelector::leftArrowClicked)
    );
    leftArrowBtn->setPosition(m_inputNode->getPosition() - ccp(m_inputNode->getContentWidth() / 2 + leftArrowSpr->getContentSize().width / 2, 0));
    leftArrowBtn->setRotation(180);
    leftArrowSpr->setScaleY(-.4f);
    this->addChild(leftArrowBtn);

    auto rightArrowSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    rightArrowSpr->setScale(.4f);
    auto rightArrowBtn = CCMenuItemSpriteExtra::create(
        rightArrowSpr,
        nullptr,
        this,
        menu_selector(SessionSelector::rightArrowClicked)
    );
    rightArrowBtn->setPosition(m_inputNode->getPosition() + ccp(m_inputNode->getContentWidth() / 2 + rightArrowSpr->getContentSize().width / 2, 0));
    this->addChild(rightArrowBtn);

    setMaximumCount(count);
    setCurrentCount(1, false);

    return true;
}

void SessionSelector::leftArrowClicked(CCObject*) {
    setCurrentCount(m_currentCount - 1, true, true);
}
void SessionSelector::rightArrowClicked(CCObject*) {
    setCurrentCount(m_currentCount + 1, true, true);
}

void SessionSelector::setCallback(Function<void(int)> callback) {
    this->m_callback = std::move(callback);
}

void SessionSelector::setMaximumCount(int count, bool runCallback) {
    count = std::max(count, 1);
    m_maxCount = count;

    if (m_currentCount > m_maxCount) {
        setCurrentCount(m_maxCount, runCallback);
    }

    if (!m_isInputOpened)
        m_inputNode->setString(fmt::format("{}/{}", m_currentCount, m_maxCount));
}
void SessionSelector::setCurrentCount(int count, bool ignoreIfUnchanged, bool runCallback) {
    if (m_currentCount == count && ignoreIfUnchanged) return;
    count = std::clamp(count, 1, m_maxCount);
    m_currentCount = count;

    std::string newText;

    if (m_isInputOpened)
        newText = fmt::format("{}", m_currentCount);
    else
        newText = fmt::format("{}/{}", m_currentCount, m_maxCount);

    m_inputNode->setString(newText);

    if (runCallback && m_callback)
        m_callback(m_currentCount);
}

int SessionSelector::getCurrentCount() {
    return m_currentCount;
}

void SessionSelector::textInputOpened() {
    m_isInputOpened = true;
    setCurrentCount(m_currentCount, false);
}
void SessionSelector::textInputClosed() {
    m_isInputOpened = false;
    setCurrentCount(m_currentCount, false);
}

void SessionSelector::textChanged(std::string const& str) {
    auto numRes = geode::utils::numFromString<int>(str);

    if (numRes.isErr()) return;

    setCurrentCount(numRes.unwrap(), true, true);
}

void SessionSelector::setEnabled(bool value) {
    m_inputNode->setEnabled(value);
    CCMenu::setEnabled(value);
}