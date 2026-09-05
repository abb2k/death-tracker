#include "SessionCell.hpp"
#include <managers/StatsManager.hpp>
#include <utils/DateFormatter.hpp>

SessionCell* SessionCell::create(float width, Session const& session){
    auto ret = new SessionCell();
    if (ret && ret->init(width, session)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SessionCell::init(float width, Session const& session){
    if (!CCMenu::init()) return false;

    this->session = session;

    this->setContentSize({width, 30});

    auto bg = CCScale9Sprite::create("GJ_square05.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChild(bg);

    auto tp = std::chrono::system_clock::from_time_t(session.sessionStartDate);

    std::string dateStr = DateFormatter::format(tp);

    auto dateLabel = CCLabelBMFont::create(dateStr.c_str(), "bigFont.fnt");
    dateLabel->setPosition({5, this->getContentHeight() - 7.5f});
    dateLabel->setAnchorPoint({0, 1});
    dateLabel->setScale(.2f);
    this->addChild(dateLabel);

    long long deathCount = 0;

    for (const auto& death : session.data.deaths)
    {
        deathCount += death.second;
    }
    
    long long runCount = 0;

    for (const auto& run : session.data.runs)
    {
        runCount += run.second;
    }

    auto infoLabel = CCLabelBMFont::create(
        fmt::format("deaths: {} | runs: {} | playtime: {}", deathCount, runCount, StatsManager::workingTime(session.data.playtimeGeneral.playtimeF0 + session.data.playtimeGeneral.playtimeRuns)).c_str(),
        "chatFont.fnt"
    );
    infoLabel->setPosition({5, 5});
    infoLabel->setAnchorPoint({0, 0});
    infoLabel->setScale(.35f);
    this->addChild(infoLabel);

    toggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .4f,
        true
    );
    toggler->setPosition({
        this->getContentWidth() - toggler->getContentWidth() / 2 - 5,
        this->getContentHeight() - toggler->getContentHeight() / 2 - 5
    });
    toggler->setCallback([&](auto state){
        selected = state;
    });
    this->addChild(toggler);

    return true;
}

void SessionCell::setSelected(bool b){
    if (selected != b)
        toggler->toggleWithCallback(b);
}