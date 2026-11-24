#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/general/SimpleToggler.hpp>

using namespace geode::prelude;

class SessionCell : public CCMenu {
    public:
        static SessionCell* create(float width, Session const& session);

        Session getSession() const { 
            return m_session;
        }

        bool isSelected() const { return m_selected; };
        void setSelected(bool b);

    private:
        bool init(float width, Session const& session);

        Session m_session;

        bool m_selected = true;
        SimpleToggler* m_toggler;

};