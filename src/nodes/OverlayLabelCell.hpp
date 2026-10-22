#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class OverlayLabelCell : public CCMenu {
    public:
        static OverlayLabelCell* create(float width, OverlayLabel const& label);

        void onDeleteCallback(geode::Function<void(OverlayLabelCell*)> onDelete);
        void onEditCallback(geode::Function<void(OverlayLabelCell*)> onEdit);

    private:
        bool init(float width, OverlayLabel const& label);

        OverlayLabel m_myLabelData;

        void onDelete(CCObject*);
        void onEdit(CCObject*);

        geode::Function<void(OverlayLabelCell*)> m_onDelete = NULL;
        geode::Function<void(OverlayLabelCell*)> m_onEdit = NULL;
};