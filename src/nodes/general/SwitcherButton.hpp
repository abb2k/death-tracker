#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class SwitcherButton : public CCMenuItemSpriteExtra {
    protected:
        bool init(CCSize size, CCScale9Sprite* bgSpr, const std::vector<std::string>& options);
        
    public:
        static SwitcherButton* create(CCSize size, const std::string& spr, const std::vector<std::string>& options);
        static SwitcherButton* createWithSpriteFrameName(CCSize size, const std::string& spr, const std::vector<std::string>& options);

        void setOptionChangedCallback(geode::Function<void(int)> callback);

        int getCurrentOptionIndex() const{
            return currentOption;
        }

    private:

        std::vector<std::string> options;
        int currentOption = 0;

        geode::Label* textLabel;
        CCScale9Sprite* mySpr;

        geode::Function<void(int)> onOptionChanged = NULL;

        void onClicked(CCObject*);

        void setTextForCurrentOption();
};