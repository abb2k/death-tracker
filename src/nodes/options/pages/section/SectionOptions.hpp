#pragma once

#include <Geode/Geode.hpp>
#include <nodes/options/OptionsNode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;


class SectionOptions : public OptionsNode {
    public:
        static SectionOptions* create(const CCSize& size);
    private:
        virtual bool setup() override;

        void saveSections();
        ScrollLayer* m_mainScroll;
        CCScale9Sprite* m_scrollBG;

        void onStartPoses(CCObject*);

        bool doesSectionExist(const Section& section);

        void onAddSection(CCObject* sender);
        void addSection(const Section& section);
    public:
        virtual void onOpened() override;
        virtual void onClosed() override;
};