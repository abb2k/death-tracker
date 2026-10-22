#include <utils/Save.hpp>


std::vector<V2LabelLayout> Save::getOldLayout() {
    return Mod::get()->getSavedValue<std::vector<V2LabelLayout>>("Layout");
}

void Save::setOldLayout(const std::vector<V2LabelLayout>& layout) {
    Mod::get()->setSavedValue("Layout", layout);
}

DTLayoutV3 Save::getLayout() {
    return Mod::get()->getSavedValue<DTLayoutV3>("DTLayoutV3");
}

void Save::setLayout(const DTLayoutV3& layout) {
    Mod::get()->setSavedValue("DTLayoutV3", layout);
}

CustomizationData Save::getNewBestCustomizations() {
    return Mod::get()->getSavedValue<CustomizationData>("NewBestCustomizations", CustomizationData{.color = to4B(Mod::get()->getSavedValue<ccColor3B>("NewBestColor"))});
}

void Save::setNewBestCustomizations(const CustomizationData& data) {
    Mod::get()->setSavedValue("NewBestCustomizations", data);
}

CustomizationData Save::getSessionBestCustomizations() {
    return Mod::get()->getSavedValue<CustomizationData>("SessionBestCustomizations", CustomizationData{.color = to4B(Mod::get()->getSavedValue<ccColor3B>("SessionBestColor"))});
}

void Save::setSessionBestCustomizations(const CustomizationData& data) {
    Mod::get()->setSavedValue("SessionBestCustomizations", data);
}

bool Save::getExportWOutLabels() {
    return Mod::get()->getSavedValue<bool>("ExportWLabels");
}

void Save::setExportWOutLabels(const bool& b) {
    Mod::get()->setSavedValue("ExportWLabels", b);
}

bool Save::getDidConvertToV3() {
    return Mod::get()->getSavedValue<bool>("DidConvertToV3");
}
void Save::setDidConvertToV3(const bool& b) {
    Mod::get()->setSavedValue("DidConvertToV3", b);
}

std::string Save::getLastOpenedVersion() {
    return Mod::get()->getSavedValue<std::string>("LastOpenedVersion");
}

void Save::setLastOpenedVersion(const std::string& verion) {
    Mod::get()->setSavedValue("LastOpenedVersion", verion);
}

stringCustomazations Save::getFrom0Customazations() {
    return Mod::get()->getSavedValue<stringCustomazations>("F0Zustom");
}
void Save::setFrom0Customazations(const stringCustomazations& from0Custom) {
    Mod::get()->setSavedValue("F0Zustom", from0Custom);
}

stringCustomazations Save::getRunsCustomazations() {
    return Mod::get()->getSavedValue<stringCustomazations>("RunCustom");
}
void Save::setRunCustomazations(const stringCustomazations& runCustom) {
    Mod::get()->setSavedValue("RunCustom", runCustom);
}

stringCustomazations Save::getSessionF0Customazations() {
    return Mod::get()->getSavedValue<stringCustomazations>("SessionF0Custom");
}
void Save::setSessionF0Customazations(const stringCustomazations& sessionF0Custom) {
    Mod::get()->setSavedValue("SessionF0Custom", sessionF0Custom);
}

stringCustomazations Save::getSessionRunCustomazations() {
    return Mod::get()->getSavedValue<stringCustomazations>("SessionRunCustom");
}
void Save::setSessionRunCustomazations(const stringCustomazations& sessionRunCustom) {
    Mod::get()->setSavedValue("SessionRunCustom", sessionRunCustom);
}

std::vector<DTGraphInfo> Save::getGraphs() {
    return Mod::get()->getSavedValue<std::vector<DTGraphInfo>>("graphs");
}
void Save::setGraphs(const std::vector<DTGraphInfo>& graphs) {
    Mod::get()->setSavedValue("graphs", graphs);
}

bool Save::wasTutorialSeen(const std::string& tutorialID) {
    auto tutorialsSeen = Mod::get()->getSavedValue<std::set<std::string>>("tutorialsSeen");

    return tutorialsSeen.contains(tutorialID);
}
void Save::setTutorialSeen(const std::string& tutorialID) {
    auto tutorialsSeen = Mod::get()->getSavedValue<std::set<std::string>>("tutorialsSeen");

    if (tutorialsSeen.contains(tutorialID)) return;

    tutorialsSeen.insert(tutorialID);

    Mod::get()->setSavedValue("tutorialsSeen", tutorialsSeen);
}

std::vector<DTLayoutPreset> Save::getLayoutPresets() {
    return Mod::get()->getSavedValue<std::vector<DTLayoutPreset>>("layoutPresets");
}
void Save::setLayoutPresets(const std::vector<DTLayoutPreset>& presets) {
    Mod::get()->setSavedValue("layoutPresets", presets);
}

bool Save::getLevelWasPTTransferred(std::string const& lvlKey) {
    auto PTTransferData = Mod::get()->getSavedValue<std::set<std::string>>("PTTransferData");

    if (PTTransferData.contains(lvlKey))
        return true;
    
    return false;
}
void Save::setLevelWasPTTransferred(std::string const& lvlKey) {
    auto PTTransferData = Mod::get()->getSavedValue<std::set<std::string>>("PTTransferData");

    if (PTTransferData.contains(lvlKey)) return;

    PTTransferData.insert(lvlKey);

    Mod::get()->setSavedValue("PTTransferData", PTTransferData);
}

