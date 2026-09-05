#include "BackupCell.hpp"
#include <managers/StatsManager.hpp>
#include <utils/DateFormatter.hpp>

BackupCell* BackupCell::create(float width, const std::string& levelKey, const long long backupTime){
    auto ret = new BackupCell();
    if (ret && ret->init(width, levelKey, backupTime)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool BackupCell::init(float width, const std::string& levelKey, const long long backupTime){
    if (!CCMenu::init()) return false;

    this->levelKey = levelKey;
    this->backupTime = backupTime;

    this->setContentSize({width, 40});

    auto bg = CCScale9Sprite::create("GJ_square01.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.5f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChild(bg);

    auto folderIcon = CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png");
    folderIcon->setScale(.75f);
    folderIcon->setPosition({folderIcon->getScaledContentWidth() / 2 + 5, this->getContentHeight() / 2});
    this->addChild(folderIcon);

    auto revertBtnSpr = ButtonSprite::create(
        "Revert",
        "bigFont.fnt",
        "GJ_button_03.png"
    );
    revertBtnSpr->setScale(.35f);
    revertBtnSpr->setCascadeOpacityEnabled(true);
    auto revertBtn = CCMenuItemSpriteExtra::create(
        revertBtnSpr,
        this,
        menu_selector(BackupCell::onRevert)
    );
    revertBtn->setID("revert-btn");
    revertBtn->setPosition({width - revertBtn->getContentWidth() / 2 - 5, this->getContentHeight() / 1.5f});
    this->addChild(revertBtn);

    auto deleteBackupBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    deleteBackupBtnSpr->setScale(.35f);
    deleteBackupBtnSpr->setCascadeOpacityEnabled(true);
    auto deleteBackupBtn = CCMenuItemSpriteExtra::create(
        deleteBackupBtnSpr,
        this,
        menu_selector(BackupCell::onDelete)
    );
    deleteBackupBtn->setID("delete-btn");
    deleteBackupBtn->setPosition({revertBtn->getPositionX(), this->getContentHeight() / 3.0f - 1});
    this->addChild(deleteBackupBtn);

    titleLabel = CCLabelBMFont::create("Loading data...", "bigFont.fnt");
    titleLabel->setScale(.3f);
    titleLabel->setAnchorPoint({0, 1});
    titleLabel->setWidth(width - folderIcon->getScaledContentWidth() - 10 - revertBtn->getContentWidth() / 2 - 5);
    titleLabel->setPosition({folderIcon->getScaledContentWidth() + 10, this->getContentHeight() / 1.5f + titleLabel->getScaledContentHeight() / 2});
    this->addChild(titleLabel);

    descriptionLabel = CCLabelBMFont::create("...", "chatFont.fnt");
    descriptionLabel->setScale(.4f);
    descriptionLabel->setAnchorPoint({0, .5f});
    descriptionLabel->setWidth(width - folderIcon->getScaledContentWidth() - 10 - revertBtn->getContentWidth() / 2 - 5);
    descriptionLabel->setPosition({folderIcon->getScaledContentWidth() + 10, this->getContentHeight() / 3.0f});
    this->addChild(descriptionLabel);

    LoadBackupData();

    return true;
}

void BackupCell::LoadBackupData(){
    getBackupListener.spawn(
        "DT-load-backups-data-task",
        LoadBackupDataFuture(),
        [&](GetBackupFuture::Output val) {
            this->onBackupLoaded(val);
        }
    );
}

GetBackupFuture BackupCell::LoadBackupDataFuture(){
    auto backup = StatsManager::getBackupData(this->levelKey, this->backupTime);

    int deathCount = -1;
    if (backup.isOk() && backup.unwrap().from0.has_value()){
        auto deaths = backup.unwrap().from0.value();

        for (const auto& death : deaths.deaths)
            deathCount += death.second;

        for (const auto& death : deaths.runs)
            deathCount += death.second;
    }

    auto backupFileSizeInKB = StatsManager::getBackupFileSize(this->levelKey, this->backupTime) / 1024.0;

    co_return GetBackupTaskResult{
        .backup = backup,
        .deathCount = deathCount,
        .backupFileSizeInKB = backupFileSizeInKB
    };
}

void BackupCell::onBackupLoaded(GetBackupFuture::Output out){
    if (out.backup.isErr()){
        titleLabel->setString("Failed to load backup data.");
        return;
    }

    backupData = out.backup.unwrap();

    auto timeTDate = static_cast<time_t>(backupData.value().backupDate);

    tm timeInfo = {};
#if defined(_WIN32)
    gmtime_s(&timeInfo, &timeTDate);
#else
    gmtime_r(&timeTDate, &timeInfo);
#endif

    titleLabel->setString(DateFormatter::format(timeInfo).c_str());
    descriptionLabel->setString(
        fmt::format(
            "Attempts: {}, Sessions: {}, Size: {:.2f} KB", 
            out.deathCount,
            backupData->sessionNames.has_value() ? std::to_string(backupData->sessionNames.value().size()) : "None",
            out.backupFileSizeInKB
        ).c_str()
    );
}

void BackupCell::onRevert(CCObject* sender){
    if (onRevertCallback != NULL)
        onRevertCallback(this);
}

void BackupCell::onDelete(CCObject* sender){
    if (onDeleteCallback != NULL)
        onDeleteCallback(this);
}