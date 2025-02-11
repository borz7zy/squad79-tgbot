//
// Created by Stas on 11.2.25..
//

#pragma once
#include "../main.hpp"
#include "logger.hpp"
#include "../singletone.hpp"

class JaroWinkler : public Singleton<JaroWinkler>
{
public:
    JaroWinkler() = default;
    virtual ~JaroWinkler() = default;

    // Prevent copying and moving
    JaroWinkler(const JaroWinkler&) = delete;
    JaroWinkler& operator=(const JaroWinkler&) = delete;
    JaroWinkler(JaroWinkler&&) = delete;
    JaroWinkler& operator=(JaroWinkler&&) = delete;

    [[nodiscard]] double jaro_distance(const std::string& s1, const std::string& s2) const;

    [[nodiscard]] double jaro_winkler_distance(const std::string& s1, const std::string& s2, double prefix_weight) const;

private:
    static constexpr size_t MAX_PREFIX_LENGTH = 4;
};