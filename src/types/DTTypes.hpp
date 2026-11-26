#pragma once

using namespace geode::prelude;

using Deaths = std::map<std::string, int>;
using NewBests = std::set<double>;

class SaveDeletionEvent : public Event<SaveDeletionEvent, bool()> {
public:
    using Event::Event;
};

struct ErrorWithCode{
    std::string m_error;
    int m_code;

    ErrorWithCode() = default;

    ErrorWithCode(std::string error_, bool code_ = 0)
        : m_error(std::move(error_))
        , m_code(code_)
    {}

    ErrorWithCode(const char* error_, bool code_ = 0)
        : m_error(error_ ? error_ : "")
        , m_code(code_)
    {}

    friend std::ostream& operator<<(std::ostream& os, ErrorWithCode const& err) {
        return os << err.m_error;
    }
};

struct Run_s{
    std::optional<double> m_start;
    double m_end = 0;
    std::optional<long long> m_startTimestamp;
    std::optional<long long> m_endTimestamp;
    bool m_cheated = false;

    Run_s(std::optional<double> _start, double _end) : m_start(_start), m_end(_end) {}

    Run_s() = default;
};
typedef struct Run_s Run;

typedef struct PlaytimePair {
    uint64_t m_playtimeF0 = 0;
    uint64_t m_playtimeRuns = 0;

    PlaytimePair& operator+=(const PlaytimePair& other);
    void reset();
} PlaytimePair;

template <>
struct matjson::Serialize<PlaytimePair> {
    static Result<PlaytimePair> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const PlaytimePair& value);
};

struct ComplexPercent {
    using Units = int64_t;
    static constexpr Units unitsPerPercent = 1000000;
    static constexpr uint8_t maxDecimalDepth = 6;

    Units m_units = 0;

    static Result<ComplexPercent> fromPercent(double percent);
    double asPercent() const;
    bool isValid() const;
};

struct ComplexDeathOccurrence {
    std::optional<long long> m_runStartTimestamp;
    std::optional<long long> m_runEndTimestamp;
    bool m_cheated = false;
};

struct SessionTimestampRange {
    long long m_start = 0;
    long long m_end = 0;
};

struct ComplexDeath {
    std::optional<ComplexPercent> m_runStartPercent = std::nullopt;
    ComplexPercent m_runEndPercent;

    uint64_t m_deaths = 0;
    std::vector<ComplexDeathOccurrence> m_occurrences;
};

struct ComplexDeathSummary {
    ComplexPercent m_queriedPercent;
    uint8_t m_decimalDepth = 0;
    uint64_t m_deaths = 0;
    std::vector<ComplexDeath> m_records;
};

struct ComplexDeathMap {
    using Run = std::pair<ComplexPercent::Units, std::optional<ComplexPercent::Units>>;
    using Storage = std::map<Run, ComplexDeath>;

    Storage m_records;

    static Result<ComplexDeathMap> readDeathMap(const matjson::Value& value, bool runKeys);
    static Result<ComplexDeathMap> fromLegacy(const Deaths& deaths, bool runKeys);
    
    Result<> insert(ComplexDeath death);
    Result<> recordDeath(ComplexPercent endPercent, std::optional<ComplexPercent> startPercent = std::nullopt, ComplexDeathOccurrence occurrence = {});
    Result<> recordDeath(double endPercent, std::optional<double> startPercent = std::nullopt, ComplexDeathOccurrence occurrence = {});

    Result<ComplexDeathSummary> getComplexDeath(double percent, uint8_t decimalDepth = ComplexPercent::maxDecimalDepth) const;
    Result<ComplexDeathSummary> getComplexDeath(int percent) const;

    bool adjust(double endPercent, std::optional<double> startPercent, int amount);
    bool contains(const std::string& legacyKey) const;
    bool contains(double endPercent, std::optional<double> startPercent) const;

