#pragma once

#include <types/DTTypes.hpp>

using namespace geode::prelude;

class SimpleToggler;

class SessionCell : public CCMenu {
public:
    static SessionCell* create(float width, Session session);

    Session getSession() const { return m_session; }

    bool isSelected() const { return m_selected; };
    void setSelected(bool b);

private:
    bool init(float width, Session session);

    Session m_session;

    bool m_selected = true;
    SimpleToggler* m_toggler;
};