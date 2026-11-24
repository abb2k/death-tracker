#pragma once

using namespace geode::prelude;

#include <types/DTTypes.hpp>
#include <nodes/graphs/GraphPoint.hpp>

template <class T>
class OptionSwitcher;
class GraphPointDisplay;
class GraphCell;
class GraphHolder;

struct GraphPointDisplayPtrCompare {
    bool operator()(GraphPointDisplay const* a, GraphPointDisplay const* b) const;
};

class DTGraphLayer : public Popup, public GraphPointDelegate, public FLAlertLayerProtocol {
public:
    static DTGraphLayer* create();

private:
    bool init() override;

    geode::Label* m_noGraphLabel;

    void OnPointSelected(GraphPoint* point) override;
    void OnPointDeselected(GraphPoint* point) override;

    void keyDown(enumKeyCodes key, double d) override;
    void keyUp(enumKeyCodes key, double d) override;

    void onClose(cocos2d::CCObject*) override;

    void update(float dt) override;

    GraphHolder* m_graph;

    ScrollLayer* m_graphsScroll;
    Scrollbar* m_graphsScrollbar;

    void onAddGraph(CCObject*);

    void addGraph();
    void addGraph(const DTGraphInfo& info);
    void removeGraph(std::string_view graphName);

    void openOptionsFor(GraphCell* cell);
    CCNode* m_graphsPage;
    CCMenu* m_graphOptionsPage;

    void onColor(CCObject*);

    TextInput* m_nameInput;
    OptionSwitcher<DTGraphCoverage>* m_coverageSwitcher;
    OptionSwitcher<DTGraphType>* m_typeSwitcher;
    TextInput* m_thicknessBaseInput;
    TextInput* m_thicknessOutlineInput;
    CCSprite* m_colorBaseBtnSpr;
    CCSprite* m_colorOutlineBtnSpr;
    TextInput* m_pointScaleInput;
    CCSprite* m_pointColorBtnSpr;

    std::optional<GraphCell*> m_editedGraph = std::nullopt;

    void updateColor(cocos2d::ccColor4B const& color);

    std::optional<Function<void(const ccColor4B&)>> m_callbacksForColorPopups = std::nullopt;

    void saveAllGraphs();

    void onOk(CCObject*);
    void onDelete(CCObject*);

    void closeOptionsTab();

    FLAlertLayer* m_deleteNotification;

    void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

    std::map<GraphPoint*, GraphPointDisplay*> m_displaysForPoints{};

    std::set<GraphPointDisplay*, GraphPointDisplayPtrCompare> m_displays{};

    bool m_holdingShift;

    void updateGRapgCellLayout();

    std::vector<GraphCell*> getGraphCells(CCNode* contentLayer);
};