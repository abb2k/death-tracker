#pragma once

using namespace geode::prelude;

class GraphPoint;
class DTGraphNode;

class GraphPointDelegate {
public:
    virtual void OnPointSelected(GraphPoint*) = 0;

    virtual void OnPointDeselected(GraphPoint*) = 0;
};

class GraphPoint : public CCMenuItem {
public:
    static GraphPoint* create(std::string_view run, float rate, ccColor4B color, DTGraphNode* relatedGraph);

    void setDelegate(GraphPointDelegate* Delegate);

    ~GraphPoint();

    std::string getRun() const { return m_Run; }

    float getRate() const { return m_Rate; }
    
    DTGraphNode* getRelatedGraph() const { return m_relatedGraph; }

private:
    bool init(std::string_view run, float rate, ccColor4B color, DTGraphNode* relatedGraph);
    
    void update(float delta) override;

    CCSprite* m_c;

    GraphPointDelegate* m_Delegate = nullptr;
    bool m_oneTimeCall;

    DTGraphNode* m_relatedGraph;

    std::string m_Run;

    float m_Rate;
};