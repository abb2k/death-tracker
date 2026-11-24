#include "SessionCell.hpp"

#include <managers/StatsManager.hpp>
#include <utils/DateFormatter.hpp>
#include <nodes/general/SimpleToggler.hpp>

SessionCell* SessionCell::create(float width, Session session) {
    auto ret = new SessionCell();

    if (ret && ret->init(width, std::move(session))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool SessionCell::init(float width, Session session) {
    if (!CCMenu::init()) return false;

    this->m_session = std::move(session);

    this->setContentSize({width, 30});

    auto bg = CCScale9Sprite::create("GJ_square05.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChild(bg);

    std::tm tm{};
    std::time_t time = static_cast<std::time_t>(m_session.m_sessionStartDate);
    #if defined(_WIN32)
        localtime_s(&tm, &time);
    #else
        localtime_r(&time, &tm);
    #endif

    std::string dateStr = DateFormatter::timeFormat(tm);

    auto dateLabel = geode::Label::create(dateStr.c_str(), "bigFont.fnt");
    dateLabel->setPosition({5, this->getContentHeight() - 7.5f});
    dateLabel->setAnchorPoint({0, 1});
    dateLabel->setScale(.2f);
    this->addChild(dateLabel);

    auto deathCount = m_session.m_data.m_deaths.totalDeaths();
    auto runCount = m_session.m_data.m_runs.totalDeaths();

    auto infoLabel = geode::Label::create(
        fmt::format("deaths: {} | runs: {} | playtime: {}", deathCount, runCount, StatsManager::workingTime(m_session.m_data.m_playtimeGeneral.m_playtimeF0 + m_session.m_data.m_playtimeGeneral.m_playtimeRuns)).c_str(),
        "chatFont.fnt"
    );
    infoLabel->setPosition({5, 5});
    infoLabel->setAnchorPoint({0, 0});
    infoLabel->setScale(.35f);
    this->addChild(infoLabel);

    m_toggler = SimpleToggler::createWithDefaults(
        .4f,
        true
    );
    m_toggler->setPosition({
        this->getContentWidth() - m_toggler->getContentWidth() / 2 - 5,
        this->getContentHeight() - m_toggler->getContentHeight() / 2 - 5
    });
    m_toggler->setCallback([&](auto state) {
        m_selected = state;
    });
    this->addChild(m_toggler);

    return true;
}

void SessionCell::setSelected(bool b) {
    if (m_selected != b)
        m_toggler->toggleWithCallback(b);
}