DTLayoutV3 Save::getDefaultLayout() {
    return DTLayoutV3{
        .columns = std::vector<DTColumnInfo>{
            DTColumnInfo{ .orderPos = 0, .currentWidth = 120.0f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 1, .currentWidth = 100.0f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 2, .currentWidth = 100.0f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 3, .currentWidth = 129.430908203125f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 4, .currentWidth = 125.6539306640625f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 5, .currentWidth = 116.2230224609375f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 6, .currentWidth = 99.3603515625f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 7, .currentWidth = 257.3111572265625f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 8, .currentWidth = 114.939208984375f, .color = {255, 255, 255} },
            DTColumnInfo{ .orderPos = 9, .currentWidth = 131.4425048828125f, .color = {255, 255, 255} },
        },
        .labels = std::vector<DTLabelInfo>{
            DTLabelInfo{
                .minPlacementRange = 0,
                .maxPlacementRange = 2,
                .layer = 0,
                .labelName = "level name",
                .text = "{lvln}:",
                .font = "goldFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(1),
                .labelColor = {255, 213, 118, 255},
                .textColor = {255, 255, 255, 255},
                .scale = 0.5f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 0,
                .maxPlacementRange = 0,
                .layer = 1,
                .labelName = "attempts",
                .text = "gd - {att}{nl}death tracker - {dtatt}",
                .font = "chatFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {170, 170, 170, 255},
                .textColor = {255, 255, 255, 255},
                .scale = 0.6499999761581421f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 0,
                .maxPlacementRange = 0,
                .layer = 2,
                .labelName = "playtime",
                .text = "overall - {ptgen}{nl}from 0: {ptf0}{nl}runs: {ptruns}{nl} {nl}paused time: {pptgen}{nl}dead time: {dptgen}",
                .font = "gjFont17.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {170, 170, 170, 255},
                .textColor = {255, 255, 255, 255},
                .scale = 0.2800000309944153f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 0,
                .maxPlacementRange = 0,
                .layer = 3,
                .labelName = "legacy playtime",
                .text = "calculated playtime{nl} {nl}overall: {aptgen}{nl}from 0: {aptf0}{nl}runs: {aptruns}",
                .font = "gjFont17.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {121, 119, 119, 255},
                .textColor = {99, 98, 98, 255},
                .scale = 0.2500000298023224f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .minPlacementRange = 1,
                .maxPlacementRange = 1,
                .layer = 1,
                .labelName = "from 0",
                .text = "{general}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {64, 138, 98, 255},
                .textColor = {123, 255, 184, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 2,
                .maxPlacementRange = 2,
                .layer = 1,
                .labelName = "runs",
                .text = "{runs}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {37, 86, 70, 255},
                .textColor = {42, 167, 102, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 3,
                .maxPlacementRange = 4,
                .layer = 0,
                .labelName = "Session Date",
                .text = "{sdate}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(1),
                .labelColor = {255, 147, 44, 255},
                .textColor = {160, 74, 16, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .minPlacementRange = 3,
                .maxPlacementRange = 3,
                .layer = 1,
                .labelName = "session playtime",
                .text = "overall: {ptsgen}{nl}from 0: {ptsf0}{nl}runs: {ptsruns}{nl}paused time: {pptsgen}{nl}dead time: {dptsgen}",
                .font = "gjFont17.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {176, 150, 110, 255},
                .textColor = {255, 255, 255, 255},
                .scale = 0.30000001192092896f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 3,
                .maxPlacementRange = 3,
                .layer = 2,
                .labelName = "session from 0",
                .text = "{s0}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {160, 109, 60, 255},
                .textColor = {255, 174, 77, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 4,
                .maxPlacementRange = 4,
                .layer = 1,
                .labelName = "session attempts",
                .text = "attempts: {satt}",
                .font = "chatFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(1),
                .labelColor = {156, 96, 46, 255},
                .textColor = {255, 149, 66, 255},
                .scale = 0.699999988079071f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .minPlacementRange = 4,
                .maxPlacementRange = 4,
                .layer = 2,
                .labelName = "session runs",
                .text = "{sruns}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {102, 68, 31, 255},
                .textColor = {206, 127, 42, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 5,
                .maxPlacementRange = 5,
                .layer = 0,
                .labelName = "to 100",
                .text = "{rt100}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {24, 125, 115, 255},
                .textColor = {36, 225, 255, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 5,
                .maxPlacementRange = 5,
                .layer = 1,
                .labelName = "level in runs",
                .text = "{lvlruns}",
                .font = "gjFont11.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {73, 87, 170, 255},
                .textColor = {113, 124, 255, 255},
                .scale = 0.3999999761581421f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .minPlacementRange = 6,
                .maxPlacementRange = 6,
                .layer = 0,
                .labelName = "best runs",
                .text = "{bruns}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {0, 63, 109, 255},
                .textColor = {80, 160, 255, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 7,
                .maxPlacementRange = 7,
                .layer = 0,
                .labelName = "section",
                .text = "{section}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {64, 29, 92, 255},
                .textColor = {198, 125, 255, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .minPlacementRange = 8,
                .maxPlacementRange = 8,
                .layer = 0,
                .labelName = "session to 100",
                .text = "{srt100}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {137, 72, 31, 255},
                .textColor = {248, 130, 43, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .minPlacementRange = 9,
                .maxPlacementRange = 9,
                .layer = 0,
                .labelName = "session best runs",
                .text = "{sbruns}",
                .font = "bigFont.fnt",
                .horizontalAlignment = static_cast<CCTextAlignment>(0),
                .labelColor = {168, 83, 46, 255},
                .textColor = {255, 110, 44, 255},
                .scale = 0.3499999940395355f,
                .isExpanded = true,
                .wrapping = static_cast<WrappingMode>(3)
            },
        }
    };
}