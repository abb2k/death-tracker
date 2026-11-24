#include "GraphHolder.hpp"

#include <nodes/graphs/DTGraphNode.hpp>
#include <nodes/general/AdvancedScrollLayer.hpp>

GraphHolder* GraphHolder::create(CCSize scaling) {
    auto ret = new GraphHolder();

    if (ret && ret->init(std::move(scaling))) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool GraphHolder::init(CCSize scaling) {
    if (!CCNode::init()) return false;
    
    this->setContentSize(scaling);
    this->setAnchorPoint({.5f, .5f});

    m_labelEvery = m_labelEveryBase;

    m_scrollLayer = AdvancedScrollLayer::create(scaling, scaling + ccp(.01f,.01f));
    m_scrollLayer->setHorizontalScrollbarPosition(true);
    m_scrollLayer->m_minZoom = 5;
    this->addChild(m_scrollLayer);

    m_LabelsContainer = CCNode::create();
    m_LabelsContainer->setID("labels-container");
    this->addChild(m_LabelsContainer);

    CCPoint LabelsVerticalMaskShape[4] = {
        ccp(0, 0),
        ccp(40, 0),
        ccp(40, scaling.height + m_labelMaskOffset),
        ccp(0, scaling.height + m_labelMaskOffset)
    };

    auto LabelsVerticalMask = CCClippingNode::create();
    auto verticalStencil = CCDrawNode::create();
    verticalStencil->drawPolygon(LabelsVerticalMaskShape, 4, ccc4FFromccc4B({0, 0, 0, 0}), .01f, ccc4FFromccc4B({0,0,0,0}));
    LabelsVerticalMask->setStencil(verticalStencil);
    LabelsVerticalMask->setContentSize({40, scaling.height + m_labelMaskOffset});
    LabelsVerticalMask->setAnchorPoint({1,0});
    LabelsVerticalMask->setPositionY(-m_labelMaskOffset / 2);
    m_LabelsContainer->addChild(LabelsVerticalMask);

    m_LabelsVerticalContainer = CCNode::create();
    m_LabelsVerticalContainer->setID("labels-vertical-container");
    m_LabelsVerticalContainer->setPositionX(LabelsVerticalMask->getContentWidth());
    LabelsVerticalMask->addChild(m_LabelsVerticalContainer);

    CCPoint LabelsHorizontalMaskShape[4] = {
        ccp(0, 0),
        ccp(scaling.width + m_labelMaskOffset, 0),
        ccp(scaling.width + m_labelMaskOffset, 40),
        ccp(0, 40)
    };

    auto LabelsHorizontalMask = CCClippingNode::create();
    auto horizontalStencil = CCDrawNode::create();
    horizontalStencil->drawPolygon(LabelsHorizontalMaskShape, 4, ccc4FFromccc4B({0, 0, 0, 0}), .01f, ccc4FFromccc4B({0,0,0,0}));
    LabelsHorizontalMask->setStencil(horizontalStencil);
    LabelsHorizontalMask->setContentSize({scaling.width + m_labelMaskOffset, 40});
    LabelsHorizontalMask->setAnchorPoint({0,1});
    LabelsHorizontalMask->setPositionX(-m_labelMaskOffset / 2);
    m_LabelsContainer->addChild(LabelsHorizontalMask);

    m_LabelsHorizontalContainer = CCNode::create();
    m_LabelsHorizontalContainer->setID("labels-horizontal-container");
    m_LabelsHorizontalContainer->setPositionY(LabelsHorizontalMask->getContentHeight());
    LabelsHorizontalMask->addChild(m_LabelsHorizontalContainer);

    GraphHolder::refreshBackground(m_fillColor, m_outlineThickness, m_outlineColor);

    scheduleUpdate();

    return true;
}

void GraphHolder::setBGFillColor(ccColor4B color) {
    GraphHolder::refreshBackground(std::move(color), std::move(m_outlineThickness), std::move(m_outlineColor));
}

void GraphHolder::setOutlineThickness(float thickness) {
    GraphHolder::refreshBackground(std::move(m_fillColor), std::move(thickness), std::move(m_outlineColor));
}

void GraphHolder::setOutlineColor(ccColor4B color) {
    GraphHolder::refreshBackground(std::move(m_fillColor), std::move(m_outlineThickness), std::move(color));
}

void GraphHolder::refreshBackground(ccColor4B newFillColor, float newOutlineThickness, ccColor4B newOutlineColor) {
    if (m_fillNode) m_fillNode->clear();
    else m_fillNode = CCDrawNode::create();

    if (m_outlineNode) m_outlineNode->clear();
    else m_outlineNode = CCDrawNode::create();

    m_fillColor = std::move(newFillColor);
    m_outlineThickness = std::move(newOutlineThickness);
    m_outlineColor = std::move(newOutlineColor);

    auto size = m_scrollLayer->m_content->getContentSize() * m_scrollLayer->getCurrentZoom();
    
    CCPoint MaskShape[4] = {
        ccp(0, 0),
        ccp(size.width, 0),
        ccp(size.width, size.height),
        ccp(0, size.height)
    };

    if (!m_mask) {
        m_mask = CCClippingNode::create();
        m_mask->setID("mask");
        this->addChild(m_mask);

        m_graphContainer = CCNode::create();
        m_mask->addChild(m_graphContainer);

        m_coverageShadeNode = CCDrawNode::create();
        m_coverageShadeNode->setID("coverage-shade");
        m_coverageShadeNode->setZOrder(-2);
        m_graphContainer->addChild(m_coverageShadeNode);

        m_coverageLineNode = CCDrawNode::create();
        m_coverageLineNode->setID("coverage-boundaries");
        m_coverageLineNode->setZOrder(-1);
        m_coverageLineNode->m_bUseArea = false;
        m_graphContainer->addChild(m_coverageLineNode);
    }

    m_fillNode->drawPolygon(MaskShape, 4, ccc4FFromccc4B(m_fillColor), m_outlineThickness, ccc4FFromccc4B(m_outlineColor));
    m_fillNode->setID("fill");
    m_fillNode->setZOrder(-1);
    m_mask->setStencil(m_fillNode);
    m_mask->addChild(m_fillNode);

    m_outlineNode->drawPolygon(MaskShape, 4, ccc4FFromccc4B({0, 0, 0, 0}), m_outlineThickness, ccc4FFromccc4B(m_outlineColor));
    m_outlineNode->setID("outline");
    m_outlineNode->setZOrder(2);
    m_mask->addChild(m_outlineNode);
}


void GraphHolder::refreshGrid() {

    if (m_boldGridNode) m_boldGridNode->clear();
    else {
        m_boldGridNode = CCDrawNode::create();
        m_boldGridNode->setZOrder(-1);
        m_boldGridNode->m_bUseArea = false;
        m_mask->addChild(m_boldGridNode);
    }

    auto lowerCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0, 0}));
    auto heigherCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace(m_scrollLayer->m_content->getContentSize()));

    CCSize devidedSize = (heigherCorner - lowerCorner - m_inGraphOffset) / 100;

    for (int i = 0; i <= 100; i++) {
        if (floor(static_cast<float>(i) / m_labelEvery) == static_cast<float>(i) / m_labelEvery) {
            m_boldGridNode->drawSegment(ccp(0, i * devidedSize.height), ccp(100 * devidedSize.width, i * devidedSize.height), 0.2f, ccc4FFromccc4B(m_boldGridColor));
            m_boldGridNode->drawSegment(ccp(i * devidedSize.width, 0), ccp(i * devidedSize.width, 100 * devidedSize.height), 0.2f, ccc4FFromccc4B(m_boldGridColor));
        }
    }
}

