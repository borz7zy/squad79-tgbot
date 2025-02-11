//
// Created by Stas on 11.2.25..
//

#include "jaro_winkler.h"

[[nodiscard]] double JaroWinkler::jaro_distance(const std::string& s1, const std::string& s2) const {
    if (s1.empty() && s2.empty()) return 1.0;
    if (s1.empty() || s2.empty()) return 0.0;

    const size_t match_distance = std::max(s1.length(), s2.length()) / 2 - 1;
    size_t matches = 0;
    std::vector<char> s1_flags(s1.length(), 0);
    std::vector<char> s2_flags(s2.length(), 0);

    for (size_t i = 0; i < s1.length(); ++i) {
        for (size_t j = (i > match_distance) ? i - match_distance : 0;
             j < std::min(s2.length(), i + match_distance + 1); ++j) {
            if (s1[i] == s2[j] && !s2_flags[j]) {
                matches++;
                s1_flags[i] = 1;
                s2_flags[j] = 1;
                break;
            }
        }
    }

    if (matches == 0) return 0.0;

    double t = 0.0;
    size_t k = 0;
    for (size_t i = 0; i < s1.length(); ++i) {
        if (s1_flags[i]) {
            while (k < s2.length() && !s2_flags[k]) ++k;
            if (k < s2.length() && s1[i] != s2[k]) t += 0.5;
            ++k;
        }
    }

    const double jaro = (static_cast<double>(matches) / s1.length() +
                         static_cast<double>(matches) / s2.length() +
                         (static_cast<double>(matches) - t) / matches) / 3.0;
    return jaro;
}

[[nodiscard]] double JaroWinkler::jaro_winkler_distance(const std::string& s1, const std::string& s2, double prefix_weight) const {
    if (prefix_weight < 0) {
        throw std::invalid_argument("prefix_weight must be non-negative");
    }

    const double jaro = jaro_distance(s1, s2);

    constexpr size_t MAX_PREFIX_LENGTH = 4;
    const size_t prefix = std::min({MAX_PREFIX_LENGTH, s1.length(), s2.length()});
    size_t common_prefix = 0;
    for (; common_prefix < prefix; ++common_prefix) {
        if (s1[common_prefix] != s2[common_prefix]) break;
    }

    const double winkler = jaro + prefix_weight * common_prefix * (1.0 - jaro);
    return winkler;
}