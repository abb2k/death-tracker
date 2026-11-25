#pragma once

using namespace geode::prelude;

class ConvertSaveSetting : public SettingV3 {
public:
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

    bool load(matjson::Value const& json) override;
    bool save(matjson::Value& json) const override;

    bool isDefaultValue() const override;
    void reset() override;

    SettingNodeV3* createNode(float width) override;
};

class ConvertSaveSettingNode : public SettingNodeV3 {
public:
    static ConvertSaveSettingNode* create(std::shared_ptr<ConvertSaveSetting> setting, float width);

    bool hasUncommittedChanges() const override;
    bool hasNonDefaultValue() const override;

    std::shared_ptr<ConvertSaveSetting> getSetting() const;

private:
    ButtonSprite* m_buttonSprite;
    CCMenuItemSpriteExtra* m_button;

    bool init(std::shared_ptr<ConvertSaveSetting> setting, float width);
    
    void updateState(CCNode* invoker) override;
    void onButton(CCObject*);

    void onCommit() override;
    void onResetToDefault() override;
};

$execute {
    (void)Mod::get()->registerCustomSettingType("button-type-dt", &ConvertSaveSetting::parse);
}
