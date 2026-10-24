#pragma once

#include <Geode/Geode.hpp>
#include <matjson/stl_serialize.hpp>

using namespace geode::prelude;

typedef std::map<std::string, int> Deaths;
typedef std::set<int> NewBests;

class SaveDeletionEvent : public Event<SaveDeletionEvent, bool()> {
public:
    using Event::Event;
};

struct ErrorWithCode{
    std::string error;
    int code;

    ErrorWithCode() = default;

    ErrorWithCode(std::string error_, bool code_ = 0)
        : error(std::move(error_))
        , code(code_)
    {}

    ErrorWithCode(const char* error_, bool code_ = 0)
        : error(error_ ? error_ : "")
        , code(code_)
    {}

    friend std::ostream& operator<<(std::ostream& os, ErrorWithCode const& err) {
        return os << err.error;
    }
};

struct Run_s{
    std::optional<int> start;
    int end;

    Run_s(std::optional<int> _start, int _end) : start(_start), end(_end) {}

    Run_s() = default;
};
typedef struct Run_s Run;

typedef struct PlaytimePair {
    uint64_t playtimeF0 = 0;
    uint64_t playtimeRuns = 0;

    PlaytimePair& operator+=(const PlaytimePair& other);
    void reset();
} PlaytimePair;

template <>
struct matjson::Serialize<PlaytimePair> {
    static Result<PlaytimePair> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const PlaytimePair& value);
};

typedef struct GeneralData {
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    PlaytimePair playtimeGeneral;
    PlaytimePair playtimePaused;
    PlaytimePair playtimeDead;

    GeneralData& operator+=(const GeneralData& other);
} GeneralData;

template <>
struct matjson::Serialize<GeneralData> {
    static Result<GeneralData> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const GeneralData& value);
};

typedef struct {
    std::string ownerLevelKey;
    long long lastPlayed;
    long long sessionStartDate;
    long long groupID;
    GeneralData data;
} Session;

template <>
struct matjson::Serialize<Session> {
    static Result<Session> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const Session& value);
};

typedef struct Section {
    std::string name;
    int startPercent;
    int endPercent;

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
    std::map<long long, std::set<std::string>, std::greater<long long>> group;
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
    std::string groupName;
    std::map<long long, SessionGrouping, std::greater<long long>> grouping;
} SessionCategory;

template <>
struct matjson::Serialize<SessionCategory> {
    static Result<SessionCategory> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const SessionCategory& value);
};

typedef struct GradientData {
    ccColor4B colorA = {255, 255, 255, 255};
    ccColor4B colorB = {255, 255, 255, 255};
    float duration = 1;
    float stagger = 0.2f;
} GradientData;

typedef struct WaveData {
    float speed = 1;
    float distanceY = 5;
    float distanceX = 0;
    float offsetPerIndex = .3f;
} WaveData;

typedef struct CustomizationData {
    ccColor4B color;

    bool gradientEnabled;
    GradientData gradient;

    bool waveEnabled;
    WaveData wave;
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


typedef struct LevelMetadeta {
    std::map<int, int> runsToShow{};
    int sharedRunToShow = 0;
    bool showAnyRun = true;
    std::set<std::string> linkedLevels{};
    std::string levelName = "Unknown name";
    int attempts = 0;
    int difficulty = 0;
    int hideUpto = 0;
    int realEndPercent = 100;
    bool resetAsDeath = true;
    bool autoBackup = true;
    std::vector<Section> sections{};
    bool hasGottenDataFromPT = false;
    std::vector<SessionCategory> sessionGroups;
} LevelMetadeta;

template <>
struct matjson::Serialize<std::map<int, int>> {
    static Result<std::map<int, int>> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const std::map<int, int>& value);
};

