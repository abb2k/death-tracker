#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SimpleToggler.hpp>

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

    private:
        bool init(float width, OverlayLabel const& label);

        OverlayLabel m_myLabelData;
        CCScale9Sprite* m_background = nullptr;
        SimpleTextArea* m_labelName = nullptr;
        SimpleToggler* m_enabledToggle = nullptr;
        TextInput* m_orderInput = nullptr;
        bool m_updatingControls = false;
        bool m_isEditing = false;

        void updateBackgroundColor();

        void onDelete(CCObject*);
        void onEdit(CCObject*);

        geode::Function<void(OverlayLabelCell*)> m_onDelete = NULL;
        geode::Function<void(OverlayLabelCell*)> m_onEdit = NULL;
        geode::Function<void(OverlayLabelCell*)> m_onChanged = NULL;
};