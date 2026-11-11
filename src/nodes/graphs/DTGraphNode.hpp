#pragma once

#include <Geode/Geode.hpp>
#include <managers/StatsManager.hpp>
#include <nodes/graphs/GraphPoint.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SessionSelector.hpp>
#include <nodes/layers/DTLayer.hpp>

using namespace geode::prelude;


class DTGraphNode : public CCNode {
    protected:
        bool init();
        
    public:
        static DTGraphNode* create();

        void setInfo(const DTGraphInfo& info);
        void setScaling(const CCSize& scaling);

        std::optional<DTGraphInfo> getInfo() const{
            return info;
        };

        std::optional<Session> sessionToShow = std::nullopt;

        int runPercent = 0;

        void updateDeaths();

        GraphPointDelegate* delegate;

        CCNode* pointHolder;

    private:

        void updateGraphContent();

        void onDeathsUpdated(GetTFuture<Deaths>::Output);

        void getGeneralDeaths();
        void getGeneralRuns();
        void getSessionDeaths();
        void getSessionRuns();

        void getDeathsAsync(geode::Function<Deaths(GeneralData const&)>&& dataGetter, bool session);

        std::optional<DTGraphInfo> info = std::nullopt;
        Deaths deaths{};

        CCSize scaling;

        CCDrawNode* lineNode = nullptr;

        std::vector<CCPoint> points{};

        async::TaskHolder<GetTFuture<Deaths>::Output> getDeathsListener;
};