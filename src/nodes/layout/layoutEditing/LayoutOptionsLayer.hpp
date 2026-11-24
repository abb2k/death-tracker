#pragma once

using namespace geode::prelude;

class DTLabel;
class LayoutColumn;
class FontSelectionCell;
class SimpleToggler;

class LayoutOptionsLayer : public CCLayer, public SliderDelegate {
public:
    static LayoutOptionsLayer* create(CCSize size);

    void setEditedNodeTo(DTLabel* label);
    void setEditedNodeTo(LayoutColumn* column);

    void close();

    bool isEditingNode() const {
        return m_editedLabel.has_value() || m_editedColumn.has_value();
    }

    Function<void()> m_onBackedOut = NULL;
    Function<void()> m_onEnter = NULL;

    void keyBackClicked() override;

private:
    bool init(CCSize size);

    CCSize m_size;

    std::optional<DTLabel*> m_editedLabel = std::nullopt;
    std::optional<LayoutColumn*> m_editedColumn = std::nullopt;

    void switchToMenu(uint8_t menuID);

    CCMenu* m_labelSettingsNode = nullptr;
    CCMenu* m_columnSettingsNode = nullptr;
    CCMenu* m_fontSelectionNode = nullptr;
    CCMenu* m_specialKeysNode = nullptr;

    uint8_t m_currentPage = 0;

    TextInput* m_labelNameInput;
    TextInput* m_labelTextInput;
    TextInput* m_fontSizeInput;
    
    void onTextColorBtnClicked(CCObject*);
    void onLabelColorBtnClicked(CCObject*);
    
    void updateColor(cocos2d::ccColor4B const& color);
    
    CCSprite* m_textColorBtnSprite;
    CCSprite* m_labelColorBtnSprite;
    
    void scaleSliderChanged(CCObject*);
    Slider* m_scaleSlider;
    
    Function<void(cocos2d::ccColor4B const&)> m_colorChangeFunc = NULL;
    
    void onAlignmentChanged(CCObject* sender);
    CCMenu* m_alignmentMenu;
    
    void onWrappingBtn(CCObject*);
    void updateWrapModeBtnVisuals();
    CCMenuItemSpriteExtra* m_wrappingModeBtn;
    
    void onFontSelection(CCObject*);
    TextInput* m_fontSelectedIndicatorLabel;
    
    void onBack(CCObject*);
    void onDelete(CCObject*);
    
    std::map<std::string, FontSelectionCell*> m_allFontCells{};
    FontSelectionCell* m_currentlySelectedFontCell = nullptr;
    void onFontSelected(FontSelectionCell* cell);
    ScrollLayer* m_fontsScroll;

    void onSpecialKeysClicked(CCObject*);

    TextInput* m_labelTextSpecialKeysInput;
    ScrollLayer* m_specialKeysScroll;

    void onSpecialKeyAdded(std::string_view str);

    void setAllInputs(CCNode* node, bool enabled);

    TextInput* m_columnWidthInput;
    bool m_ignoreNextOrganization;

    void sliderEnded(Slider* slider) override;
    void sliderUpdate();

    void updateFontSelectFont();

    SimpleToggler* m_hideHeaderToggler;

    void onAddColumnToLeft(CCObject*);
    void onAddColumnToRight(CCObject*);
};