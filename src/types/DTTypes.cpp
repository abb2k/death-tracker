#include "DTTypes.hpp"

#include <managers/StatsManager.hpp>

float DTColumnInfo_s::minWidth = 40.0f;

Result<ComplexDeathMap> ComplexDeathMap::readDeathMap(const matjson::Value& value, bool runKeys) {
    auto complex = value.as<ComplexDeathMap>();
    if (complex.isOk())
        return Ok(std::move(complex).unwrap());

    auto legacy = value.as<Deaths>();
    if (legacy.isErr())
        return Err("Invalid death map: {}", legacy.unwrapErr());

    return ComplexDeathMap::fromLegacy(legacy.unwrap(), runKeys);
}

Result<ComplexPercent> ComplexPercent::fromPercent(double percent) {

    auto scaled = static_cast<long double>(percent) * unitsPerPercent;
    auto minUnits = std::numeric_limits<Units>::min() + unitsPerPercent;
    auto maxUnits = std::numeric_limits<Units>::max() - unitsPerPercent;

    if (scaled < static_cast<long double>(minUnits) || scaled > static_cast<long double>(maxUnits))
        return Err("Percent outside range");

    return Ok(ComplexPercent{
        .units = static_cast<Units>(std::llround(scaled))
    });
}

double ComplexPercent::asPercent() const {
    return static_cast<double>(units) / unitsPerPercent;
}

bool ComplexPercent::isValid() const {
    return units >= std::numeric_limits<Units>::min() + unitsPerPercent && units <= std::numeric_limits<Units>::max() - unitsPerPercent;
}

Result<ComplexDeathMap> ComplexDeathMap::fromLegacy(const Deaths& deaths, bool runKeys) {
    ComplexDeathMap result;

    for (const auto& [key, count] : deaths) {
        if (count < 0)
            return Err("Negative death count in legacy death \"{}\"", key);
        if (count == 0)
            continue;

        std::optional<int> start;
        int end = 0;
        if (runKeys) {
            auto splitResult = StatsManager::splitRunKey(key);

            if (splitResult.isErr())
                return Err("Invalid legacy run \"{}\"", key);
            
            auto run = std::move(splitResult).unwrap();

            start = run.start;
            end = run.end;
        }
        else {
            auto runNum = geode::utils::numFromString<int>(key);
            if (runNum.isErr())
                return Err("Invalid legacy death percent \"{}\"", key);

            end = runNum.unwrap();
        }

        auto endPercent = ComplexPercent::fromPercent(end);
        if (endPercent.isErr())
            return Err("Invalid legacy run end in run \"{}\"", key);

        std::optional<ComplexPercent> startPercent;

        if (start.has_value()) {
            auto parsedStart = ComplexPercent::fromPercent(start.value());
            if (parsedStart.isErr())
                return Err("Invalid legacy run start in run \"{}\"", key);
            
            startPercent = parsedStart.unwrap();
        }

        ComplexDeath entry{
            .runStartPercent = startPercent,
            .runEndPercent = endPercent.unwrap(),
            .deaths = static_cast<uint64_t>(count)
        };

        auto insertResult = result.insert(std::move(entry));
        if (insertResult.isErr())
            return Err("Failed converting legacy run \"{}\": {}", key, insertResult.unwrapErr());
    }

    return Ok(std::move(result));
}

Result<> ComplexDeathMap::insert(ComplexDeath death) {
    if (!death.runEndPercent.isValid() || (death.runStartPercent.has_value() && !death.runStartPercent->isValid()))
        return Err("Cannot insert an invalid percentage");

    if (death.deaths == 0)
        return Ok();

    Run key {
        death.runEndPercent.units,
        death.runStartPercent.has_value() ? std::optional<ComplexPercent::Units>(death.runStartPercent->units) : std::nullopt
    };

    auto [it, inserted] = records.try_emplace(key, std::move(death));

    if (inserted)
        return Ok();

    auto& stored = it->second;
    auto& incoming = death;

    if (std::numeric_limits<uint64_t>::max() - stored.deaths < incoming.deaths)
        return Err("Death overflow");
    
    stored.deaths += incoming.deaths;
    stored.occurrences.insert(
        stored.occurrences.end(),
        std::make_move_iterator(incoming.occurrences.begin()),
        std::make_move_iterator(incoming.occurrences.end())
    );
    return Ok();
}

