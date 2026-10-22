#include "SectionCell.hpp"
#include <managers/StatsManager.hpp>
#include <utils/Dev.hpp>

SectionCell* SectionCell::create(float width, const Section& section) {
    auto ret = new SectionCell();
    if (ret && ret->init(width, section)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SectionCell::init(float width, const Section& section) {
    if (!CCMenu::init()) return false;

    this->section = section;

    this->setContentSize({width, 40});

    bg = CCScale9Sprite::create("GJ_square05.png");//"square01_001.png"
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChild(bg);

    nameInput = TextInput::create(width / 1.75f, "Section Name", "bigFont.fnt");
    nameInput->setString(section.name);
    nameInput->setAnchorPoint({0, .5});
    nameInput->setScale(.8f);
    nameInput->setPosition({3, this->getContentHeight() / 2});
    nameInput->setCallback([&](const std::string& newText) {
        this->section.name = newText;

        if (this->onChanged) this->onChanged();
    });
    nameInput->setID("name");
    nameInput->setFilter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_#");
    this->addChild(nameInput);

    auto deleteSprite = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    deleteSprite->setScale(.65f);
    auto deleteButton = CCMenuItemSpriteExtra::create(
        deleteSprite,
        this, menu_selector(SectionCell::onDelete)
    );
    deleteButton->setPosition({this->getContentWidth() - deleteButton->getContentWidth() / 2 - 2, this->getContentHeight() / 2});
    this->addChild(deleteButton);

    endPercentInput = TextInput::create(width / 5, "endPer", "bigFont.fnt");
    endPercentInput->setString(std::to_string(section.endPercent));
    endPercentInput->setAnchorPoint({1, .5});
    endPercentInput->setScale(.75f);
    endPercentInput->setCommonFilter(CommonFilter::Uint);
    endPercentInput->setPosition({this->getContentWidth() - deleteButton->getContentWidth() - 3, this->getContentHeight() / 2});
    endPercentInput->setCallback([&](const std::string& newText) {
        auto newPercentRes = utils::numFromString<int>(newText);
        if (newPercentRes.isErr()) return;
        int newPercent = newPercentRes.unwrap();

        if (newPercent > 100) {
            newPercent = 100;
            endPercentInput->setString(std::to_string(newPercent));
        }
        if (newPercent < 0) {
            newPercent = 0;
            endPercentInput->setString(std::to_string(newPercent));
        }
        this->section.endPercent = newPercent;

        endPercentInput->setString(std::to_string(newPercent));

        checkValidity();

        if (this->onChanged) this->onChanged();
    });
    endPercentInput->setID("end-per");
    this->addChild(endPercentInput);

    auto runSeperator = CCLabelBMFont::create("-", "bigFont.fnt");
    runSeperator->setPosition({endPercentInput->getPositionX() - endPercentInput->getScaledContentWidth() - 10, this->getContentHeight() / 2});
    runSeperator->setScale(.75f);
    this->addChild(runSeperator);

    startPercentInput = TextInput::create(width / 5, "startPer", "bigFont.fnt");
    startPercentInput->setString(std::to_string(section.startPercent));
    startPercentInput->setAnchorPoint({1, .5});
    startPercentInput->setScale(.75f);
    startPercentInput->setCommonFilter(CommonFilter::Uint);
    startPercentInput->setPosition({runSeperator->getPositionX() - 5 - runSeperator->getScaledContentWidth() / 2, this->getContentHeight() / 2});
    startPercentInput->setCallback([&](const std::string& newText) {
        auto newPercentRes = utils::numFromString<int>(newText);
        if (newPercentRes.isErr()) return;
        int newPercent = newPercentRes.unwrap();

        if (newPercent > 100) {
            newPercent = 100;
            startPercentInput->setString(std::to_string(newPercent));
        }
        if (newPercent < 0) {
            newPercent = 0;
            startPercentInput->setString(std::to_string(newPercent));
        }
        this->section.startPercent = newPercent;

        checkValidity();

        if (this->onChanged) this->onChanged();
    });
    startPercentInput->setID("start-per");
    this->addChild(startPercentInput);

    checkValidity();

    return true;
}

void SectionCell::onDelete(CCObject* sender) {
    this->removeFromParent();
    if (this->onDeleted) this->onDeleted();
}

void SectionCell::checkValidity() {
    if (section.isValid()) {
        bg->setColor({255, 255, 255});
    } else {
        bg->setColor({255, 100, 100});
    }
}

void SectionCell::fade(bool in, float time) {
    if (in) {
        this->runAction(CCFadeTo::create(time, 255));
        this->setEnabled(true);
        Dev::fadeTextInput(startPercentInput, true, time);
        Dev::fadeTextInput(endPercentInput, true, time);
        Dev::fadeTextInput(nameInput, true, time);
    }
    else {
        this->runAction(CCFadeTo::create(time, 0));
        this->setEnabled(false);
        Dev::fadeTextInput(startPercentInput, false, time);
        Dev::fadeTextInput(endPercentInput, false, time);
        Dev::fadeTextInput(nameInput, false, time);
    }
}