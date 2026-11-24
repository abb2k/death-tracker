#include "DTGraphNode.hpp"

#include <managers/StatsManager.hpp>
#include <nodes/general/SessionSelector.hpp>
#include <nodes/DTLayer.hpp>
#include <nodes/graphs/GraphPoint.hpp>

DTGraphNode* DTGraphNode::create() {
    auto ret = new DTGraphNode();
    if (ret && ret->init()) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool DTGraphNode::init() {

    m_pointHolder = CCMenu::create();
    m_pointHolder->setZOrder(1);
    m_pointHolder->ignoreAnchorPointForPosition(false);
    m_pointHolder->setPosition({0,0});
    m_pointHolder->setContentSize({0,0});
    this->addChild(m_pointHolder);

    return true;
}

void DTGraphNode::setScaling(const CCSize& scaling) {
    this->setContentSize(scaling);
    this->m_scaling = scaling / 100;

    updateGraphContent();
}

void DTGraphNode::setInfo(const DTGraphInfo& info) {
    bool coverageChanged = this->m_info.has_value() ? this->m_info.value().m_coverage != info.m_coverage : true;
    this->m_info = info;

    if (coverageChanged) {
        switch (info.m_coverage)
        {
        case DTGraphCoverage::GeneralCover :
            getGeneralDeaths();
            break;
        case DTGraphCoverage::GeneralRunsCover :
            getGeneralRuns();
            break;
        case DTGraphCoverage::SectionContentCover :
            getSectionDeaths();
            break;
        case DTGraphCoverage::SectionGeneralCover :
            getGeneralDeaths();
            break;
        case DTGraphCoverage::SectionSessionCover :
            getSessionDeaths();
            break;
        case DTGraphCoverage::SessionCover :
            getSessionDeaths();
            break;
        case DTGraphCoverage::SessionRunsCover :
            getSessionRuns();
            break;
        
        default:
            break;
        }
    }

    this->setZOrder(std::numeric_limits<int>::max() - info.m_orderPos);
    this->setVisible(info.m_isEnabled);

    updateGraphContent();
}

void DTGraphNode::updateDeaths() {
    if (!this->m_info.has_value()) return;

    switch (this->m_info.value().m_coverage)
    {
    case DTGraphCoverage::GeneralCover :
        getGeneralDeaths();
        break;
    case DTGraphCoverage::GeneralRunsCover :
        getGeneralRuns();
        break;
    case DTGraphCoverage::SectionContentCover :
        getSectionDeaths();
        break;
    case DTGraphCoverage::SectionGeneralCover :
        getGeneralDeaths();
        break;
    case DTGraphCoverage::SectionSessionCover :
        getSessionDeaths();
        break;
    case DTGraphCoverage::SessionCover :
        getSessionDeaths();
        break;
    case DTGraphCoverage::SessionRunsCover :
        getSessionRuns();
        break;
    
    default:
        break;
    }
}

void DTGraphNode::setSectionToShow(const Section& section) {
    m_sectionToShow = section;
    updateGraphContent();
}

std::optional<std::pair<float, float>> DTGraphNode::getCoverageRange() {
    if (!m_info.has_value() || !isVisible()) return std::nullopt;

    if (isRunCoverage(m_info->m_coverage))
        return std::pair<float, float>{static_cast<float>(m_runPercent), 100};

    if (m_info->m_coverage != DTGraphCoverage::SectionContentCover)
        return std::pair<float, float>{0, 100};
    
    if (!m_sectionToShow.has_value() || !m_sectionToShow->isValid() || m_sectionToShow->m_startPercent == m_sectionToShow->m_endPercent)
        return std::nullopt;
    
    return std::pair<float, float>{
        static_cast<float>(m_sectionToShow->m_startPercent),
        static_cast<float>(m_sectionToShow->m_endPercent)
    };
}

void DTGraphNode::updateGraphContent() {
    if (!this->m_info.has_value()) return;
    
    if (!m_deaths.size()) {
        if (m_lineNode != nullptr) m_lineNode->clear();

        m_pointHolder->removeAllChildrenWithCleanup(true);
        return;
    }

    const bool runCoverage = isRunCoverage(m_info.value().m_coverage);

    float RunStartPercent = runCoverage ? m_runPercent : 0;
    bool hasRunStart = runCoverage;

    if (!runCoverage) {
        for (const auto& death : m_deaths) {
            auto runStartRes = StatsManager::splitRunKey(death.first);
            if (runStartRes.isErr()) continue;
            auto start = runStartRes.unwrap().m_start;
            if (start != std::nullopt) {
                if (!hasRunStart || start < RunStartPercent) {
                    RunStartPercent = start.value();
                    hasRunStart = true;
                }
            }
        }
    }

    m_points.clear();

    std::vector<std::vector<CCPoint>> graphLines;
    std::vector<std::pair<CCPoint, std::string>> sectionGraphPoints;

    struct Sorterrator {
        bool operator()(const Run& a, const Run& b) const {
            if (a.m_start != b.m_start)
                return a.m_start < b.m_start;
            return a.m_end < b.m_end;
        }
    };

    std::map<Run, int, Sorterrator> sortedDeaths{};

    if (isSectionCoverage(m_info.value().m_coverage)) {
        auto dtLayer = DTLayer::get();
        if (dtLayer == nullptr || dtLayer->m_MyLevelStats.isErr()) {
            if (m_lineNode != nullptr) m_lineNode->clear();
            m_pointHolder->removeAllChildrenWithCleanup(true);
            return;
        }

        auto sections = dtLayer->m_MyLevelStats.unwrap().m_metadata.m_sections;
        std::sort(sections.begin(), sections.end(), [](const Section& a, const Section& b) {
            if (a.m_startPercent != b.m_startPercent) return a.m_startPercent < b.m_startPercent;
            return a.m_endPercent < b.m_endPercent;
        });

        std::vector<CCPoint> groupedSectionLinePoints;

        for (const auto& section : sections) {
            if (!section.isValid() || section.m_startPercent == section.m_endPercent) continue;

            if (m_info.value().m_coverage == DTGraphCoverage::SectionContentCover && (!m_sectionToShow.has_value() || section.m_startPercent != m_sectionToShow->m_startPercent || section.m_endPercent != m_sectionToShow->m_endPercent || section.m_name != m_sectionToShow->m_name)) continue;

            std::map<double, int> deathsByPercent;
            for (const auto& death : m_deaths) {
                auto runRes = StatsManager::splitRunKey(death.first);
                if (runRes.isErr()) continue;

                const auto& run = runRes.unwrap();
                if (isRunCoverage(m_info.value().m_coverage) && (run.m_start.value_or(0) != m_runPercent || run.m_end < m_runPercent)) continue;

                const double percent = run.m_end;
                deathsByPercent[percent] += death.second;
            }

            int totalDeaths = 0;
            for (const auto& [percent, count] : deathsByPercent) {
                if (percent >= section.m_startPercent)
                    totalDeaths += count;
            }
            if (totalDeaths <= 0) continue;

            auto deathsAtOrAbove = [&](double percent) {
                int count = 0;
                for (auto it = deathsByPercent.lower_bound(percent); it != deathsByPercent.end(); ++it)
                    count += it->second;
                return count;
            };
            auto deathsAbove = [&](double percent) {
                int count = 0;
                for (auto it = deathsByPercent.upper_bound(percent); it != deathsByPercent.end(); ++it)
                    count += it->second;
                return count;
            };
            auto rateAt = [&](double percent) {
                bool completedLevelSection = section.m_endPercent == 100 && percent == 100;
                int count = m_info.value().m_type == DTGraphType::Passrate && !completedLevelSection ? deathsAbove(percent) : deathsAtOrAbove(percent);
                return count / static_cast<float>(totalDeaths) * 100;
            };

            if (m_info.value().m_coverage == DTGraphCoverage::SectionContentCover) {
                std::vector<CCPoint> linePoints;
                auto addSectionPoint = [&](double percent) {
                    auto position = ccp(percent * m_scaling.width, rateAt(percent) * m_scaling.height);
                    linePoints.push_back(position);
                    sectionGraphPoints.emplace_back(
                        position,
                        fmt::format("{} {}%", section.m_name, static_cast<int>(percent))
                    );
                };

                addSectionPoint(section.m_startPercent);
                for (const auto& [percent, _] : deathsByPercent) {
                    if (percent <= section.m_startPercent || percent >= section.m_endPercent) continue;
                    addSectionPoint(percent);
                }
                addSectionPoint(section.m_endPercent);

                graphLines.push_back(std::move(linePoints));
                continue;
            }

            if (groupedSectionLinePoints.empty()) {
                groupedSectionLinePoints.emplace_back(
                    section.m_startPercent * m_scaling.width,
                    100 * m_scaling.height
                );
            }

            auto position = ccp(
                section.m_endPercent * m_scaling.width,
                rateAt(section.m_endPercent) * m_scaling.height
            );
            groupedSectionLinePoints.push_back(position);
            sectionGraphPoints.emplace_back(position, section.m_name);
        }

        if (!groupedSectionLinePoints.empty())
            graphLines.push_back(std::move(groupedSectionLinePoints));
    }
    else {
        for (const auto& death : m_deaths) {
            auto runRes = StatsManager::splitRunKey(death.first);
            if (runRes.isErr()) continue;

            const auto& run = runRes.unwrap();
            if (run.m_start.value_or(0) != m_runPercent || run.m_end < m_runPercent) continue;
            sortedDeaths[run] += death.second;
        }

    if (m_info.value().m_type == DTGraphType::Passrate) {
        int bestRun = 0;

        int overallDeaths = 0;

        //<run, <passes, reaches>>
        
        std::map<Run, std::pair<int, int>, Sorterrator> deathsWithPassCount{};

        for (std::map<Run, int, Sorterrator>::reverse_iterator it = sortedDeaths.rbegin(); it != sortedDeaths.rend(); ++it)
        {
            //log::info("r: ({}, {}), d: {} | O: {}", it->first.m_start, it->first.m_end, it->second, overallDeaths);

            deathsWithPassCount[it->first] = {overallDeaths, overallDeaths + it->second};
            overallDeaths += it->second;

            if (it->first.m_end > bestRun) bestRun = it->first.m_end;
        }

        //log::info("added extras");

        CCPoint previousPoint = ccp(-1, -1);

        std::optional<int> endPointIndex = std::nullopt;
        bool firstPercentPoint = false;

        bool secondPercentPoint = false;
        int firstAfterSecondPointIndex = -1;
        int firstAfterSecondPoint = -1;

        for (const auto& dwp : deathsWithPassCount)
        {
            //save point
            CCPoint myPoint = ccp(dwp.first.m_end, dwp.second.first / static_cast<float>(dwp.second.second) * 100);

            //log::info("{} | {}", previousPoint, myPoint);
            //add extra points
            if (previousPoint.x != -1) {

                //add a before point if needed
                if (previousPoint.x != myPoint.x - 1) {
                    //log::info("p: {}, {} | {}", previousPoint, myPoint, bestRun);
                    if (previousPoint.x != myPoint.x - 2 && previousPoint.y != 100 && previousPoint.x + 1 <= bestRun) {
                        m_points.push_back(ccp(previousPoint.x + 1, 100) * m_scaling);
                    }

                    if (myPoint.x - 1 <= bestRun && myPoint.y != 100)
                        m_points.push_back(ccp(myPoint.x - 1, 100) * m_scaling);
                }
            }

            if (myPoint.x == 100)
                endPointIndex = static_cast<int>(m_points.size());

            if (!secondPercentPoint && firstAfterSecondPoint == -1 && myPoint.x > 2) {
                firstAfterSecondPoint = myPoint.x;
                firstAfterSecondPointIndex = static_cast<int>(m_points.size());
            }

            if (myPoint.x == 1)
                firstPercentPoint = true;
            if (myPoint.x == 2)
                secondPercentPoint = true;

            m_points.emplace_back(myPoint * m_scaling);
            previousPoint = myPoint;
        }

        if (m_points.size()) {
            int added = 0;
            if (runCoverage) {
                m_points.insert(m_points.begin(), ccp(m_runPercent * m_scaling.width, 100 * m_scaling.height));
                added++;
            }
            else if (m_points[0].x != 0) {
                m_points.insert(m_points.begin(), ccp(0, 100 * m_scaling.height));
                added++;
                if (!firstPercentPoint) {
                        if (!secondPercentPoint && firstAfterSecondPointIndex != -1) {
                        m_points.insert(std::next(m_points.begin(), firstAfterSecondPointIndex + 1), ccp((firstAfterSecondPoint - 1) * m_scaling.width, 100 * m_scaling.height));
                        added++;
                    }
                }
            }

            if (endPointIndex.has_value()) {
                m_points[endPointIndex.value() + added].y = 100 * m_scaling.height;
                if (runCoverage
                    ? endPointIndex.value() + added > 1
                    : endPointIndex.value() - 1 != 0)
                    m_points.erase(std::next(m_points.begin(), endPointIndex.value() + added - 1 ));
            }
            else {
                m_points.push_back(ccp(100 * m_scaling.width, 0));
            }
        }
    }
    else if (m_info.value().m_type == DTGraphType::Reachrate) {

        int overallCount = 0;
        std::vector<std::pair<int, int>> percentageDeaths{};
        for (std::map<Run, int, Sorterrator>::reverse_iterator it = sortedDeaths.rbegin(); it != sortedDeaths.rend(); ++it)
        {
            overallCount += it->second;
            percentageDeaths.emplace(percentageDeaths.begin(), it->first.m_end, overallCount);
        }

        if (percentageDeaths.size()) {
             if (percentageDeaths[0].first > RunStartPercent) {
                int RunStartPercentTemp = RunStartPercent == -1 ? RunStartPercent + 1 : RunStartPercent;
                percentageDeaths.emplace(percentageDeaths.begin(), RunStartPercentTemp, overallCount);
            }
            
            if (percentageDeaths.back().first < 100) {
                int nextPercent = percentageDeaths.back().first + 1;
                if (nextPercent <= 100) {
                    percentageDeaths.emplace_back(nextPercent, 0);
                }

                if (percentageDeaths.back().first != 100)
                    percentageDeaths.emplace_back(100, 0);
            }
        }

        for (int i = 0; i < percentageDeaths.size(); i++)
        {
            float reachRate = static_cast<float>(percentageDeaths[i].second) / overallCount;
            m_points.emplace_back(percentageDeaths[i].first * m_scaling.width, reachRate * m_scaling.height * 100);
        }
        
    }

        graphLines.push_back(m_points);
    }

    if (m_lineNode != nullptr) m_lineNode->clear();
    else {
        m_lineNode = CCDrawNode::create();
        m_lineNode->m_bUseArea = false;
        this->addChild(m_lineNode);
    }
    m_lineNode->setID(m_info.value().m_name + "-graph");

    m_pointHolder->removeAllChildrenWithCleanup(true);
    
    
    //log::info("{} | {}", myGraph.points.size(), deaths.size());

    std::vector<CCPoint> segmentList{};

    bool isFirst = false;
    CCPoint prevPoint;
    auto addGraphPoint = [&](const CCPoint& linePoint, std::string_view pointText) {
        if (linePoint.x >= 0 && linePoint.x <= 100 * m_scaling.width && linePoint.y >= 0 && linePoint.y <= 100 * m_scaling.height)
        {
            auto GP = GraphPoint::create(pointText, linePoint.y / m_scaling.height, m_info.value().m_pointColor, this);
            GP->setDelegate(m_delegate);
            GP->setPosition(linePoint);
            GP->setScale(m_info.value().m_pointScale);
            m_pointHolder->addChild(GP);
        }
    };

    if (isSectionCoverage(m_info.value().m_coverage)) {
        for (const auto& [position, label] : sectionGraphPoints) {
            addGraphPoint(position, label);
        }
    }
    else {
        for (const CCPoint& linePoint : m_points) {
            int percent = linePoint.x / m_scaling.width;
            auto pointText = fmt::format("{}", percent);
            if (RunStartPercent != 0)
                pointText = fmt::format("{}-{}", RunStartPercent, percent);
            addGraphPoint(linePoint, pointText);
        }
    }

    for (const auto& linePoints : graphLines) {
        std::vector<CCPoint> segmentList;
        for (size_t i = 1; i < linePoints.size(); ++i) {
            segmentList.push_back(linePoints[i - 1]);
            segmentList.push_back(linePoints[i]);
        }

        if (segmentList.empty()) continue;
        m_lineNode->drawLines(segmentList.data(), segmentList.size(), m_info.value().m_thickness + m_info.value().m_outlineThickness, ccc4FFromccc4B(m_info.value().m_outlineColor));
        m_lineNode->drawLines(segmentList.data(), segmentList.size(), m_info.value().m_thickness, ccc4FFromccc4B(m_info.value().m_color));
    }
}

void DTGraphNode::getGeneralDeaths() {
    getDeathsAsync(
        [](GeneralData const& data) { return data.m_deaths.toLegacyDeaths(); },
        false
    );
}

void DTGraphNode::getGeneralRuns() {
    getDeathsAsync(
        [](GeneralData const& data) { return data.m_runs.toLegacyDeaths(); },
        false
    );
}

void DTGraphNode::getSectionDeaths() {
    getDeathsAsync(
        [](GeneralData const& data) {
            auto allDeaths = data.m_deaths.toLegacyDeaths();
            StatsManager::mergeMapsAdd(allDeaths, data.m_runs.toLegacyDeaths());
            return allDeaths;
        },
        false
    );
}

void DTGraphNode::getSessionDeaths() {
    if (!m_sessionToShow.has_value()) {
        onDeathsUpdated(Err("no session"));
        return;
    }

    onDeathsUpdated(Ok(m_sessionToShow.value().m_data.m_deaths.toLegacyDeaths()));
}

void DTGraphNode::getSessionRuns() {
    if (!m_sessionToShow.has_value()) {
        onDeathsUpdated(Err("no session"));
        return;
    }

    onDeathsUpdated(Ok(m_sessionToShow.value().m_data.m_runs.toLegacyDeaths()));
}


void DTGraphNode::onDeathsUpdated(GetTFuture<Deaths>::Output deaths) {
    if (deaths.isErr()) {
        log::error("Failed to get deaths for graph {}: {}", m_info.value().m_name, deaths.unwrapErr().m_error);
        this->m_deaths.clear();
    }
    else {
        this->m_deaths = std::move(deaths).unwrap();
    }

    updateGraphContent();
}

void DTGraphNode::getDeathsAsync(Function<Deaths(GeneralData const&)>&& dataGetter, bool session) {
    m_getDeathsListener.cancel();
    m_getDeathsListener.spawn(fmt::format("DT-get-deaths-session:{}-task", session), DTLayer::get()->getTFor<Deaths>(
        std::move(dataGetter),
        [](Deaths const& a, Deaths const& b) {
            auto map = a;
            StatsManager::mergeMapsAdd(map, b);
            return map;
        }, session),
        [&](GetTFuture<Deaths>::Output out) {
            onDeathsUpdated(std::move(out));
        }
    );
}

bool DTGraphNode::isRunCoverage(DTGraphCoverage coverage) {
    return coverage == DTGraphCoverage::GeneralRunsCover || coverage == DTGraphCoverage::SessionRunsCover;
}

bool DTGraphNode::isSectionCoverage(DTGraphCoverage coverage) {
    return coverage == DTGraphCoverage::SectionContentCover || coverage == DTGraphCoverage::SectionGeneralCover || coverage == DTGraphCoverage::SectionSessionCover;
}