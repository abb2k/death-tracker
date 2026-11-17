#pragma once

#include <Geode/Geode.hpp>
#include <nodes/optionNodes/OptionsNode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SimpleToggler.hpp>

#include <managers/LinkedLevelsCacheManager.hpp>


using namespace geode::prelude;

class LinkOptions : public OptionsNode {
    public:
        static LinkOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        void getAllLevelsData();

        void onGetLevels(GetLevelsFuture::Output const& out);
        void onGetLevelsProgress(const GetLevelsTaskProgress& progress);

        unsigned int currentLinkedPage;
        unsigned int currentPlayedPage;

        std::optional<unsigned int> maxLinkedPage;
        std::optional<unsigned int> maxPlayedPage;

        void updateScrollsContent(GetLevelsFuture::Output const& levels);

        ScrollLayer* linkedScroll;
        ScrollLayer* playedScroll;

        SimpleTextArea* nonePlayedFoundText;
        SimpleTextArea* noneLinkedFoundText;

        static const int PER_PAGE_COUNT;

        void onLinkCellClicked(const std::string& levelKey, bool isLeftAligned);

        void onSwitchedPage(CCObject* sender);
        void updatePageLabels();
        CCLabelBMFont* playedScrollPageLabel;
        CCLabelBMFont* linkedScrollPageLabel;

        std::string searchStr;
        bool searchForID;
        TextInput* searchBar;

        void update(float dt);

        CCLabelBMFont* loadingLabel;
        LoadingCircle* loadingCircle;
        CCScale9Sprite* loadingShadow;

    public:
        virtual void onOpened();
        virtual void onClosed();
};