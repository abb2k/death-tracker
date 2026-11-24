#include "BeforeCalcMode.hpp"

#include <nodes/DTLayer.hpp>
#include <nodes/general/SwitcherButton.hpp>

BeforeCalcMode* BeforeCalcMode::create(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate) {
    auto ret = new BeforeCalcMode();

    if (ret && ret->init(std::move(size), std::move(onVariablesChanged), std::move(onCalculate))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool BeforeCalcMode::init(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate) {
    if (!CalculatorMode::init(std::move(size), std::move(onVariablesChanged), std::move(onCalculate))) return false;

    m_beforePercentInput = TextInput::create(m_size.width / 1.2f, "before%");
    m_beforePercentInput->setCommonFilter(CommonFilter::Uint);
    m_beforePercentInput->setCallback([&](auto newStr) {
        sendVars();
    });
    m_beforePercentInput->setPosition({m_size.width / 2, m_size.height});
    m_beforePercentInput->setAnchorPoint({.5f, 1});
    this->addChild(m_beforePercentInput);

    auto beforePercentInputLabel = geode::Label::create("Before percent", "bigFont.fnt");
    beforePercentInputLabel->setScale(.45f);
    m_beforePercentInput->setPositionY(m_beforePercentInput->getPositionY() - beforePercentInputLabel->getScaledContentHeight());
    beforePercentInputLabel->setPosition(m_beforePercentInput->getPosition() + ccp(1.5f, 0));
    beforePercentInputLabel->setAnchorPoint({.5f, 0});
    this->addChild(beforePercentInputLabel);

    auto sf0Btn = SwitcherButton::create({m_size.width / 1.5f / (1 / .85f), 25 / (1 / .85f)}, "GJ_button_01.png", {"General", "Session"});
    sf0Btn->setPosition({m_size.width / 2, sf0Btn->getScaledContentHeight() / 2 + 2});
    sf0Btn->setOptionChangedCallback([&](int opt) {
        m_isGeneral = opt == 0;
    });
    
    auto m = CCMenu::createWithItem(sf0Btn);
    m->setPosition({0,0});
    this->addChild(m);

    return true;
}

void BeforeCalcMode::calculate() {
    if (m_beforePercentInput->getString() == "" || DTLayer::get()->m_MyLevelStats.isErr()) return;

    auto numRes = geode::utils::numFromString<int>(m_beforePercentInput->getString());
    if (numRes.isErr()) return;
    auto num = numRes.unwrap();

    auto& meStats = DTLayer::get()->m_MyLevelStats.unwrap();

    if (meStats.m_from0.isErr()) return;

    auto deadsHandle = async::spawn(DTLayer::get()->getTFor<Deaths>([](GeneralData const& data) {
        Deaths runs{};
        StatsManager::mergeMapsAdd(runs, data.m_deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, !m_isGeneral));
    deadsHandle.setName("DT-before-calculator-deaths-task");

    auto deadsRes = deadsHandle.blockOn();

    if (deadsRes.isErr()) m_onCalculate(deadsRes.unwrapErr().m_error);
    auto deads = deadsRes.unwrap();

    int totalPastDeaths = 0;

    for (const auto& death : deads)
    {
        auto run = StatsManager::splitRunKey(death.first);
        if (run.isErr()) continue;

        if (run.unwrap().m_end < num)
            totalPastDeaths += death.second;
    }
    
    m_onCalculate(fmt::format("{}x", totalPastDeaths));
}

std::string BeforeCalcMode::getName() const{
    return "Before";
}
std::string BeforeCalcMode::getDescription() const{
    return "The amount of times you didnt get to a percentage from the target run";
}

void BeforeCalcMode::open() {
    m_beforePercentInput->setEnabled(true);
    this->setVisible(true);

    sendVars();
}
void BeforeCalcMode::close() {
    m_beforePercentInput->setEnabled(false);
    this->setVisible(false);
}

void BeforeCalcMode::sendVars() {
    m_onVariablesChanged(m_beforePercentInput->getString() != "" ? fmt::format(
        "{}<",
        m_beforePercentInput->getString()
    ) : "");
}