#include "CreateBackupPopup.hpp"
#include <nodes/SimpleToggler.hpp>

CreateBackupPopup* CreateBackupPopup::create() {
    auto ret = new CreateBackupPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CreateBackupPopup::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(250, 120, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Create Backup");

    auto saveGeneralLabel = CCLabelBMFont::create("Save General Progress:", "bigFont.fnt");
    saveGeneralLabel->setScale(.4f);
    saveGeneralLabel->setPosition({5, m_size.height - saveGeneralLabel->getScaledContentHeight() / 2 - 30});
    saveGeneralLabel->setAnchorPoint({0, 1});
    m_mainLayer->addChild(saveGeneralLabel);

    auto btnMenu = CCMenu::create();
    btnMenu->setPosition({0,0});
    m_mainLayer->addChild(btnMenu);

    auto saveGeneralToggler = SimpleToggler::createWithDefaults(
        .6f,
        true
    );
    saveGeneralToggler->setPosition({m_size.width - saveGeneralToggler->getScaledContentWidth() / 2 - 5, saveGeneralLabel->getPositionY() - saveGeneralLabel->getScaledContentHeight() / 2});
    saveGeneralToggler->setCallback([this](bool state) {
        this->general = state;
    });
    btnMenu->addChild(saveGeneralToggler);

    auto saveSessionLabel = CCLabelBMFont::create("Sessions Amount:", "bigFont.fnt");
    saveSessionLabel->setScale(.4f);
    saveSessionLabel->setPosition({5, saveGeneralLabel->getPositionY() - saveGeneralLabel->getScaledContentHeight() - 20});
    saveSessionLabel->setAnchorPoint({0, 1});
    m_mainLayer->addChild(saveSessionLabel);

    auto saveSessionsInput = TextInput::create(65, "Amount", "bigFont.fnt");
    saveSessionsInput->setPosition({m_size.width - saveSessionsInput->getScaledContentWidth() / 2 - 5, saveSessionLabel->getPositionY() - saveSessionLabel->getScaledContentHeight() / 2});
    saveSessionsInput->setCommonFilter(CommonFilter::Uint);
    saveSessionsInput->setString("All");
    m_mainLayer->addChild(saveSessionsInput);

    auto saveSessionsToggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .6f,
        true
    );
    saveSessionsToggler->setPosition(saveSessionsInput->getPosition() - ccp(saveSessionsInput->getContentWidth() / 2 + saveSessionsToggler->getScaledContentWidth() / 2 + 5, 0));
    btnMenu->addChild(saveSessionsToggler);

    saveSessionsInput->setCallback([&](std::string const& str) {
        auto numRes = utils::numFromString<int>(str);
        if (numRes.isErr()) return;
        this->sessions = numRes.unwrap();
    });

    saveSessionsToggler->setCallback([&, saveSessionsInput](bool state) {
        if (state) {
            this->sessions = std::nullopt;
            saveSessionsInput->setString("All");
            saveSessionsInput->setEnabled(false);
            this->sessions = -1;
        }
        else {
            saveSessionsInput->setString("2");
            saveSessionsInput->setEnabled(true);
            this->sessions = 2;
        }
    });

    if (saveSessionsInput->getString() == "All") saveSessionsInput->setEnabled(false);

    auto createBtnSpr = ButtonSprite::create("Create");
    createBtnSpr->setScale(.5f);
    auto createBtn = CCMenuItemSpriteExtra::create(
        createBtnSpr,
        this,
        menu_selector(CreateBackupPopup::createBackupClicked)
    );
    createBtn->setPosition({m_size.width / 2, createBtn->getScaledContentHeight() / 2 + 10});
    btnMenu->addChild(createBtn);

    return true;
}

void CreateBackupPopup::createBackupClicked(cocos2d::CCObject* sender) {
    if (callback != NULL) {
        callback(this->general, this->sessions);
    }

    this->onClose(sender);
}