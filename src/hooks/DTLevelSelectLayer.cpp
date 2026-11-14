#include <hooks/DTLevelSelectLayer.hpp>

#include <nodes/layers/DTLayer.hpp>
#include <utils/Settings.hpp>

bool DTLevelSelectLayer::init(int page) {
    if (!LevelSelectLayer::init(page)) return false;

    auto robBtnMenu = getChildByID("info-menu");

    CCPoint robBtnPos = {0, 0};

    if (robBtnMenu) {
        m_fields->robDTButton = static_cast<CCMenuItemSpriteExtra*>(robBtnMenu->getChildByID("death-tracker-button"));

        if (m_fields->robDTButton) {
            robBtnPos = m_fields->robDTButton->getPosition();
            m_fields->robDTButton->setVisible(false);
            m_fields->robDTButton->setEnabled(false);
        }

        robBtnMenu->updateLayout();
    }

    auto s = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
    auto s2 = CCSprite::createWithSpriteFrameName("miniSkull_001.png");
    s2->setPosition(s->getContentSize() / 2);
    s->addChild(s2);
    s->setScale(0.75f);
    s->setScale(0.8f);
    m_fields->dtButton = CCMenuItemSpriteExtra::create(
        s,
        nullptr,
        this,
        menu_selector(DTLevelSelectLayer::openDTLayer)
    );
    m_fields->dtButton->setID("dt-skull-button");
    m_fields->dtButton->setZOrder(1);
    m_fields->dtButton->setPosition(robBtnPos);

    robBtnMenu->addChild(m_fields->dtButton);

    m_fields->dtButton->setVisible(getLevelForCurrentPage().has_value());
    m_fields->dtButton->setEnabled(getLevelForCurrentPage().has_value());

    return true;
}

void DTLevelSelectLayer::openDTLayer(CCObject*) {

    auto levelOpt = getLevelForCurrentPage();
    if (!levelOpt.has_value()) return;

    auto dtLayer = DTLayer::create(levelOpt.value());
    dtLayer->show();
}

void DTLevelSelectLayer::onNext(CCObject* sender) {
    LevelSelectLayer::onNext(sender);

    m_fields->dtButton->setVisible(getLevelForCurrentPage().has_value());
    m_fields->dtButton->setEnabled(getLevelForCurrentPage().has_value());

    m_fields->robDTButton->setVisible(false);
}

void DTLevelSelectLayer::onPrev(CCObject* sender) {
    LevelSelectLayer::onPrev(sender);
    m_fields->dtButton->setVisible(getLevelForCurrentPage().has_value());
    m_fields->dtButton->setEnabled(getLevelForCurrentPage().has_value());

    m_fields->robDTButton->setVisible(false);
}

std::optional<GJGameLevel*> DTLevelSelectLayer::getLevelForCurrentPage() {
    int realPage;

    if (m_scrollLayer->m_page < 0){
        int N = m_scrollLayer->m_dynamicObjects->count();
        int x = std::abs(m_scrollLayer->m_page);

        int remainder = x % N;
        realPage = (N) - remainder;

        if (realPage == m_scrollLayer->m_dynamicObjects->count()) realPage = 0;
    }
    else{
        realPage = m_scrollLayer->m_page % m_scrollLayer->m_dynamicObjects->count();
    }

    auto level = m_scrollLayer->m_dynamicObjects->objectAtIndex(realPage);

    if (level == nullptr){
        return std::nullopt;
    }

    if (auto lvl = typeinfo_cast<GJGameLevel*>(level)){
        auto lvlReal = LevelTools::getLevel(lvl->m_levelID.value(), true);

        if (lvl->m_levelID.value() < 0) return std::nullopt;

        return lvlReal;
    }

    return std::nullopt;
}