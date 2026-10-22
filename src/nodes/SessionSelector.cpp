#include <nodes/SessionSelector.hpp>
#include <utils/Dev.hpp>

SessionSelector* SessionSelector::create(int count) {
    auto ret = new SessionSelector();
    if (ret && ret->init(count)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SessionSelector::init(int count) {
    if (!CCMenu::init()) return false;

    this->setContentSize({ 180, 30 });
    this->setPosition({0, 0});
    this->ignoreAnchorPointForPosition(false);

    inputNode = geode::TextInput::create(120, "Session", "gjFont17.fnt");
    inputNode->setPosition(this->getContentSize() / 2);
    inputNode->getInputNode()->setDelegate(this);
    this->addChild(inputNode);

    auto leftArrowSpr = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    leftArrowSpr->setScale(.4f);
    auto leftArrowBtn = CCMenuItemSpriteExtra::create(
        leftArrowSpr,
        nullptr,
        this,
        menu_selector(SessionSelector::leftArrowClicked)
    );
    leftArrowBtn->setPosition(inputNode->getPosition() - ccp(inputNode->getContentWidth() / 2 + leftArrowSpr->getContentSize().width / 2, 0));
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
    rightArrowBtn->setPosition(inputNode->getPosition() + ccp(inputNode->getContentWidth() / 2 + rightArrowSpr->getContentSize().width / 2, 0));
    this->addChild(rightArrowBtn);

    setMaximumCount(count);
    setCurrentCount(1, false);

    return true;
}

void SessionSelector::leftArrowClicked(CCObject*) {
    setCurrentCount(currentCount - 1, true, true);
}
void SessionSelector::rightArrowClicked(CCObject*) {
    setCurrentCount(currentCount + 1, true, true);
}

void SessionSelector::setCallback(geode::Function<void(int)> callback) {
    this->callback = std::move(callback);
}

void SessionSelector::setMaximumCount(int count, bool runCallback) {
    count = std::max(count, 1);
    maxCount = count;

    if (currentCount > maxCount) {
        setCurrentCount(maxCount, runCallback);
    }

    if (!isInputOpened)
        inputNode->setString(fmt::format("{}/{}", currentCount, maxCount));
}
void SessionSelector::setCurrentCount(int count, bool ignoreIfUnchanged, bool runCallback) {
    if (currentCount == count && ignoreIfUnchanged) return;
    count = std::clamp(count, 1, maxCount);
    currentCount = count;

    std::string newText;

    if (isInputOpened)
        newText = fmt::format("{}", currentCount);
    else
        newText = fmt::format("{}/{}", currentCount, maxCount);

    inputNode->setString(newText);

    if (runCallback && callback)
        callback(currentCount);
}

int SessionSelector::getCurrentCount() {
    return currentCount;
}

void SessionSelector::textInputOpened(CCTextInputNode* input) {
    isInputOpened = true;
    setCurrentCount(currentCount, false);
}
void SessionSelector::textInputClosed(CCTextInputNode* input) {
    isInputOpened = false;
    setCurrentCount(currentCount, false);
}

void SessionSelector::textChanged(CCTextInputNode* input) {
    auto numRes = geode::utils::numFromString<int>(input->getString());

    if (numRes.isErr()) return;

    setCurrentCount(numRes.unwrap(), true, true);
}

void SessionSelector::setEnabled(bool value) {
    inputNode->setEnabled(value);
    CCMenu::setEnabled(value);
}