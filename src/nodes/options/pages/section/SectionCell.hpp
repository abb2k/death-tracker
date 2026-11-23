#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class SectionCell : public CCMenu {
    public:
        static SectionCell* create(float width, const Section& section);

        geode::Function<void()> onChanged;
        geode::Function<void()> onDeleted;

        Section getSection() const { 
            return section; 
        }

        void fade(bool in, float time);

    private:
        bool init(float width, const Section& section);

        Section section;

        TextInput* nameInput;
        TextInput* endPercentInput;
        TextInput* startPercentInput;
        CCScale9Sprite* bg;

        void checkValidity();

        void onDelete(CCObject* sender);
};