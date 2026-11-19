#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class LevelLinkCell : public CCMenu {
    public:
        static LevelLinkCell* create(float width, const std::string& levelKey, const LevelMetadata& data);

        void setAlignment(bool left);

        void setCallback(geode::Function<void(const std::string& levelKey, bool isLeftAligned)> callback);

        void setEnabledAndFade(bool b);
        bool isEnabledAndFade() const { return enabledAndFade; }

    private:
        bool init(float width, const std::string& levelKey, const LevelMetadata& data);

        std::string levelKey;

        geode::Function<void(const std::string& levelKey, bool isLeftAligned)> callback = NULL;

        bool isAlignedLeft;

        void onArrow(CCObject*);

        CCMenuItemSpriteExtra* arrowBtn;
        GJDifficultySprite* diffSpr;
        geode::Label* title;
        float titleWidth;
        geode::Label* att;
        float attWidth;

        bool enabledAndFade = true;
};