Result<> ComplexDeathMap::recordDeath(ComplexPercent endPercent, std::optional<ComplexPercent> startPercent, ComplexDeathOccurrence occurrence) {
    ComplexDeath death{
        .runStartPercent = startPercent,
        .runEndPercent = endPercent,
        .deaths = 1
    };

    if (occurrence.runStartTimestamp.has_value() || occurrence.runEndTimestamp.has_value())
        death.occurrences.push_back(std::move(occurrence));
    
    return insert(std::move(death));
}

Result<> ComplexDeathMap::recordDeath(double endPercent, std::optional<double> startPercent, ComplexDeathOccurrence occurrence) {
    auto end = ComplexPercent::fromPercent(endPercent);
    if (end.isErr())
        return Err("Invalid death end percent: {}", end.unwrapErr());

    std::optional<ComplexPercent> start;
    if (startPercent.has_value()) {
        auto parsedStart = ComplexPercent::fromPercent(startPercent.value());
        if (parsedStart.isErr())
            return Err("Invalid death start percent: {}", parsedStart.unwrapErr());
        start = parsedStart.unwrap();
    }

    return recordDeath(end.unwrap(), start, std::move(occurrence));
}

Result<ComplexDeathSummary> ComplexDeathMap::getComplexDeath(double percent, uint8_t decimalDepth) const {
    if (decimalDepth > ComplexPercent::maxDecimalDepth)
        return Err("Decimal depth must be between 0 and {}", ComplexPercent::maxDecimalDepth);

    auto parsedPercent = ComplexPercent::fromPercent(percent);
    if (parsedPercent.isErr())
        return Err("Invalid query percent: {}", parsedPercent.unwrapErr());

    ComplexPercent::Units factor = 1;
    for (auto depth = decimalDepth; depth < ComplexPercent::maxDecimalDepth; ++depth)
        factor *= 10;

    auto units = parsedPercent.unwrap().units;
    auto quotient = units / factor;

    if (units < 0 && units % factor != 0) quotient--;

    auto lower = quotient * factor;
    auto upper = lower + factor;

    ComplexDeathSummary summary;
    summary.queriedPercent.units = lower;
    summary.decimalDepth = decimalDepth;

    for (auto it = records.lower_bound({lower, std::nullopt}); it != records.end() && it->first.first < upper; ++it) {

        summary.records.push_back(it->second);
        if (std::numeric_limits<uint64_t>::max() - summary.deaths < it->second.deaths)
            return Err("death count overflow");

        summary.deaths += it->second.deaths;
    }

    return Ok(std::move(summary));
}

Result<ComplexDeathSummary> ComplexDeathMap::getComplexDeath(int percent) const {
    return getComplexDeath(static_cast<double>(percent), 0);
}

bool ComplexDeathMap::adjust(double endPercent, std::optional<double> startPercent, int amount) {
    if (amount == 0)
        return true;

    auto parsedEnd = ComplexPercent::fromPercent(endPercent);
    if (parsedEnd.isErr())
        return false;
    auto end = parsedEnd.unwrap();

    std::optional<ComplexPercent> start;
    if (startPercent) {
        auto parsedStart = ComplexPercent::fromPercent(*startPercent);
        if (parsedStart.isErr())
            return false;
        start = parsedStart.unwrap();
    }

    if (amount > 0) {
        return insert(ComplexDeath{
            .runStartPercent = start,
            .runEndPercent = end,
            .deaths = static_cast<uint64_t>(amount)
        }).isOk();
    }

    auto required = static_cast<uint64_t>(-static_cast<int64_t>(amount));
    Run key{
        end.units,
        start ? std::optional<ComplexPercent::Units>(start->units) : std::nullopt
    };
    auto it = records.find(key);
    if (it == records.end() || it->second.deaths < required)
        return false;

    it->second.deaths -= required;
    auto occurrencesToRemove = std::min<uint64_t>(required, it->second.occurrences.size());
    it->second.occurrences.resize(it->second.occurrences.size() - occurrencesToRemove);

    if (it->second.deaths == 0)
        records.erase(it);

    return true;
}

