#pragma once

using namespace geode::prelude;

class OptionsNode;

class DTLevelSpecificSettingsLayer : public geode::Popup {
public:
    static DTLevelSpecificSettingsLayer* create();

    void keyBackClicked() override;
    void keyDown(enumKeyCodes key, double timestamp) override;
    void keyUp(enumKeyCodes key, double timestamp) override;
private:
    bool init() override;

    void onClose(CCObject*) override;

    CCNode* createPageBtn(std::string_view text, OptionsNode* page);
    void insertPage(OptionsNode* page);
    CCNode* createSeperator();

    void sendUnderlineTo(CCNode* node, bool instant = false);
    void makeUnderlineIdle(CCObject* node);

    void switchPage(CCObject* sender);

    std::map<CCObject*, OptionsNode*> m_pages{};
    OptionsNode* m_currentPage = nullptr;

    CCScale9Sprite* m_optionUnderline;

    OptionsNode* m_manPage = nullptr;
    bool m_canMan = false;
};