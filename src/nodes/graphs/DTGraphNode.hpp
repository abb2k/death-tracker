#pragma once

#include <nodes/layout/usage/SpecialKey.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class GraphPointDelegate;

class DTGraphNode : public CCNode {
public:
    static DTGraphNode* create();

    void setInfo(const DTGraphInfo& info);
    void setScaling(const CCSize& scaling);

    std::optional<DTGraphInfo> getInfo() const { return m_info; };
    std::optional<std::pair<float, float>> getCoverageRange();

    void setRunPercent(int runPercent);

    void updateDeaths();
    void setSectionToShow(const Section& section);

    void setDelegate(GraphPointDelegate* delegate);

    CCNode* getPointHolder() const { return m_pointHolder; }

    void setSessionToShow(std::optional<Session> session);
    void setSectionToShow(std::optional<Section> section);

private:
    bool init();

    void updateGraphContent();

    void onDeathsUpdated(GetTFuture<Deaths>::Output);

    void getGeneralDeaths();
    void getGeneralRuns();
    void getSectionDeaths();
    void getSessionDeaths();
    void getSessionRuns();

    void getDeathsAsync(Function<Deaths(GeneralData const&)>&& dataGetter, bool session);

    std::optional<DTGraphInfo> m_info = std::nullopt;
    Deaths m_deaths{};

    CCSize m_scaling;

    CCDrawNode* m_lineNode = nullptr;

    std::vector<CCPoint> m_points{};

    async::TaskHolder<GetTFuture<Deaths>::Output> m_getDeathsListener;

    bool isRunCoverage(DTGraphCoverage coverage);

    bool isSectionCoverage(DTGraphCoverage coverage);

    CCNode* m_pointHolder;

    GraphPointDelegate* m_delegate;

    int m_runPercent = 0;

    std::optional<Session> m_sessionToShow = std::nullopt;
    std::optional<Section> m_sectionToShow = std::nullopt;
};