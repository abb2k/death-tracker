#pragma once

#include <Geode/Geode.hpp>
#include <nodes/OverlayLabelCell.hpp>
#include <regex>
#include <nodes/SimpleToggler.hpp>

using namespace geode::prelude;

class LabelEditLayer : public Popup {
    private:
        bool init() override;

        void onLabelAdded(CCObject*);

        std::vector<OverlayLabelCell*> m_labels{};

        OverlayLabelCell* m_currentEdit = nullptr;
        bool m_updatingControls = false;

        ScrollLayer* m_scroll = nullptr;
        Scrollbar* m_labelsScrollbar = nullptr;
        CCMenu* m_editMenu = nullptr;
        CCScale9Sprite* m_noLabelEditedOverlay = nullptr;
        CCLabelBMFont* m_noLabelEditedText = nullptr;
        SimpleToggler* m_freeModeToggle = nullptr;
        Slider* m_xSlider = nullptr;
        Slider* m_ySlider = nullptr;
        Slider* m_offsetSlider = nullptr;
        TextInput* m_nameInput = nullptr;
        TextInput* m_textInput = nullptr;
        TextInput* m_offsetInput = nullptr;

        void onCellDelete(OverlayLabelCell* cell);
        void onCellEdit(OverlayLabelCell* cell);
        void setLabelEditEnabled(bool enabled);
        void updateAnchorButtons();

        CCLabelBMFont* m_noLabelsText;

        void onSliderX(CCObject*);
        void onSliderY(CCObject*);
        void onOffsetSliderChanged(CCObject*);
        void onSizeSliderChanged(CCObject*);
        void setCurrentLabelSize(float size, bool updateInput = true);

        std::map<CCObject*, geode::Anchor> m_anchorButtons{};

        CCMenuItemSpriteExtra* createAnchorButton(Anchor anchor, std::string const& text);

        void onAnchorClicked(CCObject*);

        CCClippingNode* m_previewArea;
        CCMenu* m_previewAreaMenu;

        void onRichTextBtn(CCObject*);

        void updateSavedLabels();

        comm::ListenerHandle m_overlayLabelsChangedListener;

        void RefreshPreviewLabels();

        std::vector<RichTextArea*> m_previewLabels{};

        TextInput* m_sizeInput = nullptr;
        Slider* m_sizeSlider = nullptr;

        Anchor invertAnchor(Anchor anchor);
        CCPoint anchorPoint(Anchor anchor);
        CCPoint anchorDirection(Anchor anchor);
        CCPoint anchorDirection(CCPoint anchor);

        CCScale9Sprite* m_disabledAnchorSlidersFG = nullptr;

        float OFFSET_SLIDER_MIN = 0.f;
        float OFFSET_SLIDER_MAX = 20.f;

        Result<GeneralData> getDeathsData(char scope);
        std::string formatDeaths(const GeneralData& data, const std::smatch& match);
        std::string expandDeaths(const std::string& text);

    public:
        static LabelEditLayer* create();
};