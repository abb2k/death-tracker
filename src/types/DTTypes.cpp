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
        .m_units = static_cast<Units>(std::llround(scaled))
    });
}

double ComplexPercent::asPercent() const {
    return static_cast<double>(m_units) / unitsPerPercent;
}

bool ComplexPercent::isValid() const {
    return m_units >= std::numeric_limits<Units>::min() + unitsPerPercent && m_units <= std::numeric_limits<Units>::max() - unitsPerPercent;
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

            start = run.m_start;
            end = run.m_end;
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
            .m_runStartPercent = startPercent,
            .m_runEndPercent = endPercent.unwrap(),
            .m_deaths = static_cast<uint64_t>(count)
        };

        auto insertResult = result.insert(std::move(entry));
        if (insertResult.isErr())
            return Err("Failed converting legacy run \"{}\": {}", key, insertResult.unwrapErr());
    }

    return Ok(std::move(result));
}

Result<> ComplexDeathMap::insert(ComplexDeath death) {
    if (!death.m_runEndPercent.isValid() || (death.m_runStartPercent.has_value() && !death.m_runStartPercent->isValid()))
        return Err("Cannot insert an invalid percentage");

    if (death.m_deaths == 0)
        return Ok();

    Run key {
        death.m_runEndPercent.m_units,
        death.m_runStartPercent.has_value() ? std::optional<ComplexPercent::Units>(death.m_runStartPercent->m_units) : std::nullopt
    };

    auto [it, inserted] = m_records.try_emplace(key, std::move(death));

    if (inserted)
        return Ok();

    auto& stored = it->second;
    auto& incoming = death;

    if (std::numeric_limits<uint64_t>::max() - stored.m_deaths < incoming.m_deaths)
        return Err("Death overflow");
    
    stored.m_deaths += incoming.m_deaths;
    stored.m_occurrences.insert(
        stored.m_occurrences.end(),
        std::make_move_iterator(incoming.m_occurrences.begin()),
        std::make_move_iterator(incoming.m_occurrences.end())
    );
    return Ok();
}

Result<> ComplexDeathMap::recordDeath(ComplexPercent endPercent, std::optional<ComplexPercent> startPercent, ComplexDeathOccurrence occurrence) {
    ComplexDeath death{
        .m_runStartPercent = startPercent,
        .m_runEndPercent = endPercent,
        .m_deaths = 1
    };

    if (occurrence.m_runStartTimestamp.has_value() || occurrence.m_runEndTimestamp.has_value())
        death.m_occurrences.push_back(std::move(occurrence));
    
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

    auto units = parsedPercent.unwrap().m_units;
    auto quotient = units / factor;

    if (units < 0 && units % factor != 0) quotient--;

    auto lower = quotient * factor;
    auto upper = lower + factor;

    ComplexDeathSummary summary;
    summary.m_queriedPercent.m_units = lower;
    summary.m_decimalDepth = decimalDepth;

    auto eligibleRuns = filterCheatedRuns();
    for (auto it = eligibleRuns.m_records.lower_bound({lower, std::nullopt}); it != eligibleRuns.m_records.end() && it->first.first < upper; ++it) {

        summary.m_records.push_back(it->second);
        if (std::numeric_limits<uint64_t>::max() - summary.m_deaths < it->second.m_deaths)
            return Err("death count overflow");

        summary.m_deaths += it->second.m_deaths;
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
            .m_runStartPercent = start,
            .m_runEndPercent = end,
            .m_deaths = static_cast<uint64_t>(amount)
        }).isOk();
    }

    auto required = static_cast<uint64_t>(-static_cast<int64_t>(amount));
    Run key{
        end.m_units,
        start ? std::optional<ComplexPercent::Units>(start->m_units) : std::nullopt
    };
    auto it = m_records.find(key);
    if (it == m_records.end() || it->second.m_deaths < required)
        return false;

    it->second.m_deaths -= required;
    auto occurrencesToRemove = std::min<uint64_t>(required, it->second.m_occurrences.size());
    it->second.m_occurrences.resize(it->second.m_occurrences.size() - occurrencesToRemove);

    if (it->second.m_deaths == 0)
        m_records.erase(it);

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
        startUnits = start.unwrap().m_units;
    }

    return m_records.contains({end.unwrap().m_units, startUnits});
}

uint64_t ComplexDeathMap::totalDeaths() const {
    uint64_t total = 0;
    for (const auto& [key, death] : filterCheatedRuns().m_records) {
        if (std::numeric_limits<uint64_t>::max() - total < death.m_deaths)
            return std::numeric_limits<uint64_t>::max();
        total += death.m_deaths;
    }
    return total;
}

Deaths ComplexDeathMap::toLegacyDeaths(bool includeCheated) const {
    Deaths result;
    auto eligibleRuns = includeCheated ? *this : filterCheatedRuns();
    
    for (const auto& [key, death] : eligibleRuns.m_records) {
        const auto end = static_cast<int64_t>(std::floor(static_cast<double>(death.m_runEndPercent.m_units) / ComplexPercent::unitsPerPercent));

        auto legacyKey = std::to_string(end);

        if (death.m_runStartPercent.has_value()) {
            const auto start = static_cast<int64_t>(std::floor(
                static_cast<double>(death.m_runStartPercent.value().m_units) / ComplexPercent::unitsPerPercent
            ));

            legacyKey = fmt::format("{}-{}", start, end);
        }

        auto& count = result[legacyKey];
        const auto available = static_cast<uint64_t>(std::numeric_limits<int>::max() - count);
        count += static_cast<int>(std::min(available, death.m_deaths));
    }

    return result;
}

