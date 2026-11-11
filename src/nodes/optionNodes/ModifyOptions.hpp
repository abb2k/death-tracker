#pragma once

#include <Geode/Geode.hpp>
#include <nodes/optionNodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/DTLabel.hpp>
#include <nodes/SessionSelector.hpp>
#include <nodes/SwitcherButton.hpp>

#include <nodes/DecimalPopup.hpp>

using namespace geode::prelude;


class ModifyOptions : public OptionsNode {
    public:
        static ModifyOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        DTLabel* myLabel;

        void update(float dt);
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

        CCLabelBMFont* runLabel;
        CCLabelBMFont* addRunSeperator;
        CCLabelBMFont* runAmountLabel;
        CCLabelBMFont* NBLabel;
        CCLabelBMFont* f0AmountLabel;
        CCLabelBMFont* f0Label;

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
        virtual void onOpened();
        virtual void onClosed();
};