    uint64_t totalDeaths() const;
    Deaths toLegacyDeaths(bool includeCheated = false) const;
    Result<> merge(const ComplexDeathMap& other);
    ComplexDeathMap filterOccurrences(const std::vector<SessionTimestampRange>& ranges) const;
    ComplexDeathMap filterCheatedRuns() const;
    ComplexDeathMap filterOnlyCheatedRuns() const;

    operator Deaths() const {
        return toLegacyDeaths();
    }
};

template <>
struct matjson::Serialize<ComplexPercent> {
    static Result<ComplexPercent> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const ComplexPercent& value);
};

template <>
struct matjson::Serialize<ComplexDeathOccurrence> {
    static Result<ComplexDeathOccurrence> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const ComplexDeathOccurrence& value);
};

template <>
struct matjson::Serialize<ComplexDeath> {
    static Result<ComplexDeath> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const ComplexDeath& value);
};

template <>
struct matjson::Serialize<ComplexDeathMap> {
    static Result<ComplexDeathMap> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const ComplexDeathMap& value);
};

typedef struct GeneralData {
    ComplexDeathMap m_deaths;
    ComplexDeathMap m_runs;
    NewBests m_newBests;
    PlaytimePair m_playtimeGeneral;
    PlaytimePair m_playtimePaused;
    PlaytimePair m_playtimeDead;

    GeneralData& operator+=(const GeneralData& other);
} GeneralData;

template <>
struct matjson::Serialize<GeneralData> {
    static Result<GeneralData> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const GeneralData& value);
};

struct Session {
    std::string m_ownerLevelKey;
    long long m_lastPlayed;
    long long m_sessionStartDate;
    long long m_groupID;
    std::vector<SessionTimestampRange> m_timeRanges;
    std::optional<GeneralData> m_legacyData;
    bool m_dataIsOverride = false;
    GeneralData m_data;

    void recordTimestamp(long long timestamp);
};

template <>
struct matjson::Serialize<SessionTimestampRange> {
    static Result<SessionTimestampRange> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const SessionTimestampRange& value);
};

template <>
struct matjson::Serialize<Session> {
    static Result<Session> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Session& value);
};

typedef struct Section {
    std::string m_name;
    int m_startPercent;
    int m_endPercent;

    bool isPercentInSection(int percent) const;

    bool isValid() const;

    bool operator==(const Section& other) const;

} Section;

template <>
struct matjson::Serialize<Section> {
    static Result<Section> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Section& value);
};

typedef struct SessionGrouping {
    std::map<long long, std::set<std::string>, std::greater<long long>> m_group;
} SessionGrouping;

template <>
struct matjson::Serialize<std::map<long long, std::set<std::string>, std::greater<long long>>> {
    static Result<std::map<long long, std::set<std::string>, std::greater<long long>>> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const std::map<long long, std::set<std::string>, std::greater<long long>>& value);
};

template <>
struct matjson::Serialize<SessionGrouping> {
    static Result<SessionGrouping> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const SessionGrouping& value);
};

template <>
struct matjson::Serialize<std::map<long long, SessionGrouping, std::greater<long long>>> {
    static Result<std::map<long long, SessionGrouping, std::greater<long long>>> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const std::map<long long, SessionGrouping, std::greater<long long>>& value);
};

typedef struct SessionCategory {
    std::string m_groupName;
    std::map<long long, SessionGrouping, std::greater<long long>> m_grouping;
} SessionCategory;

template <>
struct matjson::Serialize<SessionCategory> {
    static Result<SessionCategory> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const SessionCategory& value);
};

typedef struct GradientData {
    ccColor4B m_colorA = {255, 255, 255, 255};
    ccColor4B m_colorB = {255, 255, 255, 255};
    float m_duration = 1;
    float m_stagger = 0.2f;
} GradientData;

typedef struct WaveData {
    float m_speed = 1;
    float m_distanceY = 5;
    float m_distanceX = 0;
    float m_offsetPerIndex = .3f;
} WaveData;