Result<> ComplexDeathMap::merge(const ComplexDeathMap& other) {
    for (const auto& [key, death] : other.m_records) {
        auto result = insert(death);
        if (result.isErr())
            return Err("Failed merging death record: {}", result.unwrapErr());
    }

    return Ok();
}

ComplexDeathMap ComplexDeathMap::filterOccurrences(const std::vector<SessionTimestampRange>& ranges) const {
    ComplexDeathMap filtered;

    for (const auto& [key, death] : m_records) {
        ComplexDeath matching = death;
        matching.m_deaths = 0;
        matching.m_occurrences.clear();

        for (const auto& occurrence : death.m_occurrences) {
            auto timestamp = occurrence.m_runEndTimestamp ? occurrence.m_runEndTimestamp : occurrence.m_runStartTimestamp;
            if (!timestamp.has_value())
                continue;

            auto inRange = std::any_of(ranges.begin(), ranges.end(), [&](const SessionTimestampRange& range) {
                return timestamp.value() >= range.m_start && timestamp.value() <= range.m_end;
            });
            if (inRange)
                matching.m_occurrences.push_back(occurrence);
        }

        matching.m_deaths = matching.m_occurrences.size();
        if (matching.m_deaths != 0)
            filtered.m_records.emplace(key, std::move(matching));
    }

    return filtered;
}

ComplexDeathMap ComplexDeathMap::filterCheatedRuns() const {
    ComplexDeathMap filtered;

    for (const auto& [key, death] : m_records) {
        ComplexDeath eligible = death;
        eligible.m_occurrences.clear();

        auto untrackedDeaths = death.m_deaths > death.m_occurrences.size()
            ? death.m_deaths - death.m_occurrences.size()
            : 0;
        for (const auto& occurrence : death.m_occurrences) {
            if (!occurrence.m_cheated)
                eligible.m_occurrences.push_back(occurrence);
        }

        eligible.m_deaths = untrackedDeaths + eligible.m_occurrences.size();
        if (eligible.m_deaths != 0)
            filtered.m_records.emplace(key, std::move(eligible));
    }

    return filtered;
}

ComplexDeathMap ComplexDeathMap::filterOnlyCheatedRuns() const {
    ComplexDeathMap filtered;

    for (const auto& [key, death] : m_records) {
        ComplexDeath cheated = death;
        cheated.m_occurrences.clear();

        for (const auto& occurrence : death.m_occurrences) {
            if (occurrence.m_cheated)
                cheated.m_occurrences.push_back(occurrence);
        }

        cheated.m_deaths = cheated.m_occurrences.size();
        if (cheated.m_deaths != 0)
            filtered.m_records.emplace(key, std::move(cheated));
    }

    return filtered;
}

void Session::recordTimestamp(long long timestamp) {
    auto latestRange = m_timeRanges.end();
    for (auto it = m_timeRanges.begin(); it != m_timeRanges.end(); ++it) {
        if (timestamp >= it->m_start && (latestRange == m_timeRanges.end() || it->m_start > latestRange->m_start))
            latestRange = it;
    }

    if (latestRange != m_timeRanges.end()) {
        latestRange->m_end = std::max(latestRange->m_end, timestamp);
        return;
    }

    auto start = !m_legacyData.has_value() && m_sessionStartDate > 0 && timestamp >= m_sessionStartDate ? m_sessionStartDate : timestamp;
    m_timeRanges.push_back({ start, std::max(start, timestamp) });
}

PlaytimePair& PlaytimePair::operator+=(const PlaytimePair& other) {
    m_playtimeF0 += other.m_playtimeF0;
    m_playtimeRuns += other.m_playtimeRuns;
    return *this;
}

void PlaytimePair::reset() {
    m_playtimeF0 = 0;
    m_playtimeRuns = 0;
}

GeneralData& GeneralData::operator+=(const GeneralData& other) {
    auto deathsResult = m_deaths.merge(other.m_deaths);
    if (deathsResult.isErr())
        log::error("Failed to merge death records: {}", deathsResult.unwrapErr());
    auto runsResult = m_runs.merge(other.m_runs);
    if (runsResult.isErr())
        log::error("Failed to merge run records: {}", runsResult.unwrapErr());
    m_newBests.insert(other.m_newBests.begin(), other.m_newBests.end());
    m_playtimeGeneral += other.m_playtimeGeneral;
    m_playtimePaused += other.m_playtimePaused;
    m_playtimeDead += other.m_playtimeDead;
    return *this;
}

bool Section::isPercentInSection(int percent) const {
    return percent >= m_startPercent && percent < m_endPercent ||
        percent >= m_startPercent && m_endPercent == 100;
}

