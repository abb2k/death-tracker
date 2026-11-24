#pragma once

using namespace geode::prelude;

class FontSelectionCell : public CCMenu {
public:
    static FontSelectionCell* create(std::string font, Function<void(FontSelectionCell* onSelected)> onFontChosen);

    void deselect();
    void select();

    std::string m_font;
    
private:
    bool init(std::string font, Function<void(FontSelectionCell* onSelected)> onFontChosen);

    Function<void(FontSelectionCell* onSelected)> m_onFontChosen = NULL;

    void onUsedClicked(CCObject*);
    CCMenuItemSpriteExtra* m_useBtn;
};