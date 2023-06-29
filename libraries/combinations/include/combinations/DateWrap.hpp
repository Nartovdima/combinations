#ifndef COMBINATIONS_DATEWRAP_HPP
#define COMBINATIONS_DATEWRAP_HPP

#include <array>
#include <ctime>

enum class TimePeriods : char { d = 'd', m = 'm', q = 'q', y = 'y' };

class ExpirationOffset {
public:
    ExpirationOffset()                                 = default;
    ExpirationOffset(const ExpirationOffset& exp_offs) = default;

    ExpirationOffset& operator=(const ExpirationOffset& exp_offs) = default;

    ExpirationOffset(std::size_t number_of_periods, TimePeriods period);
    ExpirationOffset(TimePeriods period);

    std::size_t num() const;

    TimePeriods per() const;

private:
    std::size_t number_of_periods;
    TimePeriods period;
};

class Date {
public:
    Date()                 = default;
    Date(const Date& date) = default;
    Date(Date&& date)      = default;

    Date(std::tm& date);

    bool check_offset(const ExpirationOffset& offset, const Date& test_date);

    Date& operator=(const Date& tmp) = default;
    Date& operator=(Date&& tmp)      = default;

    friend bool operator==(const Date& left, const Date& right);
    friend bool operator!=(const Date& left, const Date& right);
    friend bool operator<(const Date& left, const Date& right);
    friend bool operator>(const Date& left, const Date& right);
    friend bool operator>=(const Date& left, const Date& right);
    friend bool operator<=(const Date& left, const Date& right);

private:
    std::tm date;

    static bool is_leap(std::size_t year);

    static std::size_t days_in_month(std::size_t month, std::size_t year);

    static Date normalize_date(std::size_t day, std::size_t month, std::size_t year);
};

bool operator==(const Date& left, const Date& right);
bool operator!=(const Date& left, const Date& right);
bool operator<(const Date& left, const Date& right);
bool operator>(const Date& left, const Date& right);
bool operator>=(const Date& left, const Date& right);
bool operator<=(const Date& left, const Date& right);

#endif  // COMBINATIONS_DATEWRAP_HPP