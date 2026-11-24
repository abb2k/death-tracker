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
    static DTLabel* create(DTLabelInfo info);

    const DTLabelInfo& getInfo() const { return m_info; }
    void setlayer(int layer);

    void moveUpLayer();

    static constexpr float labelTitleHeight = 15;

    void setIgnoreExtraSettings(bool ignoreExtraSettings);
    bool shouldIgnoreExtraSettings() const;
    void setPayload(const std::string& key, std::any value);
    void removePayload(const std::string& key);

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

    void setDonerRichTextEnabled(bool enabled);
    void setLabelText(const std::string& text);
    void setRunMoreButtonsVisible(bool visible, float fadeTime);

    arc::Future<std::optional<std::string>> modifyKeys();

    void setLoading(const std::shared_ptr<SpecialKey>& key);
    void completeLoading(const std::shared_ptr<SpecialKey>& key);

    void toggleExpand(CCObject*);

    void onBeingEdited();
    void onBeingEditedEnded();

    void setEditable(bool editable);

    void setExpandable(bool enabled);

    void fadeTextColorTo(ccColor4B newColor, float time);
    void fadeTitleColorTo(ccColor4B newColor, float time);

    ~DTLabel();

    void updateInfoWithColumnData();

    void attachedDecimalPopup(DecimalPopup* popup);

    void setTempPos(const CCPoint& pos);
    void setTempWidth(float width);

    const CCPoint& getTempPos() const { return m_tempPos; }
    float getTempWidth() const { return m_tempWidth; }

    CCScale9Sprite* getLeftExpandLine() const { return m_leftExpandLine; }
    CCScale9Sprite* getRightExpandLine() const { return m_rightExpandLine; }
    CCScale9Sprite* getLabelTitleBG() const { return m_labelTitleBG; }

private:
    bool init(DTLabelInfo info);

    void update(float dt) override;

    bool m_ignoreExtraSettings = false;
    std::map<std::string, std::any> m_payload{};

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

    static constexpr float m_normalBGOpacity = 150;

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

    static constexpr float m_doubleClickTimer = .2f;

    float getSpecificTitleHeight();

    bool m_runMoreButtonsVisible = true;

    void onRunMoreClicked(CCObject* sender);

    static constexpr int m_runMoreButtonFadeActionTag = 1;
    static constexpr GLubyte m_runMoreButtonOpacity = 100;

    DecimalPopup* m_decimalPopup = nullptr;

    std::set<std::string> m_keysUsed{};

    static constexpr float m_textCornerOffset = 5;
    static constexpr float moveThreshold = 5;
    static constexpr float labelLerpSpeed = 10;

    CCPoint m_tempPos = CCPoint{0, 0};
    float m_tempWidth = 0;

    CCScale9Sprite* m_leftExpandLine;
    CCScale9Sprite* m_rightExpandLine;
    CCScale9Sprite* m_labelTitleBG;

    DTLabelInfo m_info;
};