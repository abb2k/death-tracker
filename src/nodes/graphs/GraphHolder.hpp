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

    void sendKeyStuff(bool up, enumKeyCodes key);

    void setDelegate(GraphPointDelegate* delegate);

    std::vector<DTGraphNode*> getAllGraphNodes() const;

    AdvancedScrollLayer* getScrollLayer() const { return m_scrollLayer; }

private:
    bool init(CCSize scaling);

    void refreshBackground(ccColor4B newFillColor, float newOutlineThickness, ccColor4B newOutlineColor);
    void refreshCoverageIndicators(const CCSize& graphSize);

    void refreshGrid();

    void updateLabels();

    void update(float dt);

    ccColor4B m_fillColor = {0, 0, 0, 120};
    float m_outlineThickness = 0.2f;
    ccColor4B m_outlineColor = { 124, 124, 124, 255};

    static constexpr int m_labelEveryBase = 5;
    int m_labelEvery;

    CCDrawNode* m_boldGridNode = nullptr;
    static constexpr ccColor4B m_boldGridColor = { 29, 29, 29, 255 };

    ccColor4B m_boldLineColor = {115, 115, 115, 255};
    ccColor4B m_smallLineColor = {115, 115, 115, 100};
    ccColor4B m_labelColor = { 202, 202, 202, 255};

    CCDrawNode* m_fillNode = nullptr;
    CCDrawNode* m_outlineNode = nullptr;
    CCDrawNode* m_coverageShadeNode = nullptr;
    CCDrawNode* m_coverageLineNode = nullptr;
    CCClippingNode* m_mask = nullptr;

    std::set<CCSprite*> m_smallLines{};
    std::set<CCSprite*> m_boldLines{};
    std::set<geode::Label*> m_labels{};

    CCNode* m_LabelsContainer = nullptr;
    CCNode* m_LabelsVerticalContainer = nullptr;
    CCNode* m_LabelsHorizontalContainer = nullptr;

    CCNode* m_graphsContainer = nullptr;

    std::unordered_map<std::string, DTGraphNode*> m_allGraphs{};

    CCNode* m_graphContainer;

    float m_prevZoom;
    std::optional<std::pair<float, float>> m_coverageBounds = std::nullopt;
    CCSize m_coverageGraphSize = {0, 0};

    static constexpr float m_labelMaskOffset = 8.5f;

    static constexpr float m_inGraphOffset = 7.5f;

    AdvancedScrollLayer* m_scrollLayer;

    GraphPointDelegate* m_delegate;
};