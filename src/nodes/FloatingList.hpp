#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

struct FloatingListItem {
    int id;

    std::string text;
    std::string font = "bigFont.fnt";
    std::string BGTexture = "GJ_button_01.png";

    CCNode* sideButtonSprite = nullptr;
    std::function<void(int id)> sideButtonCallback = [](int) {};
};

class FloatingList : public CCNode, public CCTouchDelegate {
    public:
        static FloatingList* create(CCSize const& size, bool startOpen = false);

        void addItem(const FloatingListItem& text);
        void addItems(const std::vector<FloatingListItem>& texts);

        void removeItem(int id);
        void clearAllItems();

        void open();
        void close();

        void setCallback(geode::Function<void(const int& id)> callback);

        void setOpenDirection(bool openUpwards);

        bool isOpened() const { return isOpen; }

        std::optional<FloatingListItem> getItemForID(int id);

        void setEnabled(bool b);

        void updateListLayout();

        void setListOpenAnchor(CCPoint const& anchor);

    private:
        bool init(CCSize const& size, bool startOpen);

        geode::Function<void(const int& id)> onItemClicked;

        void itemClicked(CCObject* sender);
        void sideBtnClicked(CCObject* sender);

        void setItemEnabled(bool isEnabled);

        bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;

        std::map<CCMenuItemSpriteExtra*, FloatingListItem> itemIds{};
        std::map<CCMenuItemSpriteExtra*, FloatingListItem> extraButtonsItemIds{};

        ScrollLayer* scrollLayer;
        Scrollbar* scrollbar;

        float elementHeight = 20.f;

        float elementOffset = 5;

        bool isOpen = false;

        CCNode* clickArea;

        void onEnter() override;
        void onExit() override;
};