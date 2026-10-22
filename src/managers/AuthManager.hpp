#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class OnDonerStateChangedEvent : public Event<OnDonerStateChangedEvent, bool(bool state)> {
public:
    using Event::Event;
};

class AuthManager {
    public:
        static void authForDoner(bool forceCheck = false);

        static bool isDoner();
    private:
        static void checkDoner(std::string const& token, int accountId);

        static void sendDonerChangedEvent();

        static bool m_isDoner;
        static bool m_lastSentDonberState;

        static std::optional<int> m_lastValidatedAccountId;
};