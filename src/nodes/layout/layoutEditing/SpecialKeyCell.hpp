#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <nodes/layout/usage/SpecialKey.hpp>

class SpecialKeyCell : public CCMenu {
    public:
        static SpecialKeyCell* create(std::shared_ptr<SpecialKey> key, Function<void(const std::string&)> onSelected);


    private:
        bool init(std::shared_ptr<SpecialKey> key, Function<void(const std::string&)> onSelected);

        std::shared_ptr<SpecialKey> key;

        Function<void(const std::string&)> onSelected = NULL;

        void onAddClicked(CCObject*);
};