bool ComplexDeathMap::contains(const std::string& legacyKey) const {
    return toLegacyDeaths().contains(legacyKey);
}

bool ComplexDeathMap::contains(double endPercent, std::optional<double> startPercent) const {
    auto end = ComplexPercent::fromPercent(endPercent);
    if (end.isErr())
        return false;

    std::optional<ComplexPercent::Units> startUnits;
    if (startPercent) {
        auto start = ComplexPercent::fromPercent(*startPercent);
        if (start.isErr())
            return false;
        startUnits = start.unwrap().units;
    }

    return records.contains({end.unwrap().units, startUnits});
}

uint64_t ComplexDeathMap::totalDeaths() const {
    uint64_t total = 0;
    for (const auto& [key, death] : records) {
        if (std::numeric_limits<uint64_t>::max() - total < death.deaths)
            return std::numeric_limits<uint64_t>::max();
        total += death.deaths;
    }
    return total;
}

Deaths ComplexDeathMap::toLegacyDeaths() const {
    Deaths result;
    
    for (const auto& [key, death] : records) {
        const auto end = static_cast<int64_t>(std::floor(static_cast<double>(death.runEndPercent.units) / ComplexPercent::unitsPerPercent));

        auto legacyKey = std::to_string(end);

        if (death.runStartPercent.has_value()) {
            const auto start = static_cast<int64_t>(std::floor(
                static_cast<double>(death.runStartPercent.value().units) / ComplexPercent::unitsPerPercent
            ));

            legacyKey = fmt::format("{}-{}", start, end);
        }

        auto& count = result[legacyKey];
        const auto available = static_cast<uint64_t>(std::numeric_limits<int>::max() - count);
        count += static_cast<int>(std::min(available, death.deaths));
    }

    return result;
}

Result<> ComplexDeathMap::merge(const ComplexDeathMap& other) {
    for (const auto& [key, death] : other.records) {
        auto result = insert(death);
        if (result.isErr())
            return Err("Failed merging death record: {}", result.unwrapErr());
    }

    return Ok();
}

ComplexDeathMap ComplexDeathMap::filterOccurrences(const std::vector<SessionTimestampRange>& ranges) const {
    ComplexDeathMap filtered;

    for (const auto& [key, death] : records) {
        ComplexDeath matching = death;
        matching.deaths = 0;
        matching.occurrences.clear();

        for (const auto& occurrence : death.occurrences) {
            auto timestamp = occurrence.runEndTimestamp ? occurrence.runEndTimestamp : occurrence.runStartTimestamp;
            if (!timestamp.has_value())
                continue;

            auto inRange = std::any_of(ranges.begin(), ranges.end(), [&](const SessionTimestampRange& range) {
                return timestamp.value() >= range.start && timestamp.value() <= range.end;
            });
            if (inRange)
                matching.occurrences.push_back(occurrence);
        }

        matching.deaths = matching.occurrences.size();
        if (matching.deaths != 0)
            filtered.records.emplace(key, std::move(matching));
    }

    return filtered;
}

void Session::recordTimestamp(long long timestamp) {
    auto latestRange = timeRanges.end();
    for (auto it = timeRanges.begin(); it != timeRanges.end(); ++it) {
        if (timestamp >= it->start && (latestRange == timeRanges.end() || it->start > latestRange->start))
            latestRange = it;
    }

    if (latestRange != timeRanges.end()) {
        latestRange->end = std::max(latestRange->end, timestamp);
        return;
    }

    auto start = !legacyData.has_value() && sessionStartDate > 0 && timestamp >= sessionStartDate ? sessionStartDate : timestamp;
    timeRanges.push_back({ start, std::max(start, timestamp) });
}

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
    auto deathsResult = deaths.merge(other.deaths);
    if (deathsResult.isErr())
        log::error("Failed to merge death records: {}", deathsResult.unwrapErr());
    auto runsResult = runs.merge(other.runs);
    if (runsResult.isErr())
        log::error("Failed to merge run records: {}", runsResult.unwrapErr());
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

Result<ComplexPercent> matjson::Serialize<ComplexPercent>::fromJson(const matjson::Value& value) {
    ComplexPercent percent;
    GEODE_UNWRAP_INTO(percent.units, value.as<ComplexPercent::Units>());
    if (!percent.isValid())
        return Err("Invalid fixed-point percent");
    return Ok(percent);
}

