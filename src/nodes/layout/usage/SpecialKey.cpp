#include "SpecialKey.hpp"

SpecialKey::SpecialKey(const std::string& key, const std::string& description, bool showInAddList) {
    this->m_key = key;
    this->m_description = description;
    this->m_showInAddList = showInAddList;
}

bool SpecialKey::compareToKey(std::string_view otherKey) {
    return this->m_key == otherKey;
}

void SpecialKey::setUpdateFunction(Function<UpdateFuture(std::map<std::string, std::any>)> task) {
    this->m_updateFunction = std::move(task);
}

void SpecialKey::updateContent() {
    if (!this->m_updateFunction.has_value()) return;

    // log::info("Starting update for special key {}, {}", this->key, updateListener.isPending());

    if (!m_updateListener.isPending()) {
        if (this->m_updateStartedCallback)
            this->m_updateStartedCallback(this->shared_from_this());
    }
    else {
        cancel();
    }

    m_updateListener.spawn(
        "DT-update-key:" + m_key + "-task",
        updateFutureRunner({}),
        [&](UpdateFuture::Output val) {
            // log::info("completed update for {}", val);
            this->onUpdateCompleted(val);
        }
    );
}

UpdateFuture SpecialKey::getUpdatedContentAsync(std::map<std::string, std::any> payload) {
    co_return co_await updateFutureRunner(payload);
}

UpdateFuture SpecialKey::updateFutureRunner(std::map<std::string, std::any> payload) {
    auto value = co_await this->m_updateFunction.value()(payload);

    co_return value;
}

void SpecialKey::setUpdateCompletedCallback(Function<void(const std::shared_ptr<SpecialKey>&)> callback) {
    this->m_updateCompletedCallback = std::move(callback);
}

void SpecialKey::setUpdateStartedCallback(Function<void(const std::shared_ptr<SpecialKey>&)> callback) {
    this->m_updateStartedCallback = std::move(callback);
}

void SpecialKey::onUpdateCompleted(UpdateFuture::Output val) {
    if (val.isErr()) {
        auto errData = val.unwrapErr();
        if (errData.m_displayAsError)
            this->m_currentContent = fmt::format("Error: {}", errData.m_error);
        else
            this->m_currentContent = errData.m_error;
    }
    else
        this->m_currentContent = val.unwrap();

    if (this->m_updateCompletedCallback)
        this->m_updateCompletedCallback(this->shared_from_this());
}

void SpecialKey::cancel() {
    m_updateListener.cancel();
}

bool SpecialKey::doesRefreshWith(const std::string& other) {
    return m_toRefreshWith.contains(other);
}

void SpecialKey::refreshWith(const std::string& other) {
    if (m_toRefreshWith.contains(other)) return;

    m_toRefreshWith.insert(other);
}

void SpecialKey::refreshWith(const std::set<std::string>& others) {
    for (const auto& other : others)
    {
        refreshWith(other);
    }
    
}