template <>
struct matjson::Serialize<LevelMetadeta> {
    static Result<LevelMetadeta> fromJson(const matjson::Value& value) {

        LevelMetadeta stats;
        if (value.contains("RunsToSave")) {
            auto oldRunsToSave = value["RunsToSave"].as<std::set<int>>().unwrapOr(std::set<int>{});

            for (const auto& runToSave : oldRunsToSave)
                stats.runsToShow.insert({runToSave, runToSave});
        }
        else {
            GEODE_UNWRAP_INTO(stats.runsToShow, value["RunsToShow"].as<std::map<int, int>>());
        }
        if (value.contains("trackAnyRun")) {
            GEODE_UNWRAP_INTO(stats.showAnyRun, value["trackAnyRun"].asBool());
        }
        else {
            GEODE_UNWRAP_INTO(stats.showAnyRun, value["showAnyRun"].asBool());
        }

        GEODE_UNWRAP_INTO(stats.linkedLevels, value["LinkedLevels"].as<std::set<std::string>>());
        GEODE_UNWRAP_INTO(stats.levelName, value["levelName"].asString());
        GEODE_UNWRAP_INTO(stats.attempts, value["attempts"].asInt());
        GEODE_UNWRAP_INTO(stats.difficulty, value["difficulty"].asInt());
        GEODE_UNWRAP_INTO(stats.hideUpto, value["hideUpto"].asInt());
        GEODE_UNWRAP_INTO(stats.realEndPercent, value["realEndPercent"].asInt());
        GEODE_UNWRAP_INTO(stats.resetAsDeath, value["resetAsDeath"].asBool());
        if (value.contains("autoBackup")) {
            GEODE_UNWRAP_INTO(stats.autoBackup, value["autoBackup"].asBool());
        }
        if (value.contains("sections")) {
            GEODE_UNWRAP_INTO(stats.sections, value["sections"].as<std::vector<Section>>());
        }
        if (value.contains("hasGottenDataFromPT")) {
            GEODE_UNWRAP_INTO(stats.hasGottenDataFromPT, value["hasGottenDataFromPT"].asBool());
        }
        if (value.contains("sessionGroups")) {
            GEODE_UNWRAP_INTO(stats.sessionGroups, value["sessionGroups"].as<std::vector<SessionCategory>>());
        }
        if (value.contains("sharedRunToShow")) {
            GEODE_UNWRAP_INTO(stats.sharedRunToShow, value["sharedRunToShow"].asInt());
        }

        return Ok(stats);
    }

    static matjson::Value toJson(const LevelMetadeta& value) {
        matjson::Value obj = matjson::makeObject({
            { "RunsToShow", value.runsToShow },
            { "showAnyRun", value.showAnyRun },
            { "LinkedLevels", value.linkedLevels },
            { "levelName", value.levelName },
            { "attempts", value.attempts },
            { "difficulty", value.difficulty },
            { "hideUpto", value.hideUpto },
            { "realEndPercent", value.realEndPercent },
            { "resetAsDeath", value.resetAsDeath },
            { "autoBackup", value.autoBackup },
            { "sections", value.sections },
            { "hasGottenDataFromPT", value.hasGottenDataFromPT },
            { "sessionGroups", value.sessionGroups },
            { "sharedRunToShow", value.sharedRunToShow }
        });
        return obj;
    }
};

struct LevelData {
    std::string levelKey;
    LevelMetadeta metadata;
    Result<GeneralData, ErrorWithCode> from0 = Err(ErrorWithCode("Not loaded", 1));
    std::set<long long> sessionNames;

    LevelData() = default;
};

struct BackupLevelData {
    std::optional<LevelMetadeta> meta;
    std::optional<GeneralData> from0 = std::nullopt;
    std::optional<std::set<long long>> sessionNames = std::nullopt;
    long long backupDate;
};

struct DTLabelInfo_s {
    int minPlacementRange;
    int maxPlacementRange;
    int layer;
    std::string labelName = "new label";
    std::string text = "";
    std::string font = "bigFont.fnt";
    CCTextAlignment horizontalAlignment = CCTextAlignment::kCCTextAlignmentCenter;
    ccColor4B labelColor = {170, 170, 170, 255};
    ccColor4B textColor = {255, 255, 255, 255};
    float scale = 1;
    bool isExpanded = false;
    WrappingMode wrapping = WrappingMode::CUTOFF_WRAP;
    bool hideHeader = false;

