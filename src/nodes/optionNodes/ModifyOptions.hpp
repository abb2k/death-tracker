#pragma once

#include <Geode/Geode.hpp>
#include <nodes/optionNodes/OptionsNode.hpp>
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

        DTLabel* myLabel;

        void update(float dt) override;
        ScrollLayer* previewScroll;
        void updatePreviewName(bool categotyIsSession, bool f0State);

        void addPlusMinusBtns(CCNode* around, bool flip, geode::Function<void(bool isPlus)> callback, float scale);
        void onPlusMinusBtn(CCObject* sender);
        std::map<CCNode*, std::pair<CCMenuItemSpriteExtra*, CCMenuItemSpriteExtra*>> holdersOfPlusMinusBtns{};
        std::map<CCMenuItemSpriteExtra*, std::shared_ptr<geode::Function<void(bool isPlus)>>> plusMinusCallbacks{};
        CCScale9Sprite* previewBG;
        CCScale9Sprite* selectionBG;
        CCScale9Sprite* runsBG;
        CCScale9Sprite* f0BG;
        CCScale9Sprite* nbBG;

        geode::Label* runLabel;
        geode::Label* addRunSeperator;
        geode::Label* runAmountLabel;
        geode::Label* NBLabel;
        geode::Label* f0AmountLabel;
        geode::Label* f0Label;

        TextInput* addNewBestInput;
        TextInput* addPercentInput;
        TextInput* f0AmountInput;
        TextInput* addRunEndInput;
        TextInput* addRunStartInput;
        TextInput* runAmountInput;

        SessionSelector* sessionSelector;
        Scrollbar* scrollbar;
        DecimalPopup* decimalPopup;

        int currF0Amount = 1;
        int currRunsAmount = 1;

        double currentF0ToAdd = 0;
        
        double currentStartRunToAdd = 0;
        double currentEndRunToAdd = 0;

        double currentNBToAdd = 0;

        SwitcherButton* sessLvlSwitcher;
        SimpleToggler* runF0Toggler;

        void removeNB(CCObject*);
        void addNB(CCObject*);

        void removeInvalidRuns(CCObject*);

    public:
        virtual void onOpened() override;
        virtual void onClosed() override;
};