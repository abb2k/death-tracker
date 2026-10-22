#include "SpecialKey.hpp"

SpecialKey::SpecialKey(const std::string& key, const std::string& description) {
    this->key = key;
    this->description = description;
}

bool SpecialKey::compareToKey(const std::string& otherKey) {
    return this->key == otherKey;
}

void SpecialKey::setUpdateFunction(geode::Function<UpdateFuture(std::map<std::string, std::any>)> task) {
    this->updateFunction = std::move(task);
}

void SpecialKey::updateContent() {
    if (!this->updateFunction.has_value()) return;

    // log::info("Starting update for special key {}, {}", this->key, updateListener.isPending());

    if (!updateListener.isPending()) {
        if (this->updateStartedCallback)
            this->updateStartedCallback(this->shared_from_this());
    }
    else {
        cancel();
    }

    updateListener.spawn(
        "DT-update-key:" + key + "-task",
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
    auto value = co_await this->updateFunction.value()(payload);

    co_return value;
}

void SpecialKey::setUpdateCompletedCallback(geode::Function<void(const std::shared_ptr<SpecialKey>&)> callback) {
    this->updateCompletedCallback = std::move(callback);
}

void SpecialKey::setUpdateStartedCallback(geode::Function<void(const std::shared_ptr<SpecialKey>&)> callback) {
    this->updateStartedCallback = std::move(callback);
}

void SpecialKey::onUpdateCompleted(UpdateFuture::Output val) {
    if (val.isErr()) {
        auto errData = val.unwrapErr();
        if (errData.displayAsError)
            this->currentContent = fmt::format("Error: {}", errData.error);
        else
            this->currentContent = errData.error;
    }
    else
        this->currentContent = val.unwrap();

    if (this->updateCompletedCallback)
        this->updateCompletedCallback(this->shared_from_this());
}

void SpecialKey::cancel() {
    updateListener.cancel();
}

bool SpecialKey::doesRefreshWith(const std::string& other) {
    return toRefreshWith.contains(other);
}

void SpecialKey::refreshWith(const std::string& other) {
    if (toRefreshWith.contains(other)) return;

    toRefreshWith.insert(other);
}

void SpecialKey::refreshWith(const std::set<std::string>& others) {
    for (const auto& other : others)
    {
        refreshWith(other);
    }
    
}