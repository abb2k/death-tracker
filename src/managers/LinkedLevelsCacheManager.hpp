#pragma once

#include <Geode/Geode.hpp>

#include <types/DTTypes.hpp>

#include <functional>

using namespace geode::prelude;

struct GetLevelsTaskProgress {
    std::string message;
    int current;
    int max;

    GetLevelsTaskProgress(std::string message_, int current_, int max_)
        : message(std::move(message_)), current(current_), max(max_) {}
};

struct LinkData {
    LevelMetadata metadata;
    std::string levelKey;
};

using GetLevelsFuture = arc::Future<std::vector<LinkData>>;

class LinkedLevelsCacheManager {
    public:
        static void getLinkedLevels(Function<void(const GetLevelsTaskProgress& progress)> progressCallback = [](auto){}, Function<void(GetLevelsFuture::Output const&)> completionCallback = [](auto){});
    private:
        static GetLevelsFuture getLevelsFuture(Function<void(const GetLevelsTaskProgress& progress)> progressCallback);

        static std::vector<LinkData> data;

        static arc::TaskHandle<void> getLevelsTask;
};