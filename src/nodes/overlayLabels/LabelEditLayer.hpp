#pragma once

#include <Geode/Geode.hpp>
#include <nodes/overlayLabels/OverlayLabelCell.hpp>
#include <regex>
#include <nodes/general/SimpleToggler.hpp>

#include <nodes/general/RichTextArea.hpp>

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
        geode::Label* m_noLabelEditedText = nullptr;
        SimpleToggler* m_freeModeToggle = nullptr;
        Slider* m_xSlider = nullptr;
        Slider* m_ySlider = nullptr;
        Slider* m_offsetSlider = nullptr;
        Slider* m_opacitySlider = nullptr;
        TextInput* m_nameInput = nullptr;
        TextInput* m_textInput = nullptr;
        TextInput* m_offsetInput = nullptr;
        TextInput* m_opacityInput = nullptr;

        void onCellDelete(OverlayLabelCell* cell);
        void onCellEdit(OverlayLabelCell* cell);
        void setLabelEditEnabled(bool enabled);
        void updateNoLabelsText();
        void updateAnchorButtons();

        geode::Label* m_noLabelsText;

        void onSliderX(CCObject*);
        void onSliderY(CCObject*);
        void onOffsetSliderChanged(CCObject*);
        void onSizeSliderChanged(CCObject*);
        void onOpacitySliderChanged(CCObject*);
        void setCurrentLabelSize(float size, bool updateInput = true);
        void setCurrentLabelOpacity(int opacity, bool updateInput = true);

        std::map<CCObject*, geode::Anchor> m_anchorButtons{};

        CCMenuItemSpriteExtra* createAnchorButton(Anchor anchor, std::string const& text);

        void onAnchorClicked(CCObject*);

        CCClippingNode* m_previewArea;
        CCMenu* m_previewAreaMenu;
        RichTextArea* m_currentPreviewLabel = nullptr;
        bool m_isDraggingPreviewLabel = false;
        CCPoint m_dragTouchLocalPoint;

        void onRichTextBtn(CCObject*);
        bool m_doUpdateCursorPos = false;
        std::optional<int> m_prevCursorPos = std::nullopt;

        void updateSavedLabels();

        comm::ListenerHandle m_overlayLabelsChangedListener;

        void RefreshPreviewLabels();

        std::vector<RichTextArea*> m_previewLabels{};

        TextInput* m_sizeInput = nullptr;
        Slider* m_sizeSlider = nullptr;

        CCScale9Sprite* m_disabledAnchorSlidersFG = nullptr;

        static constexpr float m_OFFSET_SLIDER_MIN = 0.f;
        static constexpr float m_OFFSET_SLIDER_MAX = 20.f;

        static std::string formatDeaths(const GeneralData& data, const std::smatch& match);
        static Result<GeneralData> getDeathsData(char scope);

        CCMenuItemSpriteExtra* m_richTextBtn = nullptr;

        virtual bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;

        virtual void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
        virtual void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
        virtual void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;

        void update(float dt) override;

    public:
        static LabelEditLayer* create();
        static std::string expandDeaths(std::string_view text);
        static std::string expandDeaths(std::string_view text, const std::function<Result<GeneralData>(char)>& dataProvider);

        static Anchor invertAnchor(Anchor anchor);
        static CCPoint anchorPoint(Anchor anchor);
        static CCPoint anchorDirection(Anchor anchor);
        static CCPoint anchorDirection(CCPoint anchor);
};