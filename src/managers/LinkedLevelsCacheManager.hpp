#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

struct GetLevelsTaskProgress {
    std::string m_message;
    int m_current;
    int m_max;

    GetLevelsTaskProgress(std::string message_, int current_, int max_)
        : m_message(std::move(message_)), m_current(current_), m_max(max_) {}
};

struct LinkData {
    LevelMetadata m_metadata;
    std::string m_levelKey;
};

using GetLevelsFuture = arc::Future<std::vector<LinkData>>;

class LinkedLevelsCacheManager {
public:
    static void getLinkedLevels(
        Function<void(const GetLevelsTaskProgress& progress)> progressCallback = [](auto){},
        Function<void(GetLevelsFuture::Output const&)> completionCallback = [](auto){}
    );
private:
    static GetLevelsFuture getLevelsFuture(Function<void(const GetLevelsTaskProgress& progress)> progressCallback);

    static std::vector<LinkData> m_data;

    static arc::TaskHandle<void> m_getLevelsTask;
};