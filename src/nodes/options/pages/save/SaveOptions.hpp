#pragma once

#include <nodes/options/OptionsNode.hpp>

using namespace geode::prelude;

class BackupCell;

class SaveOptions : public OptionsNode, public FLAlertLayerProtocol {
public:
    static SaveOptions* create(CCSize size);

    void onOpened() override;
    void onClosed() override;
    
private:
    bool setup() override;

    void onDelete(CCObject*);
    void onBackup(CCObject*);
    void onExport(CCObject*);
    void onImport(CCObject*);

    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

    void createChoiceAlert(std::string_view title, std::string_view desc, std::string_view btn1, std::string_view btn2, Function<void(bool)> callback);
    std::map<FLAlertLayer*, Function<void(bool)>> m_choiceAlertsMap{};

    ScrollLayer* m_backupsScrollLayer;

    void onBackupRevert(BackupCell* cell);
    void onBackupDelete(BackupCell* cell);

    void updateBackupsList();

    async::TaskHolder<file::PickResult> m_filepicklistener;
    void saveToDT(asp::fs::Path const& pick, asp::fs::Path const& source);

    void openSaveDir(CCObject*);
};