#pragma once

#include <Geode/Geode.hpp>
#include <nodes/options/OptionsNode.hpp>
#include <nodes/general/SimpleToggler.hpp>
#include <nodes/layout/usage/DTLabel.hpp>
#include <nodes/general/SessionSelector.hpp>
#include <nodes/general/SwitcherButton.hpp>

#include <nodes/layout/usage/DecimalPopup.hpp>

using namespace geode::prelude;


class ModifyOptions : public OptionsNode {
    public:
        static ModifyOptions* create(const CCSize& size);
    private:
        virtual bool setup() override;

        DTLabel* m_myLabel;

        void update(float dt) override;
        ScrollLayer* m_previewScroll;
        void updatePreviewName(bool categotyIsSession, bool f0State);

        void addPlusMinusBtns(CCNode* around, bool flip, Function<void(bool isPlus)> callback, float scale);
        void onPlusMinusBtn(CCObject* sender);
        std::map<CCNode*, std::pair<CCMenuItemSpriteExtra*, CCMenuItemSpriteExtra*>> m_holdersOfPlusMinusBtns{};
        std::map<CCMenuItemSpriteExtra*, std::shared_ptr<Function<void(bool isPlus)>>> m_plusMinusCallbacks{};
        CCScale9Sprite* m_previewBG;
        CCScale9Sprite* m_selectionBG;
        CCScale9Sprite* m_runsBG;
        CCScale9Sprite* m_f0BG;
        CCScale9Sprite* m_nbBG;

        geode::Label* m_runLabel;
        geode::Label* m_addRunSeperator;
        geode::Label* m_runAmountLabel;
        geode::Label* m_NBLabel;
        geode::Label* m_f0AmountLabel;
        geode::Label* m_f0Label;

        TextInput* m_addNewBestInput;
        TextInput* m_addPercentInput;
        TextInput* m_f0AmountInput;
        TextInput* m_addRunEndInput;
        TextInput* m_addRunStartInput;
        TextInput* m_runAmountInput;

        SessionSelector* m_sessionSelector;
        Scrollbar* m_scrollbar;
        DecimalPopup* m_decimalPopup;

        int m_currF0Amount = 1;
        int m_currRunsAmount = 1;

        double m_currentF0ToAdd = 0;
        
        double m_currentStartRunToAdd = 0;
        double m_currentEndRunToAdd = 0;

        double m_currentNBToAdd = 0;

        SwitcherButton* m_sessLvlSwitcher;
        SimpleToggler* m_runF0Toggler;

        void removeNB(CCObject*);
        void addNB(CCObject*);

        void removeInvalidRuns(CCObject*);

    public:
        virtual void onOpened() override;
        virtual void onClosed() override;
};