#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/general/SimpleToggler.hpp>

using namespace geode::prelude;

class OverlayLabelCell : public CCMenu {
    public:
        static OverlayLabelCell* create(float width, OverlayLabel const& label);

        void onDeleteCallback(geode::Function<void(OverlayLabelCell*)> onDelete);
        void onEditCallback(geode::Function<void(OverlayLabelCell*)> onEdit);
        void onChangedCallback(geode::Function<void(OverlayLabelCell*)> onChanged);

        OverlayLabel const& getData() const { return m_myLabelData; }
        void setData(OverlayLabel const& label);
        void setEditing(bool isEditing);

        CCMenuItemSprite* getDeleteButton() const { return m_deleteButton; }
        CCMenuItemSprite* getEditButton() const { return m_editButton; }
        
        geode::Label* getEnableLabel() const { return m_enableLabel; }
        SimpleToggler* getEnabledToggle() const { return m_enabledToggle; }
        geode::Label* getLevelSpecificLabel() const { return m_levelSpecificLabel; }
        SimpleToggler* getLevelSpecificToggle() const { return m_levelSpecificToggle; }

        TextInput* getOrderInput() const { return m_orderInput; }
        geode::Label* getOrderLabel() const { return m_orderLabel; }
        

    private:
        bool init(float width, OverlayLabel const& label);

        OverlayLabel m_myLabelData;
        CCScale9Sprite* m_background = nullptr;
        SimpleTextArea* m_labelName = nullptr;
        geode::Label* m_enableLabel = nullptr;
        SimpleToggler* m_enabledToggle = nullptr;
        SimpleToggler* m_levelSpecificToggle = nullptr;
        geode::Label* m_levelSpecificLabel = nullptr;
        TextInput* m_orderInput = nullptr;
        geode::Label* m_orderLabel = nullptr;
        CCMenuItemSprite* m_deleteButton = nullptr;
        CCMenuItemSprite* m_editButton = nullptr;
        bool m_updatingControls = false;
        bool m_isEditing = false;

        void updateLevelVisibility();
        void updateBackgroundColor();

        void onDelete(CCObject*);
        void onEdit(CCObject*);

        geode::Function<void(OverlayLabelCell*)> m_onDelete = NULL;
        geode::Function<void(OverlayLabelCell*)> m_onEdit = NULL;
        geode::Function<void(OverlayLabelCell*)> m_onChanged = NULL;
};