#include "DTProfilePage.hpp"

#include <alphalaneous.badgify/include/Badgify.hpp>

void DTProfilePage::loadPageFromUserInfo(GJUserScore* user) {
    ProfilePage::loadPageFromUserInfo(user);

    if (!user || !Loader::get()->isModLoaded("jouca.badgesapi") || Loader::get()->isModLoaded("alphalaneous.badgify")) return;

    checkUserDonorBadge(user->m_accountID, [this, user](bool isDonor) {
        if (!isDonor) return;

        DTProfilePage::createBadge(
            "doner-badge",
            "Death Tracker Donator",
            fmt::format("<cg>{}</c> has <cs>donated</c> to the development of\n<cc>Death Tracker</c>! :D", user->m_userName),
            -1
        );
    });
}

void DTProfilePage::checkUserDonorBadge(int userAccID, geode::Function<void(bool)> callback) {
    auto req = web::WebRequest();
    req.param("accountID", userAccID);

    async::spawn(
        req.get("https://api.abb2k.me/death-tracker/v1/get-doners"),
        [userAccID, callback = std::move(callback)](web::WebResponse response) mutable {
            auto runCallback = [callback = std::move(callback)](bool result) mutable {
                Loader::get()->queueInMainThread([callback = std::move(callback), result]() mutable {
                    callback(result);
                });
            };

            if (!response.ok() || response.json().isErr()) {
                runCallback(false);
                return;
            }

            auto json = std::move(response.json()).unwrap();

            auto donorsRes = json["data"].as<std::vector<int>>();

            if (donorsRes.isErr()) {
                runCallback(false);
                return;
            }

            auto donors = std::move(donorsRes).unwrap();

            if (std::ranges::find(donors, userAccID) == donors.end()) {
                runCallback(false);
                return;
            }

            runCallback(true);
        }
    );
}

void DTProfilePage::createBadge(const std::string& ID, const std::string& name, const std::string& description, const int& orderPrio){
    auto username_menu = typeinfo_cast<CCMenu*>(m_mainLayer->getChildByIDRecursive("username-menu"));
    auto yourBadge = CCSprite::createWithSpriteFrameName(fmt::format("{}.png"_spr, ID).c_str());
    yourBadge->setScale(.85f);
    auto bButton = CCMenuItemSpriteExtra::create(
        yourBadge,
        nullptr,
        this,
        menu_selector(DTProfilePage::onBadgeClicked)
    );
    bButton->setID(fmt::format("{}-badge", ID));
    bButton->setZOrder(-orderPrio);
    username_menu->addChild(bButton);

    m_fields->allBadges.insert(std::make_pair(bButton, std::make_pair(name, description)));
    username_menu->updateLayout();
}

void DTProfilePage::onBadgeClicked(CCObject* sender){
    if (m_fields->allBadges.contains(sender)){
        geode::createQuickPopup(m_fields->allBadges[sender].first.c_str(), m_fields->allBadges[sender].second.c_str(), "OK", nullptr, [](auto _, auto __){});
    }   
}


// void DTProfilePage::isUserInBadge(const std::string& badgeID, const int& accountID, const Badge& badge) {
//     bool found = false;
//     data::getPlayersData().listen([&found, badgeID, accountID, badge] (Result<std::vector<PlayerData>>* playersData) {
//         auto playerData = playersData->unwrapOrDefault();
//         if (playerData.empty()) {
//             return;
//         }

//         PlayerData myPlayer {};
//         bool didFindPlayer = false;

//         for (const auto& player : playerData) {
//             if (player.accountID == accountID) {
//                 myPlayer = player;
//                 didFindPlayer = true;
//                 break;
//             }
//         }

//         if (!didFindPlayer) {
//             return;
//         }

//         if (std::ranges::find(myPlayer.staffIDs, badgeID) == myPlayer.staffIDs.end() && std::ranges::find(myPlayer.achievementIDs, badgeID) == myPlayer.achievementIDs.end()) {
//             return;
//         }

//         BadgesAPI::showBadge(badge);
//     });
// }

$on_mod(Loaded) {
    alpha::badgify::registerBadge(
        "doner-badge"_spr,
        "Death Tracker Donator",
        "<cg>This user</c> has <cs>donated</c> to the development of\n<cc>Death Tracker</c>! :D",
        [] (const alpha::badgify::Badge& badge) {
            CCSprite* theBadgeSprite = CCSprite::createWithSpriteFrameName("doner-badge.png"_spr);

            DTProfilePage::checkUserDonorBadge(badge.user.data()->m_accountID, [badge, spr = geode::Ref(theBadgeSprite)](bool isDonor) mutable {
                if (isDonor) {
                    alpha::badgify::showBadge(badge, spr);
                }
            });
        }
    );
};