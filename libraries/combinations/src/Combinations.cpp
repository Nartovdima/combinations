#include "combinations/Combinations.hpp"

Combination::Combination(std::string&& name, std::vector<Leg>&& legs) : name(std::move(name)), legs(std::move(legs)) {}

std::string Combination::get_name() const {
    return name;
}

MultipleCombination::MultipleCombination(std::string&& name, std::vector<Leg>&& legs)
    : Combination(std::move(name), std::move(legs)) {}

FixedCombination::FixedCombination(std::string&& name, std::vector<Leg>&& legs)
    : MultipleCombination(std::move(name), std::move(legs)) {}

MoreCombination::MoreCombination(std::string&& name, std::size_t&& min_count, std::vector<Leg>&& legs)
    : Combination(std::move(name), std::move(legs)), min_count(min_count) {}

void Combinations::set_ratio(pugi::xml_node& leg_xml, Leg& leg) {
    const auto& ratio = leg_xml.attribute("ratio");

    if (!std::strcmp(ratio.value(), "+") || !std::strcmp(ratio.value(), "-")) {
        leg.ratio = ratio.value()[0];
    } else {
        leg.ratio = ratio.as_double();
    }
}

void Combinations::set_strike(pugi::xml_node& leg_xml, Leg& leg) {
    const auto& strike = leg_xml.attribute("strike");
    if (strike) {
        leg.strike = strike.value()[0];
        return;
    }
    const auto& strike_offset = leg_xml.attribute("strike_offset");
    if (strike_offset) {
        int len    = static_cast<int>(std::strlen(strike_offset.value()));
        leg.strike = ((strike_offset.value()[0]) == '+' ? len : -len);
        return;
    }
    leg.strike = Leg::invalid_strike;
}

void Combinations::set_expiration(pugi::xml_node& leg_xml, Leg& leg) {
    const auto& expiration = leg_xml.attribute("expiration");
    if (expiration) {
        leg.expiration = expiration.value()[0];
        return;
    }

    const auto& expiration_offset = leg_xml.attribute("expiration_offset");
    if (expiration_offset && (expiration_offset.value()[0] == '+' || expiration_offset.value()[0] == '-')) {
        int len        = static_cast<int>(std::strlen(expiration_offset.value()));
        leg.expiration = ((expiration_offset.value()[0]) == '+' ? len : -len);
        return;
    }

    if (expiration_offset) {
        std::size_t num     = 1;
        const auto& exp_str = expiration_offset.value();
        if (std::strlen(expiration_offset.value()) > 1) {
            char* end = nullptr;
            num       = std::strtoul(exp_str, &end, 10);
        }

        TimePeriods per = static_cast<TimePeriods>(exp_str[std::strlen(exp_str) - 1]);

        leg.expiration = ExpirationOffset(num, per);
        return;
    }
    leg.expiration = Leg::invalid_expiration;
}

std::vector<Leg> Combinations::parse_legs(pugi::xml_node& legs_xml) {
    std::vector<Leg> legs_storage;

    for (pugi::xml_node curr_leg : legs_xml.children("leg")) {
        Leg tmp;
        tmp.type = static_cast<InstrumentType>(curr_leg.attribute("type").value()[0]);

        set_ratio(curr_leg, tmp);
        set_strike(curr_leg, tmp);
        set_expiration(curr_leg, tmp);

        legs_storage.push_back(std::move(tmp));
    }

    return legs_storage;
}

bool Combinations::load(const std::filesystem::path& resource) {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(resource.c_str());
    if (!result) {
        return false;
    }

    const pugi::xml_node comb = doc.child("combinations");

    if (!comb) {
        return false;
    }

    for (pugi::xml_node curr_comb : comb.children("combination")) {
        pugi::xml_node legs_xml      = curr_comb.child("legs");
        std::vector<Leg> legs        = parse_legs(legs_xml);
        const auto& legs_cardinality = legs_xml.attribute("cardinality").value();
        std::string name             = curr_comb.attribute("name").value();

        if (!std::strcmp(legs_cardinality, "fixed")) {
            combinations.emplace_back(std::make_unique<FixedCombination>(std::move(name), std::move(legs)));
        }
        if (!std::strcmp(legs_cardinality, "multiple")) {
            combinations.emplace_back(std::make_unique<MultipleCombination>(std::move(name), std::move(legs)));
        }
        if (!std::strcmp(legs_cardinality, "more")) {
            combinations.emplace_back(std::make_unique<MoreCombination>(
                std::move(name), legs_xml.attribute("mincount").as_uint(), std::move(legs)));
        }
    }

    return true;
}

bool MultipleCombination::acceptable_combination(const std::vector<Component>& components,
                                                 std::vector<int>& order) const {
    if (!acceptable_type(components)) {
        return false;
    }

    std::iota(order.begin(), order.end(), 0);
    do {
        if (acceptable_legs(components, order)) {
            return true;
        }
    } while (std::next_permutation(order.begin(), order.end()));

    return false;
}

bool MultipleCombination::acceptable_type(const std::vector<Component>& components) const {
    if (components.empty() || components.size() % legs.size()) {
        return false;
    }

    std::unordered_set<InstrumentType> legs_type;

    for (const auto& it : legs) {
        legs_type.insert(it.type);
    }

    for (const auto& it : components) {
        if (!legs_type.contains(it.type)) {
            return false;
        }
    }

    return true;
}

