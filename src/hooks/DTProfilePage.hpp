#pragma once

#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

class $modify(DTProfilePage, ProfilePage) {
public:
    struct Fields{
        std::unordered_map<CCObject*, std::pair<std::string, std::string>> m_allBadges;
    };

    void loadPageFromUserInfo(GJUserScore* a2);
private:
    void createBadge(std::string_view ID, std::string_view name, std::string_view description, const int& orderPrio);

    void onBadgeClicked(CCObject* sender);

    static void checkUserDonorBadge(int userAccID, Function<void(bool)> callback);
    
    //static void isUserInBadge(std::string_view badgeID, const int& accountID, const Badge& badge);
};