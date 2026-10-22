#include "DTTypes.hpp"

float DTColumnInfo_s::minWidth = 40.0f;

PlaytimePair& PlaytimePair::operator+=(const PlaytimePair& other) {
    playtimeF0 += other.playtimeF0;
    playtimeRuns += other.playtimeRuns;
    return *this;
}

void PlaytimePair::reset() {
    playtimeF0 = 0;
    playtimeRuns = 0;
}

GeneralData& GeneralData::operator+=(const GeneralData& other) {
    for (auto const& [key, val] : other.deaths) {
        deaths[key] += val;
    }
    for (auto const& [key, val] : other.runs) {
        runs[key] += val;
    }
    newBests.insert(other.newBests.begin(), other.newBests.end());
    playtimeGeneral += other.playtimeGeneral;
    playtimePaused += other.playtimePaused;
    playtimeDead += other.playtimeDead;
    return *this;
}

bool Section::isPercentInSection(int percent) const {
    return percent >= startPercent && percent < endPercent ||
        percent >= startPercent && endPercent == 100;
}

bool Section::isValid() const {
    return !(startPercent > endPercent);
}

bool Section::operator==(const Section& other) const {
    return startPercent == other.startPercent && endPercent == other.endPercent;
}

bool DTLayoutV3::isEmpty() const {
    return columns.empty() && labels.empty();
}

Result<PlaytimePair> matjson::Serialize<PlaytimePair>::fromJson(const matjson::Value& value) {
    PlaytimePair pair;
    GEODE_UNWRAP_INTO(pair.playtimeF0, value["playtimeF0"].as<uint64_t>());
    GEODE_UNWRAP_INTO(pair.playtimeRuns, value["playtimeRuns"].as<uint64_t>());
    return Ok(pair);
}

matjson::Value matjson::Serialize<PlaytimePair>::toJson(const PlaytimePair& value) {
    return matjson::makeObject({
        { "playtimeF0", value.playtimeF0 },
        { "playtimeRuns", value.playtimeRuns }
    });
}

Result<GeneralData> matjson::Serialize<GeneralData>::fromJson(const matjson::Value& value) {
    GeneralData stats;
    GEODE_UNWRAP_INTO(stats.deaths, value["deaths"].as<Deaths>());
    GEODE_UNWRAP_INTO(stats.runs, value["runs"].as<Deaths>());
    GEODE_UNWRAP_INTO(stats.newBests, value["newBests"].as<NewBests>());
    if (value.contains("playtimeGeneral")) {
        GEODE_UNWRAP_INTO(stats.playtimeGeneral, value["playtimeGeneral"].as<PlaytimePair>());
    }
    if (value.contains("playtimePaused")) {
        GEODE_UNWRAP_INTO(stats.playtimePaused, value["playtimePaused"].as<PlaytimePair>());
    }
    if (value.contains("playtimeDead")) {
        GEODE_UNWRAP_INTO(stats.playtimeDead, value["playtimeDead"].as<PlaytimePair>());
    }
    return Ok(stats);
}

matjson::Value matjson::Serialize<GeneralData>::toJson(const GeneralData& value) {
    return matjson::makeObject({
        { "deaths", value.deaths },
        { "runs", value.runs },
        { "newBests", value.newBests },
        { "playtimeGeneral", value.playtimeGeneral },
        { "playtimePaused", value.playtimePaused },
        { "playtimeDead", value.playtimeDead }
    });
}

