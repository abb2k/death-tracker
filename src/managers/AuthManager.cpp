#include "AuthManager.hpp"

#include <argon/argon.hpp>

bool AuthManager::m_isDoner = false;
bool AuthManager::m_lastSentDonberState = false;

std::optional<int> AuthManager::m_lastValidatedAccountId = std::nullopt;

void AuthManager::authForDoner(bool forceCheck) {
    int accountId = GJAccountManager::get()->m_accountID;

    if (!m_lastValidatedAccountId.has_value())
        m_lastValidatedAccountId = accountId;
    else if (m_lastValidatedAccountId.value() != accountId)
        m_lastValidatedAccountId = accountId;
    else {
        if (accountId <= 0) {
            m_isDoner = false;
            sendDonerChangedEvent();

            return;
        }

        if (!forceCheck)
            return;
    }

    async::spawn(
        argon::startAuth(),
        [accountId](Result<std::string> result) {
            if (result.isOk()) {
                auto token = std::move(result).unwrap();
                checkDoner(token, accountId);
            } else {
                m_isDoner = false;
                sendDonerChangedEvent();
            }
        }
    );
}

void AuthManager::checkDoner(std::string const& token, int accountId) {
    auto req = web::WebRequest();
    req.bodyString(fmt::format("{{ \"accountId\": \"{}\", \"authToken\": \"{}\" }}", accountId, token));

    async::spawn(
        req.post("https://api.abb2k.me/death-tracker/v1/donor-auth-validation"),
        [](web::WebResponse result) {
            
            if (!result.json().isOk()) {
                m_isDoner = false;
                sendDonerChangedEvent();
                return;
            }

            auto json = std::move(result.json()).unwrap();

            if (!json.contains("success")) {
                m_isDoner = false;
                sendDonerChangedEvent();
                return;
            }

            auto successRes = json["success"].asBool();

            if (!successRes.isOk()) {
                m_isDoner = false;
                sendDonerChangedEvent();
                return;
            }

            m_isDoner = successRes.unwrap();
            

            sendDonerChangedEvent();
        }
    );
}

bool AuthManager::isDoner(){
    return m_isDoner;
}

void AuthManager::sendDonerChangedEvent() {
    if (m_lastSentDonberState == m_isDoner) return;

    m_lastSentDonberState = m_isDoner;
    OnDonerStateChangedEvent().send(m_isDoner);
}