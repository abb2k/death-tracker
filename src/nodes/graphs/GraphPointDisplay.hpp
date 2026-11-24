#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class GraphPointDisplay : public CCNode {
protected:
    bool init();
    
public:
    static GraphPointDisplay* create();

    void setContent(std::string run, float ratePercent, DTGraphType type, ccColor3B graphInner, ccColor3B graphOuter);

    float getPositionReal() const { return m_positionReal; }
    void setPositionReal(float positionReal);

private:
    float m_positionReal;

    geode::Label* m_percentLabel;
    geode::Label* m_rateLabel;
};