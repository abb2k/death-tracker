#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class LevelLinkCell : public CCMenu {
public:
    static LevelLinkCell* create(float width, std::string levelKey, const LevelMetadata& data);

    void setAlignment(bool left);

    void setCallback(Function<void(const std::string& levelKey, bool isLeftAligned)> callback);

    void setEnabledAndFade(bool b);
    bool isEnabledAndFade() const { return m_enabledAndFade; }

private:
    bool init(float width, std::string levelKey, const LevelMetadata& data);

    std::string m_levelKey;

    Function<void(const std::string& levelKey, bool isLeftAligned)> m_callback = NULL;

    bool m_isAlignedLeft;

    void onArrow(CCObject*);

    CCMenuItemSpriteExtra* m_arrowBtn;
    GJDifficultySprite* m_diffSpr;
    geode::Label* m_title;
    float m_titleWidth;
    geode::Label* m_att;
    float m_attWidth;

    bool m_enabledAndFade = true;
};