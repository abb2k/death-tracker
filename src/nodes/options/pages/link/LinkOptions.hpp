#pragma once

#include <nodes/options/OptionsNode.hpp>
#include <managers/LinkedLevelsCacheManager.hpp>

using namespace geode::prelude;

class LinkOptions : public OptionsNode {
public:
    static LinkOptions* create(CCSize size);
    ~LinkOptions() override;

    void onOpened() override;
    void onClosed() override;
    
private:
    bool setup() override;

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
};