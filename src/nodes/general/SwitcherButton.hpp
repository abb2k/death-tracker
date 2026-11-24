#pragma once

using namespace geode::prelude;

class SwitcherButton : public CCMenuItemSpriteExtra {
public:
    static SwitcherButton* create(CCSize size, std::string_view spr, std::vector<std::string> options);
    static SwitcherButton* createWithSpriteFrameName(CCSize size, std::string_view spr, std::vector<std::string> options);

    void setOptionChangedCallback(Function<void(int)> callback);

    int getCurrentOptionIndex() const { return m_currentOption; }

private:
    bool init(CCSize size, CCScale9Sprite* bgSpr, std::vector<std::string> options);

    std::vector<std::string> m_options;
    int m_currentOption = 0;

    geode::Label* m_textLabel;
    CCScale9Sprite* m_mySpr;

    Function<void(int)> m_onOptionChanged = NULL;

    void onClicked(CCObject*);

    void setTextForCurrentOption();
};