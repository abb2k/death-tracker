#pragma once

#include <ostream>

using namespace geode::prelude;

struct UpdateFutureError{
    std::string error = "";
    bool displayAsError = true;

    UpdateFutureError() = default;
    UpdateFutureError(std::string error_, bool displayAsError_ = true)
        : error(std::move(error_))
        , displayAsError(displayAsError_)
    {}

    UpdateFutureError(const char* error_, bool displayAsError_ = true)
        : error(error_ ? error_ : "")
        , displayAsError(displayAsError_)
    {}

    friend std::ostream& operator<<(std::ostream& os, UpdateFutureError const& err) {
        return os << err.error;
    }
};

using UpdateFuture = arc::Future<Result<std::string, UpdateFutureError>>;

#ifndef BIND_UPDATE_FUNC
#define BIND_UPDATE_FUNC(METHOD) [&](std::map<std::string, std::any> payload) { return METHOD(payload); }
#endif

class SpecialKey : public CCObject, public std::enable_shared_from_this<SpecialKey> {
    private:
        std::string currentContent;

        std::string key;
        std::string description;

        std::optional<geode::Function<UpdateFuture(std::map<std::string, std::any>)>> updateFunction = std::nullopt;

        geode::Function<void(const std::shared_ptr<SpecialKey>&)> updateCompletedCallback = nullptr;
        geode::Function<void(const std::shared_ptr<SpecialKey>&)> updateStartedCallback = nullptr;

        async::TaskHolder<UpdateFuture::Output> updateListener;

        UpdateFuture updateFutureRunner(std::map<std::string, std::any> payload);
        void onUpdateCompleted(UpdateFuture::Output val);

        bool updateOngoing = false;

        std::set<std::string> toRefreshWith{};

    public:
        SpecialKey(const std::string& key, const std::string& description);
        ~SpecialKey() {
            //log::info("destroyed special key {}", key);
            cancel();
        }

        bool compareToKey(const std::string& otherKey);

        void setUpdateFunction(geode::Function<UpdateFuture(std::map<std::string, std::any>)> task);

        void updateContent();
        UpdateFuture getUpdatedContentAsync(std::map<std::string, std::any> payload);

        std::string getContent() const{
            return this->currentContent;
        }
        std::string getKey() const{
            return this->key;
        }
        std::string getDescription() const{
            return this->description;
        }

        void setUpdateCompletedCallback(geode::Function<void(const std::shared_ptr<SpecialKey>&)> callback);
        void setUpdateStartedCallback(geode::Function<void(const std::shared_ptr<SpecialKey>&)> callback);

        void cancel();

        bool doesRefreshWith(const std::string& other);

        void refreshWith(const std::string& other);
        void refreshWith(const std::set<std::string>& others);
};