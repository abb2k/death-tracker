#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class GraphPointDisplay : public CCNode {
public:
    static GraphPointDisplay* create();

    void setContent(std::string run, float ratePercent, DTGraphType type, ccColor3B graphInner, ccColor3B graphOuter);

    float getPositionReal() const { return m_positionReal; }
    void setPositionReal(float positionReal);

private:
    bool init();
    
    float m_positionReal;

    geode::Label* m_percentLabel;
    geode::Label* m_rateLabel;
};