void GraphHolder::updateLabels() {
    auto tempT = geode::Label::create("100", "chatFont.fnt");
    tempT->setScale(0.4f);
    float XForPr = tempT->getScaledContentSize().width;

    m_smallLines.clear();
    m_boldLines.clear();
    m_labels.clear();

    m_LabelsHorizontalContainer->removeAllChildrenWithCleanup(true);
    m_LabelsVerticalContainer->removeAllChildrenWithCleanup(true);

    auto HlowerCorner = m_LabelsHorizontalContainer->getParent()->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0, 0}));
    auto HheigherCorner = m_LabelsHorizontalContainer->getParent()->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace(m_scrollLayer->m_content->getContentSize()));

    CCSize devidedHSize = (HheigherCorner - HlowerCorner - m_inGraphOffset) / 100;

    auto VlowerCorner = m_LabelsVerticalContainer->getParent()->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0, 0}));
    auto VheigherCorner = m_LabelsVerticalContainer->getParent()->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace(m_scrollLayer->m_content->getContentSize()));

    CCSize devidedVSize = (VheigherCorner - VlowerCorner - m_inGraphOffset) / 100;

    for (int i = 0; i <= 100; i++)
    {
        auto labelPr = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPr->setPositionX(i * devidedHSize.width);
        labelPr->setRotation(90);
        labelPr->setColor({m_smallLineColor.r, m_smallLineColor.g, m_smallLineColor.b});
        labelPr->setOpacity(m_smallLineColor.a);
        m_LabelsHorizontalContainer->addChild(labelPr);

        float closestBigLabel = static_cast<float>(i) / m_labelEvery;
        
        if (floor(closestBigLabel) == closestBigLabel) {
            labelPr->setScaleX(0.2f);

            labelPr->setColor({m_boldLineColor.r, m_boldLineColor.g, m_boldLineColor.b});
            labelPr->setOpacity(m_boldLineColor.a);

            auto labelPrText = geode::Label::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPrText->setPositionX(i * devidedHSize.width);
            labelPrText->setScale(0.4f);
            labelPrText->setPositionY(-labelPr->getScaledContentSize().width - labelPrText->getScaledContentSize().height);
            labelPrText->setColor({m_labelColor.r, m_labelColor.g, m_labelColor.b});
            labelPrText->setOpacity(m_labelColor.a);
            m_LabelsHorizontalContainer->addChild(labelPrText);

            m_boldLines.insert(labelPr);
            m_labels.insert(labelPrText);
        }
        else {
            labelPr->setScaleX(0.1f);
            labelPr->setScaleY(0.8f);

            m_smallLines.insert(labelPr);
        }
        labelPr->setPositionY(-labelPr->getScaledContentSize().width);

        //

        auto labelPS = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPS->setPositionY(i * devidedVSize.height);
        labelPS->setColor({m_smallLineColor.r, m_smallLineColor.g, m_smallLineColor.b});
        labelPS->setOpacity(m_smallLineColor.a);
        m_LabelsVerticalContainer->addChild(labelPS);

        if (floor(closestBigLabel) == closestBigLabel) {
            labelPS->setScaleX(0.2f);

            labelPS->setColor({m_boldLineColor.r, m_boldLineColor.g, m_boldLineColor.b});
            labelPS->setOpacity(m_boldLineColor.a);

            auto labelPSText = geode::Label::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPSText->setPositionY(i * devidedVSize.height);
            labelPSText->setScale(0.4f);
            labelPSText->setPositionX(-labelPS->getScaledContentSize().width - XForPr);
            labelPSText->setColor({m_labelColor.r, m_labelColor.g, m_labelColor.b});
            labelPSText->setOpacity(m_labelColor.a);
            m_LabelsVerticalContainer->addChild(labelPSText);

            m_boldLines.insert(labelPS);
            m_labels.insert(labelPSText);
        }
        else {
            labelPS->setScaleX(0.1f);
            labelPS->setScaleY(0.8f);

            m_smallLines.insert(labelPS);
        }

        labelPS->setPositionX(-labelPS->getScaledContentSize().width);
    }
}

