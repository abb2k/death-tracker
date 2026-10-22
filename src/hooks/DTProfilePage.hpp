#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include <Geode/modify/ProfilePage.hpp>

class $modify(DTProfilePage, ProfilePage) {
    struct Fields{
        std::unordered_map<CCObject*, std::pair<std::string, std::string>> allBadges;
    };        

    public:
        static void checkUserDonorBadge(int userAccID, geode::Function<void(bool)> callback);
    
        void loadPageFromUserInfo(GJUserScore* a2);

        void createBadge(const std::string& ID, const std::string& name, const std::string& description, const int& orderPrio);

        void onBadgeClicked(CCObject* sender);

        //static void isUserInBadge(const std::string& badgeID, const int& accountID, const Badge& badge);
};