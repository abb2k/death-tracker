#pragma once

#include <Geode/Geode.hpp>
#include <nodes/FloatingList.hpp>

using namespace geode::prelude;

class LabelEditSpecialKeyEditorPopup : public Popup {
    protected:
        bool init(geode::Function<void(const std::string&)> callback);

        void onKeyTypeClick(CCObject*);
        void onScopeClick(CCObject*);
        void onOperatorClick(CCObject*);
        void updateControls();
        void onCreateKey(CCObject*);

        geode::Function<void(const std::string&)> m_callback;
        CCMenuItemSpriteExtra* m_keyTypeButton = nullptr;
        CCMenuItemSpriteExtra* m_scopeButton = nullptr;
        CCMenuItemSpriteExtra* m_operatorButton = nullptr;
        CCLabelBMFont* m_scopeLabel = nullptr;
        CCLabelBMFont* m_operatorLabel = nullptr;
        FloatingList* m_keyTypeList = nullptr;
        FloatingList* m_scopeList = nullptr;
        FloatingList* m_operatorList = nullptr;
        TextInput* m_firstValue = nullptr;
        TextInput* m_secondValue = nullptr;
        int m_keyType = 0;
        int m_scope = 0;
        int m_operator = 0;

    public:
        static LabelEditSpecialKeyEditorPopup* create(geode::Function<void(const std::string&)> callback);
};