void GraphHolder::refreshCoverageIndicators(const CCSize& graphSize) {
    std::optional<std::pair<float, float>> newBounds;

    for (const auto& [_, graph] : m_allGraphs) {
        auto range = graph->getCoverageRange();
        if (!range.has_value()) continue;

        if (!newBounds.has_value()) {
            newBounds = range;
        }
        else {
            newBounds->first = std::min(newBounds->first, range->first);
            newBounds->second = std::max(newBounds->second, range->second);
        }
    }

    if (newBounds == m_coverageBounds &&
        graphSize.width == m_coverageGraphSize.width &&
        graphSize.height == m_coverageGraphSize.height) return;

    m_coverageBounds = newBounds;
    m_coverageGraphSize = graphSize;
    m_coverageShadeNode->clear();
    m_coverageLineNode->clear();

    if (!newBounds.has_value() ||
        (newBounds->first <= 0 && newBounds->second >= 100)) return;

    float startX = graphSize.width * newBounds->first / 100;
    float endX = graphSize.width * newBounds->second / 100;
    ccColor4F shadeColor = ccc4FFromccc4B({0, 0, 0, 65});
    ccColor4F clearColor = ccc4f(0, 0, 0, 0);

    if (startX > 0) {
        CCPoint leftShade[4] = {
            ccp(0, 0),
            ccp(startX, 0),
            ccp(startX, graphSize.height),
            ccp(0, graphSize.height)
        };
        m_coverageShadeNode->drawPolygon(leftShade, 4, shadeColor, 0, clearColor);
    }

    if (endX < graphSize.width) {
        CCPoint rightShade[4] = {
            ccp(endX, 0),
            ccp(graphSize.width, 0),
            ccp(graphSize.width, graphSize.height),
            ccp(endX, graphSize.height)
        };
        m_coverageShadeNode->drawPolygon(rightShade, 4, shadeColor, 0, clearColor);
    }

    ccColor4F lineColor = ccc4FFromccc4B({220, 220, 220, 190});
    auto drawDottedLine = [&](float x) {
        for (float y = 0; y < graphSize.height; y += 6) {
            m_coverageLineNode->drawSegment(
                ccp(x, y),
                ccp(x, std::min(y + 3, graphSize.height)),
                .75f,
                lineColor
            );
        }
    };

    if (startX > 0 && startX < graphSize.width) drawDottedLine(startX);
    if (endX > 0 && endX < graphSize.width) drawDottedLine(endX);
}