bool MultipleCombination::check_strike(const std::variant<char, int>& leg_strike,
                                       std::unordered_map<char, double>& strikes, double& last_strike,
                                       std::size_t& last_signs_amount, const double& test_strike) const {
    if (std::holds_alternative<char>(leg_strike)) {
        const auto& symb = std::get<char>(leg_strike);
        if (symb == Leg::invalid_strike) {
            last_strike       = test_strike;
            last_signs_amount = 0;
            return true;
        }
        if (strikes.contains(symb)) {
            const auto& it = strikes.find(symb);
            if (it->second != test_strike) {
                return false;
            }
        } else {
            strikes.insert({symb, test_strike});
        }
        last_signs_amount = 0;
    } else if (std::holds_alternative<int>(leg_strike)) {
        const auto& strike_offset = std::get<int>(leg_strike);
        if (strike_offset != 0 && strike_offset == static_cast<int>(last_signs_amount)) {
            if (test_strike != last_strike) {
                return false;
            }
        } else if ((strike_offset > 0 && test_strike <= last_strike) ||
                   (strike_offset < 0 && test_strike >= last_strike)) {
            return false;
        }
        last_signs_amount = strike_offset;
    }
    last_strike = test_strike;
    return true;
}

bool MultipleCombination::check_expiration(const std::variant<char, int, ExpirationOffset>& leg_expiration,
                                           std::unordered_map<char, Date>& expirations, Date& last_expiration,
                                           std::size_t& last_signs_amount, const Date& test_expiration) const {
    if (std::holds_alternative<char>(leg_expiration)) {
        const auto& symb = std::get<char>(leg_expiration);
        if (symb == Leg::invalid_expiration) {
            last_expiration   = test_expiration;
            last_signs_amount = 0;
            return true;
        }
        if (expirations.contains(symb)) {
            const auto& it = expirations.find(symb);
            if (it->second != test_expiration) {
                return false;
            }
        } else {
            expirations.insert({symb, test_expiration});
        }
        last_signs_amount = 0;
    } else if (std::holds_alternative<int>(leg_expiration)) {
        const auto& expiration_offset = std::get<int>(leg_expiration);
        if (expiration_offset != 0 && expiration_offset == static_cast<int>(last_signs_amount)) {
            if (test_expiration != last_expiration) {
                return false;
            }
        } else if ((expiration_offset > 0 && test_expiration <= last_expiration) ||
                   (expiration_offset < 0 && test_expiration >= last_expiration)) {
            return false;
        }
        last_signs_amount = expiration_offset;
    } else if (std::holds_alternative<ExpirationOffset>(leg_expiration)) {
        const auto& exp_off = std::get<ExpirationOffset>(leg_expiration);
        if (!last_expiration.check_offset(exp_off, test_expiration)) {
            return false;
        }
        return true;
    }
    last_expiration = test_expiration;
    return true;
}

bool MultipleCombination::acceptable_legs(const std::vector<Component>& components,
                                          const std::vector<int>& order) const {
    std::unordered_map<char, double> strikes;
    double last_strike                   = 0;
    std::size_t strike_last_signs_amount = 0;
    std::unordered_map<char, Date> expirations;
    Date last_expiration;
    std::size_t expiration_last_signs_amount = 0;

    for (std::size_t curr_comp_ind = 0; curr_comp_ind < components.size(); curr_comp_ind++) {
        const auto& leg            = legs[curr_comp_ind % legs.size()];
        const auto& curr_component = components[order[curr_comp_ind]];

        if (!(curr_comp_ind % legs.size())) {
            strikes.clear();
            last_strike              = 0;
            strike_last_signs_amount = 0;

            expirations.clear();
            expiration_last_signs_amount = 0;
        }

        if (leg.type != curr_component.type) {
            return false;
        }

        if ((std::holds_alternative<double>(leg.ratio) && std::get<double>(leg.ratio) != curr_component.ratio) ||
            (std::holds_alternative<char>(leg.ratio) &&
             std::get<char>(leg.ratio) != ((curr_component.ratio > 0) ? '+' : '-'))) {
            return false;
        }

        if (!check_strike(leg.strike, strikes, last_strike, strike_last_signs_amount, curr_component.strike)) {
            return false;
        }

        if (!check_expiration(leg.expiration, expirations, last_expiration, expiration_last_signs_amount,
                              curr_component.expiration)) {
            return false;
        }
    }
    return true;
}

bool FixedCombination::acceptable_type(const std::vector<Component>& components) const {
    return components.size() == legs.size();
}

bool MoreCombination::acceptable_type(const std::vector<Component>& components) const {
    return components.size() >= min_count;
}

bool MoreCombination::acceptable_legs(const std::vector<Component>& components, const std::vector<int>& order) const {
    const auto& leg = legs[0];

    for (const auto& it : order) {
        const auto& curr_component = components[it];

        if (leg.type != curr_component.type &&
            !(leg.type == InstrumentType::O &&
              (curr_component.type == InstrumentType::C || curr_component.type == InstrumentType::P))) {
            return false;
        }

        if ((std::holds_alternative<double>(leg.ratio) && std::get<double>(leg.ratio) != curr_component.ratio) ||
            (std::holds_alternative<char>(leg.ratio) &&
             std::get<char>(leg.ratio) != ((curr_component.ratio > 0) ? '+' : '-'))) {
            return false;
        }
    }

    return true;
}

bool MoreCombination::acceptable_combination(const std::vector<Component>& components, std::vector<int>& order) const {
    std::iota(order.begin(), order.end(), 0);
    return acceptable_type(components) && acceptable_legs(components, order);
}

std::string Combinations::classify(const std::vector<Component>& components, std::vector<int>& order) const {
    std::vector<int> tmp_order(components.size());

    for (const auto& comb : combinations) {
        if (comb->acceptable_combination(components, tmp_order)) {
            order.resize(tmp_order.size());
            for (std::size_t i = 0; i < tmp_order.size(); i++) {
                order[tmp_order[i]] = static_cast<int>(i + 1);
            }
            return comb->get_name();
        }
    }

    return "Unclassified";
}
