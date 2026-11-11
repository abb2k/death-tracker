#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class FontSelectionCell : public CCMenu {
    public:
        static FontSelectionCell* create(const std::string& font, geode::Function<void(FontSelectionCell* onSelected)> onFontChosen);

        void deselect();
        void select();

        std::string font;
    private:
        bool init(const std::string& font, geode::Function<void(FontSelectionCell* onSelected)> onFontChosen);

        geode::Function<void(FontSelectionCell* onSelected)> onFontChosen = NULL;

        void onUsedClicked(CCObject*);
        CCMenuItemSpriteExtra* useBtn;
};