#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class DateFormatter {
public:
    static std::string format(const tm& dateTime);
};