void GraphHolder::setSmallLinesColor(ccColor4B newColor) {
    m_smallLineColor = std::move(newColor);
    for (auto line : m_smallLines) {
        line->setColor({m_smallLineColor.r, m_smallLineColor.g, m_smallLineColor.b});
        line->setOpacity(m_smallLineColor.a);
    }
}

void GraphHolder::setBoldLinesColor(ccColor4B newColor) {
    m_boldLineColor = std::move(newColor);
    for (auto line : m_boldLines) {
        line->setColor({m_boldLineColor.r, m_boldLineColor.g, m_boldLineColor.b});
        line->setOpacity(m_boldLineColor.a);
    }
}

void GraphHolder::setLabelsColor(ccColor4B newColor) {
    m_labelColor = std::move(newColor);
    for (auto label : m_labels) {
        label->setColor({m_labelColor.r, m_labelColor.g, m_labelColor.b});
        label->setOpacity(m_labelColor.a);
    }
}

void GraphHolder::update(float dt) {
    if (m_prevZoom != m_scrollLayer->getCurrentZoom()) {
        m_prevZoom = m_scrollLayer->getCurrentZoom();

        m_labelEvery = m_labelEveryBase / m_scrollLayer->getCurrentZoom();

        auto lowerCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0, 0}));
        auto heigherCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace(m_scrollLayer->m_content->getContentSize()));

        CCSize size = (heigherCorner - lowerCorner - m_inGraphOffset);

        for (const auto& [_, graph] : m_allGraphs)
        {
            graph->setScaling(size);
        }

        GraphHolder::refreshGrid();
        GraphHolder::updateLabels();
    }

    auto posInSpaceH = m_LabelsHorizontalContainer->getParent()->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0,0}));
    auto posInSpaceV = m_LabelsVerticalContainer->getParent()->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0,0}));

    m_LabelsHorizontalContainer->setPositionX(posInSpaceH.x + m_inGraphOffset / 2);
    m_LabelsVerticalContainer->setPositionY(posInSpaceV.y + m_inGraphOffset / 2);
    m_boldGridNode->setPosition({posInSpaceH.x - m_labelMaskOffset / 2 + m_inGraphOffset / 2, posInSpaceV.y - m_labelMaskOffset / 2 + m_inGraphOffset / 2});
    m_graphContainer->setPosition({posInSpaceH.x - m_labelMaskOffset / 2 + m_inGraphOffset / 2, posInSpaceV.y - m_labelMaskOffset / 2 + m_inGraphOffset / 2});

    auto graphLowerCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0, 0}));
    auto graphHigherCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace(m_scrollLayer->m_content->getContentSize()));
    GraphHolder::refreshCoverageIndicators(graphHigherCorner - graphLowerCorner - m_inGraphOffset);
}

