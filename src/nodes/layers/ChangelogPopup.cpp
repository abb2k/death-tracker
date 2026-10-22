#include "ChangelogPopup.hpp"

ChangelogPopup* ChangelogPopup::create() {
    auto ret = new ChangelogPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

// a little help from better edit on this one :)
bool ChangelogPopup::init() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    // @geode-ignore(unknown-resource)
    if (!Popup::init(240, 250, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Changelog");
    

    auto changelogOpt = Mod::get()->getMetadata().getChangelog();
    auto version = Mod::get()->getVersion();
    std::string changes = "-1";

    if (changelogOpt.has_value()) {
        auto changelog = changelogOpt.value();

        bool validHeader = false;
        for (auto line : string::split(changelog, "\n")) {
            string::trimIP(line);
            if (line.empty()) continue;
            
            auto it = line.begin();
            switch (*it) {
                case '#': {
                    while (it != line.end() && (*it == '#' || std::isspace(*it))) {
                        it++;
                    }
                    auto parse = VersionInfo::parse(std::string(it, line.end()));

                    if (parse.isErr()) continue;

                    auto ver = parse.unwrap();

                    validHeader = version == ver;
                    if (validHeader)
                        changes = "\n# " + ver.toNonVString() + "\n\n";

                } break;

                default: {
                    if (changes.empty() || !validHeader) continue;

                    changes += line + "\n";
                } break;
            }
        }
    }

    if (changes == "-1") return false;

    auto textArea = MDTextArea::create(changes, m_size - ccp(25, 15 + m_title->getScaledContentHeight() / 2 + (m_size.height - m_title->getPositionY())));
    m_mainLayer->addChildAtPosition(textArea, Anchor::Center);
    textArea->setPositionY(textArea->getPositionY() - m_title->getScaledContentHeight() / 2);

    // @geode-ignore(unknown-resource)
    auto lower = CCSprite::createWithSpriteFrameName("geode.loader/mods-list-bottom-sapphire.png");
    lower->setAnchorPoint({0, .5f});
    lower->setScale(m_size.width / lower->getContentWidth());
    m_mainLayer->addChild(lower);

    auto jolly = CCSprite::createWithSpriteFrameName("happy.png"_spr);
    jolly->setScale(.9f);
    jolly->setPosition(m_size + ccp(3, 3));
    jolly->setAnchorPoint({1, 1});
    m_mainLayer->addChild(jolly);

    return true;
}