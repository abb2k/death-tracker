#pragma once

using namespace geode::prelude;

struct FloatingListItem {
    int m_id;

    std::string m_text;
    std::string m_font = "bigFont.fnt";
    std::string m_BGTexture = "GJ_button_01.png";

    CCNode* m_sideButtonSprite = nullptr;
    std::function<void(int id)> m_sideButtonCallback = [](int) {};
};

class FloatingList : public CCNode, public CCTouchDelegate {
public:
    static FloatingList* create(CCSize size, bool startOpen = false);

    void addItem(const FloatingListItem& text);
    void addItems(const std::vector<FloatingListItem>& texts);

    void removeItem(int id);
    void clearAllItems();

    void open();
    void close();

    void setCallback(Function<void(const int& id)> callback);

    void setOpenDirection(bool openUpwards);

    bool isOpened() const { return m_isOpen; }

    std::optional<FloatingListItem> getItemForID(int id);

    void setEnabled(bool b);

    void updateListLayout();

    void setListOpenAnchor(CCPoint const& anchor);

private:
    bool init(CCSize size, bool startOpen);

    Function<void(const int& id)> m_onItemClicked;

    void itemClicked(CCObject* sender);
    void sideBtnClicked(CCObject* sender);

    void setItemEnabled(bool isEnabled);

    bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;

    std::map<CCMenuItemSpriteExtra*, FloatingListItem> m_itemIds{};
    std::map<CCMenuItemSpriteExtra*, FloatingListItem> m_extraButtonsItemIds{};

    ScrollLayer* m_scrollLayer;
    Scrollbar* m_scrollbar;

    float m_elementHeight = 20.f;

    float m_elementOffset = 5;

    bool m_isOpen = false;

    CCNode* m_clickArea;

    void onEnter() override;
    void onExit() override;
};