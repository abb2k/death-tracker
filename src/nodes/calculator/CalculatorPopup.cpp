#include "CalculatorPopup.hpp"

#include <nodes/general/SwitcherButton.hpp>

#include <nodes/calculator/BeforeCalcMode.hpp>
#include <nodes/calculator/PastCalcMode.hpp>

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
    if (!Popup::init(200, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Calculator");

    m_calcBG = TextInput::create(m_size.width / 1.2f, "NA");
    m_calcBG->setPosition({
        m_size.width / 2,
        m_title->getPositionY() - m_title->getScaledContentHeight() / 2 - m_calcBG->getScaledContentHeight() / 2 - 5
    });
    CCTouchDispatcher::get()->removeDelegate(m_calcBG);
    CCMouseDispatcher::get()->removeDelegate(m_calcBG);
    m_mainLayer->addChild(m_calcBG);

    auto modeLabel = geode::Label::create("Mode:", "bigFont.fnt");
    modeLabel->setScale(.5f);
    modeLabel->setPosition({
        m_size.width / 2,
        m_calcBG->getPositionY() - m_calcBG->getScaledContentHeight() / 2 - modeLabel->getScaledContentHeight() / 2 - 5
    });
    m_mainLayer->addChild(modeLabel);

    auto menu = CCMenu::create();
    menu->setContentSize({0, 0});
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);

    auto calcBtnSpr = ButtonSprite::create("Calculate", "goldFont.fnt", "GJ_button_03.png");
    calcBtnSpr->setScale(.8f);
    m_calcBtn = CCMenuItemSpriteExtra::create(
        calcBtnSpr,
        this,
        menu_selector(CalculatorPopup::onCalcBtn)
    );
    m_calcBtn->setPosition({
        m_size.width / 2,
        m_calcBtn->getContentHeight() + 1.5f
    });
    menu->addChild(m_calcBtn);

    auto modeSize = ccp(
        m_size.width / 1.2f,
        (modeLabel->getPositionY() - modeLabel->getScaledContentHeight() / 2) / 1.5f - m_calcBtn->getPositionY()
    );
    
    addMode(PastCalcMode::create(modeSize, [&](auto str) { this->varsChanged(str); }, [&](auto str) { this->onCalc(str); }));
    addMode(BeforeCalcMode::create(modeSize, [&](auto str) { this->varsChanged(str); }, [&](auto str) { this->onCalc(str); }));

    std::vector<std::string> modesTexts{};
    for (const auto& mode : m_modes)
    {
        modesTexts.push_back(mode->getName());
    }

    m_modeButton = SwitcherButton::create({m_size.width / 1.5f, 25}, "GJ_button_01.png", modesTexts);
    m_modeButton->setPosition({
        m_size.width / 2,
        modeLabel->getPositionY() - modeLabel->getScaledContentHeight() / 2 - m_modeButton->getScaledContentHeight() / 2 - 5
    });
    m_modeButton->setOptionChangedCallback([&](auto currOption) {
        m_modes[m_currentMode]->close();
        m_currentMode = currOption;
        m_modes[m_currentMode]->open();

        updateText();
    });
    menu->addChild(m_modeButton);

    auto modeInfoBtnSpr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    modeInfoBtnSpr->setScale(.65f);
    auto modeInfobtn = CCMenuItemSpriteExtra::create(
        modeInfoBtnSpr,
        this,
        menu_selector(CalculatorPopup::onModeInfo)
    );
    modeInfobtn->setPosition({
        m_modeButton->getPositionX() + m_modeButton->getScaledContentWidth() / 2 + modeInfobtn->getScaledContentWidth() / 2 + 5,
        m_modeButton->getPositionY()
    });
    menu->addChild(modeInfobtn);

    m_modes[m_currentMode]->open();

    return true;
}

void CalculatorPopup::onModeInfo(CCObject*) {
    auto modeID = m_modeButton->getCurrentOptionIndex();

    FLAlertLayer::create(m_modes[modeID]->getName().c_str(), m_modes[modeID]->getDescription().c_str(), "OK")->show();
}

void CalculatorPopup::varsChanged(std::string const& vars) {
    m_currentVars = vars;
    m_currentCalc = std::nullopt;

    updateText();
}
void CalculatorPopup::onCalc(std::string const& calculation) {
    m_currentCalc = calculation;

    updateText();
}

void CalculatorPopup::addMode(CalculatorMode* mode) {
    m_mainLayer->addChild(mode);
    mode->close();
    mode->setPosition((m_size.width - mode->getContentWidth()) / 2, (m_size.width - mode->getContentWidth()) / 2 + 1.5f + m_calcBtn->getPositionY());

    m_modes.push_back(mode);
}

void CalculatorPopup::updateText() {
    std::string str = m_currentVars;
    if (m_currentCalc.has_value())
        str += " = " + m_currentCalc.value();

    m_calcBG->setString(str);
}

void CalculatorPopup::onCalcBtn(CCObject*) {
    m_modes[m_modeButton->getCurrentOptionIndex()]->calculate();
}