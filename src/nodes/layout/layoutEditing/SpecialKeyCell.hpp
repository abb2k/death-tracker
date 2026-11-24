#pragma once

using namespace geode::prelude;

class SpecialKey;

class SpecialKeyCell : public CCMenu {
    public:
        static SpecialKeyCell* create(std::shared_ptr<SpecialKey> key, Function<void(const std::string&)> onSelected);


    private:
        bool init(std::shared_ptr<SpecialKey> key, Function<void(const std::string&)> onSelected);

        std::shared_ptr<SpecialKey> m_key;

        Function<void(const std::string&)> m_onSelected = NULL;

        void onAddClicked(CCObject*);
};