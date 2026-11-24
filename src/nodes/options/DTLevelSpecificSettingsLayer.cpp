#include "DTLevelSpecificSettingsLayer.hpp"

#include <actions/CCResizeTo.hpp>
#include <nodes/DTLayer.hpp>

#include <nodes/options/pages/run/RunOptions.hpp>
#include <nodes/options/pages/link/LinkOptions.hpp>
#include <nodes/options/pages/modify/ModifyOptions.hpp>
#include <nodes/options/pages/save/SaveOptions.hpp>
#include <nodes/options/pages/section/SectionOptions.hpp>

DTLevelSpecificSettingsLayer* DTLevelSpecificSettingsLayer::create() {
    auto popup = new DTLevelSpecificSettingsLayer;
    if (popup->init()) {
        popup->autorelease();
        return popup;
    }
    delete popup;
    return nullptr;
}

bool DTLevelSpecificSettingsLayer::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    if (!Popup::init(winSize.width - 30, winSize.height - 30, "geode.loader/GE_square01.png"))
        return false;

    this->setTitle("Level Options", "goldFont.fnt", .8f, 15);

    auto pages = CCMenu::create();
    pages->setLayout(RowLayout::create()
        ->setGap(15)
        ->setGrowCrossAxis(true)
        ->setCrossAxisOverflow(false)
    );
    pages->setContentSize({m_size.width - 5, 25});
    pages->setPosition({m_size.width / 2, this->m_title->getPositionY() - this->m_title->getScaledContentHeight() / 2 - pages->getScaledContentHeight() / 2});
    m_mainLayer->addChild(pages);

    auto seperator = CCScale9Sprite::create("square.png");
    seperator->setContentSize({pages->getContentWidth(), 1});
    seperator->setScaleY(8 / pages->getContentHeight());
    seperator->setPosition({pages->getPositionX(), pages->getPositionY() - pages->getScaledContentHeight() / 2 - 5});
    seperator->setAnchorPoint({.5f, 1});
    m_mainLayer->addChild(seperator);

    m_optionUnderline = CCScale9Sprite::create("square.png");
    m_optionUnderline->setScaleY(.5f);
    m_optionUnderline->setContentHeight(1);
    m_mainLayer->addChild(m_optionUnderline);

    auto firstOption = DTLevelSpecificSettingsLayer::createPageBtn("Run", RunOptions::create(ccp(m_size.width - 10, seperator->getPositionY() - 10)));

    pages->addChild(firstOption);
    pages->addChild(DTLevelSpecificSettingsLayer::createSeperator());
    pages->addChild(DTLevelSpecificSettingsLayer::createPageBtn("Modify", ModifyOptions::create(ccp(m_size.width - 10, seperator->getPositionY() - 10))));
    pages->addChild(DTLevelSpecificSettingsLayer::createSeperator());
    pages->addChild(DTLevelSpecificSettingsLayer::createPageBtn("Save", SaveOptions::create(ccp(m_size.width - 10, seperator->getPositionY() - 10))));
    pages->addChild(DTLevelSpecificSettingsLayer::createSeperator());
    pages->addChild(DTLevelSpecificSettingsLayer::createPageBtn("Link", LinkOptions::create(ccp(m_size.width - 10, seperator->getPositionY() - 10))));
    pages->addChild(DTLevelSpecificSettingsLayer::createSeperator());
    pages->addChild(DTLevelSpecificSettingsLayer::createPageBtn("Section", SectionOptions::create(ccp(m_size.width - 10, seperator->getPositionY() - 10))));
    // pages->addChild(DTLevelSpecificSettingsLayer::createSeperator());
    // pages->addChild(DTLevelSpecificSettingsLayer::createPageBtn("Session", SessionOptions::create(ccp(m_size.width - 10, seperator->getPositionY() - 10))));

    pages->updateLayout();
    
    sendUnderlineTo(firstOption, true);
    switchPage(firstOption);

    return true;
}

