#include <utils/DateFormatter.hpp>
#include <utils/Settings.hpp>

std::string format(const tm& dateTime){
    return Settings::is12HourClockFormat()
        ? fmt::format("{:%m/%d/%Y %I:%M%p}", dateTime)
        : fmt::format("{:%m/%d/%Y %H:%M}", dateTime);
}