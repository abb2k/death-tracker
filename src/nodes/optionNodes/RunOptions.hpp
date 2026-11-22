#pragma once

#include <Geode/Geode.hpp>
#include <nodes/optionNodes/OptionsNode.hpp>
#include <nodes/SimpleToggler.hpp>
#include <nodes/cells/PercentCell.hpp>

using namespace geode::prelude;


class RunOptions : public OptionsNode, public SliderDelegate {
    public:
        static RunOptions* create(const CCSize& size);

    private:
        virtual bool setup() override;

        void onAddNewRun(CCObject*);
        bool addNewRun(int percent);

        TextInput* runAdditionInput;
        TextInput* RealEndPerInput;
        TextInput* HidUpToInput;

        ScrollLayer* runsScrollLayer;

        void PercentCellClicked(PercentCell* cell);
        void PercentMaxHideValChanged(PercentCell* cell);

        void createRunCell(int percent, int maxToHide);

        void onStartPoses(CCObject*);
        void onSharedHideScroll(CCObject*);

        Slider* sharedHideScroll;
        void sliderEnded(Slider* slider) override;
        TextInput* sharedRunsToShowInput;

        Scrollbar* runsScrollBar;

    public:
        virtual void onOpened() override;
        virtual void onClosed() override;
};