Result<Session> matjson::Serialize<Session>::fromJson(const matjson::Value& value) {
    Session session;
    GEODE_UNWRAP_INTO(session.lastPlayed, value["lastPlayed"].as<long long>());
    GEODE_UNWRAP_INTO(session.sessionStartDate, value["sessionStartDate"].as<long long>());
    if (value.contains("newBests")) {
        GEODE_UNWRAP_INTO(session.data.newBests, value["newBests"].as<NewBests>());
    }
    if (value.contains("deaths")) {
        GEODE_UNWRAP_INTO(session.data.deaths, value["deaths"].as<Deaths>());
    }
    if (value.contains("runs")) {
        GEODE_UNWRAP_INTO(session.data.runs, value["runs"].as<Deaths>());
    }
    if (value.contains("playtimeGeneral")) {
        GEODE_UNWRAP_INTO(session.data.playtimeGeneral, value["playtimeGeneral"].as<PlaytimePair>());
    }
    if (value.contains("playtimePaused")) {
        GEODE_UNWRAP_INTO(session.data.playtimePaused, value["playtimePaused"].as<PlaytimePair>());
    }
    if (value.contains("playtimeDead")) {
        GEODE_UNWRAP_INTO(session.data.playtimeDead, value["playtimeDead"].as<PlaytimePair>());
    }
    if (value.contains("data")) {
        GEODE_UNWRAP_INTO(session.data, value["data"].as<GeneralData>());
    }
    return Ok(session);
}

matjson::Value matjson::Serialize<Session>::toJson(const Session& value) {
    return matjson::makeObject({
        { "lastPlayed", value.lastPlayed },
        { "sessionStartDate", value.sessionStartDate },
        { "data", value.data }
    });
}

Result<Section> matjson::Serialize<Section>::fromJson(const matjson::Value& value) {
    Section section;
    GEODE_UNWRAP_INTO(section.name, value["name"].asString());
    GEODE_UNWRAP_INTO(section.startPercent, value["start"].asInt());
    GEODE_UNWRAP_INTO(section.endPercent, value["end"].asInt());
    return Ok(section);
}

matjson::Value matjson::Serialize<Section>::toJson(const Section& value) {
    return matjson::makeObject({
        { "name", value.name },
        { "start", value.startPercent },
        { "end", value.endPercent }
    });
}

Result<std::map<long long, std::set<std::string>, std::greater<long long>>> matjson::Serialize<std::map<long long, std::set<std::string>, std::greater<long long>>>::fromJson(const matjson::Value& value) {
    std::map<long long, std::set<std::string>, std::greater<long long>> resMap;
    auto objRes = value.as<std::map<std::string, std::set<std::string>>>().unwrapOr(std::map<std::string, std::set<std::string>>{});
    for (auto const& [strKey, strSet] : objRes) {
        auto res = geode::utils::numFromString<long long>(strKey);
        if (res.isErr()) continue;
        resMap.insert({res.unwrap(), strSet});
    }
    return Ok(resMap);
}

matjson::Value matjson::Serialize<std::map<long long, std::set<std::string>, std::greater<long long>>>::toJson(const std::map<long long, std::set<std::string>, std::greater<long long>>& value) {
    std::map<std::string, std::set<std::string>> obj;
    for (auto const& kv : value) {
        obj[std::to_string(kv.first)] = kv.second;
    }
    return matjson::Value(obj);
}

Result<SessionGrouping> matjson::Serialize<SessionGrouping>::fromJson(const matjson::Value& value) {
    SessionGrouping grouping;
    GEODE_UNWRAP_INTO(grouping.group, value["group"].as<std::map<long long, std::set<std::string>, std::greater<long long>>>());
    return Ok(grouping);
}

matjson::Value matjson::Serialize<SessionGrouping>::toJson(const SessionGrouping& value) {
    return matjson::makeObject({
        { "group", value.group }
    });
}

Result<std::map<long long, SessionGrouping, std::greater<long long>>> matjson::Serialize<std::map<long long, SessionGrouping, std::greater<long long>>>::fromJson(const matjson::Value& value) {
    std::map<long long, SessionGrouping, std::greater<long long>> resMap;
    auto objRes = value.as<std::map<std::string, SessionGrouping>>().unwrapOr(std::map<std::string, SessionGrouping>{});
    for (auto const& [strKey, sessionGrouping] : objRes) {
        auto res = geode::utils::numFromString<long long>(strKey);
        if (res.isErr()) continue;
        resMap.insert({res.unwrap(), sessionGrouping});
    }
    return Ok(resMap);
}

matjson::Value matjson::Serialize<std::map<long long, SessionGrouping, std::greater<long long>>>::toJson(const std::map<long long, SessionGrouping, std::greater<long long>>& value) {
    std::map<std::string, SessionGrouping> obj;
    for (auto const& kv : value) {
        obj[std::to_string(kv.first)] = kv.second;
    }
    return matjson::Value(obj);
}

