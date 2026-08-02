#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/OptionSwitcher.hpp>
#include <nodes/SessionSelector.hpp>
#include <nodes/FloatingList.hpp>

using namespace geode::prelude;

class GraphCell : public CCNode {
    public:
        static GraphCell* create(float width, const DTGraphInfo& graphInfo);

        DTGraphInfo getinfo() const{
            return graphInfo;
        };

        geode::Function<void(GraphCell* cell, bool up)> onArrowCallback = NULL;
        geode::Function<void(GraphCell* cell)> onOptionsCallback = NULL;
        geode::Function<void(GraphCell* cell)> onEnabledChanged = NULL;
        geode::Function<void(GraphCell* cell)> onInfoChangedCallback = NULL;

        geode::Function<bool(const std::string&, GraphCell*)> canChangeNameTo;

        geode::Function<void(DTGraphInfo, GraphCell*)> beforeDeletion;
        geode::Function<void(DTGraphInfo)> onDeleted;
        
        geode::Function<void(GraphCell* cell, std::optional<Session>)> onNewSession = NULL;
        geode::Function<void(GraphCell* cell, int)> onNewRun = NULL;

        std::string oldName;

        bool setName(const std::string& name);
        void setCoverage(DTGraphCoverage coverage);
        void setType(DTGraphType type);
        void setOrderPos(int pos);
        void setThickness(float thickness);
        void setOutlineThickness(float outlineThickness);
        void setPointSize(float pointSize);
        void setColor(ccColor4B color);
        void setOutlineColor(ccColor4B color);
        void setPointColor(ccColor4B color);

        void onInfoChanged(bool updateGraph);

        void setEnabledInfo(bool b, bool changeToggler, bool callback);
        void setEnabled(bool value);
        
        void deleteMe();
        
        void resendSession();
        
        CCNode* everythingParent;
        CCMenu* mainMenu;
        CCLabelBMFont* sessionSelectorLabel;
        SessionSelector* sessionSelector;
        CCLabelBMFont* runLabel;
        TextInput* runInput;
        CCMenuItemSpriteExtra* runSelectHelperBtn;
    private:
        bool init(float width, const DTGraphInfo& graphInfo);
        
        DTGraphInfo graphInfo; 
        
        CCScale9Sprite* bg;

        OptionSwitcher<DTGraphType>* typeSwitcher;
        SimpleToggler* enableToggleBtn;
        SimpleTextArea* label;
        CCSprite* outerColor;
        CCSprite* innerColor;

        void onArrowUp(CCObject*);
        void onArrowDown(CCObject*);
        void onOptions(CCObject*);

        void checkForOptVisibilityChange();

        void setSessionOptionsVisible(bool visible);
        void setRunOptionsVisible(bool visible);

        CCNode* sessionContainer;
        CCMenu* runContainer;
        
        

        CCNode* extrasContainer;

        void sendNewSession(int index);

        FloatingList* runsFloatList;

        void onRunSelectHelper(CCObject*);
};