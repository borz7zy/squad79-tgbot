//
// Created by Stas on 11.2.25..
//

#pragma once
#include "../../main.hpp"
#include "../../utils/logger.hpp"
#include "../../singletone.hpp"
#include "helper.h"
#include "../../utils/jaro_winkler.h"

class AnswerDatabase : public Singleton<AnswerDatabase>
{
public:
    AnswerDatabase() noexcept;
    virtual ~AnswerDatabase() = default;

    AnswerDatabase(const AnswerDatabase&) = delete;
    AnswerDatabase& operator=(const AnswerDatabase&) = delete;
    AnswerDatabase(AnswerDatabase&&) = delete;
    AnswerDatabase& operator=(AnswerDatabase&&) = delete;

    [[nodiscard]] Answer find_best_match(const Message &message, const std::vector<Message>& context) const noexcept;
    void add_answer(std::string_view question, const Answer& answer, double heat_level);

private:
    void load_from_file() noexcept;
    void save_to_file() const noexcept;
    [[nodiscard]] static double convert_heat(int heat_level) noexcept;

    const std::string filename_ = "mini_ai_db.txt";
    std::vector<AnswerRecord> answers_;
};