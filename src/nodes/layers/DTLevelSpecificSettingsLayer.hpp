#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OptionsNode.hpp>

using namespace geode::prelude;

class DTLevelSpecificSettingsLayer : public geode::Popup {
    public:
        static DTLevelSpecificSettingsLayer* create();

        void keyBackClicked() override;
    private:
        bool init() override;

        void onClose(CCObject*) override;

        CCNode* createPageBtn(const std::string& text, OptionsNode* page);
        CCNode* createSeperator();

        void sendUnderlineTo(CCNode* node, bool instant = false);
        void makeUnderlineIdle(CCObject* node);

        void switchPage(CCObject* sender);

        std::map<CCObject*, OptionsNode*> pages{};
        OptionsNode* currentPage = nullptr;

        CCScale9Sprite* optionUnderline;
};