#include <nodes/DTGraphNode.hpp>

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

    pointHolder = CCMenu::create();
    pointHolder->setZOrder(1);
    pointHolder->ignoreAnchorPointForPosition(false);
    pointHolder->setPosition({0,0});
    pointHolder->setContentSize({0,0});
    this->addChild(pointHolder);

    return true;
}

void DTGraphNode::setScaling(const CCSize& scaling) {
    this->setContentSize(scaling);
    this->scaling = scaling / 100;

    updateGraphContent();
}

void DTGraphNode::setInfo(const DTGraphInfo& info) {
    bool coverageChanged = this->info.has_value() ? this->info.value().coverage != info.coverage : true;
    this->info = info;

    if (coverageChanged) {
        switch (info.coverage)
        {
        case DTGraphCoverage::GeneralCover :
            getGeneralDeaths();
            break;
        case DTGraphCoverage::GeneralRunsCover :
            getGeneralRuns();
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

    this->setZOrder(std::numeric_limits<int>::max() - info.orderPos);
    this->setVisible(info.isEnabled);

    updateGraphContent();
}

void DTGraphNode::updateDeaths() {
    if (!this->info.has_value()) return;

    switch (this->info.value().coverage)
    {
    case DTGraphCoverage::GeneralCover :
        getGeneralDeaths();
        break;
    case DTGraphCoverage::GeneralRunsCover :
        getGeneralRuns();
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

void DTGraphNode::updateGraphContent() {
    if (!this->info.has_value()) return;
    
    if (!deaths.size()) {
        if (lineNode != nullptr) lineNode->clear();

        pointHolder->removeAllChildrenWithCleanup(true);
        return;
    }

    float RunStartPercent = 0;
    bool hasRunStart = false;
    for (const auto& death : deaths) {
        auto runStartRes = StatsManager::splitRunKey(death.first);
        if (runStartRes.isErr()) continue;
        auto start = runStartRes.unwrap().start;
        if (start != std::nullopt) {
            if (!hasRunStart || start < RunStartPercent) {
                RunStartPercent = start.value();
                hasRunStart = true;
            }
        }
    }

    points.clear();

    struct Sorterrator {
        bool operator()(const Run& a, const Run& b) const {
            if (a.start != b.start)
                return a.start < b.start;
            return a.end < b.end;
        }
    };

    std::map<Run, int, Sorterrator> sortedDeaths{};

    for (const auto& death : deaths) {
        auto runRes = StatsManager::splitRunKey(death.first);
        if (runRes.isErr()) continue;

        if (runRes.unwrap().start != std::nullopt) {
            if (runRes.unwrap().start.value() == runPercent)
                sortedDeaths[runRes.unwrap()] += death.second;
        }
        else
            sortedDeaths[runRes.unwrap()] += death.second;
    }

    if (info.value().type == DTGraphType::Passrate) {
        int bestRun = 0;

        int overallDeaths = 0;

        //<run, <passes, reaches>>
        
        std::map<Run, std::pair<int, int>, Sorterrator> deathsWithPassCount{};

        for (std::map<Run, int, Sorterrator>::reverse_iterator it = sortedDeaths.rbegin(); it != sortedDeaths.rend(); ++it)
        {
            //log::info("r: ({}, {}), d: {} | O: {}", it->first.start, it->first.end, it->second, overallDeaths);

            deathsWithPassCount[it->first] = {overallDeaths, overallDeaths + it->second};
            overallDeaths += it->second;

            if (it->first.end > bestRun) bestRun = it->first.end;
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
            CCPoint myPoint = ccp(dwp.first.end, dwp.second.first / static_cast<float>(dwp.second.second) * 100);

            //log::info("{} | {}", previousPoint, myPoint);
            //add extra points
            if (previousPoint.x != -1) {

                //add a before point if needed
                if (previousPoint.x != myPoint.x - 1) {
                    //log::info("p: {}, {} | {}", previousPoint, myPoint, bestRun);
                    if (previousPoint.x != myPoint.x - 2 && previousPoint.y != 100 && previousPoint.x + 1 <= bestRun) {
                        points.push_back(ccp(previousPoint.x + 1, 100) * scaling);
                    }

                    if (myPoint.x - 1 <= bestRun && myPoint.y != 100)
                        points.push_back(ccp(myPoint.x - 1, 100) * scaling);
                }
            }

            if (myPoint.x == 100)
                endPointIndex = static_cast<int>(points.size());

            if (!secondPercentPoint && firstAfterSecondPoint == -1 && myPoint.x > 2) {
                firstAfterSecondPoint = myPoint.x;
                firstAfterSecondPointIndex = static_cast<int>(points.size());
            }

            if (myPoint.x == 1)
                firstPercentPoint = true;
            if (myPoint.x == 2)
                secondPercentPoint = true;

            points.emplace_back(myPoint * scaling);
            previousPoint = myPoint;
        }

        if (points.size()) {
            int added = 0;
            if (points[0].x != 0) {
                points.insert(points.begin(), ccp(0, 100 * scaling.height));
                added++;
                if (!firstPercentPoint) {


                        if (!secondPercentPoint && firstAfterSecondPointIndex != -1) {
                        points.insert(std::next(points.begin(), firstAfterSecondPointIndex + 1), ccp((firstAfterSecondPoint - 1) * scaling.width, 100 * scaling.height));
                        added++;
                    }
                }
            }

            if (endPointIndex.has_value()) {
                points[endPointIndex.value() + added].y = 100 * scaling.height;
                if (endPointIndex.value() - 1 != 0)
                    points.erase(std::next(points.begin(), endPointIndex.value() + added - 1 ));
            }
            else {
                points.push_back(ccp(100 * scaling.width, 0));
            }
        }
    }
    else if (info.value().type == DTGraphType::Reachrate) {

        int overallCount = 0;
        std::vector<std::pair<int, int>> percentageDeaths{};
        for (std::map<Run, int, Sorterrator>::reverse_iterator it = sortedDeaths.rbegin(); it != sortedDeaths.rend(); ++it)
        {
            overallCount += it->second;
            percentageDeaths.emplace(percentageDeaths.begin(), it->first.end, overallCount);
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
            points.emplace_back(percentageDeaths[i].first * scaling.width, reachRate * scaling.height * 100);
        }
        
    }

    if (lineNode != nullptr) lineNode->clear();
    else {
        lineNode = CCDrawNode::create();
        lineNode->m_bUseArea = false;
        this->addChild(lineNode);
    }
    lineNode->setID(info.value().name + "-graph");

    pointHolder->removeAllChildrenWithCleanup(true);
    
    
    //log::info("{} | {}", myGraph.points.size(), deaths.size());

    std::vector<CCPoint> segmentList{};

    bool isFirst = false;
    CCPoint prevPoint;
    for (const CCPoint& linePoint : points)
    {
        if (linePoint.x >= 0 && linePoint.x <= 100 * scaling.width && linePoint.y >= 0 && linePoint.y <= 100 * scaling.height)
        {
            int percent = linePoint.x / scaling.width;
            auto pointText = fmt::format("{}", percent);
            if (RunStartPercent != 0)
                pointText = fmt::format("{}-{}", RunStartPercent, percent);

            auto GP = GraphPoint::create(pointText, linePoint.y / scaling.height, info.value().pointColor);
            GP->relatedGraph = this;
            GP->setDelegate(delegate);
            GP->setPosition(linePoint);
            GP->setScale(info.value().pointScale);
            pointHolder->addChild(GP);
        }

        if (!isFirst) {
            prevPoint = linePoint;
            isFirst = true;
            continue;
        }

        segmentList.push_back(prevPoint);
        segmentList.push_back(linePoint);

        prevPoint = linePoint;
    }

    lineNode->drawLines(&segmentList[0], segmentList.size(), info.value().thickness + info.value().outlineThickness, ccc4FFromccc4B(info.value().outlineColor));
    lineNode->drawLines(&segmentList[0], segmentList.size(), info.value().thickness, ccc4FFromccc4B(info.value().color));
}

void DTGraphNode::getGeneralDeaths() {
    getDeathsAsync(
        [](GeneralData const& data) { return data.deaths; },
        false
    );
}

void DTGraphNode::getGeneralRuns() {
    getDeathsAsync(
        [](GeneralData const& data) { return data.runs; },
        false
    );
}

void DTGraphNode::getSessionDeaths() {
    if (!sessionToShow.has_value()) {
        onDeathsUpdated(Err("no session"));
        return;
    }

    onDeathsUpdated(Ok(sessionToShow.value().data.deaths));
}

void DTGraphNode::getSessionRuns() {
    if (!sessionToShow.has_value()) {
        onDeathsUpdated(Err("no session"));
        return;
    }

    onDeathsUpdated(Ok(sessionToShow.value().data.runs));
}


void DTGraphNode::onDeathsUpdated(GetTFuture<Deaths>::Output deaths) {
    if (deaths.isErr()) {
        log::error("Failed to get deaths for graph {}: {}", info.value().name, deaths.unwrapErr().error);
        this->deaths.clear();
    }
    else {
        this->deaths = std::move(deaths).unwrap();
    }

    updateGraphContent();
}

void DTGraphNode::getDeathsAsync(geode::Function<Deaths(GeneralData const&)>&& dataGetter, bool session) {
    getDeathsListener.cancel();
    getDeathsListener.spawn(fmt::format("DT-get-deaths-session:{}-task", session), DTLayer::get()->getTFor<Deaths>(
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