Result<SessionCategory> matjson::Serialize<SessionCategory>::fromJson(const matjson::Value& value) {
    SessionCategory category;
    GEODE_UNWRAP_INTO(category.groupName, value["groupName"].asString());
    GEODE_UNWRAP_INTO(category.grouping, value["grouping"].as<std::map<long long, SessionGrouping, std::greater<long long>>>());
    return Ok(category);
}

matjson::Value matjson::Serialize<SessionCategory>::toJson(const SessionCategory& value) {
    return matjson::makeObject({
        { "groupName", value.groupName },
        { "grouping", value.grouping }
    });
}

Result<GradientData> matjson::Serialize<GradientData>::fromJson(const matjson::Value& value) {
    GradientData data;
    GEODE_UNWRAP_INTO(data.colorA, value["colorA"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(data.colorB, value["colorB"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(data.duration, value["duration"].asDouble());
    GEODE_UNWRAP_INTO(data.stagger, value["stagger"].asDouble());
    return Ok(data);
}

matjson::Value matjson::Serialize<GradientData>::toJson(const GradientData& value) {
    return matjson::makeObject({
        { "colorA", value.colorA },
        { "colorB", value.colorB },
        { "duration", value.duration },
        { "stagger", value.stagger }
    });
}

Result<WaveData> matjson::Serialize<WaveData>::fromJson(const matjson::Value& value) {
    WaveData data;
    GEODE_UNWRAP_INTO(data.speed, value["speed"].asDouble());
    GEODE_UNWRAP_INTO(data.distanceY, value["distanceY"].asDouble());
    GEODE_UNWRAP_INTO(data.distanceX, value["distanceX"].asDouble());
    GEODE_UNWRAP_INTO(data.offsetPerIndex, value["offsetPerIndex"].asDouble());
    return Ok(data);
}

matjson::Value matjson::Serialize<WaveData>::toJson(const WaveData& value) {
    return matjson::makeObject({
        { "speed", value.speed },
        { "distanceY", value.distanceY },
        { "distanceX", value.distanceX },
        { "offsetPerIndex", value.offsetPerIndex }
    });
}

Result<CustomizationData> matjson::Serialize<CustomizationData>::fromJson(const matjson::Value& value) {
    CustomizationData data;
    GEODE_UNWRAP_INTO(data.color, value["color"].as<ccColor4B>());

    GEODE_UNWRAP_INTO(data.gradientEnabled, value["gradientEnabled"].asBool());
    GEODE_UNWRAP_INTO(data.gradient, value["gradient"].as<GradientData>());

    GEODE_UNWRAP_INTO(data.waveEnabled, value["waveEnabled"].asBool());
    GEODE_UNWRAP_INTO(data.wave, value["wave"].as<WaveData>());

    return Ok(data);
}

matjson::Value matjson::Serialize<CustomizationData>::toJson(const CustomizationData& value) {
    return matjson::makeObject({
        { "color", value.color },
        { "gradientEnabled", value.gradientEnabled },
        { "gradient", value.gradient },
        { "waveEnabled", value.waveEnabled },
        { "wave", value.wave }
    });
}

Result<std::map<int, int>> matjson::Serialize<std::map<int, int>>::fromJson(const matjson::Value& value) {
    std::map<int, int> resMap;
    auto objRes = value.as<std::map<std::string, int>>().unwrapOr(std::map<std::string, int>{});
    for (auto const& [strFirst, second] : objRes) {
        auto res = geode::utils::numFromString<int>(strFirst);
        if (res.isErr()) continue;
        resMap.insert({res.unwrap(), second});
    }
    return Ok(resMap);
}

matjson::Value matjson::Serialize<std::map<int, int>>::toJson(const std::map<int, int>& value) {
    std::map<std::string, int> obj;
    for (auto const& kv : value) {
        obj[std::to_string(kv.first)] = kv.second;
    }
    return matjson::Value(obj);
}

Result<DTLabelInfo> matjson::Serialize<DTLabelInfo>::fromJson(const matjson::Value& value) {
    DTLabelInfo info;
    GEODE_UNWRAP_INTO(info.minPlacementRange, value["minPlacementRange"].asInt());
    GEODE_UNWRAP_INTO(info.maxPlacementRange, value["maxPlacementRange"].asInt());
    GEODE_UNWRAP_INTO(info.layer, value["layer"].asInt());
    GEODE_UNWRAP_INTO(info.labelName, value["labelName"].asString());
    GEODE_UNWRAP_INTO(info.text, value["text"].asString());
    GEODE_UNWRAP_INTO(info.font, value["font"].asString());
    GEODE_UNWRAP_INTO(auto horizontalAlignment, value["horizontalAlignment"].asInt());
    info.horizontalAlignment = static_cast<CCTextAlignment>(horizontalAlignment);
    GEODE_UNWRAP_INTO(info.labelColor, value["labelColor"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.textColor, value["color"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.scale, value["scale"].asDouble());
    GEODE_UNWRAP_INTO(info.isExpanded, value["isExpanded"].asBool());
    if (value.contains("wrapping")) {
        GEODE_UNWRAP_INTO(auto wrapping, value["wrapping"].asInt());
        info.wrapping = static_cast<WrappingMode>(wrapping);
    }
    if (value.contains("hideHeader")) {
        GEODE_UNWRAP_INTO(info.hideHeader, value["hideHeader"].asBool());
    }
    return Ok(info);
}

matjson::Value matjson::Serialize<DTLabelInfo>::toJson(const DTLabelInfo& value) {
    return matjson::makeObject({
        { "minPlacementRange", value.minPlacementRange },
        { "maxPlacementRange", value.maxPlacementRange },
        { "layer", value.layer },
        { "labelName", value.labelName },
        { "text", value.text },
        { "font", value.font },
        { "horizontalAlignment", static_cast<int>(value.horizontalAlignment) },
        { "color", value.textColor },
        { "labelColor", value.labelColor },
        { "scale", value.scale },
        { "isExpanded", value.isExpanded },
        { "wrapping", static_cast<int>(value.wrapping) },
        { "hideHeader", value.hideHeader }
    });
}

Result<DTColumnInfo> matjson::Serialize<DTColumnInfo>::fromJson(const matjson::Value& value) {
    DTColumnInfo info;
    GEODE_UNWRAP_INTO(info.orderPos, value["orderPos"].asInt());
    GEODE_UNWRAP_INTO(info.currentWidth, value["currentWidth"].asDouble());
    GEODE_UNWRAP_INTO(info.color, value["color"].as<ccColor3B>());
    return Ok(info);
}

matjson::Value matjson::Serialize<DTColumnInfo>::toJson(const DTColumnInfo& value) {
    return matjson::makeObject({
        { "orderPos", value.orderPos },
        { "currentWidth", value.currentWidth },
        { "color", value.color }
    });
}

Result<DTLayoutV3> matjson::Serialize<DTLayoutV3>::fromJson(const matjson::Value& value) {
    DTLayoutV3 info;
    GEODE_UNWRAP_INTO(info.columns, value["columns"].as<std::vector<DTColumnInfo>>());
    GEODE_UNWRAP_INTO(info.labels, value["labels"].as<std::vector<DTLabelInfo>>());
    return Ok(info);
}

matjson::Value matjson::Serialize<DTLayoutV3>::toJson(const DTLayoutV3& value) {
    return matjson::makeObject({
        { "columns", value.columns },
        { "labels", value.labels }
    });
}

Result<DTLayoutPreset> matjson::Serialize<DTLayoutPreset>::fromJson(const matjson::Value& value) {
    DTLayoutPreset preset;
    GEODE_UNWRAP_INTO(preset.name, value["name"].asString());
    GEODE_UNWRAP_INTO(preset.layout, value["layout"].as<DTLayoutV3>());
    if (value.contains("newBestColor")) {
        GEODE_UNWRAP_INTO(preset.newBestColor, value["newBestColor"].as<ccColor3B>());
    }
    if (value.contains("sessionBestColor")) {
        GEODE_UNWRAP_INTO(preset.sessionBestColor, value["sessionBestColor"].as<ccColor3B>());
    }
    return Ok(preset);
}
matjson::Value matjson::Serialize<DTLayoutPreset>::toJson(const DTLayoutPreset& value) {
    return matjson::makeObject({
        { "name", value.name },
        { "layout", value.layout },
        { "newBestColor", value.newBestColor },
        { "sessionBestColor", value.sessionBestColor }
    });
}

Result<V2LabelLayout> matjson::Serialize<V2LabelLayout>::fromJson(const matjson::Value& value) {
    V2LabelLayout layout;
    GEODE_UNWRAP_INTO(layout.labelName, value["labelName"].asString());
    GEODE_UNWRAP_INTO(layout.text, value["text"].asString());
    GEODE_UNWRAP_INTO(layout.line, value["line"].asInt());
    GEODE_UNWRAP_INTO(layout.position, value["position"].asInt());
    GEODE_UNWRAP_INTO(layout.color, value["color"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(auto alignment, value["alignment"].asInt());
    layout.alignment = static_cast<CCTextAlignment>(alignment);
    GEODE_UNWRAP_INTO(layout.font, value["font"].asInt());
    GEODE_UNWRAP_INTO(layout.fontSize, value["fontSize"].asDouble());
    return Ok(layout);
}

matjson::Value matjson::Serialize<V2LabelLayout>::toJson(const V2LabelLayout& value) {
    return matjson::makeObject({
        { "labelName", value.labelName },
        { "text", value.text },
        { "line", value.line },
        { "position", value.position },
        { "color", value.color },
        { "alignment", static_cast<int>(value.alignment) },
        { "font", value.font },
        { "fontSize", value.fontSize }
    });
}

Result<stringCustomazations> matjson::Serialize<stringCustomazations>::fromJson(const matjson::Value& value) {
    stringCustomazations sc;
    GEODE_UNWRAP_INTO(sc.seperator, value["seperator"].asString());
    GEODE_UNWRAP_INTO(sc.format, value["format"].asString());
    if (value.contains("alternateStrength")) {
        GEODE_UNWRAP_INTO(sc.alternateStrength, value["alternateStrength"].asInt());
    }

    return Ok(sc);
}

matjson::Value matjson::Serialize<stringCustomazations>::toJson(const stringCustomazations& value) {
    return matjson::makeObject({
        { "seperator", value.seperator },
        { "format", value.format },
        { "alternateStrength", value.alternateStrength }
    });
}

Result<V2Session> matjson::Serialize<V2Session>::fromJson(const matjson::Value& value) {
    V2Session session;
    GEODE_UNWRAP_INTO(session.lastPlayed, value["lastPlayed"].as<long long>());
    GEODE_UNWRAP_INTO(session.deaths, value["deaths"].as<Deaths>());
    GEODE_UNWRAP_INTO(session.runs, value["runs"].as<Deaths>());
    GEODE_UNWRAP_INTO(session.newBests, value["newBests"].as<NewBests>());
    GEODE_UNWRAP_INTO(session.currentBest, value["currentBest"].asInt());
    if (value.contains("sessionStartDate")) {
        GEODE_UNWRAP_INTO(session.sessionStartDate, value["sessionStartDate"].as<long long>());
    } else {
        session.sessionStartDate = -1;
    }
    return Ok(session);
}

matjson::Value matjson::Serialize<V2Session>::toJson(const V2Session& value) {
    return matjson::makeObject({
        { "lastPlayed", value.lastPlayed },
        { "deaths", value.deaths },
        { "runs", value.runs },
        { "newBests", value.newBests },
        { "currentBest", value.currentBest },
        { "sessionStartDate", value.sessionStartDate }
    });
}

Result<V2LevelStats> matjson::Serialize<V2LevelStats>::fromJson(const matjson::Value& value) {
    V2LevelStats stats;
    GEODE_UNWRAP_INTO(stats.deaths, value["deaths"].as<Deaths>());
    GEODE_UNWRAP_INTO(stats.runs, value["runs"].as<Deaths>());
    GEODE_UNWRAP_INTO(stats.newBests, value["newBests"].as<NewBests>());
    GEODE_UNWRAP_INTO(stats.currentBest, value["currentBest"].asInt());
    GEODE_UNWRAP_INTO(stats.sessions, value["sessions"].as<std::vector<V2Session>>());
    if (value.contains("RunsToSave")) {
        GEODE_UNWRAP_INTO(stats.RunsToSave, value["RunsToSave"].as<std::vector<int>>());
    }
    if (value.contains("LinkedLevels")) {
        GEODE_UNWRAP_INTO(stats.LinkedLevels, value["LinkedLevels"].as<std::vector<std::string>>());
    } else {
        stats.LinkedLevels = value["LinkedLevels"].as<std::vector<std::string>>().unwrapOr(std::vector<std::string>{});
    }
    if (value.contains("levelName")) {
        GEODE_UNWRAP_INTO(stats.levelName, value["levelName"].asString());
    } else {
        stats.levelName = "-1";
    }
    if (value.contains("attempts")) {
        GEODE_UNWRAP_INTO(stats.attempts, value["attempts"].asInt());
    } else {
        stats.attempts = -1;
    }
    if (value.contains("difficulty")) {
        GEODE_UNWRAP_INTO(stats.difficulty, value["difficulty"].asInt());
    } else {
        stats.difficulty = 0;
    }
    if (value.contains("hideRunLength")) {
        GEODE_UNWRAP_INTO(stats.hideRunLength, value["hideRunLength"].asInt());
    } else {
        stats.hideRunLength = 0;
    }
    if (value.contains("hideUpto")) {
        GEODE_UNWRAP_INTO(stats.hideUpto, value["hideUpto"].asInt());
    } else {
        stats.hideUpto = 0;
    }
    return Ok(stats);
}

matjson::Value matjson::Serialize<V2LevelStats>::toJson(const V2LevelStats& value) {
    return matjson::makeObject({
        { "deaths", value.deaths },
        { "runs", value.runs },
        { "newBests", value.newBests },
        { "currentBest", value.currentBest },
        { "sessions", value.sessions },
        { "RunsToSave", value.RunsToSave },
        { "LinkedLevels", value.LinkedLevels },
        { "levelName", value.levelName },
        { "attempts", value.attempts },
        { "difficulty", value.difficulty },
        { "hideRunLength", value.hideRunLength },
        { "hideUpto", value.hideUpto }
    });
}

Result<DTGraphInfo> matjson::Serialize<DTGraphInfo>::fromJson(const matjson::Value& value) {
    DTGraphInfo info;
    GEODE_UNWRAP_INTO(auto coverageInt, value["coverage"].asInt());
    info.coverage = static_cast<DTGraphCoverage>(coverageInt);
    GEODE_UNWRAP_INTO(auto typeInt, value["type"].asInt());
    info.type = static_cast<DTGraphType>(typeInt);
    GEODE_UNWRAP_INTO(info.thickness, value["thickness"].asDouble());
    GEODE_UNWRAP_INTO(info.outlineThickness, value["outlineThickness"].asDouble());
    GEODE_UNWRAP_INTO(info.color, value["color"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.outlineColor, value["outlineColor"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.pointColor, value["pointColor"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.pointScale, value["pointScale"].asDouble());
    GEODE_UNWRAP_INTO(info.name, value["name"].asString());
    GEODE_UNWRAP_INTO(info.orderPos, value["orderPos"].asInt());
    GEODE_UNWRAP_INTO(info.isEnabled, value["isEnabled"].asBool());
    return Ok(info);
}

matjson::Value matjson::Serialize<DTGraphInfo>::toJson(const DTGraphInfo& value) {
    return matjson::makeObject({
        { "coverage", static_cast<int>(value.coverage) },
        { "type", static_cast<int>(value.type) },
        { "thickness", value.thickness },
        { "outlineThickness", value.outlineThickness },
        { "outlineColor", value.outlineColor },
        { "color", value.color },
        { "pointScale", value.pointScale },
        { "pointColor", value.pointColor },
        { "name", value.name },
        { "orderPos", value.orderPos },
        { "isEnabled", value.isEnabled }
    });
}