typedef struct CustomizationData {
    ccColor4B m_color;

    bool m_gradientEnabled;
    GradientData m_gradient;

    bool m_waveEnabled;
    WaveData m_wave;
} CustomizationData;

template <>
struct matjson::Serialize<GradientData> {
    static Result<GradientData> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const GradientData& value);
};

template <>
struct matjson::Serialize<WaveData> {
    static Result<WaveData> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const WaveData& value);
};

template <>
struct matjson::Serialize<CustomizationData> {
    static Result<CustomizationData> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const CustomizationData& value);
};


typedef struct LevelMetadata {
    std::map<int, int> m_runsToShow{};
    int m_sharedRunToShow = 0;
    bool m_showAnyRun = true;
    std::set<std::string> m_linkedLevels{};
    std::string m_levelName = "Unknown name";
    int m_attempts = 0;
    int m_difficulty = 0;
    int m_hideUpto = 0;
    int m_realEndPercent = 100;
    bool m_resetAsDeath = true;
    bool m_autoBackup = true;
    std::vector<Section> m_sections{};
    bool m_hasGottenDataFromPT = false;
    std::vector<SessionCategory> m_sessionGroups;
} LevelMetadata;

template <>
struct matjson::Serialize<std::map<int, int>> {
    static Result<std::map<int, int>> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const std::map<int, int>& value);
};

template <>
struct matjson::Serialize<LevelMetadata> {
    static Result<LevelMetadata> fromJson(const matjson::Value& value) {

        LevelMetadata stats;
        if (value.contains("RunsToSave")) {
            auto oldRunsToSave = value["RunsToSave"].as<std::set<int>>().unwrapOr(std::set<int>{});

            for (const auto& runToSave : oldRunsToSave)
                stats.m_runsToShow.insert({runToSave, runToSave});
        }
        else {
            GEODE_UNWRAP_INTO(stats.m_runsToShow, value["RunsToShow"].as<std::map<int, int>>());
        }
        if (value.contains("trackAnyRun")) {
            GEODE_UNWRAP_INTO(stats.m_showAnyRun, value["trackAnyRun"].asBool());
        }
        else {
            GEODE_UNWRAP_INTO(stats.m_showAnyRun, value["showAnyRun"].asBool());
        }

        GEODE_UNWRAP_INTO(stats.m_linkedLevels, value["LinkedLevels"].as<std::set<std::string>>());
        GEODE_UNWRAP_INTO(stats.m_levelName, value["levelName"].asString());
        GEODE_UNWRAP_INTO(stats.m_attempts, value["attempts"].asInt());
        GEODE_UNWRAP_INTO(stats.m_difficulty, value["difficulty"].asInt());
        GEODE_UNWRAP_INTO(stats.m_hideUpto, value["hideUpto"].asInt());
        GEODE_UNWRAP_INTO(stats.m_realEndPercent, value["realEndPercent"].asInt());
        GEODE_UNWRAP_INTO(stats.m_resetAsDeath, value["resetAsDeath"].asBool());
        if (value.contains("autoBackup")) {
            GEODE_UNWRAP_INTO(stats.m_autoBackup, value["autoBackup"].asBool());
        }
        if (value.contains("sections")) {
            GEODE_UNWRAP_INTO(stats.m_sections, value["sections"].as<std::vector<Section>>());
        }
        if (value.contains("hasGottenDataFromPT")) {
            GEODE_UNWRAP_INTO(stats.m_hasGottenDataFromPT, value["hasGottenDataFromPT"].asBool());
        }
        if (value.contains("sessionGroups")) {
            GEODE_UNWRAP_INTO(stats.m_sessionGroups, value["sessionGroups"].as<std::vector<SessionCategory>>());
        }
        if (value.contains("sharedRunToShow")) {
            GEODE_UNWRAP_INTO(stats.m_sharedRunToShow, value["sharedRunToShow"].asInt());
        }

        return Ok(stats);
    }

