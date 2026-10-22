#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OverlayLabelCell.hpp>

using namespace geode::prelude;

class LabelEditLayer : public Popup {
    private:
        bool init() override;

        void onLabelAdded(CCObject*);

        std::set<OverlayLabelCell*> m_labels{};

        OverlayLabelCell* m_currentEdit = nullptr;

        ScrollLayer* m_scroll = nullptr;

        void onCellDelete(OverlayLabelCell* cell);
        void onCellEdit(OverlayLabelCell* cell);

        CCLabelBMFont* m_noLabelsText;

        void onSliderX(CCObject*);
        void onSliderY(CCObject*);

        std::map<CCObject*, geode::Anchor> m_anchorButtons{};

        CCMenuItemSpriteExtra* createAnchorButton(Anchor anchor, std::string const& text);

        void onAnchorClicked(CCObject*);

        CCClippingNode* m_previewArea;
        CCMenu* m_previewAreaMenu;

    public:
        static LabelEditLayer* create();
};