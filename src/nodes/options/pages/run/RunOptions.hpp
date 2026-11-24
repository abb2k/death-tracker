#pragma once

#include <nodes/options/OptionsNode.hpp>

using namespace geode::prelude;

class PercentCell;

class RunOptions : public OptionsNode, public SliderDelegate {
public:
    static RunOptions* create(CCSize size);

    void onOpened() override;
    void onClosed() override;

private:
    bool setup() override;

    void onAddNewRun(CCObject*);
    bool addNewRun(int percent);

    TextInput* m_runAdditionInput;
    TextInput* m_RealEndPerInput;
    TextInput* m_HidUpToInput;

    ScrollLayer* m_runsScrollLayer;

    void PercentCellClicked(PercentCell* cell);
    void PercentMaxHideValChanged(PercentCell* cell);

    void createRunCell(int percent, int maxToHide);

    void onStartPoses(CCObject*);
    void onSharedHideScroll(CCObject*);

    Slider* m_sharedHideScroll;
    void sliderEnded(Slider* slider) override;
    TextInput* m_sharedRunsToShowInput;

    Scrollbar* m_runsScrollBar;
};