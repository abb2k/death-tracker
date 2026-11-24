#include "BackupCell.hpp"

#include <managers/StatsManager.hpp>
#include <utils/DateFormatter.hpp>

BackupCell* BackupCell::create(float width, std::string levelKey, long long backupTime) {
    auto ret = new BackupCell();

    if (ret && ret->init(std::move(width), std::move(levelKey), std::move(backupTime))) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool BackupCell::init(float width, std::string levelKey, long long backupTime) {
    if (!CCMenu::init()) return false;

    this->m_levelKey = std::move(levelKey);
    this->m_backupTime = std::move(backupTime);

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

    m_titleLabel = geode::Label::create("Loading data...", "bigFont.fnt");
    m_titleLabel->setAnchorPoint({0, 1});
    m_titleLabel->setLimitLabelWidth(width - folderIcon->getScaledContentWidth() - 10 - revertBtn->getContentWidth() / 2 - 5, .3f);
    m_titleLabel->setPosition({folderIcon->getScaledContentWidth() + 10, this->getContentHeight() / 1.5f + m_titleLabel->getScaledContentHeight() / 2});
    this->addChild(m_titleLabel);

    m_descriptionLabel = geode::Label::create("...", "chatFont.fnt");
    m_descriptionLabel->setAnchorPoint({0, .5f});
    m_descriptionLabel->setLimitLabelWidth(width - folderIcon->getScaledContentWidth() - 10 - revertBtn->getContentWidth() / 2 - 5, .4f);
    m_descriptionLabel->setPosition({folderIcon->getScaledContentWidth() + 10, this->getContentHeight() / 3.0f});
    this->addChild(m_descriptionLabel);

    LoadBackupData();

    return true;
}

void BackupCell::LoadBackupData() {
    m_getBackupListener.spawn(
        "DT-load-backups-data-task",
        LoadBackupDataFuture(),
        [&](GetBackupFuture::Output val) {
            this->onBackupLoaded(val);
        }
    );
}

GetBackupFuture BackupCell::LoadBackupDataFuture() {
    auto backup = StatsManager::getBackupData(this->m_levelKey, this->m_backupTime);

    int deathCount = -1;
    if (backup.isOk() && backup.unwrap().m_from0.has_value()) {
        auto deaths = backup.unwrap().m_from0.value();

        deathCount += static_cast<int>(deaths.m_deaths.totalDeaths());
        deathCount += static_cast<int>(deaths.m_runs.totalDeaths());
    }

    auto backupFileSizeInKB = StatsManager::getBackupFileSize(this->m_levelKey, this->m_backupTime) / 1024.0;

    co_return GetBackupTaskResult{
        .m_backup = backup,
        .m_deathCount = deathCount,
        .m_backupFileSizeInKB = backupFileSizeInKB
    };
}

void BackupCell::onBackupLoaded(GetBackupFuture::Output out) {
    if (out.m_backup.isErr()) {
        m_titleLabel->setString("Failed to load backup data.");
        return;
    }

    m_backupData = out.m_backup.unwrap();

    auto timeTDate = static_cast<time_t>(m_backupData.value().m_backupDate);

    tm timeInfo = {};
#if defined(_WIN32)
    gmtime_s(&timeInfo, &timeTDate);
#else
    gmtime_r(&timeTDate, &timeInfo);
#endif

    m_titleLabel->setString(DateFormatter::timeFormat(timeInfo).c_str());
    m_descriptionLabel->setString(
        fmt::format(
            "Attempts: {}, Sessions: {}, Size: {:.2f} KB", 
            out.m_deathCount,
            m_backupData->m_sessionNames.has_value() ? std::to_string(m_backupData->m_sessionNames.value().size()) : "None",
            out.m_backupFileSizeInKB
        ).c_str()
    );
}

void BackupCell::onRevert(CCObject* sender) {
    if (m_onRevertCallback != NULL)
        m_onRevertCallback(this);
}

void BackupCell::onDelete(CCObject* sender) {
    if (m_onDeleteCallback != NULL)
        m_onDeleteCallback(this);
}

void BackupCell::setOnRevertCallback(Function<void(BackupCell*)> callback) {
    m_onRevertCallback = std::move(callback);
}

void BackupCell::setOnDeleteCallback(Function<void(BackupCell*)> callback) {
    m_onDeleteCallback = std::move(callback);
}