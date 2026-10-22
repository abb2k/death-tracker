#include "ClaimDonoSettingType.hpp"

#include <managers/AuthManager.hpp>

Result<std::shared_ptr<SettingV3>> ClaimDonoSetting::parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
    auto res = std::make_shared<ClaimDonoSetting>();
    auto root = checkJson(json, "ClaimDonoSetting");

    res->init(key, modID, root);
    res->parseNameAndDescription(root);
    res->parseEnableIf(root);
    
    root.checkUnknownKeys();
    return root.ok(std::static_pointer_cast<SettingV3>(res));
}

bool ClaimDonoSetting::load(matjson::Value const& json) {
    return true;
}
bool ClaimDonoSetting::save(matjson::Value& json) const {
    return true;
}
    
bool ClaimDonoSetting::isDefaultValue() const {
    return true;
}
void ClaimDonoSetting::reset() {}

SettingNodeV3* ClaimDonoSetting::createNode(float width) {
    return ClaimDonoSettingNode::create(
        std::static_pointer_cast<ClaimDonoSetting>(shared_from_this()),
        width
    );
}


bool ClaimDonoSettingNode::init(std::shared_ptr<ClaimDonoSetting> setting, float width) {
    if (!SettingNodeV3::init(setting, width))
        return false;
        
    m_input = TextInput::create(85, "Token");
    m_input->setScale(.75f);
    m_input->setPasswordMode(true);
    m_input->setFilter("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    auto spr = CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png");
    spr->setCascadeOpacityEnabled(true);
    spr->setCascadeColorEnabled(true);
    spr->setScale(.45f);
    m_button = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(ClaimDonoSettingNode::onButton)
    );
    spr->setScaleX(spr->getScaleX() * -1);

    this->getButtonMenu()->addChildAtPosition(m_input, Anchor::Left);
    this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Right, {-15, 0});
    this->getButtonMenu()->setContentWidth(62);
    this->getButtonMenu()->updateLayout();

    auto layer = CCLayer::create();
    this->getButtonMenu()->addChild(layer);
    layer->setPosition(m_input->getPosition());
    layer->ignoreAnchorPointForPosition(false);
    layer->setScale(.5f);

    m_loadingCircle = LoadingCircle::create();
    m_loadingCircle->setParentLayer(layer);
    m_loadingCircle->show();
    m_loadingCircle->setVisible(false);

    if (AuthManager::isDoner()) {
        disableStuff();
    }

    m_donerListenerHandle = OnDonerStateChangedEvent().listen([&](bool state) { if (state) disableStuff(); });

    this->updateState(nullptr);
    
    return true;
}

void ClaimDonoSettingNode::updateState(CCNode* invoker) {
    SettingNodeV3::updateState(invoker);

    auto shouldEnable = this->getSetting()->shouldEnable();
    // m_input->setEnabled(shouldEnable);
    // m_button->setEnabled(shouldEnable);
    // m_button->setOpacity(shouldEnable ? 255 : 155);
    // m_button->setColor(shouldEnable ? ccWHITE : ccGRAY);
}
void ClaimDonoSettingNode::onButton(CCObject*) {
    m_loadingCircle->setVisible(true);

    auto request = web::WebRequest();

    request.bodyString(fmt::format("{{ \"accountId\":\"{}\", \"donorToken\":\"{}\"}}", GJAccountManager::get()->m_accountID, m_input->getString()));

    m_claimHolder.spawn(
        request.post("https://api.abb2k.me/death-tracker/v1/claim-donor"),
        [&](web::WebResponse webRes) {
            m_loadingCircle->setVisible(false);

            auto jsonRes = webRes.json();

            AuthManager::authForDoner(true);

            if (jsonRes.isErr()) {
                geode::createQuickPopup("Error", "Invalid response from the server.", "OK", nullptr, [](auto _, auto __){});
                return;
            }

            auto json = std::move(jsonRes).unwrap();

            if (!json.contains("success")) {
                geode::createQuickPopup("Error", "Invalid response from the server.", "OK", nullptr, [](auto _, auto __){});
                return;
            }

            auto successRes = json["success"].asBool();

            if (successRes.isErr()) {
                geode::createQuickPopup("Error", "Invalid response from the server.", "OK", nullptr, [](auto _, auto __){});
                return;
            }

            if (successRes.unwrap()) {
                geode::createQuickPopup("Success!", "Your token has been claimed! Enjoy!", "OK", nullptr, [](auto _, auto __){});

                disableStuff();
            }
            else {
                std::string message = "Token claim failed for unknown reasons";

                if (json.contains("message")) {
                    message = json["message"].asString().unwrapOr(message);
                }
                
                geode::createQuickPopup("Error", message, "OK", nullptr, [](auto _, auto __){});
            }
        }
    );
}

void ClaimDonoSettingNode::onCommit() {}
void ClaimDonoSettingNode::onResetToDefault() {}

ClaimDonoSettingNode* ClaimDonoSettingNode::create(std::shared_ptr<ClaimDonoSetting> setting, float width) {
    auto ret = new ClaimDonoSettingNode();
    if (ret->init(setting, width)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClaimDonoSettingNode::hasUncommittedChanges() const {
    return false;
}
bool ClaimDonoSettingNode::hasNonDefaultValue() const {
    return false;
}

std::shared_ptr<ClaimDonoSetting> ClaimDonoSettingNode::getSetting() const {
    return std::static_pointer_cast<ClaimDonoSetting>(SettingNodeV3::getSetting());
}

void ClaimDonoSettingNode::disableStuff() {
    m_input->setEnabled(false);
    m_button->setEnabled(false);
    m_button->setOpacity(155);
    m_button->setColor(ccGRAY);
}