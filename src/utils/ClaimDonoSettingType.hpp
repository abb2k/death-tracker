#include <Geode/loader/SettingV3.hpp>
#include <Geode/loader/Mod.hpp>

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <nodes/layers/FileConversionLayer.hpp>

using namespace geode::prelude;

class ClaimDonoSetting : public SettingV3 {
public:
    static Result<std::shared_ptr<SettingV3>> parse(std::string const& key, std::string const& modID, matjson::Value const& json);

    bool load(matjson::Value const& json) override;
    bool save(matjson::Value& json) const override;

    bool isDefaultValue() const override;
    void reset() override;

    SettingNodeV3* createNode(float width) override;
};

class ClaimDonoSettingNode : public SettingNodeV3 {
protected:
    TextInput* m_input;
    CCMenuItemSpriteExtra* m_button;
    LoadingCircle* m_loadingCircle;

    async::TaskHolder<web::WebResponse> m_claimHolder;

    comm::ListenerHandle m_donerListenerHandle;

    bool init(std::shared_ptr<ClaimDonoSetting> setting, float width);
    
    void updateState(CCNode* invoker) override;
    void onButton(CCObject*);

    void onCommit() override;
    void onResetToDefault() override;

    void disableStuff();

public:
    static ClaimDonoSettingNode* create(std::shared_ptr<ClaimDonoSetting> setting, float width);

    bool hasUncommittedChanges() const override;
    bool hasNonDefaultValue() const override;

    std::shared_ptr<ClaimDonoSetting> getSetting() const;
};

$execute {
    (void)Mod::get()->registerCustomSettingType("claim-dono-setting-type-dt", &ClaimDonoSetting::parse);
}
