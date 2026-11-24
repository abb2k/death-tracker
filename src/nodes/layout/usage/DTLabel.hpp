#pragma once

#include <types/DTTypes.hpp>
#include <any>

using namespace geode::prelude;

class LayoutColumn;
class DecimalPopup;
class SpecialKey;
class RichTextArea;

class DTLabel : public CCMenu {
public:
    static DTLabel* create(const DTLabelInfo& info);

    DTLabelInfo m_info;

    void moveUpLayer();

    CCPoint m_tempPos = CCPoint{0, 0};
    float m_tempWidth = 0;

    static float labelTitleHeight;
    static float moveThreshold;
    static float labelLerpSpeed;

    bool m_ignoreExtraSettings;

    std::map<std::string, std::any> m_payload{};

    void removeFromColumns();

    void addColumnAsHolder(LayoutColumn* column);
    void removeColumnAsHolder(LayoutColumn* column);
    bool isPartOfColumn(LayoutColumn* column);
    bool isAlone();

    struct ColumnComperator {
        bool operator() (LayoutColumn* a, LayoutColumn* b) const;
    };

    std::set<LayoutColumn*, ColumnComperator> getHolders();

    void setLabelName(const std::string& newName);
    void setFontSize(float newSize);
    void setFont(const std::string& fnt);
    void setLabelColor(const ccColor4B& newColor);
    void setTextColor(const ccColor4B& newColor);
    void setTextAlignment(CCTextAlignment alignment);
    void setTextWrapping(WrappingMode wrapMode);
    void setHideHeader(bool hideHeader);

    float m_textCornerOffset = 5;

    void setDonerRichTextEnabled(bool enabled);
    void setLabelText(const std::string& text);
    void setRunMoreButtonsVisible(bool visible, float fadeTime);

    arc::Future<std::optional<std::string>> modifyKeys();

    void setLoading(const std::shared_ptr<SpecialKey>& key);
    void completeLoading(const std::shared_ptr<SpecialKey>& key);
    
    std::set<std::string> m_keysUsed{};

    void toggleExpand(CCObject*);

    void onBeingEdited();
    void onBeingEditedEnded();

    void setEditable(bool editable);

    void setExpandable(bool enabled);

    void fadeTextColorTo(ccColor4B newColor, float time);
    void fadeTitleColorTo(ccColor4B newColor, float time);

    CCScale9Sprite* m_leftExpandLine;
    CCScale9Sprite* m_rightExpandLine;
    CCScale9Sprite* m_labelTitleBG;

    ~DTLabel();

    void updateInfoWithColumnData();

    void attachedDecimalPopup(DecimalPopup* popup);

private:
    bool init(const DTLabelInfo& info);

    void update(float dt) override;

    async::TaskHolder<std::optional<std::string>> m_modifyListener;

    bool m_isEditable;
    bool m_donerRichTextEnabled = false;

    bool m_isExpandable = true;

    std::set<std::shared_ptr<SpecialKey>> m_currentlyLoadingFor{};
    
    std::set<LayoutColumn*, ColumnComperator> m_holders{};

    CCScale9Sprite* m_bg;
    
    CCMenu* m_menu;
    CCMenuItemSpriteExtra* m_expandBtn;
    RichTextArea* m_labelTitleArea;

    CCNode* m_labelTextContainer;
    RichTextArea* m_labelText;
    float m_currentHeight;

    CCSprite* m_hideBGSpr;

    float m_normalBGOpacity = 150;

    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

    bool m_isBeingTouched;
    CCPoint m_touchStartPoint;
    bool m_isMovingLabel;

    CCPoint m_currentTouchPosition;

    std::set<LayoutColumn*, ColumnComperator> m_holdersSave{};

    void onSettings();
    void onMoveBegan();
    void onMoveUpdate(float dt);
    void onMoveEnded();

    bool m_currentlyExpandingLeft;
    bool m_currentlyExpandingRight;

    LoadingCircle* m_loadingCircle;

    CCSprite* m_glow;

    arc::Future<std::string> modifyStrRecursive(const std::string& str);

    bool m_isBeingEdited = false;
    bool m_isHovering;

    float m_doubleClickTimer = .2f;

    float getSpecificTitleHeight();

    bool m_runMoreButtonsVisible = true;

    void onRunMoreClicked(CCObject* sender);

    int m_runMoreButtonFadeActionTag = 1;
    GLubyte m_runMoreButtonOpacity = 100;

    DecimalPopup* m_decimalPopup = nullptr;
};