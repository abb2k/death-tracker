#pragma once

#include <Geode/Geode.hpp>
#include <unordered_map>
#include <functional>
#include <types/DTTypes.hpp>
#include <utils/SpecialKey.hpp>

class LayoutColumn;

using namespace geode::prelude;

using WaveKeyParams = std::tuple<float, float, float, float>;
using GradientKeyParams = std::tuple<ccColor4B, ccColor4B, float, float>;
using CPlusKeyParams = int;

class DTLabel : public CCMenu {
    public:
        static DTLabel* create(const DTLabelInfo& info);

        DTLabelInfo info;

        void moveUpLayer();

        CCPoint tempPos = CCPoint{0, 0};
        float tempWidth = 0;

        static float labelTitleHeight;
        static float moveThreshold;
        static float labelLerpSpeed;

        bool ignoreExtraSettings;

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

        float textCornerOffset = 5;

        void setDonerRichTextEnabled(bool enabled);
        void setLabelText(const std::string& text);

        arc::Future<std::optional<std::string>> modifyKeys();

        void setLoading(const std::shared_ptr<SpecialKey>& key);
        void completeLoading(const std::shared_ptr<SpecialKey>& key);
        
        std::set<std::string> keysUsed{};

        void toggleExpand(CCObject*);

        void onBeingEdited();
        void onBeingEditedEnded();

        void setEditable(bool editable);

        void setExpandable(bool enabled);

        void fadeTextColorTo(ccColor4B newColor, float time);
        void fadeTitleColorTo(ccColor4B newColor, float time);

        CCScale9Sprite* leftExpandLine;
        CCScale9Sprite* rightExpandLine;
        CCScale9Sprite* labelTitleBG;

        ~DTLabel();
    private:
        bool init(const DTLabelInfo& info);

        void update(float dt) override;

        async::TaskHolder<std::optional<std::string>> modifyListener;

        bool isEditable;
        bool donerRichTextEnabled = false;

        bool isExpandable = true;

        std::set<std::shared_ptr<SpecialKey>> currentlyLoadingFor{};
        
        std::set<LayoutColumn*, ColumnComperator> holders{};

        CCScale9Sprite* bg;
        
        CCMenu* menu;
        CCMenuItemSpriteExtra* expandBtn;
        RichTextArea* labelTitleArea;

        CCNode* labelTextContainer;
        RichTextArea* labelText;
        float currentHeight;

        CCSprite* hideBGSpr;

        float normalBGOpacity = 150;

        void registerWithTouchDispatcher() override;
        bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
        void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;

        bool isBeingTouched;
        CCPoint touchStartPoint;
        bool isMovingLabel;

        CCPoint currentTouchPosition;

        std::set<LayoutColumn*, ColumnComperator> holdersSave{};

        void onSettings();
        void onMoveBegan();
        void onMoveUpdate(float dt);
        void onMoveEnded();

        bool currentlyExpandingLeft;
        bool currentlyExpandingRight;

        void updateInfoWithColumnData();

        LoadingCircle* loadingCircle;

        CCSprite* glow;

        arc::Future<std::string> modifyStrRecursive(const std::string& str);

        bool isBeingEdited = false;
        bool isHovering;

        float doubleClickTimer = .2f;

        float getSpecificTitleHeight();

        std::shared_ptr<RichTextKey<WaveKeyParams>> makeWaveKey();
        std::shared_ptr<RichTextKey<GradientKeyParams>> makeGradientKey();
        std::shared_ptr<RichTextKey<CPlusKeyParams>> makeCPlusKey();
        std::shared_ptr<RichTextKey<ccColor4B>> makeColor4BKey();
};