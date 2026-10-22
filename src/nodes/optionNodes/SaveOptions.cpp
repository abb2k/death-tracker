#include <nodes/optionNodes/SaveOptions.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/layers/DTLevelSpecificSettingsLayer.hpp>
#include <utils/Dev.hpp>
#include <nodes/layers/CreateBackupPopup.hpp>
#include <nodes/layers/ImportPopup.hpp>

/*
-- auto backup settings --

- toggle auto backups on/off
- create on level exit
- create on pause
- create on exiting death tracker menu

*/

SaveOptions* SaveOptions::create(const CCSize& size) {
    auto ret = new SaveOptions();
    if (ret && ret->initWithSetup(size)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool SaveOptions::setup() {

    auto deleteBtnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
    auto deleteBtn = CCMenuItemSpriteExtra::create(
        deleteBtnSpr,
        this,
        menu_selector(SaveOptions::onDelete)
    );
    deleteBtn->setPosition({deleteBtn->getContentWidth() / 2 + 5, deleteBtn->getContentHeight() / 2 + 5});
    this->addChild(deleteBtn);

    auto deleteBtnLabel = CCLabelBMFont::create("Delete Save", "bigFont.fnt");
    deleteBtnLabel->setScale(.45f);
    deleteBtnLabel->setWidth(size.width / 2 - deleteBtn->getContentWidth());
    deleteBtnLabel->setAnchorPoint({0, .5f});
    deleteBtnLabel->setPosition(deleteBtn->getPosition() + ccp(deleteBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(deleteBtnLabel);

    auto openSaveBtnSpr = CCSprite::createWithSpriteFrameName("backArrowPlain_01_001.png");
    auto openSaveBtn = CCMenuItemSpriteExtra::create(
        openSaveBtnSpr,
        this,
        menu_selector(SaveOptions::openSaveDir)
    );
    openSaveBtn->setPosition(deleteBtn->getPosition() + ccp(0, deleteBtn->getScaledContentHeight()));
    this->addChild(openSaveBtn);

    auto openSaveBtnLabel = CCLabelBMFont::create("Open save directory", "bigFont.fnt");
    openSaveBtnLabel->setScale(.45f);
    openSaveBtnLabel->setWidth(size.width / 2 - openSaveBtn->getContentWidth());
    openSaveBtnLabel->setAnchorPoint({0, .5f});
    openSaveBtnLabel->setPosition(openSaveBtn->getPosition() + ccp(openSaveBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(openSaveBtnLabel);

    auto backupBtnSpr = CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
    backupBtnSpr->setScale(.7f);
    auto backupBtn = CCMenuItemSpriteExtra::create(
        backupBtnSpr,
        this,
        menu_selector(SaveOptions::onBackup)
    );
    backupBtn->setPosition({size.width / 2 + backupBtn->getContentWidth() / 2, size.height - backupBtn->getContentHeight() / 2});
    this->addChild(backupBtn);

    auto backupBtnLabel = CCLabelBMFont::create("Create Backup", "bigFont.fnt");
    backupBtnLabel->setScale(.6f);
    backupBtnLabel->setWidth(size.width / 2 - backupBtn->getContentWidth());
    backupBtnLabel->setAnchorPoint({0, .5f});
    backupBtnLabel->setPosition(backupBtn->getPosition() + ccp(backupBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(backupBtnLabel);

    auto autoBackupToggler = SimpleToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        .6f,
        DTLayer::get()->m_MyLevelStats.isOk() ? DTLayer::get()->m_MyLevelStats.unwrap().metadata.autoBackup : false
    );
    autoBackupToggler->setCallback([&](bool toggled) {
        if (DTLayer::get()->m_MyLevelStats.isErr()) return;

        auto& stats = DTLayer::get()->m_MyLevelStats.unwrap();
        stats.metadata.autoBackup = toggled;
        (void)StatsManager::setMetadata(stats.metadata, stats.levelKey);
    });
    autoBackupToggler->setPosition(backupBtn->getPosition() - ccp(0, backupBtn->getContentWidth() / 2 + autoBackupToggler->getContentWidth() / 2 + 5));
    this->addChild(autoBackupToggler);

    auto autoBackupsLabel = CCLabelBMFont::create("Auto Backups", "bigFont.fnt");
    autoBackupsLabel->setScale(.6f);
    autoBackupsLabel->setWidth(size.width / 2 - autoBackupToggler->getContentWidth());
    autoBackupsLabel->setAnchorPoint({0, .5f});
    autoBackupsLabel->setPosition(autoBackupToggler->getPosition() + ccp(autoBackupToggler->getContentWidth() / 2 + 5, 0));
    this->addChild(autoBackupsLabel);

    auto backupsScrollLabel = CCLabelBMFont::create("Backups", "bigFont.fnt");
    backupsScrollLabel->setScale(.6f);
    backupsScrollLabel->setWidth(size.width / 2);
    backupsScrollLabel->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    backupsScrollLabel->setPosition({autoBackupsLabel->getPositionX() + autoBackupsLabel->getScaledContentWidth() / 2, autoBackupsLabel->getPositionY() - autoBackupsLabel->getContentHeight() / 2 - backupsScrollLabel->getContentHeight() / 2 - 5});
    this->addChild(backupsScrollLabel);

    backupsScrollLayer = ScrollLayer::create({size.width / 2 - 10, backupsScrollLabel->getPositionY() - backupsScrollLabel->getContentHeight() / 2 - 5});
    backupsScrollLayer->setPositionX(backupsScrollLabel->getPositionX());
    backupsScrollLayer->setPositionY(5 + backupsScrollLayer->getContentHeight() / 2);
    backupsScrollLayer->ignoreAnchorPointForPosition(false);
    auto backupsScrollBG = CCScale9Sprite::create("square02_small.png");
    backupsScrollBG->setContentSize(backupsScrollLayer->getContentSize());
    backupsScrollBG->setZOrder(-1);
    backupsScrollBG->ignoreAnchorPointForPosition(true);
    backupsScrollBG->setID("bg");
    backupsScrollLayer->addChild(backupsScrollBG);
    backupsScrollLayer->m_contentLayer->setLayout(ColumnLayout::create()
        ->setAutoGrowAxis(backupsScrollLayer->getContentHeight())
        ->setAxisAlignment(AxisAlignment::End)
        ->setCrossAxisOverflow(false)
    );
    this->addChild(backupsScrollLayer);

    updateBackupsList();

    auto exportBtnSpr = CCSprite::createWithSpriteFrameName("GJ_shareBtn_001.png");
    exportBtnSpr->setScale(0.5f);
    auto exportBtn = CCMenuItemSpriteExtra::create(
        exportBtnSpr,
        this,
        menu_selector(SaveOptions::onExport)
    );
    exportBtn->setPosition({exportBtn->getContentWidth() / 2 + 5, size.height - exportBtn->getContentHeight() / 2});
    this->addChild(exportBtn);

    auto exportBtnLabel = CCLabelBMFont::create("Export Save", "bigFont.fnt");
    exportBtnLabel->setScale(.6f);
    exportBtnLabel->setWidth(size.width / 2 - exportBtn->getContentWidth());
    exportBtnLabel->setAnchorPoint({0, .5f});
    exportBtnLabel->setPosition(exportBtn->getPosition() + ccp(exportBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(exportBtnLabel);

    auto importBtnSpr = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    // @geode-ignore(unknown-resource)
    auto importBtnSpr1 = CCSprite::createWithSpriteFrameName("geode.loader/install.png");
    importBtnSpr1->setPosition(importBtnSpr->getContentSize() / 2);
    importBtnSpr1->setScale(1.2f);
    importBtnSpr->addChild(importBtnSpr1);
    importBtnSpr->setCascadeOpacityEnabled(true);
    importBtnSpr->setScale(0.75f);
    auto importBtn = CCMenuItemSpriteExtra::create(
        importBtnSpr,
        this,
        menu_selector(SaveOptions::onImport)
    );
    importBtn->setPosition(exportBtn->getPosition() - ccp(0, exportBtn->getContentHeight() / 2 + importBtn->getContentHeight() / 2 + 5));
    this->addChild(importBtn);

    auto importBtnLabel = CCLabelBMFont::create("Import Save", "bigFont.fnt");
    importBtnLabel->setScale(.6f);
    importBtnLabel->setWidth(size.width / 2 - importBtn->getContentWidth());
    importBtnLabel->setAnchorPoint({0, .5f});
    importBtnLabel->setPosition(importBtn->getPosition() + ccp(importBtn->getContentWidth() / 2 + 5, 0));
    this->addChild(importBtnLabel);

    auto saveOverallInfo = TutorialButton::create(.75f, "save-overall", [
        &, 
        backupBtnLabel, 
        backupBtn, 
        autoBackupsLabel, 
        autoBackupToggler, 
        exportBtn, 
        exportBtnLabel,
        importBtn,
        importBtnLabel,
        deleteBtn,
        deleteBtnLabel
    ](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("Here you have some options to manage your save", TutorialCharacterFace::TCFNormal)
            ->appendDialogue("Here you are able to <cy>create backups</c> of your save", TutorialCharacterFace::TCFNormalTilted)
            ->joinHighlight(backupBtn)
            ->joinHighlight(backupBtnLabel)
            ->joinTransform(TutorialBoxPlacement::TBPBottomRight, .75f)
            ->appendDialogue("And also toggle <cy>auto backups</c> for this specific level!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(autoBackupToggler)
            ->joinHighlight(autoBackupsLabel)
            ->appendDialogue("You can also <cp>export</c> your save to pass it around to another device", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(exportBtn)
            ->joinHighlight(exportBtnLabel)
            ->joinTransform(TutorialBoxPlacement::TBPBottomLeft, .75f)
            ->appendDialogue("And also <cj>import</c> a save that someone else sent you!", TutorialCharacterFace::TCFHappy)
            ->joinHighlight(importBtn)
            ->joinHighlight(importBtnLabel)
            ->appendDialogue("And lastly you are able to <cr>completely erase your save data</c> for this level", TutorialCharacterFace::TCFNormal)
            ->joinHighlight(deleteBtn)
            ->joinHighlight(deleteBtnLabel)
            ->joinTransform(TutorialBoxPlacement::TBPLeft, .75f);
    });
    saveOverallInfo->setPosition(size - saveOverallInfo->getScaledContentSize() / 2 + ccp(2, 2));
    this->addChild(saveOverallInfo);

    auto saveBackupsInfo = TutorialButton::create(.75f, "save-backup", [&, backupsScrollLabel](DTTutorialLayer* tutorialLayer) {
        tutorialLayer->appendDialogue("Here you have a list of all your backups for this level", TutorialCharacterFace::TCFNormal)
            ->joinTransform(TutorialBoxPlacement::TBPBottomLeft, .65f)
            ->joinHighlight(backupsScrollLabel)
            ->joinHighlight(backupsScrollLayer)
            ->appendDialogue("On each backup you have some info", TutorialCharacterFace::TCFNormalTilted);
        for (const auto& backupCell : backupsScrollLayer->m_contentLayer->getChildrenExt<CCNode*>())
        {
            tutorialLayer->joinHighlight(backupCell);
        }
        tutorialLayer->appendDialogue("A <cp>revert</c> button for <cr>overriding</c> your current save with the backup", TutorialCharacterFace::TCFNormal);
        for (const auto& backupCell : backupsScrollLayer->m_contentLayer->getChildrenExt<CCNode*>())
        {
            tutorialLayer->joinHighlight(backupCell->getChildByID("revert-btn"));
        }
        tutorialLayer->appendDialogue("And a <cr>trashcan</c> button for <cr>deleting</c> a backup!", TutorialCharacterFace::TCFHappy);
        for (const auto& backupCell : backupsScrollLayer->m_contentLayer->getChildrenExt<CCNode*>())
        {
            tutorialLayer->joinHighlight(backupCell->getChildByID("delete-btn"));
        }
    });
    saveBackupsInfo->setPosition(backupsScrollLabel->getPosition() + ccp(backupsScrollLabel->getScaledContentWidth() / 2 + saveBackupsInfo->getScaledContentWidth() / 2, 0));
    this->addChild(saveBackupsInfo);

    this->setEnabled(false);
    this->setOpacity(0);
    backupsScrollBG->setOpacity(0);
    for (const auto& child : CCArrayExt<BackupCell*>(backupsScrollLayer->m_contentLayer->getChildren())) {
        child->setOpacity(0);
        child->setEnabled(false);
    }
    
    backupsScrollLayer->setMouseEnabled(false);

    return true;
}

void SaveOptions::onOpened() {
    float fadeTime = .2f;
    this->setEnabled(true);
    this->runAction(CCFadeTo::create(fadeTime, 255));
    backupsScrollLayer->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 100));
    for (const auto& child : backupsScrollLayer->m_contentLayer->getChildrenExt<BackupCell*>()) {
        child->runAction(CCFadeTo::create(fadeTime, 255));
        child->setEnabled(true);
    }

    backupsScrollLayer->setMouseEnabled(true);
}
void SaveOptions::onClosed() {
    float fadeTime = .2f;
    this->setEnabled(false);
    this->runAction(CCFadeTo::create(fadeTime, 0));
    backupsScrollLayer->getChildByID("bg")->runAction(CCFadeTo::create(fadeTime, 0));
    for (const auto& child : backupsScrollLayer->m_contentLayer->getChildrenExt<BackupCell*>()) {
        child->runAction(CCFadeTo::create(fadeTime, 0));
        child->setEnabled(false);
    }

    backupsScrollLayer->setMouseEnabled(false);
}

void SaveOptions::onDelete(CCObject*) {
    createChoiceAlert("WARNING!", "Doing this will delete ALL saved progress from this levels death tracker save.\nAre you sure you want to do this?", "No", "Yes", [&](bool btn2) {
        if (btn2) {
            if (DTLayer::get()->DeleteSave())
                settingsLayer->keyBackClicked();
        }
    });
}

void SaveOptions::onBackup(CCObject*) {
    if (DTLayer::get()->m_MyLevelStats.isErr()) return;

    auto popup = CreateBackupPopup::create();
    popup->setCallback([this](bool general, std::optional<int> sessions) {
        StatsManager::addBackup(DTLayer::get()->m_MyLevelStats.unwrap().levelKey, general, sessions, true);
        
        updateBackupsList();
    });

    popup->show();
}


void SaveOptions::FLAlert_Clicked(FLAlertLayer* layer, bool btn2) {
    if (!choiceAlertsMap.contains(layer)) return;

    choiceAlertsMap[layer](btn2);

    choiceAlertsMap.erase(layer);
}

void SaveOptions::createChoiceAlert(const std::string& title, const std::string& desc, const std::string& btn1, const std::string& btn2, geode::Function<void(bool)> callback) {
    auto alert = FLAlertLayer::create(this, title.c_str(), desc, btn1.c_str(), btn2.c_str());
    alert->show();
    choiceAlertsMap.insert({alert, std::move(callback)});
}

void SaveOptions::onBackupRevert(BackupCell* cell) {
    if (cell->getBackupData() == nullptr) return;

    createChoiceAlert("WARNING!", "Reverting to this backup will overwrite your current death tracker save for this level.\nAre you sure you want to do this?", "No", "Yes", [&, cell](bool btn2) {
        if (btn2) {
            Result<> metaRes = Ok();
            Result<> generalRes = Ok();
            Result<> sessionsRes = Ok();

            if (cell->getBackupData()->meta.has_value())
                metaRes = StatsManager::setMetadata(cell->getBackupData()->meta.value(), cell->getLevelKey());
            if (cell->getBackupData()->from0.has_value())
                generalRes = StatsManager::setGeneral(cell->getBackupData()->from0.value(), cell->getLevelKey());
            if (cell->getBackupData()->sessionNames.has_value()) {
                sessionsRes = StatsManager::reveretBackupSessions(cell->getLevelKey(), cell->getBackupTime());
            }

            if (generalRes.isErr() || sessionsRes.isErr() || metaRes.isErr()) {
                if (metaRes.isErr())
                    log::error("{}", metaRes.unwrapErr());
                if (generalRes.isErr())
                    log::error("{}", generalRes.unwrapErr());
                if (sessionsRes.isErr())
                    log::error("{}", sessionsRes.unwrapErr());

                geode::Notification::create("Failed to revert to backup!", NotificationIcon::Error)->show();
            }
            else geode::Notification::create("Successfully reverted to backup!", NotificationIcon::Success)->show();

            DTLayer::get()->onClose(nullptr);
            settingsLayer->keyBackClicked();
        }
    });
}
void SaveOptions::onBackupDelete(BackupCell* cell) {
    createChoiceAlert("WARNING!", "Deleting this backup is irreversible.\nAre you sure you want to do this?", "No", "Yes", [&, cell](bool btn2) {
        if (btn2) {
            auto deleteBackupRes = StatsManager::deleteBackup(cell->getLevelKey(), cell->getBackupTime());
            if (deleteBackupRes.isErr()) {
                log::error("{}", deleteBackupRes.unwrapErr());
                return;
            }

            updateBackupsList();
        }
    });
}

void SaveOptions::updateBackupsList() {
    backupsScrollLayer->m_contentLayer->removeAllChildren();

    auto lvlKey = StatsManager::getLevelKey(DTLayer::get()->m_Level);

    if (lvlKey.isOk()) {
        auto backupsList = StatsManager::getBackupsCount(lvlKey.unwrap());
        int z = 0;
        for (const auto& backupTime : backupsList) {
            auto cell = BackupCell::create(backupsScrollLayer->getContentWidth() - 5, lvlKey.unwrap(), backupTime);
            cell->onRevertCallback = std::bind(&SaveOptions::onBackupRevert, this, std::placeholders::_1);
            cell->onDeleteCallback = std::bind(&SaveOptions::onBackupDelete, this, std::placeholders::_1);
            backupsScrollLayer->m_contentLayer->addChild(cell);

            if (!this->isEnabled()) {
                cell->setOpacity(0);
                cell->setEnabled(false);
            }
            cell->setZOrder(z++);
        }
    }
    

    backupsScrollLayer->m_contentLayer->updateLayout();
    backupsScrollLayer->moveToTop();
}

void SaveOptions::onExport(CCObject*) {
    auto dtLayer = DTLayer::get();
    if (dtLayer == nullptr || dtLayer->m_MyLevelStats.isErr()) return;

    filepicklistener.spawn(
        "DT-export-picker-task",
        file::pick(file::PickMode::SaveFile, file::FilePickOptions{
            .defaultPath = dtLayer->m_MyLevelStats.unwrap().metadata.levelName + ".dt",
            .filters = {
                file::FilePickOptions::Filter{
                    .description = "Death Tracker file",
                    .files = {
                        "*.dt"
                    }
                },
                file::FilePickOptions::Filter{
                    .description = "CSV file",
                    .files = {
                        "*.csv"
                    }
                }
            }
        }),
        [&](file::PickResult result) {
            auto dtLayer = DTLayer::get();
            if (dtLayer == nullptr || dtLayer->m_MyLevelStats.isErr()) return;

            if (result.isErr()) {
                log::error("{}", result.unwrapErr());
                return;
            }
            auto pickOpt = result.unwrap();
            if (!pickOpt.has_value()) return;
            auto pick = pickOpt.value();

            auto source = StatsManager::getSavesFolderPath() / dtLayer->m_MyLevelStats.unwrap().levelKey;

            // if (pick.extension() == ".csv") {
            //     saveToCSV(pick, source);
            //     return;
            // }

            pick = pick.replace_extension(".dt");

            saveToDT(pick, source);
        }
    );
}

void SaveOptions::onImport(CCObject*) {
    ImportPopup::create()->show();
}

void SaveOptions::saveToDT(std::filesystem::path const& pick, std::filesystem::path const& source) {
    auto zipFileRes = geode::utils::file::Zip::create(pick);
    if (zipFileRes.isErr()) {
        Notification::create("Failed to create zip file!", NotificationIcon::Error)->show();
        log::error("{}", zipFileRes.unwrapErr());
        return;
    }

    auto zip = std::move(zipFileRes).unwrap();

    for (const auto& entry : std::filesystem::directory_iterator(source)) {
        Result<> didFileWrite = Err("");
        
        if (entry.path().filename() == "backups") continue;

        if (std::filesystem::is_directory(entry)) {
            didFileWrite = zip.addAllFrom(entry);
        }
        else {
            didFileWrite = zip.addFrom(entry);
        }

        if (didFileWrite.isErr()) {
            Notification::create("Failed to write zip file!", NotificationIcon::Error)->show();
            return;
        }
    }
}

void SaveOptions::openSaveDir(CCObject*) {
    auto dtLayer = DTLayer::get();
    if (dtLayer == nullptr) return;

    auto lvlKeyRes = StatsManager::getLevelKey(DTLayer::get()->m_Level);
    if (lvlKeyRes.isErr()) return;

    file::openFolder(StatsManager::getSavesFolderPath() / lvlKeyRes.unwrap());
}