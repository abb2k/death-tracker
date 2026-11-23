#pragma once

#include <Geode/Geode.hpp>
#include <nodes/options/OptionsNode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/general/SimpleToggler.hpp>

#include <managers/LinkedLevelsCacheManager.hpp>

#include <atomic>
#include <memory>

using namespace geode::prelude;

class LinkOptions : public OptionsNode {
    public:
        static LinkOptions* create(const CCSize& size);
        ~LinkOptions() override;
    private:
        virtual bool setup() override;

        void getAllLevelsData();

        void onGetLevels(GetLevelsFuture::Output const& out);
        void onGetLevelsProgress(const GetLevelsTaskProgress& progress);

        std::shared_ptr<std::atomic<LinkOptions*>> callbackOwner = std::make_shared<std::atomic<LinkOptions*>>(nullptr);
        std::vector<LinkData> cachedLevels;
        bool levelsLoaded = false;
        bool levelsLoading = false;

        unsigned int currentLinkedPage = 0;
        unsigned int currentPlayedPage = 0;

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
        geode::Label* playedScrollPageLabel;
        geode::Label* linkedScrollPageLabel;

        std::string searchStr;
        bool searchForID = false;
        TextInput* searchBar;

        void update(float dt) override;

        geode::Label* loadingLabel;
        LoadingCircle* loadingCircle;
        CCScale9Sprite* loadingShadow;

    public:
        virtual void onOpened() override;
        virtual void onClosed() override;
};