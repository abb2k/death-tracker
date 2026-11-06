#pragma once

#include <Geode/Geode.hpp>
#include <nodes/DTLabel.hpp>
#include <nodes/LayoutColumn.hpp>
#include <nodes/FontSelectionCell.hpp>
#include <nodes/ScrollableTextInput.hpp>
#include <nodes/SimpleToggler.hpp>

using namespace geode::prelude;

class LayoutOptionsLayer : public CCLayer, public SliderDelegate {
    public:
        static LayoutOptionsLayer* create(const CCSize& size);

        void setEditedNodeTo(DTLabel* label);
        void setEditedNodeTo(LayoutColumn* column);

        void close();

        bool isEditingNode() const {
            return editedLabel.has_value() || editedColumn.has_value();
        }

        geode::Function<void()> onBackedOut = NULL;
        geode::Function<void()> onEnter = NULL;

        void keyBackClicked() override;

    private:
        bool init(const CCSize& size);

        CCSize size;

        std::optional<DTLabel*> editedLabel = std::nullopt;
        std::optional<LayoutColumn*> editedColumn = std::nullopt;

        void switchToMenu(uint8_t menuID);

        CCMenu* labelSettingsNode = nullptr;
        CCMenu* columnSettingsNode = nullptr;
        CCMenu* fontSelectionNode = nullptr;
        CCMenu* specialKeysNode = nullptr;

        uint8_t currentPage = 0;

        TextInput* labelNameInput;
        ScrollableTextInput* labelTextInput;
        TextInput* fontSizeInput;
        
        void onTextColorBtnClicked(CCObject*);
        void onLabelColorBtnClicked(CCObject*);
        
        void updateColor(cocos2d::ccColor4B const& color);
        
        CCSprite* textColorBtnSprite;
        CCSprite* labelColorBtnSprite;
        
        void scaleSliderChanged(CCObject*);
        Slider* scaleSlider;
        
        geode::Function<void(cocos2d::ccColor4B const&)> colorChangeFunc = NULL;
        
        void onAlignmentChanged(CCObject* sender);
        CCMenu* alignmentMenu;
        
        void onWrappingBtn(CCObject*);
        void updateWrapModeBtnVisuals();
        CCMenuItemSpriteExtra* wrappingModeBtn;
        
        void onFontSelection(CCObject*);
        TextInput* fontSelectedIndicatorLabel;
        
        void onBack(CCObject*);
        void onDelete(CCObject*);
        
        std::map<std::string, FontSelectionCell*> allFontCells{};
        FontSelectionCell* currentlySelectedFontCell = nullptr;
        void onFontSelected(FontSelectionCell* cell);
        ScrollLayer* fontsScroll;

        void onSpecialKeysClicked(CCObject*);

        ScrollableTextInput* labelTextSpecialKeysInput;
        ScrollLayer* specialKeysScroll;

        void onSpecialKeyAdded(const std::string& str);

        void setAllInputs(CCNode* node, bool enabled);

        TextInput* columnWidthInput;
        bool ignoreNextOrganization;

        void sliderEnded(Slider* slider) override;
        void sliderUpdate();

        void updateFontSelectFont();

        SimpleToggler* hideHeaderToggler;

        void onAddColumnToLeft(CCObject*);
        void onAddColumnToRight(CCObject*);
    };