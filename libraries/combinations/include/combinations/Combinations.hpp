#ifndef COMBINATIONS_COMBINATIONS_HPP
#define COMBINATIONS_COMBINATIONS_HPP

#include <algorithm>
#include <filesystem>
#include <memory>
#include <numeric>
#include <pugixml.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "Component.hpp"
#include "DateWrap.hpp"

class Combination;

struct Leg {
    InstrumentType type;
    std::variant<char, double> ratio;
    std::variant<char, int> strike;
    std::variant<char, int, ExpirationOffset> expiration;

    static const char invalid_strike     = '\u0000';
    static const char invalid_expiration = '\u0000';
};

class Combinations {
public:
    Combinations() = default;

    static void set_ratio(pugi::xml_node& leg_xml, Leg& leg);
    static void set_strike(pugi::xml_node& leg_xml, Leg& leg);
    static void set_expiration(pugi::xml_node& leg_xml, Leg& leg);

    std::vector<Leg> parse_legs(pugi::xml_node& legs_xml);

    bool load(const std::filesystem::path& resource);

    std::string classify(const std::vector<Component>& components, std::vector<int>& order) const;

private:
    std::vector<std::unique_ptr<Combination>> combinations;
};

class Combination {
public:
    Combination(std::string&& name, std::vector<Leg>&& legs);

    std::string get_name() const;

    virtual bool acceptable_combination(const std::vector<Component>& components, std::vector<int>& order) const = 0;

protected:
    std::string name;
    const std::vector<Leg> legs;

    virtual bool acceptable_type(const std::vector<Component>& components, const std::vector<int>& order) const = 0;
    virtual bool acceptable_legs(const std::vector<Component>& components, const std::vector<int>& order) const = 0;
};

class MultipleCombination: public Combination {
public:
    MultipleCombination(std::string&& name, std::vector<Leg>&& legs);

protected:
    bool acceptable_combination(const std::vector<Component>& components, std::vector<int>& order) const override;

    bool check_strike(const std::variant<char, int>& leg_strike, std::unordered_map<char, double>& strikes,
                      double& last_strike, std::size_t& last_signs_amount, const double& test_strike) const;

    bool check_expiration(const std::variant<char, int, ExpirationOffset>& leg_expiration,
                          std::unordered_map<char, Date>& expirations, Date& last_expiration,
                          std::size_t& last_signs_amount, const Date& test_expiration) const;

    virtual bool acceptable_type(const std::vector<Component>& components,
                                 const std::vector<int>& order) const override;
    bool acceptable_legs(const std::vector<Component>& components, const std::vector<int>& order) const;
};

class FixedCombination: public MultipleCombination {
public:
    FixedCombination(std::string&& name, std::vector<Leg>&& legs);

private:
    bool acceptable_type(const std::vector<Component>& components, const std::vector<int>& order) const override;
};

class MoreCombination: public Combination {
public:
    MoreCombination(std::string&& name, std::size_t&& min_count, std::vector<Leg>&& legs);

    bool acceptable_combination(const std::vector<Component>& components, std::vector<int>& order) const override;

protected:
    std::size_t min_count;

    bool acceptable_type(const std::vector<Component>& components, const std::vector<int>& order) const;
    bool acceptable_legs(const std::vector<Component>& components, const std::vector<int>& order) const;
};

#endif  // COMBINATIONS_COMBINATIONS_HPP
