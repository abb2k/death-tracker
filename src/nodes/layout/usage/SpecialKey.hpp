#pragma once

#include <ostream>

#include <any>

using namespace geode::prelude;

struct UpdateFutureError{
    std::string m_error = "";
    bool m_displayAsError = true;

    UpdateFutureError() = default;
    UpdateFutureError(std::string error_, bool displayAsError_ = true)
        : m_error(std::move(error_))
        , m_displayAsError(displayAsError_)
    {}

    UpdateFutureError(const char* error_, bool displayAsError_ = true)
        : m_error(error_ ? error_ : "")
        , m_displayAsError(displayAsError_)
    {}

    friend std::ostream& operator<<(std::ostream& os, UpdateFutureError const& err) {
        return os << err.m_error;
    }
};

template<class T>
using GetTFuture = arc::Future<Result<T, UpdateFutureError>>;

using UpdateFuture = GetTFuture<std::string>;

#ifndef BIND_UPDATE_FUNC
#define BIND_UPDATE_FUNC(METHOD) [&](std::map<std::string, std::any> payload) { return METHOD(payload); }
#endif

class SpecialKey : public CCObject, public std::enable_shared_from_this<SpecialKey> {
    private:
        std::string m_currentContent;

        std::string m_key;
        std::string m_description;
        bool m_showInAddList;

        std::optional<Function<UpdateFuture(std::map<std::string, std::any>)>> m_updateFunction = std::nullopt;

        Function<void(const std::shared_ptr<SpecialKey>&)> m_updateCompletedCallback = nullptr;
        Function<void(const std::shared_ptr<SpecialKey>&)> m_updateStartedCallback = nullptr;

        async::TaskHolder<UpdateFuture::Output> m_updateListener;

        UpdateFuture updateFutureRunner(std::map<std::string, std::any> payload);
        void onUpdateCompleted(UpdateFuture::Output val);

        bool m_updateOngoing = false;

        std::set<std::string> m_toRefreshWith{};

    public:
        SpecialKey(const std::string& key, const std::string& description, bool showInAddList = true);
        ~SpecialKey() {
            //log::info("destroyed special key {}", key);
            cancel();
        }

        bool compareToKey(std::string_view otherKey);

        void setUpdateFunction(Function<UpdateFuture(std::map<std::string, std::any>)> task);

        void updateContent();
        UpdateFuture getUpdatedContentAsync(std::map<std::string, std::any> payload);

        std::string getContent() const{
            return this->m_currentContent;
        }
        std::string getKey() const{
            return this->m_key;
        }
        std::string getDescription() const{
            return this->m_description;
        }
        bool shouldShowInAddList() const {
            return this->m_showInAddList;
        }

        void setUpdateCompletedCallback(Function<void(const std::shared_ptr<SpecialKey>&)> callback);
        void setUpdateStartedCallback(Function<void(const std::shared_ptr<SpecialKey>&)> callback);

        void cancel();

        bool doesRefreshWith(const std::string& other);

        void refreshWith(const std::string& other);
        void refreshWith(const std::set<std::string>& others);
};