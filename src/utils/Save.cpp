#include "Save.hpp"

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
    return Mod::get()->getSavedValue<CustomizationData>("NewBestCustomizations", CustomizationData{.m_color = to4B(Mod::get()->getSavedValue<ccColor3B>("NewBestColor"))});
}

void Save::setNewBestCustomizations(const CustomizationData& data) {
    Mod::get()->setSavedValue("NewBestCustomizations", data);
}

CustomizationData Save::getSessionBestCustomizations() {
    return Mod::get()->getSavedValue<CustomizationData>("SessionBestCustomizations", CustomizationData{.m_color = to4B(Mod::get()->getSavedValue<ccColor3B>("SessionBestColor"))});
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

OverlayLabelSettings Save::getOverlayLabels() {
    return Mod::get()->getSavedValue<OverlayLabelSettings>("overlayLabelsSettings");
}
void Save::setOverlayLabels(const OverlayLabelSettings& labels) {
    Mod::get()->setSavedValue("overlayLabelsSettings", labels);

    OverlayLabelsChangedEvent().send();
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

bool Save::getDidInitLayout() {
    return Mod::get()->getSavedValue<bool>("didInitLayout");
}
void Save::setDidInitLayout(const bool& b) {
    Mod::get()->setSavedValue("didInitLayout", b);
}

bool Save::getDidInitDefaultOverlayLabels() {
    return Mod::get()->getSavedValue<bool>("didInitDefaultOverlayLabels");
}
void Save::setDidInitDefaultOverlayLabels(const bool& b) {
    Mod::get()->setSavedValue("didInitDefaultOverlayLabels", b);
}

DTLayoutV3 Save::getDefaultLayout() {
    return DTLayoutV3{
        .m_columns = std::vector<DTColumnInfo>{
            DTColumnInfo{ .m_orderPos = 0, .m_currentWidth = 120.0f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 1, .m_currentWidth = 100.0f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 2, .m_currentWidth = 100.0f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 3, .m_currentWidth = 129.430908203125f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 4, .m_currentWidth = 125.6539306640625f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 5, .m_currentWidth = 116.2230224609375f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 6, .m_currentWidth = 99.3603515625f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 7, .m_currentWidth = 257.3111572265625f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 8, .m_currentWidth = 114.939208984375f, .m_color = {255, 255, 255} },
            DTColumnInfo{ .m_orderPos = 9, .m_currentWidth = 131.4425048828125f, .m_color = {255, 255, 255} },
        },
        .m_labels = std::vector<DTLabelInfo>{
            DTLabelInfo{
                .m_minPlacementRange = 0,
                .m_maxPlacementRange = 2,
                .m_layer = 0,
                .m_labelName = "level name",
                .m_text = "{lvln}:",
                .m_font = "goldFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(1),
                .m_labelColor = {255, 213, 118, 255},
                .m_textColor = {255, 255, 255, 255},
                .m_scale = 0.5f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 0,
                .m_maxPlacementRange = 0,
                .m_layer = 1,
                .m_labelName = "attempts",
                .m_text = "gd - {att}{nl}death tracker - {dtatt}",
                .m_font = "chatFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {170, 170, 170, 255},
                .m_textColor = {255, 255, 255, 255},
                .m_scale = 0.6499999761581421f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 0,
                .m_maxPlacementRange = 0,
                .m_layer = 2,
                .m_labelName = "playtime",
                .m_text = "overall - {ptgen}{nl}from 0: {ptf0}{nl}runs: {ptruns}{nl} {nl}paused time: {pptgen}{nl}dead time: {dptgen}",
                .m_font = "gjFont17.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {170, 170, 170, 255},
                .m_textColor = {255, 255, 255, 255},
                .m_scale = 0.2800000309944153f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 0,
                .m_maxPlacementRange = 0,
                .m_layer = 3,
                .m_labelName = "legacy playtime",
                .m_text = "calculated playtime{nl} {nl}overall: {aptgen}{nl}from 0: {aptf0}{nl}runs: {aptruns}",
                .m_font = "gjFont17.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {121, 119, 119, 255},
                .m_textColor = {99, 98, 98, 255},
                .m_scale = 0.2500000298023224f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .m_minPlacementRange = 1,
                .m_maxPlacementRange = 1,
                .m_layer = 1,
                .m_labelName = "from 0",
                .m_text = "{general}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {64, 138, 98, 255},
                .m_textColor = {123, 255, 184, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 2,
                .m_maxPlacementRange = 2,
                .m_layer = 1,
                .m_labelName = "runs",
                .m_text = "{runs}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {37, 86, 70, 255},
                .m_textColor = {42, 167, 102, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 3,
                .m_maxPlacementRange = 4,
                .m_layer = 0,
                .m_labelName = "Session Date",
                .m_text = "{sdate}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(1),
                .m_labelColor = {255, 147, 44, 255},
                .m_textColor = {160, 74, 16, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .m_minPlacementRange = 3,
                .m_maxPlacementRange = 3,
                .m_layer = 1,
                .m_labelName = "session playtime",
                .m_text = "overall: {ptsgen}{nl}from 0: {ptsf0}{nl}runs: {ptsruns}{nl}paused time: {pptsgen}{nl}dead time: {dptsgen}",
                .m_font = "gjFont17.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {176, 150, 110, 255},
                .m_textColor = {255, 255, 255, 255},
                .m_scale = 0.30000001192092896f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 3,
                .m_maxPlacementRange = 3,
                .m_layer = 2,
                .m_labelName = "session from 0",
                .m_text = "{s0}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {160, 109, 60, 255},
                .m_textColor = {255, 174, 77, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 4,
                .m_maxPlacementRange = 4,
                .m_layer = 1,
                .m_labelName = "session attempts",
                .m_text = "attempts: {satt}",
                .m_font = "chatFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(1),
                .m_labelColor = {156, 96, 46, 255},
                .m_textColor = {255, 149, 66, 255},
                .m_scale = 0.699999988079071f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .m_minPlacementRange = 4,
                .m_maxPlacementRange = 4,
                .m_layer = 2,
                .m_labelName = "session runs",
                .m_text = "{sruns}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {102, 68, 31, 255},
                .m_textColor = {206, 127, 42, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 5,
                .m_maxPlacementRange = 5,
                .m_layer = 0,
                .m_labelName = "to 100",
                .m_text = "{rt100}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {24, 125, 115, 255},
                .m_textColor = {36, 225, 255, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 5,
                .m_maxPlacementRange = 5,
                .m_layer = 1,
                .m_labelName = "level in runs",
                .m_text = "{lvlruns}",
                .m_font = "gjFont11.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {73, 87, 170, 255},
                .m_textColor = {113, 124, 255, 255},
                .m_scale = 0.3999999761581421f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .m_minPlacementRange = 6,
                .m_maxPlacementRange = 6,
                .m_layer = 0,
                .m_labelName = "best runs",
                .m_text = "{bruns}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {0, 63, 109, 255},
                .m_textColor = {80, 160, 255, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 7,
                .m_maxPlacementRange = 7,
                .m_layer = 0,
                .m_labelName = "section",
                .m_text = "{section}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {64, 29, 92, 255},
                .m_textColor = {198, 125, 255, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
            DTLabelInfo{
                .m_minPlacementRange = 8,
                .m_maxPlacementRange = 8,
                .m_layer = 0,
                .m_labelName = "session to 100",
                .m_text = "{srt100}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {137, 72, 31, 255},
                .m_textColor = {248, 130, 43, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .m_minPlacementRange = 9,
                .m_maxPlacementRange = 9,
                .m_layer = 0,
                .m_labelName = "session best runs",
                .m_text = "{sbruns}",
                .m_font = "bigFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {168, 83, 46, 255},
                .m_textColor = {255, 110, 44, 255},
                .m_scale = 0.3499999940395355f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(3)
            },
            DTLabelInfo{
                .m_minPlacementRange = 0,
                .m_maxPlacementRange = 0,
                .m_layer = 4,
                .m_labelName = "Run Attempts",
                .m_text = "{runatt}",
                .m_font = "chatFont.fnt",
                .m_horizontalAlignment = static_cast<CCTextAlignment>(0),
                .m_labelColor = {170, 170, 170, 255},
                .m_textColor = {255, 255, 255, 255},
                .m_scale = 0.6499999761581421f,
                .m_isExpanded = true,
                .m_wrapping = static_cast<WrappingMode>(2)
            },
        }
    };
}

CustomizationData Save::getDefaultNewBestCustomizations() {
    return CustomizationData{
        .m_color = {255, 255, 0, 255},
        .m_gradientEnabled = true,
        .m_gradient = {
            .m_colorA = {255, 244, 123, 255},
            .m_colorB = {255, 255, 0, 255},
            .m_duration = 1,
            .m_stagger = 0.2
        },
        .m_waveEnabled = true,
        .m_wave = {
            .m_speed = 1.2,
            .m_distanceY = 1.5,
            .m_distanceX = 0.2,
            .m_offsetPerIndex = 0.17
        }
    };
}

CustomizationData Save::getDefaultSessionBestCustomizations() {
    return CustomizationData{
        .m_color = {255, 136, 0, 255},
        .m_gradientEnabled = true,
        .m_gradient = {
            .m_colorA = {255, 175, 85, 255},
            .m_colorB = {255, 136, 0, 255},
            .m_duration = 1,
            .m_stagger = 0.2
        },
        .m_waveEnabled = true,
        .m_wave = {
            .m_speed = 1.16,
            .m_distanceY = 0.5,
            .m_distanceX = 0,
            .m_offsetPerIndex = 2
        }
    };
}

std::vector<DTGraphInfo> Save::getDefaultGraphs() {
    return std::vector<DTGraphInfo> {
        DTGraphInfo {
            .m_isEnabled = false,
            .m_coverage = static_cast<DTGraphCoverage>(3),
            .m_type = static_cast<DTGraphType>(0),
            .m_orderPos = 0,
            .m_thickness = 1.0f,
            .m_outlineThickness = 0.75f,
#if defined(GEODE_IS_MOBILE)
            .m_pointScale = 0.175,
#else
            .m_pointScale = 0.1,
#endif
            .m_color = {122, 74, 0, 255},
            .m_outlineColor = {101, 50, 0, 255},
            .m_pointColor = {164, 76, 0, 255},
            .m_name = "session runs"
        },
        DTGraphInfo {
            .m_isEnabled = false,
            .m_coverage = static_cast<DTGraphCoverage>(2),
            .m_type = static_cast<DTGraphType>(0),
            .m_orderPos = 1,
            .m_thickness = 1.0f,
            .m_outlineThickness = 0.75f,
#if defined(GEODE_IS_MOBILE)
            .m_pointScale = 0.175,
#else
            .m_pointScale = 0.1,
#endif
            .m_color = {255, 145, 0, 255},
            .m_outlineColor = {170, 104, 16, 255},
            .m_pointColor = {114, 66, 3, 255},
            .m_name = "sess from0"
        },
        DTGraphInfo{
            .m_isEnabled = false,
            .m_coverage = static_cast<DTGraphCoverage>(1),
            .m_type = static_cast<DTGraphType>(0),
            .m_orderPos = 2,
            .m_thickness = 0.75f,
            .m_outlineThickness = 0.5f,
#if defined(GEODE_IS_MOBILE)
            .m_pointScale = 0.175,
#else
            .m_pointScale = 0.1,
#endif
            .m_color = {14, 138, 37, 253},
            .m_outlineColor = {11, 96, 32, 255},
            .m_pointColor = {5, 181, 48, 255},
            .m_name = "general runs"
        },
        DTGraphInfo {
            .m_isEnabled = true,
            .m_coverage = static_cast<DTGraphCoverage>(0),
            .m_type = static_cast<DTGraphType>(0),
            .m_orderPos = 3,
            .m_thickness = 1.0f,
            .m_outlineThickness = 0.75f,
#if defined(GEODE_IS_MOBILE)
            .m_pointScale = 0.175,
#else
            .m_pointScale = 0.1,
#endif
            .m_color = {0, 255, 55, 255},
            .m_outlineColor = {18, 162, 53, 255},
            .m_pointColor = {15, 105, 41, 255},
            .m_name = "general from0"
        }
    };
}

OverlayLabelSettings Save::getDefaultOverlayLabels() {
    return OverlayLabelSettings {
        .m_labels = {
            OverlayLabel{
                .m_labelName = "latest death",
                .m_text = "Latest Death: {(g)cd}",
                .m_enabled = false
            },
            OverlayLabel{
                .m_labelName = "best runs",
                .m_text = "Best Runs Today: {(d)3b}",
                .m_order = 1,
                .m_enabled = false
            }
        }
    };
}