    static inline const CCPoint MIN_MAX_SCALE = ccp(0.01f, 2);
};
typedef struct DTLabelInfo_s DTLabelInfo;

template <>
struct matjson::Serialize<DTLabelInfo> {
    static Result<DTLabelInfo> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTLabelInfo& value);
};

struct DTColumnInfo_s {
    static float minWidth;

    int orderPos = 0;
    float currentWidth = minWidth;
    ccColor3B color = {255, 255, 255};
};
typedef struct DTColumnInfo_s DTColumnInfo;

template <>
struct matjson::Serialize<DTColumnInfo> {
    static Result<DTColumnInfo> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTColumnInfo& value);
};

struct DTLayoutV3_s {
    std::vector<DTColumnInfo> columns;
    std::vector<DTLabelInfo> labels;

    bool isEmpty() const;
};
typedef struct DTLayoutV3_s DTLayoutV3;

template <>
struct matjson::Serialize<DTLayoutV3> {
    static Result<DTLayoutV3> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTLayoutV3& value);
};

struct DTLayoutPreset {
    std::string name;
    DTLayoutV3 layout;

    CustomizationData newBestCustomization;
    CustomizationData sessionBestCustomization;
};

template <>
struct matjson::Serialize<DTLayoutPreset> {
    static Result<DTLayoutPreset> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTLayoutPreset& value);
};

typedef struct {
    std::string labelName;
    std::string text;
    float posX;
    float posY;
    geode::Anchor anchor;
    bool freeMode;
    int order;
} OverlayLabel;

typedef struct {
    std::string labelName;
    std::string text;
    int line;
    int position;
    ccColor4B color;
    CCTextAlignment alignment;
    int font;
    float fontSize;
} V2LabelLayout;

template <>
struct matjson::Serialize<V2LabelLayout> {
    static Result<V2LabelLayout> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const V2LabelLayout& value);
};

struct stringCustomazations{
    std::string seperator = "{nl}";
    std::string format = "{per}% X{d}";
    int alternateStrength = 25;
};

template <>
struct matjson::Serialize<stringCustomazations> {
    static Result<stringCustomazations> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const stringCustomazations& value);
};

typedef struct {
    long long lastPlayed;
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    int currentBest;
    long long sessionStartDate;
} V2Session;

template <>
struct matjson::Serialize<V2Session> {
    static Result<V2Session> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const V2Session& value);
};

struct V2LevelStats_s {
    Deaths deaths;
    Deaths runs;
    NewBests newBests;
    int currentBest;
    std::vector<V2Session> sessions;
    std::vector<int> RunsToSave{-1};
    std::vector<std::string> LinkedLevels;
    std::string levelName = "Unknown name";
    int attempts;
    int difficulty;
    int hideUpto;
    int hideRunLength;
};
typedef struct V2LevelStats_s V2LevelStats;

template <>
struct matjson::Serialize<V2LevelStats> {
    static Result<V2LevelStats> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const V2LevelStats& value);
};

enum DTGraphCoverage{
    GeneralCover,
    GeneralRunsCover,
    SessionCover,
    SessionRunsCover,
    SectionCover
};

enum DTGraphType{
    Passrate,
    Reachrate
};

struct DTGraphInfo {
    bool isEnabled = true;

    DTGraphCoverage coverage = DTGraphCoverage::GeneralCover;
    DTGraphType type = DTGraphType::Passrate;

    int orderPos;

    float thickness = 1.0f;
    float outlineThickness = 0.75f;
    float pointScale = 0.1f;

    ccColor4B color = {255, 255, 255, 255};
    ccColor4B outlineColor = { 113, 113, 113, 255 };
    ccColor4B pointColor = { 113, 113, 113, 255 };

    std::string name;
};

template <>
struct matjson::Serialize<DTGraphInfo> {
    static Result<DTGraphInfo> fromJson(const matjson::Value& value);
    static matjson::Value toJson(const DTGraphInfo& value);
};
