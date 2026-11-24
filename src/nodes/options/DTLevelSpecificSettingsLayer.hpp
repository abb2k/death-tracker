#pragma once

#include <Geode/Geode.hpp>
#include <nodes/options/OptionsNode.hpp>

using namespace geode::prelude;

class DTLevelSpecificSettingsLayer : public geode::Popup {
    public:
        static DTLevelSpecificSettingsLayer* create();

        void keyBackClicked() override;
    private:
        bool init() override;

        void onClose(CCObject*) override;

        CCNode* createPageBtn(std::string_view text, OptionsNode* page);
        CCNode* createSeperator();

        void sendUnderlineTo(CCNode* node, bool instant = false);
        void makeUnderlineIdle(CCObject* node);

        void switchPage(CCObject* sender);

        std::map<CCObject*, OptionsNode*> m_pages{};
        OptionsNode* m_currentPage = nullptr;

        CCScale9Sprite* m_optionUnderline;
};