bool Section::isValid() const {
    return !(m_startPercent > m_endPercent);
}

bool Section::operator==(const Section& other) const {
    return m_startPercent == other.m_startPercent && m_endPercent == other.m_endPercent;
}

bool DTLayoutV3::isEmpty() const {
    return m_columns.empty() && m_labels.empty();
}

Result<PlaytimePair> matjson::Serialize<PlaytimePair>::fromJson(const matjson::Value& value) {
    PlaytimePair pair;
    GEODE_UNWRAP_INTO(pair.m_playtimeF0, value["playtimeF0"].as<uint64_t>());
    GEODE_UNWRAP_INTO(pair.m_playtimeRuns, value["playtimeRuns"].as<uint64_t>());
    return Ok(pair);
}

matjson::Value matjson::Serialize<PlaytimePair>::toJson(const PlaytimePair& value) {
    return matjson::makeObject({
        { "playtimeF0", value.m_playtimeF0 },
        { "playtimeRuns", value.m_playtimeRuns }
    });
}

Result<ComplexPercent> matjson::Serialize<ComplexPercent>::fromJson(const matjson::Value& value) {
    ComplexPercent percent;
    GEODE_UNWRAP_INTO(percent.m_units, value.as<ComplexPercent::Units>());
    if (!percent.isValid())
        return Err("Invalid fixed-point percent");
    return Ok(percent);
}

matjson::Value matjson::Serialize<ComplexPercent>::toJson(const ComplexPercent& value) {
    return value.m_units;
}

Result<ComplexDeathOccurrence> matjson::Serialize<ComplexDeathOccurrence>::fromJson(const matjson::Value& value) {
    ComplexDeathOccurrence occurrence;
    if (value.contains("runStartTimestamp")) {
        GEODE_UNWRAP_INTO(occurrence.m_runStartTimestamp, value["runStartTimestamp"].as<time_t>());
    }
    if (value.contains("runEndTimestamp")) {
        GEODE_UNWRAP_INTO(occurrence.m_runEndTimestamp, value["runEndTimestamp"].as<time_t>());
    }
    if (value.contains("cheated")) {
        GEODE_UNWRAP_INTO(occurrence.m_cheated, value["cheated"].asBool());
    }
    return Ok(occurrence);
}

matjson::Value matjson::Serialize<ComplexDeathOccurrence>::toJson(const ComplexDeathOccurrence& value) {
    matjson::Value json = matjson::makeObject({});
    if (value.m_runStartTimestamp.has_value())
        json["runStartTimestamp"] = value.m_runStartTimestamp.value();
    if (value.m_runEndTimestamp.has_value())
        json["runEndTimestamp"] = value.m_runEndTimestamp.value();
    json["cheated"] = value.m_cheated;
    return json;
}

Result<ComplexDeath> matjson::Serialize<ComplexDeath>::fromJson(const matjson::Value& value) {
    ComplexDeath death;
    GEODE_UNWRAP_INTO(death.m_runEndPercent, value["endUnits"].as<ComplexPercent>());
    GEODE_UNWRAP_INTO(death.m_deaths, value["deaths"].as<uint64_t>());

    if (value.contains("startUnits")) {
        ComplexPercent start;
        GEODE_UNWRAP_INTO(start, value["startUnits"].as<ComplexPercent>());
        death.m_runStartPercent = start;
    }
    if (value.contains("occurrences")) {
        GEODE_UNWRAP_INTO(death.m_occurrences, value["occurrences"].as<std::vector<ComplexDeathOccurrence>>());
    }
    return Ok(death);
}

matjson::Value matjson::Serialize<ComplexDeath>::toJson(const ComplexDeath& value) {
    matjson::Value json = matjson::makeObject({
        { "endUnits", value.m_runEndPercent },
        { "deaths", value.m_deaths },
        { "occurrences", value.m_occurrences }
    });
    if (value.m_runStartPercent.has_value())
        json["startUnits"] = value.m_runStartPercent.value();
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
    entries.reserve(value.m_records.size());
    for (const auto& [key, entry] : value.m_records)
        entries.push_back(entry);
    return matjson::makeObject({ { "entries", entries } });
}

Result<GeneralData> matjson::Serialize<GeneralData>::fromJson(const matjson::Value& value) {
    GeneralData stats;

    if (value.contains("deaths")) {
        GEODE_UNWRAP_INTO(stats.m_deaths, ComplexDeathMap::readDeathMap(value["deaths"], false));
    }

    if (value.contains("runs")) {
        GEODE_UNWRAP_INTO(stats.m_runs, ComplexDeathMap::readDeathMap(value["runs"], true));
    }

    if (value.contains("newBests")) {
        GEODE_UNWRAP_INTO(stats.m_newBests, value["newBests"].as<NewBests>());
    }
    if (value.contains("playtimeGeneral")) {
        GEODE_UNWRAP_INTO(stats.m_playtimeGeneral, value["playtimeGeneral"].as<PlaytimePair>());
    }
    if (value.contains("playtimePaused")) {
        GEODE_UNWRAP_INTO(stats.m_playtimePaused, value["playtimePaused"].as<PlaytimePair>());
    }
    if (value.contains("playtimeDead")) {
        GEODE_UNWRAP_INTO(stats.m_playtimeDead, value["playtimeDead"].as<PlaytimePair>());
    }
    return Ok(stats);
}

