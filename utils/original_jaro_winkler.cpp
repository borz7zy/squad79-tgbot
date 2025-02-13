//
// Created by Stas on 13.2.25..
//

#include "original_jaro_winkler.h"
#include <cmath>

double JaroWinkler::getThreshold() const {
    return threshold;
}

double JaroWinkler::similarity(const std::string& s1, const std::string& s2) {
    if (s1.empty() || s2.empty()) {
        throw std::invalid_argument("s1 and s2 must not be null or empty");
    }

    if (s1 == s2) {
        return 1.0;
    }

    auto mtp = matches(s1, s2);
    float m = mtp[0];
    if (m == 0) {
        return 0.0f;
    }

    double j = ((m / s1.length() + m / s2.length() + (m - mtp[1]) / m)) / THREE;
    double jw = j;

    if (j > getThreshold()) {
        jw = j + std::min(JW_COEF, 1.0 / mtp[THREE]) * mtp[2] * (1 - j);
    }
    return jw;
}

double JaroWinkler::distance(const std::string& s1, const std::string& s2) {
    return 1.0 - similarity(s1, s2);
}

std::vector<int> JaroWinkler::matches(const std::string& s1, const std::string& s2) {
    std::string max = s1.length() > s2.length() ? s1 : s2;
    std::string min = s1.length() > s2.length() ? s2 : s1;

    int range = std::max(static_cast<int>(max.length() / 2 - 1), 0);
    std::vector<int> matchIndexes(min.length(), -1);
    std::vector<bool> matchFlags(max.length(), false);
    int matches = 0;

    for (int mi = 0; mi < min.length(); ++mi) {
        char c1 = min[mi];
        for (int xi = std::max(mi - range, 0), xn = std::min(static_cast<size_t>(mi + range + 1), max.length()); xi < xn; ++xi) {
            if (!matchFlags[xi] && c1 == max[xi]) {
                matchIndexes[mi] = xi;
                matchFlags[xi] = true;
                matches++;
                break;
            }
        }
    }

    std::vector<char> ms1(matches), ms2(matches);
    for (int i = 0, si = 0; i < min.length(); ++i) {
        if (matchIndexes[i] != -1) {
            ms1[si++] = min[i];
        }
    }
    for (int i = 0, si = 0; i < max.length(); ++i) {
        if (matchFlags[i]) {
            ms2[si++] = max[i];
        }
    }

    int transpositions = 0;
    for (int mi = 0; mi < ms1.size(); ++mi) {
        if (ms1[mi] != ms2[mi]) {
            transpositions++;
        }
    }

    int prefix = 0;
    for (int mi = 0; mi < min.length(); ++mi) {
        if (s1[mi] == s2[mi]) {
            prefix++;
        } else {
            break;
        }
    }

    return {matches, transpositions / 2, prefix, static_cast<int>(max.length())};
}