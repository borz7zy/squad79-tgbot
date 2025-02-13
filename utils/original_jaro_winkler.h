//
// Created by Stas on 13.2.25..
//

#pragma once
#include "../main.hpp"
#include "logger.hpp"
#include "../singletone.hpp"

class JaroWinkler : public Singleton<JaroWinkler> {
private:
    static constexpr double DEFAULT_THRESHOLD = 0.7;
    static constexpr int THREE = 3;
    static constexpr double JW_COEF = 0.1;
    double threshold;

public:
    JaroWinkler() : threshold(DEFAULT_THRESHOLD) {}

    JaroWinkler(double threshold) : threshold(threshold) {}

    double getThreshold() const;

    // Вычислить схожесть по методу Jaro-Winkler
    double similarity(const std::string& s1, const std::string& s2);

    // Вычислить дистанцию (1 - схожесть)
    double distance(const std::string& s1, const std::string& s2);

private:
    // Получить совпадения и количество транспозиций
    std::vector<int> matches(const std::string& s1, const std::string& s2);
};