    static matjson::Value toJson(const LevelMetadata& value) {
        matjson::Value obj = matjson::makeObject({
            { "RunsToShow", value.m_runsToShow },
            { "showAnyRun", value.m_showAnyRun },
            { "LinkedLevels", value.m_linkedLevels },
            { "levelName", value.m_levelName },
            { "attempts", value.m_attempts },
            { "difficulty", value.m_difficulty },
            { "hideUpto", value.m_hideUpto },
            { "realEndPercent", value.m_realEndPercent },
            { "resetAsDeath", value.m_resetAsDeath },
            { "autoBackup", value.m_autoBackup },
            { "sections", value.m_sections },
            { "hasGottenDataFromPT", value.m_hasGottenDataFromPT },
            { "sessionGroups", value.m_sessionGroups },
            { "sharedRunToShow", value.m_sharedRunToShow }
        });
        return obj;
    }
};

struct LevelData {
    std::string m_levelKey;
    LevelMetadata m_metadata;
    Result<GeneralData, ErrorWithCode> m_from0 = Err(ErrorWithCode("Not loaded", 1));
    std::set<long long> m_sessionNames;

    LevelData() = default;
};

struct BackupLevelData {
    std::optional<LevelMetadata> m_meta;
    std::optional<GeneralData> m_from0 = std::nullopt;
    std::optional<std::set<long long>> m_sessionNames = std::nullopt;
    long long m_backupDate;
};

struct DTLabelInfo_s {
    int m_minPlacementRange;
    int m_maxPlacementRange;
    int m_layer;
    std::string m_labelName = "new label";
    std::string m_text = "";
    std::string m_font = "bigFont.fnt";
    CCTextAlignment m_horizontalAlignment = CCTextAlignment::kCCTextAlignmentCenter;
    ccColor4B m_labelColor = {170, 170, 170, 255};
    ccColor4B m_textColor = {255, 255, 255, 255};
    float m_scale = 1;
    bool m_isExpanded = false;
    WrappingMode m_wrapping = WrappingMode::CUTOFF_WRAP;
    bool m_hideHeader = false;

    static constexpr CCPoint MIN_MAX_SCALE = { 0.01f, 2.f };
};
typedef struct DTLabelInfo_s DTLabelInfo;

template <>
struct matjson::Serialize<DTLabelInfo> {
    static Result<DTLabelInfo> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTLabelInfo& value);
};

struct DTColumnInfo_s {
    static float minWidth;

    int m_orderPos = 0;
    float m_currentWidth = minWidth;
    ccColor3B m_color = {255, 255, 255};
};
typedef struct DTColumnInfo_s DTColumnInfo;

template <>
struct matjson::Serialize<DTColumnInfo> {
    static Result<DTColumnInfo> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTColumnInfo& value);
};

struct DTLayoutV3_s {
    std::vector<DTColumnInfo> m_columns;
    std::vector<DTLabelInfo> m_labels;

    bool isEmpty() const;
};
typedef struct DTLayoutV3_s DTLayoutV3;

template <>
struct matjson::Serialize<DTLayoutV3> {
    static Result<DTLayoutV3> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTLayoutV3& value);
};

struct DTLayoutPreset {
    std::string m_name;
    DTLayoutV3 m_layout;

    CustomizationData m_newBestCustomization;
    CustomizationData m_sessionBestCustomization;
};

template <>
struct matjson::Serialize<DTLayoutPreset> {
    static Result<DTLayoutPreset> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTLayoutPreset& value);
};

struct OverlayLabel_s {
    std::string m_labelName = "Label";
    std::string m_text = "New Label";

    float m_posX = .5f;
    float m_posY = .5f;
    geode::Anchor m_anchor = geode::Anchor::TopLeft;
    bool m_freeMode = false;
    int m_order = 0;
    bool m_enabled = true;

