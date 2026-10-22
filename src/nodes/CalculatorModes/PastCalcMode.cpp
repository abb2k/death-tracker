#include "PastCalcMode.hpp"

#include <nodes/layers/DTLayer.hpp>

#include <nodes/SwitcherButton.hpp>

PastCalcMode* PastCalcMode::create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    auto ret = new PastCalcMode();
    if (ret && ret->init(size, std::move(onVariablesChanged), std::move(onCalculate))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool PastCalcMode::init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    if (!CalculatorMode::init(size, std::move(onVariablesChanged), std::move(onCalculate))) return false;

    passPercentInput = TextInput::create(size.width / 1.2f, "pass%");
    passPercentInput->setCommonFilter(CommonFilter::Uint);
    passPercentInput->setCallback([&](auto newStr) {
        sendVars();
    });
    passPercentInput->setPosition({size.width / 2, size.height});
    passPercentInput->setAnchorPoint({.5f, 1});
    this->addChild(passPercentInput);

    auto passPercentInputLabel = CCLabelBMFont::create("Passed percent", "bigFont.fnt");
    passPercentInputLabel->setScale(.45f);
    passPercentInput->setPositionY(passPercentInput->getPositionY() - passPercentInputLabel->getScaledContentHeight());
    passPercentInputLabel->setPosition(passPercentInput->getPosition() + ccp(1.5f, 0));
    passPercentInputLabel->setAnchorPoint({.5f, 0});
    this->addChild(passPercentInputLabel);

    auto sf0Btn = SwitcherButton::create({size.width / 1.5f / (1 / .85f), 25 / (1 / .85f)}, "GJ_button_01.png", {"General", "Session"});
    sf0Btn->setPosition({size.width / 2, sf0Btn->getScaledContentHeight() / 2 + 2});
    sf0Btn->setOptionChangedCallback([&](int opt) {
        isGeneral = opt == 0;
    });

    auto m = CCMenu::createWithItem(sf0Btn);
    m->setPosition({0,0});
    this->addChild(m);

    return true;
}

void PastCalcMode::calculate() {
    if (passPercentInput->getString() == "" || DTLayer::get()->m_MyLevelStats.isErr()) return;

    auto numRes = geode::utils::numFromString<int>(passPercentInput->getString());
    if (numRes.isErr()) return;
    auto num = numRes.unwrap();

    auto& meStats = DTLayer::get()->m_MyLevelStats.unwrap();

    if (meStats.from0.isErr()) return; 
    auto& meGen = meStats.from0.unwrap();

    auto deadsHandle = async::spawn(DTLayer::get()->getTFor<Deaths>([](GeneralData const& data) {
        Deaths runs{};
        StatsManager::mergeMapsAdd(runs, data.deaths);
        return runs;
    },
    [](auto const& a, auto const& b) {
        auto map = a;
        StatsManager::mergeMapsAdd(map, b);
        return map;
    }, !isGeneral));
    deadsHandle.setName("DT-past-calculator-deaths-task");

    auto deadsRes = deadsHandle.blockOn();

    if (deadsRes.isErr()) onCalculate(deadsRes.unwrapErr().error);
    auto deads = deadsRes.unwrap();

    int totalPastDeaths = 0;

    for (const auto& death : deads)
    {
        auto run = StatsManager::splitRunKey(death.first);
        if (run.isErr()) continue;

        if (run.unwrap().end > num)
            totalPastDeaths += death.second;
    }
    
    onCalculate(fmt::format("{}x", totalPastDeaths));
}

std::string PastCalcMode::getName() const{
    return "Passes";
}

std::string PastCalcMode::getDescription() const{
    return "The amount of times you have passed a percentage from the target run";
}

void PastCalcMode::open() {
    passPercentInput->setEnabled(true);
    this->setVisible(true);

    sendVars();
}
void PastCalcMode::close() {
    passPercentInput->setEnabled(false);
    this->setVisible(false);
}

void PastCalcMode::sendVars() {
    onVariablesChanged(passPercentInput->getString() != "" ? fmt::format(
        "{}>",
        passPercentInput->getString()
    ) : "");
}