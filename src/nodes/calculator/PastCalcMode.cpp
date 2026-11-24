#include "PastCalcMode.hpp"

#include <nodes/DTLayer.hpp>
#include <nodes/general/SwitcherButton.hpp>

PastCalcMode* PastCalcMode::create(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate) {
    auto ret = new PastCalcMode();
    if (ret && ret->init(std::move(size), std::move(onVariablesChanged), std::move(onCalculate))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool PastCalcMode::init(CCSize size, Function<void(std::string const&)> onVariablesChanged, Function<void(std::string const&)> onCalculate) {
    if (!CalculatorMode::init(std::move(size), std::move(onVariablesChanged), std::move(onCalculate))) return false;

    m_passPercentInput = TextInput::create(m_size.width / 1.2f, "pass%");
    m_passPercentInput->setCommonFilter(CommonFilter::Uint);
    m_passPercentInput->setCallback([&](auto newStr) {
        sendVars();
    });
    m_passPercentInput->setPosition({m_size.width / 2, m_size.height});
    m_passPercentInput->setAnchorPoint({.5f, 1});
    this->addChild(m_passPercentInput);

    auto passPercentInputLabel = geode::Label::create("Passed percent", "bigFont.fnt");
    passPercentInputLabel->setScale(.45f);
    m_passPercentInput->setPositionY(m_passPercentInput->getPositionY() - passPercentInputLabel->getScaledContentHeight());
    passPercentInputLabel->setPosition(m_passPercentInput->getPosition() + ccp(1.5f, 0));
    passPercentInputLabel->setAnchorPoint({.5f, 0});
    this->addChild(passPercentInputLabel);

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

void PastCalcMode::calculate() {
    if (m_passPercentInput->getString() == "" || DTLayer::get()->m_MyLevelStats.isErr()) return;

    auto numRes = geode::utils::numFromString<int>(m_passPercentInput->getString());
    if (numRes.isErr()) return;
    auto num = numRes.unwrap();

    auto& meStats = DTLayer::get()->m_MyLevelStats.unwrap();

    if (meStats.m_from0.isErr()) return;
    auto& meGen = meStats.m_from0.unwrap();

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
    deadsHandle.setName("DT-past-calculator-deaths-task");

    auto deadsRes = deadsHandle.blockOn();

    if (deadsRes.isErr()) m_onCalculate(deadsRes.unwrapErr().m_error);
    auto deads = deadsRes.unwrap();

    int totalPastDeaths = 0;

    for (const auto& death : deads)
    {
        auto run = StatsManager::splitRunKey(death.first);
        if (run.isErr()) continue;

        if (run.unwrap().m_end > num)
            totalPastDeaths += death.second;
    }
    
    m_onCalculate(fmt::format("{}x", totalPastDeaths));
}

std::string PastCalcMode::getName() const{
    return "Passes";
}

std::string PastCalcMode::getDescription() const{
    return "The amount of times you have passed a percentage from the target run";
}

void PastCalcMode::open() {
    m_passPercentInput->setEnabled(true);
    this->setVisible(true);

    sendVars();
}
void PastCalcMode::close() {
    m_passPercentInput->setEnabled(false);
    this->setVisible(false);
}

void PastCalcMode::sendVars() {
    m_onVariablesChanged(m_passPercentInput->getString() != "" ? fmt::format(
        "{}>",
        m_passPercentInput->getString()
    ) : "");
}