    std::optional<std::string> m_levelSpecific = std::nullopt;

    bool isOverlayLabelVisibleOnLevel(GJGameLevel* level) const;
};
typedef struct OverlayLabel_s OverlayLabel;

template <>
struct matjson::Serialize<OverlayLabel> {
    static Result<OverlayLabel> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const OverlayLabel& value);
};

struct OverlayLabelSettings_s {
    static constexpr float MIN_SIZE = .01f;
    static constexpr float MAX_SIZE = 2.f;
    static constexpr int MIN_OPACITY = 0;
    static constexpr int MAX_OPACITY = 255;

    std::vector<OverlayLabel> m_labels;

    float m_size = 0.35f;
    int m_opacity = 125;
    CCPoint m_offset = ccp(1, 1) * 5;
    std::string m_font = "bigFont.fnt";
};
typedef struct OverlayLabelSettings_s OverlayLabelSettings;

template <>
struct matjson::Serialize<OverlayLabelSettings> {
    static Result<OverlayLabelSettings> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const OverlayLabelSettings& value);
};

typedef struct V2LabelLayout {
    std::string m_labelName;
    std::string m_text;
    int m_line;
    int m_position;
    ccColor4B m_color;
    CCTextAlignment m_alignment;
    int m_font;
    float m_fontSize;
} V2LabelLayout;

template <>
struct matjson::Serialize<V2LabelLayout> {
    static Result<V2LabelLayout> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const V2LabelLayout& value);
};

struct stringCustomazations{
    std::string m_seperator = "{nl}";
    std::string m_format = "{per}% X{d}";
    int m_alternateStrength = 25;
};

template <>
struct matjson::Serialize<stringCustomazations> {
    static Result<stringCustomazations> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const stringCustomazations& value);
};

typedef struct V2Session {
    long long m_lastPlayed;
    Deaths m_deaths;
    Deaths m_runs;
    NewBests m_newBests;
    int m_currentBest;
    long long m_sessionStartDate;
} V2Session;

template <>
struct matjson::Serialize<V2Session> {
    static Result<V2Session> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const V2Session& value);
};

struct V2LevelStats_s {
    Deaths m_deaths;
    Deaths m_runs;
    NewBests m_newBests;
    int m_currentBest;
    std::vector<V2Session> m_sessions;
    std::vector<int> m_RunsToSave{-1};
    std::vector<std::string> m_LinkedLevels;
    std::string m_levelName = "Unknown name";
    int m_attempts;
    int m_difficulty;
    int m_hideUpto;
    int m_hideRunLength;
};
typedef struct V2LevelStats_s V2LevelStats;

template <>
struct matjson::Serialize<V2LevelStats> {
    static Result<V2LevelStats> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const V2LevelStats& value);
};

enum DTGraphCoverage{
    GeneralCover = 0,
    GeneralRunsCover = 1,
    SessionCover = 2,
    SessionRunsCover = 3,
    SectionContentCover = 4,
    SectionGeneralCover = 5,
    SectionSessionCover = 6
};

enum DTGraphType{
    Passrate,
    Reachrate
};

struct DTGraphInfo {
    bool m_isEnabled = true;

    DTGraphCoverage m_coverage = DTGraphCoverage::GeneralCover;
    DTGraphType m_type = DTGraphType::Passrate;

    int m_orderPos;

    float m_thickness = 1.0f;
    float m_outlineThickness = 0.75f;
    float m_pointScale = 0.1f;

    ccColor4B m_color = {255, 255, 255, 255};
    ccColor4B m_outlineColor = { 113, 113, 113, 255 };
    ccColor4B m_pointColor = { 113, 113, 113, 255 };

    std::string m_name;
};

template <>
struct matjson::Serialize<DTGraphInfo> {
    static Result<DTGraphInfo> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTGraphInfo& value);
};
