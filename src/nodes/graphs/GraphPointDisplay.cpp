#include <nodes/graphs/GraphPointDisplay.hpp>
#include <managers/StatsManager.hpp>

GraphPointDisplay* GraphPointDisplay::create() {
    auto ret = new GraphPointDisplay();
    if (ret && ret->init()) {
        ret->autorelease();
    } else {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool GraphPointDisplay::init() {

    this->setContentSize({100, 35});

    auto bg = CCScale9Sprite::create("square01_001.png");
    bg->setAnchorPoint({0, 0});
    bg->setScale(.4f);
    bg->setContentSize((this->getContentSize() - ccp(0, 2.5f)) / bg->getScale());
    this->addChild(bg);

    percentLabel = CCLabelBMFont::create("", "bigFont.fnt");
    percentLabel->setScale(.4f);
    percentLabel->setPosition({this->getContentWidth() / 2, this->getContentHeight() - 6.5f});
    percentLabel->setAnchorPoint({.5f, 1});
    this->addChild(percentLabel);
    
    rateLabel = CCLabelBMFont::create("", "bigFont.fnt");
    rateLabel->setScale(.25f);
    rateLabel->setPosition({this->getContentWidth() / 2, 6});
    rateLabel->setAnchorPoint({.5f, 0});
    this->addChild(rateLabel);

    return true;
}

void GraphPointDisplay::setContent(std::string run, float ratePercent, DTGraphType type, ccColor3B graphInner, ccColor3B graphOuter) {
    auto setFittedText = [this](CCLabelBMFont* label, const std::string& text, float baseScale) {
        label->setString(text.c_str());
        label->setScale(baseScale);
        const float maxWidth = this->getContentWidth() - 8;
        if (label->getScaledContentWidth() > maxWidth) {
            label->setScale(baseScale * maxWidth / label->getScaledContentWidth());
        }
    };

    std::string runSprFormatted;
    auto runInfo = StatsManager::splitRunKey(run);
    if (runInfo.isErr())
        runSprFormatted = run;
    else if (runInfo.unwrap().start == std::nullopt)
        runSprFormatted = fmt::format("{}%", runInfo.unwrap().end);
    else
        runSprFormatted = fmt::format("{}%-{}%", runInfo.unwrap().start.value(), runInfo.unwrap().end);

    setFittedText(percentLabel, runSprFormatted, .4f);
    percentLabel->setColor(graphOuter);

    std::string rateType;

    switch (type)
    {
    case DTGraphType::Passrate :
        rateType = "Passrate";
        break;
    case DTGraphType::Reachrate :
        rateType = "Reachrate";
        break;
    
    default:
        rateType = "NA";
        break;
    }

    setFittedText(rateLabel, fmt::format("{}: {:.4f}%", rateType, ratePercent), .25f);
    rateLabel->setColor(graphInner);
}