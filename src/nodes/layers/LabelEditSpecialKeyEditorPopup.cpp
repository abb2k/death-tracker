#include "LabelEditSpecialKeyEditorPopup.hpp"

LabelEditSpecialKeyEditorPopup* LabelEditSpecialKeyEditorPopup::create(geode::Function<void(const std::string&)> callback) {
    auto ret = new LabelEditSpecialKeyEditorPopup();
    if (ret->init(std::move(callback))) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool LabelEditSpecialKeyEditorPopup::init(geode::Function<void(const std::string&)> callback) {
    // @geode-ignore(unknown-resource)
    if (!Popup::init(250, 170, "geode.loader/GE_square01.png"))
        return false;
    setTitle("Create Key");

    m_callback = std::move(callback);

    auto menu = CCMenu::create();
    menu->setPosition({0, 0});
    m_mainLayer->addChild(menu);

    auto makeSelector = [&](const char* title, float scale, SEL_MenuHandler handler) {
        auto sprite = ButtonSprite::create(title);
        sprite->setScale(scale);
        auto item = CCMenuItemSpriteExtra::create(sprite, this, handler);
        menu->addChild(item);
        return item;
    };

    m_keyTypeButton = makeSelector("Death Count", .75f, menu_selector(LabelEditSpecialKeyEditorPopup::onKeyTypeClick));
    m_keyTypeButton->setPosition({m_size.width / 2, 120});

    m_scopeButton = makeSelector("Session", .65f, menu_selector(LabelEditSpecialKeyEditorPopup::onScopeClick));
    m_scopeButton->setPosition({m_size.width / 3, 78});

    m_operatorButton = makeSelector(">=", .65f, menu_selector(LabelEditSpecialKeyEditorPopup::onOperatorClick));
    m_operatorButton->setPosition({m_size.width * 2 / 3, 78});

    m_scopeLabel = CCLabelBMFont::create("Coverage", "bigFont.fnt");
    m_scopeLabel->setScale(.5f);
    m_mainLayer->addChild(m_scopeLabel);

    m_operatorLabel = CCLabelBMFont::create("Scope", "bigFont.fnt");
    m_operatorLabel->setScale(.5f);
    m_mainLayer->addChild(m_operatorLabel);

    m_keyTypeList = FloatingList::create({100, 60});
    m_scopeList = FloatingList::create({90, 90});
    m_operatorList = FloatingList::create({85, 110});
    m_decimalList = FloatingList::create({85, 55});
    for (auto list : {m_keyTypeList, m_scopeList, m_operatorList, m_decimalList}) {
        list->setAnchorPoint({.5f, 1});
        list->setListOpenAnchor({.5f, 1});
        list->setZOrder(2);
        m_mainLayer->addChild(list);
    }

    m_keyTypeList->addItems({
        FloatingListItem{.id = 0, .text = "Death Count", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 1, .text = "Best Runs", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 2, .text = "Same Death Count", .BGTexture = "GJ_button_04.png"}
    });
    m_scopeList->addItems({
        FloatingListItem{.id = 0, .text = "Session", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 1, .text = "Level", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 2, .text = "Day", .BGTexture = "GJ_button_04.png"}
    });
    m_operatorList->addItems({
        FloatingListItem{.id = 0, .text = ">=", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 1, .text = ">", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 2, .text = "<", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 3, .text = "Range", .BGTexture = "GJ_button_04.png"}
    });
    m_decimalList->addItems({
        FloatingListItem{.id = 0, .text = "Off", .BGTexture = "GJ_button_04.png"},
        FloatingListItem{.id = 1, .text = "On", .BGTexture = "GJ_button_04.png"}
    });

    m_keyTypeList->setCallback([this](const int& id) {
        m_keyType = id;
        m_keyTypeList->close();
        updateControls();
    });
    m_scopeList->setCallback([this](const int& id) {
        m_scope = id;
        m_scopeList->close();
        updateControls();
    });
    m_operatorList->setCallback([this](const int& id) {
        m_operator = id;
        m_operatorList->close();
        updateControls();
    });
    m_decimalList->setCallback([this](const int& id) {
        m_decimals = id == 1;
        m_decimalList->close();
        updateControls();
    });

    m_firstValue = TextInput::create(75, "Percent", "bigFont.fnt");
    m_firstValue->setCommonFilter(CommonFilter::Uint);
    m_firstValue->setPosition({m_size.width / 2, 48});
    m_mainLayer->addChild(m_firstValue);

    m_secondValue = TextInput::create(75, "Max", "bigFont.fnt");
    m_secondValue->setCommonFilter(CommonFilter::Uint);
    m_secondValue->setPosition({m_size.width * 2 / 3, 48});
    m_mainLayer->addChild(m_secondValue);

    auto createButtonSprite = ButtonSprite::create("OK");
    createButtonSprite->setScale(.75f);
    auto createButton = CCMenuItemSpriteExtra::create(
        createButtonSprite,
        this,
        menu_selector(LabelEditSpecialKeyEditorPopup::onCreateKey)
    );
    createButton->setPosition({m_size.width / 2, 19});
    menu->addChild(createButton);

    updateControls();

    return true;
}

void LabelEditSpecialKeyEditorPopup::onKeyTypeClick(CCObject*) {
    if (m_keyTypeList->isOpened())
        m_keyTypeList->close();
    else {
        m_scopeList->close();
        m_operatorList->close();
        m_decimalList->close();
        m_keyTypeList->open();
    }
}

void LabelEditSpecialKeyEditorPopup::onScopeClick(CCObject*) {
    if (m_scopeList->isOpened())
        m_scopeList->close();
    else {
        m_keyTypeList->close();
        m_operatorList->close();
        m_decimalList->close();
        m_scopeList->open();
    }
}

void LabelEditSpecialKeyEditorPopup::onOperatorClick(CCObject*) {
    auto activeList = m_keyType == 0 ? m_operatorList : m_decimalList;
    if (activeList->isOpened())
        activeList->close();
    else {
        m_keyTypeList->close();
        m_scopeList->close();
        m_operatorList->close();
        m_decimalList->close();
        activeList->open();
    }
}

void LabelEditSpecialKeyEditorPopup::updateControls() {

    m_secondValue->setCommonFilter(m_keyType == 1 ? CommonFilter::Float : CommonFilter::Uint);

    auto keyTypeSprite = ButtonSprite::create(m_keyTypeList->getItemForID(m_keyType).value_or({}).text.c_str());
    keyTypeSprite->setScale(.55f);
    m_keyTypeButton->setSprite(keyTypeSprite);

    auto scopeSprite = ButtonSprite::create(m_scopeList->getItemForID(m_scope).value_or({}).text.c_str());
    scopeSprite->setScale(.5f);
    m_scopeButton->setSprite(scopeSprite);

    auto operatorText = m_keyType == 0
        ? m_operatorList->getItemForID(m_operator).value_or({}).text
        : m_decimals ? "On" : "Off";
    auto operatorSprite = ButtonSprite::create(operatorText.c_str());
    operatorSprite->setScale(.55f);
    m_operatorButton->setSprite(operatorSprite);

    if (m_keyType == 0) {
        m_operatorList->close();
        m_decimalList->close();
        m_operatorLabel->setString("Operator");
    }
    else {
        m_operatorList->close();
        m_operatorLabel->setString("Decimals");
    }

    m_scopeButton->setPosition({m_size.width / 3, 78});
    m_operatorButton->setPosition({m_size.width * 2 / 3, 78});
    m_firstValue->setVisible(m_keyType != 2);
    m_secondValue->setVisible(m_keyType == 1 || (m_keyType == 0 && m_operator == 3));

    auto setListPosition = [](FloatingList* list, CCMenuItemSpriteExtra* button) {
        list->setPosition(button->getPosition() - ccp(0, button->getScaledContentHeight() / 2));
    };
    auto setLabelPosition = [](CCLabelBMFont* label, CCMenuItemSpriteExtra* button) {
        label->setPosition(button->getPosition() + ccp(
            0,
            button->getScaledContentHeight() / 2 + label->getScaledContentHeight() / 2 + 2
        ));
    };
    setListPosition(m_keyTypeList, m_keyTypeButton);
    setListPosition(m_scopeList, m_scopeButton);
    setListPosition(m_operatorList, m_operatorButton);
    setListPosition(m_decimalList, m_operatorButton);
    setLabelPosition(m_scopeLabel, m_scopeButton);
    setLabelPosition(m_operatorLabel, m_operatorButton);

    if (m_keyType == 1) {
        m_firstValue->setPlaceholder("Count");
        m_secondValue->setPlaceholder("Start %");
        m_firstValue->setPosition({m_size.width / 3, 48});
        m_secondValue->setPosition({m_size.width * 2 / 3, 48});
    }
    else if (m_operator == 3) {
        m_firstValue->setPlaceholder("Min%");
        m_secondValue->setPlaceholder("Max%");
        m_firstValue->setPosition({m_size.width / 3, 48});
        m_secondValue->setPosition({m_size.width * 2 / 3, 48});
    }
    else {
        m_firstValue->setPlaceholder("Percent");
        m_firstValue->setPosition({m_size.width / 2, 48});
        m_secondValue->setVisible(false);
    }
}

void LabelEditSpecialKeyEditorPopup::onCreateKey(CCObject* sender) {
    if (m_keyType != 2 && (m_firstValue->getString().empty() || (m_keyType == 0 && m_operator == 3 && m_secondValue->getString().empty()))) {
        FLAlertLayer::create("Could not create key", "Fill the values to create a key", "OK")->show();
        return;
    }

    std::string scope;
    switch (m_scope) {
        case 0: scope = "s"; break;
        case 1: scope = "g"; break;
        default: scope = "d"; break;
    }

    auto key = fmt::format("{{({})", scope);
    if (m_keyType == 2) {
        key += "cd";
        if (m_decimals)
            key += "d";
    }
    else if (m_keyType == 1) {
        key += fmt::format("{}b{}", m_firstValue->getString(), m_secondValue->getString());
        if (m_decimals)
            key += "d";
    }
    else {
        key.insert(key.begin(), 'X');

        switch (m_operator) {
            case 0: key += ">="; break;
            case 1: key += ">"; break;
            case 2: key += "<"; break;
            default:
                auto minResult = utils::numFromString<int>(m_firstValue->getString());
                auto maxResult = utils::numFromString<int>(m_secondValue->getString());

                if (minResult.isErr() || maxResult.isErr() || minResult.unwrap() > maxResult.unwrap()) {
                    FLAlertLayer::create("Could not create key", "The minimum must not exceed the maximum.", "OK")->show();
                    return;
                }

                key += fmt::format("{}-", m_firstValue->getString());

                break;
        }

        if (m_operator == 3)
            key += m_secondValue->getString();
        else
            key += m_firstValue->getString();
    }

    key += "}";

    if (m_callback)
        m_callback(key);

    onClose(sender);
}
