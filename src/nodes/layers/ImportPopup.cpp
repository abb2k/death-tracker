#include "ImportPopup.hpp"
#include <nodes/layers/DTLayer.hpp>
#include <nodes/layers/ImportCompletePopup.hpp>

ImportPopup* ImportPopup::create() {
    auto ret = new ImportPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ImportPopup::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(250, 150, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Import");

    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    menu->setLayout(SimpleAxisLayout::create(Axis::Row)
        ->setMainAxisAlignment(MainAxisAlignment::Between)
        ->setCrossAxisScaling(AxisScaling::Fit)
    );
    menu->setContentWidth(175);
    menu->setPosition(m_size / 2);
    menu->setZOrder(1);
    m_mainLayer->addChild(menu);

    auto fromDTBtnSpr = ButtonSprite::create(
        // @geode-ignore(unknown-resource)
        CCSprite::createWithSpriteFrameName("geode.loader/file.png"),
        50,
        50,
        50,
        1,
        false,
        "GJ_button_02.png",
        false
    );
    auto fromDTBtn = CCMenuItemSpriteExtra::create(
        fromDTBtnSpr,
        this,
        menu_selector(ImportPopup::onFromDT)
    );
    menu->addChild(fromDTBtn);
    
    auto fromTextBtnSpr = ButtonSprite::create(
        // @geode-ignore(unknown-resource)
        CCSprite::createWithSpriteFrameName("geode.loader/changelog.png"),
        50,
        50,
        50,
        1,
        false,
        "GJ_button_02.png",
        false
    );
    fromTextBtnSpr->setCascadeOpacityEnabled(true);
    auto fromTextBtn = CCMenuItemSpriteExtra::create(
        fromTextBtnSpr,
        this,
        menu_selector(ImportPopup::onFromText)
    );
    fromTextBtn->setEnabled(false);
    fromTextBtn->setOpacity(120);
    menu->addChild(fromTextBtn);

    menu->updateLayout();

    auto fromDTBtnC1 = m_mainLayer->convertToNodeSpace(fromDTBtn->convertToWorldSpace({0, 0}));
    auto fromDTBtnC2 = m_mainLayer->convertToNodeSpace(fromDTBtn->convertToWorldSpace(fromDTBtn->getContentSize()));

    auto fromDTLabel = CCLabelBMFont::create("From DT File", "bigFont.fnt");
    fromDTLabel->setScale(.5f);
    fromDTLabel->setPosition((fromDTBtnC1 + fromDTBtnC2) / 2 + ccp(0, fromDTBtnC2.y - fromDTBtnC1.y) / 2);
    fromDTLabel->setAnchorPoint({.5f, 0});
    m_mainLayer->addChild(fromDTLabel);

    auto fromTextBtnC1 = m_mainLayer->convertToNodeSpace(fromTextBtn->convertToWorldSpace({0, 0}));
    auto fromTextBtnC2 = m_mainLayer->convertToNodeSpace(fromTextBtn->convertToWorldSpace(fromTextBtn->getContentSize()));

    auto fromTextLabel = CCLabelBMFont::create("From Text", "bigFont.fnt");
    fromTextLabel->setScale(.5f);
    fromTextLabel->setPosition((fromTextBtnC1 + fromTextBtnC2) / 2 + ccp(0, fromTextBtnC2.y - fromTextBtnC1.y) / 2);
    fromTextLabel->setAnchorPoint({.5f, 0});
    m_mainLayer->addChild(fromTextLabel);

    auto fromTextCSLabel = CCLabelBMFont::create("(coming soon)", "bigFont.fnt");
    fromTextCSLabel->setScale(.25f);
    fromTextCSLabel->setPosition((fromTextBtnC1 + fromTextBtnC2) / 2 - ccp(0, fromTextBtnC2.y - fromTextBtnC1.y) / 2);
    fromTextCSLabel->setAnchorPoint({.5f, 1});
    m_mainLayer->addChild(fromTextCSLabel);

    return true;
}

void ImportPopup::onFromDT(CCObject*) {
    auto dtLayer = DTLayer::get();
    if (dtLayer == nullptr || dtLayer->m_MyLevelStats.isErr()) return;

    fileListener.spawn(
        "DT-import-from-dt-task",
        file::pick(file::PickMode::OpenFile, file::FilePickOptions{
            .filters = {
                file::FilePickOptions::Filter{
                    .description = "Death Tracker file",
                    .files = {
                        "*.dt"
                    }
                }
            }
        }),
        [&](file::PickResult result) {
            auto dtLayer = DTLayer::get();
            if (dtLayer == nullptr || dtLayer->m_MyLevelStats.isErr()) return;

            if (result.isErr()) {
                log::error("{}", result.unwrapErr());
                return;
            }
            auto pickOpt = result.unwrap();
            if (!pickOpt.has_value()) return;
            auto pick = pickOpt.value();

            auto tempDir = Mod::get()->getSaveDir() / "importTemp";

            (void)file::createDirectory(tempDir);

            auto res = file::Unzip::intoDir(pick, tempDir);

            if (res.isErr()) {
                log::error("{}", res.unwrapErr());
                Notification::create("Failed to unzip file!", NotificationIcon::Error)->show();
                std::filesystem::remove_all(tempDir);
                return;
            }
            
            auto lvlDataRes = StatsManager::getLevelData(tempDir);
            if (lvlDataRes.isOk()) {
                auto lvlData = std::move(lvlDataRes).unwrap();

                std::vector<Session> sessions;

                for (const auto& sessionDate : lvlData.sessionNames)
                {
                    auto session = StatsManager::getSession(tempDir, sessionDate);
                    if (session.isOk())
                        sessions.push_back(session.unwrap());
                }
                
                ImportCompletePopup::create(std::move(lvlData), std::move(sessions))->show();
            }

            std::filesystem::remove_all(tempDir);
        }
    );
}
void ImportPopup::onFromText(CCObject*) {
    auto dtLayer = DTLayer::get();
    if (dtLayer == nullptr || dtLayer->m_MyLevelStats.isErr()) return;

    fileListener.spawn(
        "DT-import-from-text-task",
        file::pick(file::PickMode::OpenFile, file::FilePickOptions{
            .filters = {
                file::FilePickOptions::Filter{
                    .description = "Text file",
                    .files = {
                        "*.txt"
                    }
                }
            }
        }),
        [&](file::PickResult result) {
            auto dtLayer = DTLayer::get();
            if (dtLayer == nullptr || dtLayer->m_MyLevelStats.isErr()) return;

            if (result.isErr()) {
                log::error("{}", result.unwrapErr());
                return;
            }
            auto pickOpt = result.unwrap();
            if (!pickOpt.has_value()) return;
            auto pick = pickOpt.value();


        }
    );
}