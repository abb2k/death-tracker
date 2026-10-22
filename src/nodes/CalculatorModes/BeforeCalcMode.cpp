#include "BeforeCalcMode.hpp"

#include <nodes/layers/DTLayer.hpp>

#include <nodes/SwitcherButton.hpp>

BeforeCalcMode* BeforeCalcMode::create(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    auto ret = new BeforeCalcMode();
    if (ret && ret->init(size, std::move(onVariablesChanged), std::move(onCalculate))) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool BeforeCalcMode::init(CCSize const& size, geode::Function<void(std::string const&)>&& onVariablesChanged, geode::Function<void(std::string const&)>&& onCalculate) {
    if (!CalculatorMode::init(size, std::move(onVariablesChanged), std::move(onCalculate))) return false;

    beforePercentInput = TextInput::create(size.width / 1.2f, "before%");
    beforePercentInput->setCommonFilter(CommonFilter::Uint);
    beforePercentInput->setCallback([&](auto newStr) {
        sendVars();
    });
    beforePercentInput->setPosition({size.width / 2, size.height});
    beforePercentInput->setAnchorPoint({.5f, 1});
    this->addChild(beforePercentInput);

    auto beforePercentInputLabel = CCLabelBMFont::create("Before percent", "bigFont.fnt");
    beforePercentInputLabel->setScale(.45f);
    beforePercentInput->setPositionY(beforePercentInput->getPositionY() - beforePercentInputLabel->getScaledContentHeight());
    beforePercentInputLabel->setPosition(beforePercentInput->getPosition() + ccp(1.5f, 0));
    beforePercentInputLabel->setAnchorPoint({.5f, 0});
    this->addChild(beforePercentInputLabel);

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

void BeforeCalcMode::calculate() {
    if (beforePercentInput->getString() == "" || DTLayer::get()->m_MyLevelStats.isErr()) return;

    auto numRes = geode::utils::numFromString<int>(beforePercentInput->getString());
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
    deadsHandle.setName("DT-before-calculator-deaths-task");

    auto deadsRes = deadsHandle.blockOn();

    if (deadsRes.isErr()) onCalculate(deadsRes.unwrapErr().error);
    auto deads = deadsRes.unwrap();

    int totalPastDeaths = 0;

    for (const auto& death : deads)
    {
        auto run = StatsManager::splitRunKey(death.first);
        if (run.isErr()) continue;

        if (run.unwrap().end < num)
            totalPastDeaths += death.second;
    }
    
    onCalculate(fmt::format("{}x", totalPastDeaths));
}

std::string BeforeCalcMode::getName() const{
    return "Before";
}
std::string BeforeCalcMode::getDescription() const{
    return "The amount of times you didnt get to a percentage from the target run";
}

void BeforeCalcMode::open() {
    beforePercentInput->setEnabled(true);
    this->setVisible(true);

    sendVars();
}
void BeforeCalcMode::close() {
    beforePercentInput->setEnabled(false);
    this->setVisible(false);
}

void BeforeCalcMode::sendVars() {
    onVariablesChanged(beforePercentInput->getString() != "" ? fmt::format(
        "{}<",
        beforePercentInput->getString()
    ) : "");
}