CCNode* DTLevelSpecificSettingsLayer::createPageBtn(std::string_view text, OptionsNode* page) {

    auto ownedText = std::string(text);
    auto spr = geode::Label::create(ownedText.c_str(), "gjFont17.fnt");
    spr->setScale(.85f);
    auto button = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(DTLevelSpecificSettingsLayer::switchPage)
    );
    page->setSettingsLayer(this);
    m_pages.insert({button, page});

    if (page != nullptr) {
        page->setZOrder(2);
        page->setPosition({5, 5});
        m_mainLayer->addChild(page);
    }

    return button;
}

CCNode* DTLevelSpecificSettingsLayer::createSeperator() {
    auto spr = geode::Label::create("|", "gjFont17.fnt");
    return spr;
}

void DTLevelSpecificSettingsLayer::sendUnderlineTo(CCNode* node, bool instant) {
    m_optionUnderline->stopAllActions();

    float movementTime = instant ? 0 : .2f;

    CCPoint nodePosInSpace = m_optionUnderline->getParent()->convertToNodeSpace(node->getParent()->convertToWorldSpace(node->getPosition()));

    CCPoint leftSide = m_optionUnderline->getParent()->convertToNodeSpace(node->getParent()->convertToWorldSpace(node->getPosition() - node->getScaledContentSize() * node->getAnchorPoint()));
    CCPoint rightSide = m_optionUnderline->getParent()->convertToNodeSpace(node->getParent()->convertToWorldSpace(node->getPosition() + node->getScaledContentSize() * node->getAnchorPoint()));

    float localWidth = rightSide.x - leftSide.x;
    float localHeight = rightSide.y - leftSide.y;

    m_optionUnderline->runAction(
        CCEaseExponentialOut::create(
            CCMoveTo::create(
                movementTime,
                nodePosInSpace - ccp(0, localHeight / 2)
            )
        )
    );
    m_optionUnderline->runAction(
        CCSequence::create(
            CCEaseInOut::create(
                CCResizeTo::create(
                    movementTime,
                    {localWidth, m_optionUnderline->getContentHeight()}
                ),
                2
            ),
            CCCallFuncO::create(
                this,
                callfuncO_selector(DTLevelSpecificSettingsLayer::makeUnderlineIdle),
                m_optionUnderline
            ),
            nullptr
        )
    );
}

void DTLevelSpecificSettingsLayer::makeUnderlineIdle(CCObject* sender) {
    auto node = static_cast<CCNode*>(sender);

    float scaleAmount = 2;

    m_optionUnderline->runAction(
        CCRepeatForever::create(
            CCSequence::create(
                CCEaseInOut::create(
                    CCResizeTo::create(
                        1,
                        {node->getContentWidth() + scaleAmount, m_optionUnderline->getContentHeight()}
                    ),
                    2
                ),
                CCEaseInOut::create(
                    CCResizeTo::create(
                        1,
                        {node->getContentWidth() - scaleAmount, m_optionUnderline->getContentHeight()}
                    ),
                    2
                ),
                nullptr
            )
        )
    );
}

void DTLevelSpecificSettingsLayer::switchPage(CCObject* sender) {
    if (!m_pages.contains(sender)) return;

    auto senderNode = static_cast<CCNode*>(sender);

    if (m_currentPage == m_pages[sender]) return;

    DTLevelSpecificSettingsLayer::sendUnderlineTo(senderNode);

    if (m_currentPage != nullptr)
        m_currentPage->onClosed();

    m_currentPage = m_pages[sender];

    if (m_currentPage != nullptr)
        m_currentPage->onOpened();
}

void DTLevelSpecificSettingsLayer::onClose(CCObject*) {
    Popup::onClose(nullptr);
}

void DTLevelSpecificSettingsLayer::keyBackClicked() {
    Popup::keyBackClicked();
}