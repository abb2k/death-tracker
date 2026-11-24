#pragma once

#include <types/DTTypes.hpp>

class DTGraphNode;
class GraphPointDelegate;
class AdvancedScrollLayer;

class GraphHolder : public CCNode {
public:
    static GraphHolder* create(CCSize scaling);
    
    void setSmallLinesColor(ccColor4B newColor);
    void setBoldLinesColor(ccColor4B newColor);
    void setLabelsColor(ccColor4B newColor);
    
    void setBGFillColor(ccColor4B color);
    void setOutlineThickness(float thickness);
    void setOutlineColor(ccColor4B color);

    void addGraph(const DTGraphInfo& graph);
    DTGraphNode* getGraphNode(const std::string& graphName);
    void removeGraph(const std::string& graphName);
    void changeGraphName(const std::string& oldName, const std::string& newName);

    void setToAllGraphs(Function<void(DTGraphNode*)> graphSetFunction);

    void sendUpdateToGraphOfType(DTGraphCoverage const& coverage);

    GraphPointDelegate* delegate;

    void sendKeyStuff(bool up, enumKeyCodes key);

    std::vector<DTGraphNode*> getAllGraphNodes() const;

    AdvancedScrollLayer* scrollLayer;

private:
    bool init(CCSize scaling);

    void refreshBackground(ccColor4B newFillColor, float newOutlineThickness, ccColor4B newOutlineColor);
    void refreshCoverageIndicators(const CCSize& graphSize);

    void refreshGrid();

    void updateLabels();

    void update(float dt);

    ccColor4B fillColor = {0, 0, 0, 120};
    float outlineThickness = 0.2f;
    ccColor4B outlineColor = { 124, 124, 124, 255};

    int labelEveryBase = 5;
    int labelEvery;

    CCDrawNode* boldGridNode = nullptr;
    ccColor4B boldGridColor = { 29, 29, 29, 255 };

    ccColor4B boldLineColor = {115, 115, 115, 255};
    ccColor4B smallLineColor = {115, 115, 115, 100};
    ccColor4B labelColor = { 202, 202, 202, 255};

    CCDrawNode* fillNode = nullptr;
    CCDrawNode* outlineNode = nullptr;
    CCDrawNode* coverageShadeNode = nullptr;
    CCDrawNode* coverageLineNode = nullptr;
    CCClippingNode* mask = nullptr;

    std::set<CCSprite*> smallLines{};
    std::set<CCSprite*> boldLines{};
    std::set<geode::Label*> labels{};

    CCNode* LabelsContainer = nullptr;
    CCNode* LabelsVerticalContainer = nullptr;
    CCNode* LabelsHorizontalContainer = nullptr;

    CCNode* graphsContainer = nullptr;

    std::unordered_map<std::string, DTGraphNode*> allGraphs{};

    CCNode* graphContainer;

    float prevZoom;
    std::optional<std::pair<float, float>> coverageBounds = std::nullopt;
    CCSize coverageGraphSize = {0, 0};

    float labelMaskOffset = 8.5f;

    float inGraphOffset = 7.5f;
};