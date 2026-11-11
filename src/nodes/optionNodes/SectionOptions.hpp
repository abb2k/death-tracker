#pragma once

#include <Geode/Geode.hpp>
#include <nodes/optionNodes/OptionsNode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;


class SectionOptions : public OptionsNode {
    public:
        static SectionOptions* create(const CCSize& size);
    private:
        virtual bool setup();

        void saveSections();
        ScrollLayer* mainScroll;
        CCScale9Sprite* scrollBG;

        void onStartPoses(CCObject*);

        bool doesSectionExist(const Section& section);

        void onAddSection(CCObject* sender);
        void addSection(const Section& section);
    public:
        virtual void onOpened();
        virtual void onClosed();
};