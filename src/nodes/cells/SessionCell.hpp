#pragma once

#include <Geode/Geode.hpp>
#include <types/DTTypes.hpp>
#include <nodes/SimpleToggler.hpp>

using namespace geode::prelude;

class SessionCell : public CCMenu {
    public:
        static SessionCell* create(float width, Session const& session);

        Session getSession() const { 
            return session; 
        }

        bool isSelected() const { return selected; };
        void setSelected(bool b);

    private:
        bool init(float width, Session const& session);

        Session session;

        bool selected = true;
        SimpleToggler* toggler;

};