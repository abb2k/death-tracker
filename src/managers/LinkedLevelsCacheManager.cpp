#include "LinkedLevelsCacheManager.hpp"

#include <managers/StatsManager.hpp>

std::vector<LinkData> LinkedLevelsCacheManager::m_data{};

arc::TaskHandle<void> LinkedLevelsCacheManager::m_getLevelsTask{};

void LinkedLevelsCacheManager::getLinkedLevels(
    Function<void(const GetLevelsTaskProgress& progress)> progressCallback,
    Function<void(GetLevelsFuture::Output const&)> completionCallback
) {
    m_getLevelsTask.detach();

    m_getLevelsTask = async::spawn(
        getLevelsFuture(std::move(progressCallback)),
        [completionCallback = std::move(completionCallback)](GetLevelsFuture::Output out) mutable {
            m_data = std::move(out);

            completionCallback(m_data);
        }
    );
}

GetLevelsFuture LinkedLevelsCacheManager::getLevelsFuture(
    Function<void(const GetLevelsTaskProgress& progress)> progressCallback
) {
    auto progresObject = GetLevelsTaskProgress{"Getting list of played levels...", -1, -1};

    progressCallback(progresObject);

    auto allLevels = StatsManager::allV3FileLevelKeys();
    if (allLevels.empty()) co_return std::vector<LinkData>{};

    progresObject.m_message = "Getting level data...";
    progresObject.m_current = 0;
    progresObject.m_max = static_cast<int>(allLevels.size());

    progressCallback(progresObject);

    std::vector<LinkData> metadatas{};
    metadatas.reserve(allLevels.size());

    int i = 0;
    for (const auto& levelKey : allLevels)
    {
        auto meta = StatsManager::getMetadata(levelKey);
        if (meta.isOk()) {
            LinkData ldata{};
            ldata.m_metadata = meta.unwrap();
            ldata.m_levelKey = levelKey;

            metadatas.emplace_back(std::move(ldata));
        }

        co_await arc::yield();

        i++;
        progresObject.m_current = i;
        progressCallback(progresObject);
    }
    
    co_return metadatas;
}