#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

struct GetBackupTaskResult {
    Result<BackupLevelData> backup;
    int deathCount;
    double backupFileSizeInKB;
};

using GetBackupFuture = arc::Future<GetBackupTaskResult>;

class BackupCell : public CCMenu {
    public:
        static BackupCell* create(float width, const std::string& levelKey, const long long backupTime);

        geode::Function<void(BackupCell*)> onRevertCallback = NULL;
        geode::Function<void(BackupCell*)> onDeleteCallback = NULL;

        std::string getLevelKey() const {
            return levelKey;
        }
        long long getBackupTime() const {
            return backupTime;
        }
        BackupLevelData* getBackupData() {
            return backupData ? &backupData.value() : nullptr;
        }

    private:
        bool init(float width, const std::string& levelKey, const long long backupTime);

        std::string levelKey;
        long long backupTime;

        void LoadBackupData();
        GetBackupFuture LoadBackupDataFuture();
        async::TaskHolder<GetBackupFuture::Output> getBackupListener;
        void onBackupLoaded(GetBackupFuture::Output out);

        std::optional<BackupLevelData> backupData = std::nullopt;

        CCLabelBMFont* titleLabel;
        CCLabelBMFont* descriptionLabel;

        void onRevert(CCObject* sender);
        void onDelete(CCObject* sender);
};