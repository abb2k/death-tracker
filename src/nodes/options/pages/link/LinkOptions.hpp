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

        std::shared_ptr<std::atomic<LinkOptions*>> m_callbackOwner = std::make_shared<std::atomic<LinkOptions*>>(nullptr);
        std::vector<LinkData> m_cachedLevels;
        bool m_levelsLoaded = false;
        bool m_levelsLoading = false;

        unsigned int m_currentLinkedPage = 0;
        unsigned int m_currentPlayedPage = 0;

        std::optional<unsigned int> m_maxLinkedPage;
        std::optional<unsigned int> m_maxPlayedPage;

        void updateScrollsContent(GetLevelsFuture::Output const& levels);

        ScrollLayer* m_linkedScroll;
        ScrollLayer* m_playedScroll;

        SimpleTextArea* m_nonePlayedFoundText;
        SimpleTextArea* m_noneLinkedFoundText;

        static constexpr int PER_PAGE_COUNT = 10;

        void onLinkCellClicked(const std::string& levelKey, bool isLeftAligned);

        void onSwitchedPage(CCObject* sender);
        void updatePageLabels();
        geode::Label* m_playedScrollPageLabel;
        geode::Label* m_linkedScrollPageLabel;

        std::string m_searchStr;
        bool m_searchForID = false;
        TextInput* m_searchBar;

        void update(float dt) override;

        geode::Label* m_loadingLabel;
        LoadingCircle* m_loadingCircle;
        CCScale9Sprite* m_loadingShadow;

    public:
        virtual void onOpened() override;
        virtual void onClosed() override;
};