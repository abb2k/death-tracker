#pragma once

#include <Geode/Geode.hpp>
#include <nodes/layers/DTLayer.hpp>
#include <nodes/graphs/GraphPoint.hpp>
#include <nodes/graphs/GraphHolder.hpp>
#include <nodes/cells/GraphCell.hpp>
#include <nodes/OptionSwitcher.hpp>
#include <nodes/graphs/GraphPointDisplay.hpp>
#include <nodes/FloatingList.hpp>

using namespace geode::prelude;

struct GraphPointDisplayPtrCompare {
    bool operator()(GraphPointDisplay const* a, GraphPointDisplay const* b) const {
        if (a == b) return false;
        if (!a) return true;
        if (!b) return false;
        if (a->positionReal > b->positionReal) return true;
        if (a->positionReal < b->positionReal) return false;
        return a < b;
    }
};

class DTGraphLayer : public Popup, public GraphPointDelegate, public FLAlertLayerProtocol {
    protected:
        bool init() override;
    public:
        static DTGraphLayer* create();

    private:
        //graph stuff

        CCLabelBMFont* noGraphLabel;

        //change the displayed point
        void OnPointSelected(GraphPoint* point) override;
        //have no point displayed if the deselected point was the was selected prior
        void OnPointDeselected(GraphPoint* point) override;

        void keyDown(enumKeyCodes key, double d) override;
        void keyUp(enumKeyCodes key, double d) override;

        //handle the layer closing and refresh the text
        void onClose(cocos2d::CCObject*) override;

        void update(float dt) override;

        GraphHolder* graph;

        ScrollLayer* graphsScroll;
        Scrollbar* graphsScrollbar;

        void onAddGraph(CCObject*);

        void addGraph();
        void addGraph(const DTGraphInfo& info);
        void removeGraph(const std::string& graphName);

        void openOptionsFor(GraphCell* cell);
        CCNode* graphsPage;
        CCMenu* graphOptionsPage;

        void onColor(CCObject*);

        TextInput* nameInput;
        OptionSwitcher<DTGraphCoverage>* coverageSwitcher;
        OptionSwitcher<DTGraphType>* typeSwitcher;
        TextInput* thicknessBaseInput;
        TextInput* thicknessOutlineInput;
        CCSprite* colorBaseBtnSpr;
        CCSprite* colorOutlineBtnSpr;
        TextInput* pointScaleInput;
        CCSprite* pointColorBtnSpr;

        std::optional<GraphCell*> editedGraph = std::nullopt;

        void updateColor(cocos2d::ccColor4B const& color);

        std::optional<geode::Function<void(const ccColor4B&)>> callbacksForColorPopups = std::nullopt;

        void saveAllGraphs();

        void onOk(CCObject*);
        void onDelete(CCObject*);

        void closeOptionsTab();

        FLAlertLayer* deleteNotification;

        void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

        std::map<GraphPoint*, GraphPointDisplay*> displaysForPoints{};

        std::set<GraphPointDisplay*, GraphPointDisplayPtrCompare> displays{};

        bool holdingShift;

        void updateGRapgCellLayout();
    };