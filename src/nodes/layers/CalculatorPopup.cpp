#include "CalculatorPopup.hpp"

#include <nodes/CalculatorModes/BeforeCalcMode.hpp>
#include <nodes/CalculatorModes/PastCalcMode.hpp>

CalculatorPopup* CalculatorPopup::create() {
    auto ret = new CalculatorPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CalculatorPopup::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(200, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Calculator");

    calcBG = TextInput::create(m_size.width / 1.2f, "NA");
    calcBG->setPosition({
        m_size.width / 2,
        m_title->getPositionY() - m_title->getScaledContentHeight() / 2 - calcBG->getScaledContentHeight() / 2 - 5
    });
    calcBG->getInputNode()->setTouchEnabled(false);
    m_mainLayer->addChild(calcBG);

    auto modeLabel = CCLabelBMFont::create("Mode:", "bigFont.fnt");
    modeLabel->setScale(.5f);
    modeLabel->setPosition({
        m_size.width / 2,
        calcBG->getPositionY() - calcBG->getScaledContentHeight() / 2 - modeLabel->getScaledContentHeight() / 2 - 5
    });
    m_mainLayer->addChild(modeLabel);

    auto menu = CCMenu::create();
    menu->setContentSize({0, 0});
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);

    auto calcBtnSpr = ButtonSprite::create("Calculate", "goldFont.fnt", "GJ_button_03.png");
    calcBtnSpr->setScale(.8f);
    calcBtn = CCMenuItemSpriteExtra::create(
        calcBtnSpr,
        this,
        menu_selector(CalculatorPopup::onCalcBtn)
    );
    calcBtn->setPosition({
        m_size.width / 2,
        calcBtn->getContentHeight() + 1.5f
    });
    menu->addChild(calcBtn);

    auto modeSize = ccp(
        m_size.width / 1.2f,
        (modeLabel->getPositionY() - modeLabel->getScaledContentHeight() / 2) / 1.5f - calcBtn->getPositionY()
    );
    
    addMode(PastCalcMode::create(modeSize, [&](auto str) { this->varsChanged(str); }, [&](auto str) { this->onCalc(str); }));
    addMode(BeforeCalcMode::create(modeSize, [&](auto str) { this->varsChanged(str); }, [&](auto str) { this->onCalc(str); }));

    std::vector<std::string> modesTexts{};
    for (const auto& mode : modes)
    {
        modesTexts.push_back(mode->getName());
    }

    modeButton = SwitcherButton::create({m_size.width / 1.5f, 25}, "GJ_button_01.png", modesTexts);
    modeButton->setPosition({
        m_size.width / 2,
        modeLabel->getPositionY() - modeLabel->getScaledContentHeight() / 2 - modeButton->getScaledContentHeight() / 2 - 5
    });
    modeButton->setOptionChangedCallback([&](auto currOption) {
        modes[currentMode]->close();
        currentMode = currOption;
        modes[currentMode]->open();

        updateText();
    });
    menu->addChild(modeButton);

    auto modeInfoBtnSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    modeInfoBtnSpr->setScale(.65f);
    auto modeInfobtn = CCMenuItemSpriteExtra::create(
        modeInfoBtnSpr,
        this,
        menu_selector(CalculatorPopup::onModeInfo)
    );
    modeInfobtn->setPosition({
        modeButton->getPositionX() + modeButton->getScaledContentWidth() / 2 + modeInfobtn->getScaledContentWidth() / 2 + 5,
        modeButton->getPositionY()
    });
    menu->addChild(modeInfobtn);

    modes[currentMode]->open();

    return true;
}

void CalculatorPopup::onModeInfo(CCObject*) {
    auto modeID = modeButton->getCurrentOptionIndex();

    FLAlertLayer::create(modes[modeID]->getName().c_str(), modes[modeID]->getDescription().c_str(), "OK")->show();
}

void CalculatorPopup::varsChanged(std::string const& vars) {
    currentVars = vars;
    currentCalc = std::nullopt;

    updateText();
}
void CalculatorPopup::onCalc(std::string const& calculation) {
    currentCalc = calculation;

    updateText();
}

void CalculatorPopup::addMode(CalculatorMode* mode) {

    m_mainLayer->addChild(mode);
    mode->close();
    mode->setPosition((m_size.width - mode->getContentWidth()) / 2, (m_size.width - mode->getContentWidth()) / 2 + 1.5f + calcBtn->getPositionY());

    modes.push_back(mode);
}

void CalculatorPopup::updateText() {
    std::string str = currentVars;
    if (currentCalc.has_value())
        str += " = " + currentCalc.value();

    calcBG->setString(str);
}

void CalculatorPopup::onCalcBtn(CCObject*) {
    modes[modeButton->getCurrentOptionIndex()]->calculate();
}