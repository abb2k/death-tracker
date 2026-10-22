#include "GraphHolder.hpp"

GraphHolder* GraphHolder::create(const CCSize& scaling) {
    auto ret = new GraphHolder();
    if (ret && ret->init(scaling)) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool GraphHolder::init(const CCSize& scaling) {
    if (!CCNode::init()) return false;
    
    this->setContentSize(scaling);
    this->setAnchorPoint({.5f, .5f});

    labelEvery = labelEveryBase;

    scrollLayer = AdvancedScrollLayer::create(scaling, scaling + ccp(.01f,.01f));
    scrollLayer->setHorizontalScrollbarPosition(true);
    scrollLayer->minZoom = 5;
    this->addChild(scrollLayer);

    LabelsContainer = CCNode::create();
    LabelsContainer->setID("labels-container");
    this->addChild(LabelsContainer);

    CCPoint LabelsVerticalMaskShape[4] = {
        ccp(0, 0),
        ccp(40, 0),
        ccp(40, scaling.height + labelMaskOffset),
        ccp(0, scaling.height + labelMaskOffset)
    };

    auto LabelsVerticalMask = CCClippingNode::create();
    auto verticalStencil = CCDrawNode::create();
    verticalStencil->drawPolygon(LabelsVerticalMaskShape, 4, ccc4FFromccc4B({0, 0, 0, 0}), .01f, ccc4FFromccc4B({0,0,0,0}));
    LabelsVerticalMask->setStencil(verticalStencil);
    LabelsVerticalMask->setContentSize({40, scaling.height + labelMaskOffset});
    LabelsVerticalMask->setAnchorPoint({1,0});
    LabelsVerticalMask->setPositionY(-labelMaskOffset / 2);
    LabelsContainer->addChild(LabelsVerticalMask);

    LabelsVerticalContainer = CCNode::create();
    LabelsVerticalContainer->setID("labels-vertical-container");
    LabelsVerticalContainer->setPositionX(LabelsVerticalMask->getContentWidth());
    LabelsVerticalMask->addChild(LabelsVerticalContainer);

    CCPoint LabelsHorizontalMaskShape[4] = {
        ccp(0, 0),
        ccp(scaling.width + labelMaskOffset, 0),
        ccp(scaling.width + labelMaskOffset, 40),
        ccp(0, 40)
    };

    auto LabelsHorizontalMask = CCClippingNode::create();
    auto horizontalStencil = CCDrawNode::create();
    horizontalStencil->drawPolygon(LabelsHorizontalMaskShape, 4, ccc4FFromccc4B({0, 0, 0, 0}), .01f, ccc4FFromccc4B({0,0,0,0}));
    LabelsHorizontalMask->setStencil(horizontalStencil);
    LabelsHorizontalMask->setContentSize({scaling.width + labelMaskOffset, 40});
    LabelsHorizontalMask->setAnchorPoint({0,1});
    LabelsHorizontalMask->setPositionX(-labelMaskOffset / 2);
    LabelsContainer->addChild(LabelsHorizontalMask);

    LabelsHorizontalContainer = CCNode::create();
    LabelsHorizontalContainer->setID("labels-horizontal-container");
    LabelsHorizontalContainer->setPositionY(LabelsHorizontalMask->getContentHeight());
    LabelsHorizontalMask->addChild(LabelsHorizontalContainer);

    GraphHolder::refreshBackground(fillColor, outlineThickness, outlineColor);

    scheduleUpdate();

    return true;
}

void GraphHolder::setBGFillColor(const ccColor4B& color) {
    GraphHolder::refreshBackground(color, outlineThickness, outlineColor);
}

void GraphHolder::setOutlineThickness(float thickness) {
    GraphHolder::refreshBackground(fillColor, thickness, outlineColor);
}

void GraphHolder::setOutlineColor(const ccColor4B& color) {
    GraphHolder::refreshBackground(fillColor, outlineThickness, color);
}

void GraphHolder::refreshBackground(const ccColor4B& newFillColor, float newOutlineThickness, const ccColor4B& newOutlineColor) {
    if (fillNode) fillNode->clear();
    else fillNode = CCDrawNode::create();

    if (outlineNode) outlineNode->clear();
    else outlineNode = CCDrawNode::create();

    fillColor = newFillColor;
    outlineThickness = newOutlineThickness;
    outlineColor = newOutlineColor;

    auto size = scrollLayer->content->getContentSize() * scrollLayer->getCurrentZoom();
    
    CCPoint MaskShape[4] = {
        ccp(0, 0),
        ccp(size.width, 0),
        ccp(size.width, size.height),
        ccp(0, size.height)
    };

    if (!mask) {
        mask = CCClippingNode::create();
        mask->setID("mask");
        this->addChild(mask);

        graphContainer = CCNode::create();
        mask->addChild(graphContainer);
    }

    fillNode->drawPolygon(MaskShape, 4, ccc4FFromccc4B(newFillColor), newOutlineThickness, ccc4FFromccc4B(newOutlineColor));
    fillNode->setID("fill");
    fillNode->setZOrder(-1);
    mask->setStencil(fillNode);
    mask->addChild(fillNode);

    outlineNode->drawPolygon(MaskShape, 4, ccc4FFromccc4B({0, 0, 0, 0}), newOutlineThickness, ccc4FFromccc4B(newOutlineColor));
    outlineNode->setID("outline");
    outlineNode->setZOrder(2);
    mask->addChild(outlineNode);
}


void GraphHolder::refreshGrid() {

    if (boldGridNode) boldGridNode->clear();
    else {
        boldGridNode = CCDrawNode::create();
        boldGridNode->setZOrder(-1);
        boldGridNode->m_bUseArea = false;
        mask->addChild(boldGridNode);
    }

    auto lowerCorner = this->convertToNodeSpace(scrollLayer->content->convertToWorldSpace({0, 0}));
    auto heigherCorner = this->convertToNodeSpace(scrollLayer->content->convertToWorldSpace(scrollLayer->content->getContentSize()));

    CCSize devidedSize = (heigherCorner - lowerCorner - inGraphOffset) / 100;

    for (int i = 0; i <= 100; i++) {
        if (floor(static_cast<float>(i) / labelEvery) == static_cast<float>(i) / labelEvery) {
            boldGridNode->drawSegment(ccp(0, i * devidedSize.height), ccp(100 * devidedSize.width, i * devidedSize.height), 0.2f, ccc4FFromccc4B(boldGridColor));
            boldGridNode->drawSegment(ccp(i * devidedSize.width, 0), ccp(i * devidedSize.width, 100 * devidedSize.height), 0.2f, ccc4FFromccc4B(boldGridColor));
        }
    }
}

void GraphHolder::updateLabels() {
    auto tempT = CCLabelBMFont::create("100", "chatFont.fnt");
    tempT->setScale(0.4f);
    float XForPr = tempT->getScaledContentSize().width;

    smallLines.clear();
    boldLines.clear();
    labels.clear();

    LabelsHorizontalContainer->removeAllChildrenWithCleanup(true);
    LabelsVerticalContainer->removeAllChildrenWithCleanup(true);

    auto HlowerCorner = LabelsHorizontalContainer->getParent()->convertToNodeSpace(scrollLayer->content->convertToWorldSpace({0, 0}));
    auto HheigherCorner = LabelsHorizontalContainer->getParent()->convertToNodeSpace(scrollLayer->content->convertToWorldSpace(scrollLayer->content->getContentSize()));

    CCSize devidedHSize = (HheigherCorner - HlowerCorner - inGraphOffset) / 100;

    auto VlowerCorner = LabelsVerticalContainer->getParent()->convertToNodeSpace(scrollLayer->content->convertToWorldSpace({0, 0}));
    auto VheigherCorner = LabelsVerticalContainer->getParent()->convertToNodeSpace(scrollLayer->content->convertToWorldSpace(scrollLayer->content->getContentSize()));

    CCSize devidedVSize = (VheigherCorner - VlowerCorner - inGraphOffset) / 100;

    for (int i = 0; i <= 100; i++)
    {
        auto labelPr = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPr->setPositionX(i * devidedHSize.width);
        labelPr->setRotation(90);
        labelPr->setColor({smallLineColor.r, smallLineColor.g, smallLineColor.b});
        labelPr->setOpacity(smallLineColor.a);
        LabelsHorizontalContainer->addChild(labelPr);

        float closestBigLabel = static_cast<float>(i) / labelEvery;
        
        if (floor(closestBigLabel) == closestBigLabel) {
            labelPr->setScaleX(0.2f);

            labelPr->setColor({boldLineColor.r, boldLineColor.g, boldLineColor.b});
            labelPr->setOpacity(boldLineColor.a);

            auto labelPrText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPrText->setPositionX(i * devidedHSize.width);
            labelPrText->setScale(0.4f);
            labelPrText->setPositionY(-labelPr->getScaledContentSize().width - labelPrText->getScaledContentSize().height);
            labelPrText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPrText->setOpacity(labelColor.a);
            LabelsHorizontalContainer->addChild(labelPrText);

            boldLines.insert(labelPr);
            labels.insert(labelPrText);
        }
        else {
            labelPr->setScaleX(0.1f);
            labelPr->setScaleY(0.8f);

            smallLines.insert(labelPr);
        }
        labelPr->setPositionY(-labelPr->getScaledContentSize().width);

        //

        auto labelPS = CCSprite::createWithSpriteFrameName("gridLine01_001.png");
        labelPS->setPositionY(i * devidedVSize.height);
        labelPS->setColor({smallLineColor.r, smallLineColor.g, smallLineColor.b});
        labelPS->setOpacity(smallLineColor.a);
        LabelsVerticalContainer->addChild(labelPS);

        if (floor(closestBigLabel) == closestBigLabel) {
            labelPS->setScaleX(0.2f);

            labelPS->setColor({boldLineColor.r, boldLineColor.g, boldLineColor.b});
            labelPS->setOpacity(boldLineColor.a);

            auto labelPSText = CCLabelBMFont::create(std::to_string(i).c_str(), "chatFont.fnt");
            labelPSText->setPositionY(i * devidedVSize.height);
            labelPSText->setScale(0.4f);
            labelPSText->setPositionX(-labelPS->getScaledContentSize().width - XForPr);
            labelPSText->setColor({labelColor.r, labelColor.g, labelColor.b});
            labelPSText->setOpacity(labelColor.a);
            LabelsVerticalContainer->addChild(labelPSText);

            boldLines.insert(labelPS);
            labels.insert(labelPSText);
        }
        else {
            labelPS->setScaleX(0.1f);
            labelPS->setScaleY(0.8f);

            smallLines.insert(labelPS);
        }

        labelPS->setPositionX(-labelPS->getScaledContentSize().width);
    }
}

void GraphHolder::setSmallLinesColor(const ccColor4B& newColor) {
    smallLineColor = newColor;
    for (auto line : smallLines) {
        line->setColor({newColor.r, newColor.g, newColor.b});
        line->setOpacity(newColor.a);
    }
}

void GraphHolder::setBoldLinesColor(const ccColor4B& newColor) {
    boldLineColor = newColor;
    for (auto line : boldLines) {
        line->setColor({newColor.r, newColor.g, newColor.b});
        line->setOpacity(newColor.a);
    }
}

void GraphHolder::setLabelsColor(const ccColor4B& newColor) {
    labelColor = newColor;
    for (auto label : labels) {
        label->setColor({newColor.r, newColor.g, newColor.b});
        label->setOpacity(newColor.a);
    }
}

void GraphHolder::update(float dt) {
    if (prevZoom != scrollLayer->getCurrentZoom()) {
        prevZoom = scrollLayer->getCurrentZoom();

        labelEvery = labelEveryBase / scrollLayer->getCurrentZoom();

        auto lowerCorner = this->convertToNodeSpace(scrollLayer->content->convertToWorldSpace({0, 0}));
        auto heigherCorner = this->convertToNodeSpace(scrollLayer->content->convertToWorldSpace(scrollLayer->content->getContentSize()));

        CCSize size = (heigherCorner - lowerCorner - inGraphOffset);

        for (const auto& [_, graph] : allGraphs)
        {
            graph->setScaling(size);
        }

        GraphHolder::refreshGrid();
        GraphHolder::updateLabels();
    }

    auto posInSpaceH = LabelsHorizontalContainer->getParent()->convertToNodeSpace(scrollLayer->content->convertToWorldSpace({0,0}));
    auto posInSpaceV = LabelsVerticalContainer->getParent()->convertToNodeSpace(scrollLayer->content->convertToWorldSpace({0,0}));

    LabelsHorizontalContainer->setPositionX(posInSpaceH.x + inGraphOffset / 2);
    LabelsVerticalContainer->setPositionY(posInSpaceV.y + inGraphOffset / 2);
    boldGridNode->setPosition({posInSpaceH.x - labelMaskOffset / 2 + inGraphOffset / 2, posInSpaceV.y - labelMaskOffset / 2 + inGraphOffset / 2});
    graphContainer->setPosition({posInSpaceH.x - labelMaskOffset / 2 + inGraphOffset / 2, posInSpaceV.y - labelMaskOffset / 2 + inGraphOffset / 2});
}

void GraphHolder::addGraph(const DTGraphInfo& graph) {
    if (allGraphs.contains(graph.name)) return;

    auto lowerCorner = this->convertToNodeSpace(scrollLayer->content->convertToWorldSpace({0, 0}));
    auto heigherCorner = this->convertToNodeSpace(scrollLayer->content->convertToWorldSpace(scrollLayer->content->getContentSize()));

    CCSize size = (heigherCorner - lowerCorner - inGraphOffset);

    auto graphNode = DTGraphNode::create();
    graphNode->setInfo(graph);
    graphNode->delegate = delegate;
    graphNode->setScaling(size);
    graphContainer->addChild(graphNode);
    allGraphs.insert({graph.name, graphNode});
}

DTGraphNode* GraphHolder::getGraphNode(const std::string& graphName) {
    if (!allGraphs.contains(graphName)) return nullptr;

    return allGraphs[graphName];
}

void GraphHolder::removeGraph(const std::string& graphName) {
    if (!allGraphs.contains(graphName)) return;

    allGraphs[graphName]->removeMeAndCleanup();
    allGraphs.erase(graphName);
}

void GraphHolder::sendKeyStuff(bool up, enumKeyCodes key) {
    if (up)
        scrollLayer->keyUp(key, 0);
    else
        scrollLayer->keyDown(key, 0);
}

void GraphHolder::changeGraphName(const std::string& oldName, const std::string& newName) {
    if (!allGraphs.contains(oldName) || allGraphs.contains(newName)) return;

    auto saveTemp = allGraphs[oldName];
    allGraphs.erase(oldName);
    allGraphs.insert({newName, saveTemp});
}

void GraphHolder::setToAllGraphs(geode::Function<void(DTGraphNode*)> graphSetFunction) {
    for (const auto& [name, graph] : allGraphs) {
        graphSetFunction(graph);
    }
}

void GraphHolder::sendUpdateToGraphOfType(DTGraphCoverage coverage) {
    for (const auto& [name, graph] : allGraphs)
    {
        if (graph->getInfo().has_value() && graph->getInfo().value().coverage == coverage) {
            graph->updateDeaths();
        }
    }
}