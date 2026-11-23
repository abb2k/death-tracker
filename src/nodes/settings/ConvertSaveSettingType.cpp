#include "ConvertSaveSettingType.hpp"

Result<std::shared_ptr<SettingV3>> ConvertSaveSetting::parse(std::string const& key, std::string const& modID, matjson::Value const& json) {
    auto res = std::make_shared<ConvertSaveSetting>();
    auto root = checkJson(json, "ButtonSettingV3");

    res->init(key, modID, root);
    res->parseNameAndDescription(root);
    res->parseEnableIf(root);
    
    root.checkUnknownKeys();
    return root.ok(std::static_pointer_cast<SettingV3>(res));
}

bool ConvertSaveSetting::load(matjson::Value const& json) {
    return true;
}
bool ConvertSaveSetting::save(matjson::Value& json) const {
    return true;
}

bool ConvertSaveSetting::isDefaultValue() const {
    return true;
}
void ConvertSaveSetting::reset() {}

SettingNodeV3* ConvertSaveSetting::createNode(float width) {
    return ConvertSaveSettingNode::create(
        std::static_pointer_cast<ConvertSaveSetting>(shared_from_this()),
        width
    );
}


bool ConvertSaveSettingNode::init(std::shared_ptr<ConvertSaveSetting> setting, float width) {
    if (!SettingNodeV3::init(setting, width))
        return false;
        
    m_buttonSprite = ButtonSprite::create("Convert", "goldFont.fnt", "GJ_button_01.png", .8f);
    m_buttonSprite->setScale(.5f);
    m_button = CCMenuItemSpriteExtra::create(
        m_buttonSprite, this, menu_selector(ConvertSaveSettingNode::onButton)
    );
    this->getButtonMenu()->addChildAtPosition(m_button, Anchor::Center);
    this->getButtonMenu()->setContentWidth(60);
    this->getButtonMenu()->updateLayout();

    this->updateState(nullptr);
    
    return true;
}

void ConvertSaveSettingNode::updateState(CCNode* invoker) {
    SettingNodeV3::updateState(invoker);

    auto shouldEnable = this->getSetting()->shouldEnable();
    m_button->setEnabled(shouldEnable);
    m_buttonSprite->setCascadeColorEnabled(true);
    m_buttonSprite->setCascadeOpacityEnabled(true);
    m_buttonSprite->setOpacity(shouldEnable ? 255 : 155);
    m_buttonSprite->setColor(shouldEnable ? ccWHITE : ccGRAY);
}
void ConvertSaveSettingNode::onButton(CCObject*) {
    FileConversionLayer::create(false)->show();
}

void ConvertSaveSettingNode::onCommit() {}
void ConvertSaveSettingNode::onResetToDefault() {}

ConvertSaveSettingNode* ConvertSaveSettingNode::create(std::shared_ptr<ConvertSaveSetting> setting, float width) {
    auto ret = new ConvertSaveSettingNode();
    if (ret->init(setting, width)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ConvertSaveSettingNode::hasUncommittedChanges() const {
    return false;
}
bool ConvertSaveSettingNode::hasNonDefaultValue() const {
    return false;
}

std::shared_ptr<ConvertSaveSetting> ConvertSaveSettingNode::getSetting() const {
    return std::static_pointer_cast<ConvertSaveSetting>(SettingNodeV3::getSetting());
}