matjson::Value matjson::Serialize<GeneralData>::toJson(const GeneralData& value) {
    return matjson::makeObject({
        { "deaths", value.m_deaths },
        { "runs", value.m_runs },
        { "newBests", value.m_newBests },
        { "playtimeGeneral", value.m_playtimeGeneral },
        { "playtimePaused", value.m_playtimePaused },
        { "playtimeDead", value.m_playtimeDead }
    });
}

Result<SessionTimestampRange> matjson::Serialize<SessionTimestampRange>::fromJson(const matjson::Value& value) {
    SessionTimestampRange range;
    GEODE_UNWRAP_INTO(range.m_start, value["start"].as<long long>());
    GEODE_UNWRAP_INTO(range.m_end, value["end"].as<long long>());
    if (range.m_end < range.m_start)
        return Err("Session timestamp range ends before it starts");
    return Ok(range);
}

matjson::Value matjson::Serialize<SessionTimestampRange>::toJson(const SessionTimestampRange& value) {
    return matjson::makeObject({
        { "start", value.m_start },
        { "end", value.m_end }
    });
}

Result<Session> matjson::Serialize<Session>::fromJson(const matjson::Value& value) {
    Session session;
    GEODE_UNWRAP_INTO(session.m_lastPlayed, value["lastPlayed"].as<long long>());
    GEODE_UNWRAP_INTO(session.m_sessionStartDate, value["sessionStartDate"].as<long long>());
    if (value.contains("timeRanges")) {
        GEODE_UNWRAP_INTO(session.m_timeRanges, value["timeRanges"].as<std::vector<SessionTimestampRange>>());
    }
    if (value.contains("dataIsOverride")) {
        GEODE_UNWRAP_INTO(session.m_dataIsOverride, value["dataIsOverride"].asBool());
    }

    GeneralData savedData;
    if (value.contains("data")) {
        GEODE_UNWRAP_INTO(savedData, value["data"].as<GeneralData>());
    }

    if (value.contains("legacyData")) {
        GEODE_UNWRAP_INTO(session.m_legacyData, value["legacyData"].as<GeneralData>());
    }

    if (session.m_dataIsOverride) {
        session.m_data = std::move(savedData);
    }
    else if (session.m_legacyData.has_value()) {
        session.m_data = session.m_legacyData.value();
        session.m_data.m_newBests = std::move(savedData.m_newBests);
        session.m_data.m_playtimeGeneral = savedData.m_playtimeGeneral;
        session.m_data.m_playtimePaused = savedData.m_playtimePaused;
        session.m_data.m_playtimeDead = savedData.m_playtimeDead;
    }
    else {
        session.m_data = std::move(savedData);
        if (session.m_timeRanges.empty() && value.contains("data"))
            session.m_legacyData = session.m_data;
    }

    return Ok(session);
}

matjson::Value matjson::Serialize<Session>::toJson(const Session& value) {
    auto json = matjson::makeObject({
        { "lastPlayed", value.m_lastPlayed },
        { "sessionStartDate", value.m_sessionStartDate },
        { "timeRanges", value.m_timeRanges }
    });

    if (value.m_dataIsOverride) {
        json["data"] = value.m_data;
        json["dataIsOverride"] = true;
    }
    else {
        if (value.m_legacyData.has_value())
            json["legacyData"] = value.m_legacyData.value();

        auto sessionData = value.m_data;
        sessionData.m_deaths = {};
        sessionData.m_runs = {};
        json["data"] = sessionData;
    }

    return json;
}

Result<Section> matjson::Serialize<Section>::fromJson(const matjson::Value& value) {
    Section section;
    GEODE_UNWRAP_INTO(section.m_name, value["name"].asString());
    GEODE_UNWRAP_INTO(section.m_startPercent, value["start"].asInt());
    GEODE_UNWRAP_INTO(section.m_endPercent, value["end"].asInt());
    return Ok(section);
}

