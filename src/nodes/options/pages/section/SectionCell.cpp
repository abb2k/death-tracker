#include "SectionCell.hpp"

#include <utils/Dev.hpp>

SectionCell* SectionCell::create(float width, Section section) {
    auto ret = new SectionCell();

    if (ret && ret->init(width, std::move(section))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool SectionCell::init(float width, Section section) {
    if (!CCMenu::init()) return false;

    this->m_section = std::move(section);

    this->setContentSize({width, 40});

    m_bg = CCScale9Sprite::create("GJ_square05.png");//"square01_001.png"
    m_bg->setAnchorPoint({0, 0});
    m_bg->setScale(.5f);
    m_bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / m_bg->getScale());
    this->addChild(m_bg);

    m_nameInput = TextInput::create(width / 1.75f, "Section Name", "bigFont.fnt");
    m_nameInput->setString(m_section.m_name);
    m_nameInput->setAnchorPoint({0, .5});
    m_nameInput->setScale(.8f);
    m_nameInput->setPosition({3, this->getContentHeight() / 2});
    m_nameInput->setCallback([&](const std::string& newText) {
        this->m_section.m_name = newText;

        if (this->m_onChanged) this->m_onChanged();
    });
    m_nameInput->setID("name");
    m_nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_#");
    this->addChild(m_nameInput);

    auto deleteSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    deleteSprite->setScale(.65f);
    auto deleteButton = CCMenuItemSpriteExtra::create(
        deleteSprite,
        this, menu_selector(SectionCell::onDelete)
    );
    deleteButton->setPosition({this->getContentWidth() - deleteButton->getContentWidth() / 2 - 2, this->getContentHeight() / 2});
    this->addChild(deleteButton);

    m_endPercentInput = TextInput::create(width / 5, "endPer", "bigFont.fnt");
    m_endPercentInput->setString(std::to_string(m_section.m_endPercent));
    m_endPercentInput->setAnchorPoint({1, .5});
    m_endPercentInput->setScale(.75f);
    m_endPercentInput->setCommonFilter(CommonFilter::Uint);
    m_endPercentInput->setPosition({this->getContentWidth() - deleteButton->getContentWidth() - 3, this->getContentHeight() / 2});
    m_endPercentInput->setCallback([&](const std::string& newText) {
        auto newPercentRes = utils::numFromString<int>(newText);
        if (newPercentRes.isErr()) return;
        int newPercent = newPercentRes.unwrap();

        if (newPercent > 100) {
            newPercent = 100;
            m_endPercentInput->setString(std::to_string(newPercent));
        }
        if (newPercent < 0) {
            newPercent = 0;
            m_endPercentInput->setString(std::to_string(newPercent));
        }
        this->m_section.m_endPercent = newPercent;

        m_endPercentInput->setString(std::to_string(newPercent));

        checkValidity();

        if (this->m_onChanged) this->m_onChanged();
    });
    m_endPercentInput->setID("end-per");
    this->addChild(m_endPercentInput);

    auto runSeperator = geode::Label::create("-", "bigFont.fnt");
    runSeperator->setPosition({m_endPercentInput->getPositionX() - m_endPercentInput->getScaledContentWidth() - 10, this->getContentHeight() / 2});
    runSeperator->setScale(.75f);
    this->addChild(runSeperator);

    m_startPercentInput = TextInput::create(width / 5, "startPer", "bigFont.fnt");
    m_startPercentInput->setString(std::to_string(m_section.m_startPercent));
    m_startPercentInput->setAnchorPoint({1, .5});
    m_startPercentInput->setScale(.75f);
    m_startPercentInput->setCommonFilter(CommonFilter::Uint);
    m_startPercentInput->setPosition({runSeperator->getPositionX() - 5 - runSeperator->getScaledContentWidth() / 2, this->getContentHeight() / 2});
    m_startPercentInput->setCallback([&](const std::string& newText) {
        auto newPercentRes = utils::numFromString<int>(newText);
        if (newPercentRes.isErr()) return;
        int newPercent = newPercentRes.unwrap();

        if (newPercent > 100) {
            newPercent = 100;
            m_startPercentInput->setString(std::to_string(newPercent));
        }
        if (newPercent < 0) {
            newPercent = 0;
            m_startPercentInput->setString(std::to_string(newPercent));
        }
        this->m_section.m_startPercent = newPercent;

        checkValidity();

        if (this->m_onChanged) this->m_onChanged();
    });
    m_startPercentInput->setID("start-per");
    this->addChild(m_startPercentInput);

    checkValidity();

    return true;
}

void SectionCell::onDelete(CCObject* sender) {
    this->removeFromParent();
    if (this->m_onDeleted) this->m_onDeleted();
}

void SectionCell::checkValidity() {
    if (m_section.isValid()) {
        m_bg->setColor({255, 255, 255});
    } else {
        m_bg->setColor({255, 100, 100});
    }
}

void SectionCell::fade(bool in, float time) {
    if (in) {
        this->runAction(CCFadeTo::create(time, 255));
        this->setEnabled(true);
        Dev::fadeTextInput(m_startPercentInput, true, time);
        Dev::fadeTextInput(m_endPercentInput, true, time);
        Dev::fadeTextInput(m_nameInput, true, time);
    }
    else {
        this->runAction(CCFadeTo::create(time, 0));
        this->setEnabled(false);
        Dev::fadeTextInput(m_startPercentInput, false, time);
        Dev::fadeTextInput(m_endPercentInput, false, time);
        Dev::fadeTextInput(m_nameInput, false, time);
    }
}

void SectionCell::setOnChanged(Function<void()> onChanged) {
    m_onChanged = std::move(onChanged);
}

void SectionCell::setOnDeleted(Function<void()> onDeleted) {
    m_onDeleted = std::move(onDeleted);
}