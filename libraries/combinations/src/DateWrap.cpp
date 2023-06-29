#include "combinations/DateWrap.hpp"

ExpirationOffset::ExpirationOffset(std::size_t number_of_periods, TimePeriods period)
    : number_of_periods(number_of_periods), period(period) {}

ExpirationOffset::ExpirationOffset(TimePeriods period) : number_of_periods(1), period(period) {}

std::size_t ExpirationOffset::num() const {
    return number_of_periods;
}

TimePeriods ExpirationOffset::per() const {
    return period;
}

Date::Date(std::tm& date) : date(date) {}

bool Date::is_leap(std::size_t year) {
    return ((year % 4 && !(year % 100)) || year % 400);
}

std::size_t Date::days_in_month(std::size_t month, std::size_t year) {
    std::array<std::size_t, 12> days_in_month{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap(year) && month == 1) {
        return 29;
    }
    return days_in_month[month];
}

Date Date::normalize_date(std::size_t day, std::size_t month, std::size_t year) {
    year += month / 12;
    month %= 12;
    while (day > days_in_month(month, year)) {
        day -= days_in_month(month++, year);
        year += month / 12;
        month %= 12;
    }

    year += month / 12;
    month %= 12;

    std::tm tmp;
    tmp.tm_year = static_cast<int>(year);
    tmp.tm_mon  = static_cast<int>(month);
    tmp.tm_mday = static_cast<int>(day);
    return Date(tmp);
}

bool Date::check_offset(const ExpirationOffset& offset, const Date& test_date) {
    std::size_t day   = date.tm_mday;
    std::size_t month = date.tm_mon;
    std::size_t year  = date.tm_year;

    switch (offset.per()) {
    case TimePeriods::y:
        year += offset.num();
        break;
    case TimePeriods::q:
        month += offset.num() * 3;
        day = test_date.date.tm_mday;
        break;
    case TimePeriods::m:
        month += offset.num();
        break;
    case TimePeriods::d:
        day += offset.num();
        break;
    default:
        break;
    }

    Date new_date = normalize_date(day, month, year);
    return test_date == new_date;
}

bool operator==(const Date& left, const Date& right) {
    return left.date.tm_year == right.date.tm_year && left.date.tm_mon == right.date.tm_mon &&
           left.date.tm_mday == right.date.tm_mday;
}

bool operator!=(const Date& left, const Date& right) {
    return !(left == right);
}

bool operator<(const Date& left, const Date& right) {
    if (left.date.tm_year == right.date.tm_year) {
        if (left.date.tm_mon == right.date.tm_mon) {
            return left.date.tm_mday < right.date.tm_mday;
        }
        return left.date.tm_mon < right.date.tm_mon;
    }
    return left.date.tm_year < right.date.tm_year;
}

bool operator>(const Date& left, const Date& right) {
    return !(left < right) && (left != right);
}

bool operator>=(const Date& left, const Date& right) {
    return !(left < right);
}

bool operator<=(const Date& left, const Date& right) {
    return !(left > right);
}
