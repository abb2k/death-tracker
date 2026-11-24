#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

struct GetBackupTaskResult {
    Result<BackupLevelData> m_backup;
    int m_deathCount;
    double m_backupFileSizeInKB;
};

using GetBackupFuture = arc::Future<GetBackupTaskResult>;

class BackupCell : public CCMenu {
    public:
        static BackupCell* create(float width, const std::string& levelKey, const long long backupTime);

        Function<void(BackupCell*)> m_onRevertCallback = NULL;
        Function<void(BackupCell*)> m_onDeleteCallback = NULL;

        std::string getLevelKey() const {
            return m_levelKey;
        }
        long long getBackupTime() const {
            return m_backupTime;
        }
        BackupLevelData* getBackupData() {
            return m_backupData ? &m_backupData.value() : nullptr;
        }

    private:
        bool init(float width, const std::string& levelKey, const long long backupTime);

        std::string m_levelKey;
        long long m_backupTime;

        void LoadBackupData();
        GetBackupFuture LoadBackupDataFuture();
        async::TaskHolder<GetBackupFuture::Output> m_getBackupListener;
        void onBackupLoaded(GetBackupFuture::Output out);

        std::optional<BackupLevelData> m_backupData = std::nullopt;

        geode::Label* m_titleLabel;
        geode::Label* m_descriptionLabel;

        void onRevert(CCObject* sender);
        void onDelete(CCObject* sender);
};