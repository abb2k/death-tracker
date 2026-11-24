#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>

using namespace geode::prelude;

class SectionCell : public CCMenu {
    public:
        static SectionCell* create(float width, const Section& section);

        Function<void()> m_onChanged;
        Function<void()> m_onDeleted;

        Section getSection() const { 
            return m_section;
        }

        void fade(bool in, float time);

    private:
        bool init(float width, const Section& section);

        Section m_section;

        TextInput* m_nameInput;
        TextInput* m_endPercentInput;
        TextInput* m_startPercentInput;
        CCScale9Sprite* m_bg;

        void checkValidity();

        void onDelete(CCObject* sender);
};