matjson::Value matjson::Serialize<ComplexPercent>::toJson(const ComplexPercent& value) {
    return value.units;
}

Result<ComplexDeathOccurrence> matjson::Serialize<ComplexDeathOccurrence>::fromJson(const matjson::Value& value) {
    ComplexDeathOccurrence occurrence;
    if (value.contains("runStartTimestamp")) {
        GEODE_UNWRAP_INTO(occurrence.runStartTimestamp, value["runStartTimestamp"].as<time_t>());
    }
    if (value.contains("runEndTimestamp")) {
        GEODE_UNWRAP_INTO(occurrence.runEndTimestamp, value["runEndTimestamp"].as<time_t>());
    }
    return Ok(occurrence);
}

matjson::Value matjson::Serialize<ComplexDeathOccurrence>::toJson(const ComplexDeathOccurrence& value) {
    matjson::Value json = matjson::makeObject({});
    if (value.runStartTimestamp.has_value())
        json["runStartTimestamp"] = value.runStartTimestamp.value();
    if (value.runEndTimestamp.has_value())
        json["runEndTimestamp"] = value.runEndTimestamp.value();
    return json;
}

Result<ComplexDeath> matjson::Serialize<ComplexDeath>::fromJson(const matjson::Value& value) {
    ComplexDeath death;
    GEODE_UNWRAP_INTO(death.runEndPercent, value["endUnits"].as<ComplexPercent>());
    GEODE_UNWRAP_INTO(death.deaths, value["deaths"].as<uint64_t>());

    if (value.contains("startUnits")) {
        ComplexPercent start;
        GEODE_UNWRAP_INTO(start, value["startUnits"].as<ComplexPercent>());
        death.runStartPercent = start;
    }
    if (value.contains("occurrences")) {
        GEODE_UNWRAP_INTO(death.occurrences, value["occurrences"].as<std::vector<ComplexDeathOccurrence>>());
    }
    return Ok(death);
}

matjson::Value matjson::Serialize<ComplexDeath>::toJson(const ComplexDeath& value) {
    matjson::Value json = matjson::makeObject({
        { "endUnits", value.runEndPercent },
        { "deaths", value.deaths },
        { "occurrences", value.occurrences }
    });
    if (value.runStartPercent.has_value())
        json["startUnits"] = value.runStartPercent.value();
    return json;
}

Result<ComplexDeathMap> matjson::Serialize<ComplexDeathMap>::fromJson(const matjson::Value& value) {
    if (!value.contains("entries"))
        return Err("Complex death map is missing its entries");

    std::vector<ComplexDeath> entries;
    GEODE_UNWRAP_INTO(entries, value["entries"].as<std::vector<ComplexDeath>>());

    ComplexDeathMap result;
    for (auto& entry : entries) {
        auto insertResult = result.insert(std::move(entry));
        if (insertResult.isErr())
            return Err("Invalid complex death entry: {}", insertResult.unwrapErr());
    }
    return Ok(std::move(result));
}

matjson::Value matjson::Serialize<ComplexDeathMap>::toJson(const ComplexDeathMap& value) {
    std::vector<ComplexDeath> entries;
    entries.reserve(value.records.size());
    for (const auto& [key, entry] : value.records)
        entries.push_back(entry);
    return matjson::makeObject({ { "entries", entries } });
}

