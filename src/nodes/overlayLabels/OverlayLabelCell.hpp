#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class SimpleToggler;

class OverlayLabelCell : public CCMenu {
public:
    static OverlayLabelCell* create(float width, OverlayLabel label);

    void onDeleteCallback(Function<void(OverlayLabelCell*)> onDelete);
    void onEditCallback(Function<void(OverlayLabelCell*)> onEdit);
    void onChangedCallback(Function<void(OverlayLabelCell*)> onChanged);
    void onVisibilityChangedCallback(Function<void(OverlayLabelCell*)> onVisibilityChanged);

    OverlayLabel const& getData() const { return m_myLabelData; }
    void setData(OverlayLabel label);
    void setEditing(bool isEditing);

    CCMenuItemSprite* getDeleteButton() const { return m_deleteButton; }
    CCMenuItemSprite* getEditButton() const { return m_editButton; }
    
    geode::Label* getEnableLabel() const { return m_enableLabel; }
    SimpleToggler* getEnabledToggle() const { return m_enabledToggle; }
    geode::Label* getLevelSpecificLabel() const { return m_levelSpecificLabel; }
    SimpleToggler* getLevelSpecificToggle() const { return m_levelSpecificToggle; }

    TextInput* getOrderInput() const { return m_orderInput; }
    geode::Label* getOrderLabel() const { return m_orderLabel; }
    
    void updateLevelVisibility();
private:
    bool init(float width, OverlayLabel label);

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

    void updateBackgroundColor();

    void onDelete(CCObject*);
    void onEdit(CCObject*);

    Function<void(OverlayLabelCell*)> m_onDelete = NULL;
    Function<void(OverlayLabelCell*)> m_onEdit = NULL;
    Function<void(OverlayLabelCell*)> m_onChanged = NULL;
    Function<void(OverlayLabelCell*)> m_onVisibilityChanged = NULL;
};