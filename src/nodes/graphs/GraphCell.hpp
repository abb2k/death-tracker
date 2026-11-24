#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class SessionSelector;
template <class T>
class OptionSwitcher;
class SimpleToggler;
class FloatingList;

class GraphCell : public CCNode {
    public:
        static GraphCell* create(float width, const DTGraphInfo& graphInfo);

        DTGraphInfo getinfo() const{
            return m_graphInfo;
        };

        Function<void(GraphCell* cell, bool up)> m_onArrowCallback = NULL;
        Function<void(GraphCell* cell)> m_onOptionsCallback = NULL;
        Function<void(GraphCell* cell)> m_onEnabledChanged = NULL;
        Function<void(GraphCell* cell)> m_onInfoChangedCallback = NULL;

        Function<bool(const std::string&, GraphCell*)> m_canChangeNameTo;

        Function<void(DTGraphInfo, GraphCell*)> m_beforeDeletion;
        Function<void(DTGraphInfo)> m_onDeleted;
        
        Function<void(GraphCell* cell, std::optional<Session>)> m_onNewSession = NULL;
        Function<void(GraphCell* cell, int)> m_onNewRun = NULL;
        Function<void(GraphCell* cell, Section)> m_onNewSection = NULL;

        std::string m_oldName;

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
        void resendSection();
        
        CCNode* m_everythingParent;
        CCMenu* m_mainMenu;
        geode::Label* m_sessionSelectorLabel;
        SessionSelector* m_sessionSelector;
        geode::Label* m_runLabel;
        TextInput* m_runInput;
        CCMenuItemSpriteExtra* m_runSelectHelperBtn;
    private:
        bool init(float width, const DTGraphInfo& graphInfo);
        
        DTGraphInfo m_graphInfo;
        
        CCScale9Sprite* m_bg;

        OptionSwitcher<DTGraphType>* m_typeSwitcher;
        SimpleToggler* m_enableToggleBtn;
        SimpleTextArea* m_label;
        CCSprite* m_outerColor;
        CCSprite* m_innerColor;

        void onArrowUp(CCObject*);
        void onArrowDown(CCObject*);
        void onOptions(CCObject*);

        void checkForOptVisibilityChange();

        void setSessionOptionsVisible(bool visible);
        void setRunOptionsVisible(bool visible);
        void setSectionOptionsVisible(bool visible);

        CCNode* m_sessionContainer;
        CCMenu* m_sectionContainer;
        CCMenu* m_runContainer;
        
        

        CCNode* m_extrasContainer;
        geode::Label* m_sectionSelectorLabel;
        CCMenuItemSpriteExtra* m_sectionSelectorBtn;
        FloatingList* m_sectionsFloatList;
        std::vector<Section> m_sections;
        size_t m_selectedSectionIndex = 0;

        void sendNewSession(int index);
        void sendNewSection();
        void updateSectionSelector();

        FloatingList* m_runsFloatList;

        void onRunSelectHelper(CCObject*);
        void onSectionSelectHelper(CCObject*);
};