Result<GeneralData> matjson::Serialize<GeneralData>::fromJson(const matjson::Value& value) {
    GeneralData stats;

    if (value.contains("deaths")) {
        GEODE_UNWRAP_INTO(stats.deaths, ComplexDeathMap::readDeathMap(value["deaths"], false));
    }

    if (value.contains("runs")) {
        GEODE_UNWRAP_INTO(stats.runs, ComplexDeathMap::readDeathMap(value["runs"], true));
    }

    if (value.contains("newBests")) {
        GEODE_UNWRAP_INTO(stats.newBests, value["newBests"].as<NewBests>());
    }
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

Result<SessionTimestampRange> matjson::Serialize<SessionTimestampRange>::fromJson(const matjson::Value& value) {
    SessionTimestampRange range;
    GEODE_UNWRAP_INTO(range.start, value["start"].as<long long>());
    GEODE_UNWRAP_INTO(range.end, value["end"].as<long long>());
    if (range.end < range.start)
        return Err("Session timestamp range ends before it starts");
    return Ok(range);
}

matjson::Value matjson::Serialize<SessionTimestampRange>::toJson(const SessionTimestampRange& value) {
    return matjson::makeObject({
        { "start", value.start },
        { "end", value.end }
    });
}

Result<Session> matjson::Serialize<Session>::fromJson(const matjson::Value& value) {
    Session session;
    GEODE_UNWRAP_INTO(session.lastPlayed, value["lastPlayed"].as<long long>());
    GEODE_UNWRAP_INTO(session.sessionStartDate, value["sessionStartDate"].as<long long>());
    if (value.contains("timeRanges")) {
        GEODE_UNWRAP_INTO(session.timeRanges, value["timeRanges"].as<std::vector<SessionTimestampRange>>());
    }
    if (value.contains("dataIsOverride")) {
        GEODE_UNWRAP_INTO(session.dataIsOverride, value["dataIsOverride"].asBool());
    }

    GeneralData savedData;
    if (value.contains("data")) {
        GEODE_UNWRAP_INTO(savedData, value["data"].as<GeneralData>());
    }

    if (value.contains("legacyData")) {
        GEODE_UNWRAP_INTO(session.legacyData, value["legacyData"].as<GeneralData>());
    }

    if (session.dataIsOverride) {
        session.data = std::move(savedData);
    }
    else if (session.legacyData.has_value()) {
        session.data = session.legacyData.value();
        session.data.newBests = std::move(savedData.newBests);
        session.data.playtimeGeneral = savedData.playtimeGeneral;
        session.data.playtimePaused = savedData.playtimePaused;
        session.data.playtimeDead = savedData.playtimeDead;
    }
    else {
        session.data = std::move(savedData);
        if (session.timeRanges.empty() && value.contains("data"))
            session.legacyData = session.data;
    }

    return Ok(session);
}

matjson::Value matjson::Serialize<Session>::toJson(const Session& value) {
    auto json = matjson::makeObject({
        { "lastPlayed", value.lastPlayed },
        { "sessionStartDate", value.sessionStartDate },
        { "timeRanges", value.timeRanges }
    });

    if (value.dataIsOverride) {
        json["data"] = value.data;
        json["dataIsOverride"] = true;
    }
    else {
        if (value.legacyData.has_value())
            json["legacyData"] = value.legacyData.value();

        auto sessionData = value.data;
        sessionData.deaths = {};
        sessionData.runs = {};
        json["data"] = sessionData;
    }

    return json;
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
    if (value.contains("newBestCustomization")) {
        GEODE_UNWRAP_INTO(preset.newBestCustomization, value["newBestCustomization"].as<CustomizationData>());
    }
    else if (value.contains("newBestColor")) {
        GEODE_UNWRAP_INTO(auto nbColor, value["newBestColor"].as<ccColor3B>());
        preset.newBestCustomization.color = to4B(nbColor);
        preset.newBestCustomization.gradient.colorA = to4B(nbColor);
        preset.newBestCustomization.gradient.colorB = to4B(nbColor);
    }
    if (value.contains("sessionBestCustomization")) {
        GEODE_UNWRAP_INTO(preset.sessionBestCustomization, value["sessionBestCustomization"].as<CustomizationData>());
    }
    else if (value.contains("sessionBestColor")) {
        GEODE_UNWRAP_INTO(auto sbColor, value["sessionBestColor"].as<ccColor3B>());
        preset.sessionBestCustomization.color = to4B(sbColor);
        preset.sessionBestCustomization.gradient.colorA = to4B(sbColor);
        preset.sessionBestCustomization.gradient.colorB = to4B(sbColor);
    }
    return Ok(preset);
}
matjson::Value matjson::Serialize<DTLayoutPreset>::toJson(const DTLayoutPreset& value) {
    return matjson::makeObject({
        { "name", value.name },
        { "layout", value.layout },
        { "newBestCustomization", value.newBestCustomization },
        { "sessionBestCustomization", value.sessionBestCustomization }
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