void GraphHolder::addGraph(const DTGraphInfo& graph) {
    if (m_allGraphs.contains(graph.m_name)) return;

    auto lowerCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace({0, 0}));
    auto heigherCorner = this->convertToNodeSpace(m_scrollLayer->m_content->convertToWorldSpace(m_scrollLayer->m_content->getContentSize()));

    CCSize size = (heigherCorner - lowerCorner - m_inGraphOffset);

    auto graphNode = DTGraphNode::create();
    graphNode->setInfo(graph);
    graphNode->m_delegate = m_delegate;
    graphNode->setScaling(size);
    m_graphContainer->addChild(graphNode);
    m_allGraphs.insert({graph.m_name, graphNode});
}

DTGraphNode* GraphHolder::getGraphNode(const std::string& graphName) {
    if (!m_allGraphs.contains(graphName)) return nullptr;

    return m_allGraphs[graphName];
}

void GraphHolder::removeGraph(const std::string& graphName) {
    if (!m_allGraphs.contains(graphName)) return;

    m_allGraphs[graphName]->removeMeAndCleanup();
    m_allGraphs.erase(graphName);
}

void GraphHolder::sendKeyStuff(bool up, enumKeyCodes key) {
    if (up)
        m_scrollLayer->keyUp(key, 0);
    else
        m_scrollLayer->keyDown(key, 0);
}

std::vector<DTGraphNode*> GraphHolder::getAllGraphNodes() const {
    std::vector<DTGraphNode*> toReturn{};
    for (const auto& [_, graph] : m_allGraphs)
        toReturn.push_back(graph);

    return toReturn;
}

void GraphHolder::changeGraphName(const std::string& oldName, const std::string& newName) {
    if (!m_allGraphs.contains(oldName) || m_allGraphs.contains(newName)) return;

    auto saveTemp = m_allGraphs[oldName];
    m_allGraphs.erase(oldName);
    m_allGraphs.insert({newName, saveTemp});
}

void GraphHolder::setToAllGraphs(Function<void(DTGraphNode*)> graphSetFunction) {
    for (const auto& [name, graph] : m_allGraphs) {
        graphSetFunction(graph);
    }
}

void GraphHolder::sendUpdateToGraphOfType(DTGraphCoverage const& coverage) {
    for (const auto& [name, graph] : m_allGraphs)
    {
        if (graph->getInfo().has_value() && graph->getInfo().value().m_coverage == coverage) {
            graph->updateDeaths();
        }
    }
}