matjson::Value matjson::Serialize<Section>::toJson(const Section& value) {
    return matjson::makeObject({
        { "name", value.m_name },
        { "start", value.m_startPercent },
        { "end", value.m_endPercent }
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
    GEODE_UNWRAP_INTO(grouping.m_group, value["group"].as<std::map<long long, std::set<std::string>, std::greater<long long>>>());
    return Ok(grouping);
}

matjson::Value matjson::Serialize<SessionGrouping>::toJson(const SessionGrouping& value) {
    return matjson::makeObject({
        { "group", value.m_group }
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
    GEODE_UNWRAP_INTO(category.m_groupName, value["groupName"].asString());
    GEODE_UNWRAP_INTO(category.m_grouping, value["grouping"].as<std::map<long long, SessionGrouping, std::greater<long long>>>());
    return Ok(category);
}

matjson::Value matjson::Serialize<SessionCategory>::toJson(const SessionCategory& value) {
    return matjson::makeObject({
        { "groupName", value.m_groupName },
        { "grouping", value.m_grouping }
    });
}

Result<GradientData> matjson::Serialize<GradientData>::fromJson(const matjson::Value& value) {
    GradientData data;
    GEODE_UNWRAP_INTO(data.m_colorA, value["colorA"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(data.m_colorB, value["colorB"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(data.m_duration, value["duration"].asDouble());
    GEODE_UNWRAP_INTO(data.m_stagger, value["stagger"].asDouble());
    return Ok(data);
}

matjson::Value matjson::Serialize<GradientData>::toJson(const GradientData& value) {
    return matjson::makeObject({
        { "colorA", value.m_colorA },
        { "colorB", value.m_colorB },
        { "duration", value.m_duration },
        { "stagger", value.m_stagger }
    });
}

Result<WaveData> matjson::Serialize<WaveData>::fromJson(const matjson::Value& value) {
    WaveData data;
    GEODE_UNWRAP_INTO(data.m_speed, value["speed"].asDouble());
    GEODE_UNWRAP_INTO(data.m_distanceY, value["distanceY"].asDouble());
    GEODE_UNWRAP_INTO(data.m_distanceX, value["distanceX"].asDouble());
    GEODE_UNWRAP_INTO(data.m_offsetPerIndex, value["offsetPerIndex"].asDouble());
    return Ok(data);
}

matjson::Value matjson::Serialize<WaveData>::toJson(const WaveData& value) {
    return matjson::makeObject({
        { "speed", value.m_speed },
        { "distanceY", value.m_distanceY },
        { "distanceX", value.m_distanceX },
        { "offsetPerIndex", value.m_offsetPerIndex }
    });
}

Result<CustomizationData> matjson::Serialize<CustomizationData>::fromJson(const matjson::Value& value) {
    CustomizationData data;
    GEODE_UNWRAP_INTO(data.m_color, value["color"].as<ccColor4B>());

    GEODE_UNWRAP_INTO(data.m_gradientEnabled, value["gradientEnabled"].asBool());
    GEODE_UNWRAP_INTO(data.m_gradient, value["gradient"].as<GradientData>());

    GEODE_UNWRAP_INTO(data.m_waveEnabled, value["waveEnabled"].asBool());
    GEODE_UNWRAP_INTO(data.m_wave, value["wave"].as<WaveData>());

    return Ok(data);
}

matjson::Value matjson::Serialize<CustomizationData>::toJson(const CustomizationData& value) {
    return matjson::makeObject({
        { "color", value.m_color },
        { "gradientEnabled", value.m_gradientEnabled },
        { "gradient", value.m_gradient },
        { "waveEnabled", value.m_waveEnabled },
        { "wave", value.m_wave }
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
    GEODE_UNWRAP_INTO(info.m_minPlacementRange, value["minPlacementRange"].asInt());
    GEODE_UNWRAP_INTO(info.m_maxPlacementRange, value["maxPlacementRange"].asInt());
    GEODE_UNWRAP_INTO(info.m_layer, value["layer"].asInt());
    GEODE_UNWRAP_INTO(info.m_labelName, value["labelName"].asString());
    GEODE_UNWRAP_INTO(info.m_text, value["text"].asString());
    GEODE_UNWRAP_INTO(info.m_font, value["font"].asString());
    GEODE_UNWRAP_INTO(auto horizontalAlignment, value["horizontalAlignment"].asInt());
    info.m_horizontalAlignment = static_cast<CCTextAlignment>(horizontalAlignment);
    GEODE_UNWRAP_INTO(info.m_labelColor, value["labelColor"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.m_textColor, value["color"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.m_scale, value["scale"].asDouble());
    GEODE_UNWRAP_INTO(info.m_isExpanded, value["isExpanded"].asBool());
    if (value.contains("wrapping")) {
        GEODE_UNWRAP_INTO(auto wrapping, value["wrapping"].asInt());
        info.m_wrapping = static_cast<WrappingMode>(wrapping);
    }
    if (value.contains("hideHeader")) {
        GEODE_UNWRAP_INTO(info.m_hideHeader, value["hideHeader"].asBool());
    }
    return Ok(info);
}

matjson::Value matjson::Serialize<DTLabelInfo>::toJson(const DTLabelInfo& value) {
    return matjson::makeObject({
        { "minPlacementRange", value.m_minPlacementRange },
        { "maxPlacementRange", value.m_maxPlacementRange },
        { "layer", value.m_layer },
        { "labelName", value.m_labelName },
        { "text", value.m_text },
        { "font", value.m_font },
        { "horizontalAlignment", static_cast<int>(value.m_horizontalAlignment) },
        { "color", value.m_textColor },
        { "labelColor", value.m_labelColor },
        { "scale", value.m_scale },
        { "isExpanded", value.m_isExpanded },
        { "wrapping", static_cast<int>(value.m_wrapping) },
        { "hideHeader", value.m_hideHeader }
    });
}

Result<DTColumnInfo> matjson::Serialize<DTColumnInfo>::fromJson(const matjson::Value& value) {
    DTColumnInfo info;
    GEODE_UNWRAP_INTO(info.m_orderPos, value["orderPos"].asInt());
    GEODE_UNWRAP_INTO(info.m_currentWidth, value["currentWidth"].asDouble());
    GEODE_UNWRAP_INTO(info.m_color, value["color"].as<ccColor3B>());
    return Ok(info);
}

matjson::Value matjson::Serialize<DTColumnInfo>::toJson(const DTColumnInfo& value) {
    return matjson::makeObject({
        { "orderPos", value.m_orderPos },
        { "currentWidth", value.m_currentWidth },
        { "color", value.m_color }
    });
}

Result<DTLayoutV3> matjson::Serialize<DTLayoutV3>::fromJson(const matjson::Value& value) {
    DTLayoutV3 info;
    GEODE_UNWRAP_INTO(info.m_columns, value["columns"].as<std::vector<DTColumnInfo>>());
    GEODE_UNWRAP_INTO(info.m_labels, value["labels"].as<std::vector<DTLabelInfo>>());
    return Ok(info);
}

matjson::Value matjson::Serialize<DTLayoutV3>::toJson(const DTLayoutV3& value) {
    return matjson::makeObject({
        { "columns", value.m_columns },
        { "labels", value.m_labels }
    });
}

Result<DTLayoutPreset> matjson::Serialize<DTLayoutPreset>::fromJson(const matjson::Value& value) {
    DTLayoutPreset preset;
    GEODE_UNWRAP_INTO(preset.m_name, value["name"].asString());
    GEODE_UNWRAP_INTO(preset.m_layout, value["layout"].as<DTLayoutV3>());
    if (value.contains("newBestCustomization")) {
        GEODE_UNWRAP_INTO(preset.m_newBestCustomization, value["newBestCustomization"].as<CustomizationData>());
    }
    else if (value.contains("newBestColor")) {
        GEODE_UNWRAP_INTO(auto nbColor, value["newBestColor"].as<ccColor3B>());
        preset.m_newBestCustomization.m_color = to4B(nbColor);
        preset.m_newBestCustomization.m_gradient.m_colorA = to4B(nbColor);
        preset.m_newBestCustomization.m_gradient.m_colorB = to4B(nbColor);
    }
    if (value.contains("sessionBestCustomization")) {
        GEODE_UNWRAP_INTO(preset.m_sessionBestCustomization, value["sessionBestCustomization"].as<CustomizationData>());
    }
    else if (value.contains("sessionBestColor")) {
        GEODE_UNWRAP_INTO(auto sbColor, value["sessionBestColor"].as<ccColor3B>());
        preset.m_sessionBestCustomization.m_color = to4B(sbColor);
        preset.m_sessionBestCustomization.m_gradient.m_colorA = to4B(sbColor);
        preset.m_sessionBestCustomization.m_gradient.m_colorB = to4B(sbColor);
    }
    return Ok(preset);
}
matjson::Value matjson::Serialize<DTLayoutPreset>::toJson(const DTLayoutPreset& value) {
    return matjson::makeObject({
        { "name", value.m_name },
        { "layout", value.m_layout },
        { "newBestCustomization", value.m_newBestCustomization },
        { "sessionBestCustomization", value.m_sessionBestCustomization }
    });
}

bool OverlayLabel::isOverlayLabelVisibleOnLevel(GJGameLevel* level, LevelMetadata const& meta) const {
    if (!this->m_levelSpecific.has_value()) return true;

    auto levelKeyRes = StatsManager::getLevelKey(level);
    if (levelKeyRes.isErr()) return false;

    std::set<std::string> levelKeys = meta.m_linkedLevels;
    levelKeys.insert(levelKeyRes.unwrap());

    return levelKeys.contains(this->m_levelSpecific.value());
}

Result<OverlayLabel> matjson::Serialize<OverlayLabel>::fromJson(const matjson::Value& value) {
    OverlayLabel label;
    GEODE_UNWRAP_INTO(label.m_labelName, value["labelName"].asString());
    GEODE_UNWRAP_INTO(label.m_text, value["text"].asString());

    GEODE_UNWRAP_INTO(label.m_posX, value["posX"].asDouble());
    GEODE_UNWRAP_INTO(label.m_posY, value["posY"].asDouble());

    GEODE_UNWRAP_INTO(auto anchorEnumInt, value["anchor"].asInt());
    label.m_anchor = static_cast<geode::Anchor>(anchorEnumInt);

    GEODE_UNWRAP_INTO(label.m_freeMode, value["freeMode"].asBool());
    GEODE_UNWRAP_INTO(label.m_order, value["order"].asInt());
    GEODE_UNWRAP_INTO(label.m_enabled, value["enabled"].asBool());
    if (value.contains("levelSpecificKey")) {
        GEODE_UNWRAP_INTO(label.m_levelSpecific, value["levelSpecificKey"].as<std::optional<std::string>>());
    }
    return Ok(label);
}

matjson::Value matjson::Serialize<OverlayLabel>::toJson(const OverlayLabel& value) {
    return matjson::makeObject({
        { "labelName", value.m_labelName },
        { "text", value.m_text },
        { "posX", value.m_posX },
        { "posY", value.m_posY },
        { "anchor", static_cast<int>(value.m_anchor) },
        { "freeMode", value.m_freeMode },
        { "order", value.m_order },
        { "enabled", value.m_enabled },
        { "levelSpecificKey", value.m_levelSpecific }
    });
}

Result<OverlayLabelSettings> matjson::Serialize<OverlayLabelSettings>::fromJson(const matjson::Value& value) {
    OverlayLabelSettings settings;
    GEODE_UNWRAP_INTO(settings.m_labels, value["labels"].as<std::vector<OverlayLabel>>());
    GEODE_UNWRAP_INTO(settings.m_size, value["size"].asDouble());
    settings.m_size = std::clamp(settings.m_size, OverlayLabelSettings::MIN_SIZE, OverlayLabelSettings::MAX_SIZE);
    if (value.contains("opacity")) {
        GEODE_UNWRAP_INTO(settings.m_opacity, value["opacity"].asInt());
        settings.m_opacity = std::clamp(settings.m_opacity, OverlayLabelSettings::MIN_OPACITY, OverlayLabelSettings::MAX_OPACITY);
    }
    GEODE_UNWRAP_INTO(settings.m_offset.x, value["offsetX"].asDouble());
    GEODE_UNWRAP_INTO(settings.m_offset.y, value["offsetY"].asDouble());
    GEODE_UNWRAP_INTO(settings.m_font, value["font"].asString());
    return Ok(settings);
}

matjson::Value matjson::Serialize<OverlayLabelSettings>::toJson(const OverlayLabelSettings& value) {
    return matjson::makeObject({
        { "labels", value.m_labels },
        { "size", value.m_size },
        { "opacity", value.m_opacity },
        { "offsetX", value.m_offset.x },
        { "offsetY", value.m_offset.y },
        { "font", value.m_font }
    });
}

Result<V2LabelLayout> matjson::Serialize<V2LabelLayout>::fromJson(const matjson::Value& value) {
    V2LabelLayout layout;
    GEODE_UNWRAP_INTO(layout.m_labelName, value["labelName"].asString());
    GEODE_UNWRAP_INTO(layout.m_text, value["text"].asString());
    GEODE_UNWRAP_INTO(layout.m_line, value["line"].asInt());
    GEODE_UNWRAP_INTO(layout.m_position, value["position"].asInt());
    GEODE_UNWRAP_INTO(layout.m_color, value["color"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(auto alignment, value["alignment"].asInt());
    layout.m_alignment = static_cast<CCTextAlignment>(alignment);
    GEODE_UNWRAP_INTO(layout.m_font, value["font"].asInt());
    GEODE_UNWRAP_INTO(layout.m_fontSize, value["fontSize"].asDouble());
    return Ok(layout);
}

matjson::Value matjson::Serialize<V2LabelLayout>::toJson(const V2LabelLayout& value) {
    return matjson::makeObject({
        { "labelName", value.m_labelName },
        { "text", value.m_text },
        { "line", value.m_line },
        { "position", value.m_position },
        { "color", value.m_color },
        { "alignment", static_cast<int>(value.m_alignment) },
        { "font", value.m_font },
        { "fontSize", value.m_fontSize }
    });
}

Result<stringCustomazations> matjson::Serialize<stringCustomazations>::fromJson(const matjson::Value& value) {
    stringCustomazations sc;
    GEODE_UNWRAP_INTO(sc.m_seperator, value["seperator"].asString());
    GEODE_UNWRAP_INTO(sc.m_format, value["format"].asString());
    if (value.contains("alternateStrength")) {
        GEODE_UNWRAP_INTO(sc.m_alternateStrength, value["alternateStrength"].asInt());
    }

    return Ok(sc);
}

matjson::Value matjson::Serialize<stringCustomazations>::toJson(const stringCustomazations& value) {
    return matjson::makeObject({
        { "seperator", value.m_seperator },
        { "format", value.m_format },
        { "alternateStrength", value.m_alternateStrength }
    });
}

Result<V2Session> matjson::Serialize<V2Session>::fromJson(const matjson::Value& value) {
    V2Session session;
    GEODE_UNWRAP_INTO(session.m_lastPlayed, value["lastPlayed"].as<long long>());
    GEODE_UNWRAP_INTO(session.m_deaths, value["deaths"].as<Deaths>());
    GEODE_UNWRAP_INTO(session.m_runs, value["runs"].as<Deaths>());
    GEODE_UNWRAP_INTO(session.m_newBests, value["newBests"].as<NewBests>());
    GEODE_UNWRAP_INTO(session.m_currentBest, value["currentBest"].asInt());
    if (value.contains("sessionStartDate")) {
        GEODE_UNWRAP_INTO(session.m_sessionStartDate, value["sessionStartDate"].as<long long>());
    } else {
        session.m_sessionStartDate = -1;
    }
    return Ok(session);
}

matjson::Value matjson::Serialize<V2Session>::toJson(const V2Session& value) {
    return matjson::makeObject({
        { "lastPlayed", value.m_lastPlayed },
        { "deaths", value.m_deaths },
        { "runs", value.m_runs },
        { "newBests", value.m_newBests },
        { "currentBest", value.m_currentBest },
        { "sessionStartDate", value.m_sessionStartDate }
    });
}

Result<V2LevelStats> matjson::Serialize<V2LevelStats>::fromJson(const matjson::Value& value) {
    V2LevelStats stats;
    GEODE_UNWRAP_INTO(stats.m_deaths, value["deaths"].as<Deaths>());
    GEODE_UNWRAP_INTO(stats.m_runs, value["runs"].as<Deaths>());
    GEODE_UNWRAP_INTO(stats.m_newBests, value["newBests"].as<NewBests>());
    GEODE_UNWRAP_INTO(stats.m_currentBest, value["currentBest"].asInt());
    GEODE_UNWRAP_INTO(stats.m_sessions, value["sessions"].as<std::vector<V2Session>>());
    if (value.contains("RunsToSave")) {
        GEODE_UNWRAP_INTO(stats.m_RunsToSave, value["RunsToSave"].as<std::vector<int>>());
    }
    if (value.contains("LinkedLevels")) {
        GEODE_UNWRAP_INTO(stats.m_LinkedLevels, value["LinkedLevels"].as<std::vector<std::string>>());
    } else {
        stats.m_LinkedLevels = value["LinkedLevels"].as<std::vector<std::string>>().unwrapOr(std::vector<std::string>{});
    }
    if (value.contains("levelName")) {
        GEODE_UNWRAP_INTO(stats.m_levelName, value["levelName"].asString());
    } else {
        stats.m_levelName = "-1";
    }
    if (value.contains("attempts")) {
        GEODE_UNWRAP_INTO(stats.m_attempts, value["attempts"].asInt());
    } else {
        stats.m_attempts = -1;
    }
    if (value.contains("difficulty")) {
        GEODE_UNWRAP_INTO(stats.m_difficulty, value["difficulty"].asInt());
    } else {
        stats.m_difficulty = 0;
    }
    if (value.contains("hideRunLength")) {
        GEODE_UNWRAP_INTO(stats.m_hideRunLength, value["hideRunLength"].asInt());
    } else {
        stats.m_hideRunLength = 0;
    }
    if (value.contains("hideUpto")) {
        GEODE_UNWRAP_INTO(stats.m_hideUpto, value["hideUpto"].asInt());
    } else {
        stats.m_hideUpto = 0;
    }
    return Ok(stats);
}

matjson::Value matjson::Serialize<V2LevelStats>::toJson(const V2LevelStats& value) {
    return matjson::makeObject({
        { "deaths", value.m_deaths },
        { "runs", value.m_runs },
        { "newBests", value.m_newBests },
        { "currentBest", value.m_currentBest },
        { "sessions", value.m_sessions },
        { "RunsToSave", value.m_RunsToSave },
        { "LinkedLevels", value.m_LinkedLevels },
        { "levelName", value.m_levelName },
        { "attempts", value.m_attempts },
        { "difficulty", value.m_difficulty },
        { "hideRunLength", value.m_hideRunLength },
        { "hideUpto", value.m_hideUpto }
    });
}

Result<DTGraphInfo> matjson::Serialize<DTGraphInfo>::fromJson(const matjson::Value& value) {
    DTGraphInfo info;
    GEODE_UNWRAP_INTO(auto coverageInt, value["coverage"].asInt());
    if (coverageInt == 7) {
        info.m_coverage = DTGraphCoverage::GeneralRunsCover;
    }
    else if (coverageInt == 8) {
        info.m_coverage = DTGraphCoverage::SessionRunsCover;
    }
    else {
        info.m_coverage = static_cast<DTGraphCoverage>(coverageInt);
    }
    GEODE_UNWRAP_INTO(auto typeInt, value["type"].asInt());
    info.m_type = static_cast<DTGraphType>(typeInt);
    GEODE_UNWRAP_INTO(info.m_thickness, value["thickness"].asDouble());
    GEODE_UNWRAP_INTO(info.m_outlineThickness, value["outlineThickness"].asDouble());
    GEODE_UNWRAP_INTO(info.m_color, value["color"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.m_outlineColor, value["outlineColor"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.m_pointColor, value["pointColor"].as<ccColor4B>());
    GEODE_UNWRAP_INTO(info.m_pointScale, value["pointScale"].asDouble());
    GEODE_UNWRAP_INTO(info.m_name, value["name"].asString());
    GEODE_UNWRAP_INTO(info.m_orderPos, value["orderPos"].asInt());
    GEODE_UNWRAP_INTO(info.m_isEnabled, value["isEnabled"].asBool());
    return Ok(info);
}

matjson::Value matjson::Serialize<DTGraphInfo>::toJson(const DTGraphInfo& value) {
    return matjson::makeObject({
        { "coverage", static_cast<int>(value.m_coverage) },
        { "type", static_cast<int>(value.m_type) },
        { "thickness", value.m_thickness },
        { "outlineThickness", value.m_outlineThickness },
        { "outlineColor", value.m_outlineColor },
        { "color", value.m_color },
        { "pointScale", value.m_pointScale },
        { "pointColor", value.m_pointColor },
        { "name", value.m_name },
        { "orderPos", value.m_orderPos },
        { "isEnabled", value.m_isEnabled }
    });
}
