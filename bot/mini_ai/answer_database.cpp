//
// Created by Stas on 11.2.25..
//

#include "answer_database.h"

namespace fs = std::filesystem;

AnswerDatabase::AnswerDatabase() noexcept {
    load_from_file();
}

[[nodiscard]] Answer AnswerDatabase::find_best_match(const Message &message, const std::vector<Message>& context) const noexcept {
    constexpr double threshold = 0.85;
    constexpr size_t top_n = 5;
    constexpr size_t context_size = 3;
    constexpr double context_weight = 0.2;
    constexpr double cooldown_factor = 0.8;
    constexpr std::chrono::minutes cooldown_time(5);

    std::vector<std::pair<double, const Answer*>> candidates;
    candidates.reserve(answers_.size());

    auto now = std::chrono::steady_clock::now();

    for (const auto& record : answers_) {
        double distance = JaroWinkler::Get()->jaro_winkler_distance(message.text, record.question, 0.1);

        // Учитываем контекст
        double context_score = 0.0;
        for (size_t i = 0; i < std::min(context.size(), context_size); ++i) {
            context_score += JaroWinkler::Get()->jaro_winkler_distance(context[context.size() - 1 - i].text, record.question, 0.1) * std::pow(0.5, i);
        }
        distance += context_weight * context_score / std::min(context.size(), context_size);

        if (distance >= threshold) {
            double score = distance * record.heat;

            // Применяем "охлаждение"
            auto time_since_last_use = std::chrono::duration_cast<std::chrono::minutes>(now - record.last_used);
            if (time_since_last_use < cooldown_time) {
                score *= std::pow(cooldown_factor, 1.0 - static_cast<double>(time_since_last_use.count()) / cooldown_time.count());
            }

            candidates.emplace_back(score, &record.answer);
        }
    }

    if (candidates.empty()) {
        return {"Извини, я не понял тебя!"};
    }

    // Сортируем кандидатов по убыванию score
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Оставляем только top_n кандидатов
    candidates.resize(std::min(candidates.size(), top_n));

    // Взвешенный случайный выбор
    std::vector<double> weights;
    weights.reserve(candidates.size());
    for (const auto& candidate : candidates) {
        weights.push_back(candidate.first);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d(weights.begin(), weights.end());

    const Answer* best_answer = candidates[d(gen)].second;

    // Обновляем время последнего использования
    for (auto& record : answers_) {
        if (&record.answer == best_answer) {
            record.last_used = now;
            break;
        }
    }

#ifdef DEBUG
    Logger::Get()->Log("Лучший ответ найден: %s", best_answer->text.c_str());
#endif

    return *best_answer;
}

void AnswerDatabase::add_answer(std::string_view question, const Answer& answer, double heat_level) {
    answers_.push_back({std::string(question), answer, heat_level});
    save_to_file();
}

void AnswerDatabase::load_from_file() noexcept {
    if (!fs::exists(filename_)) {
        Logger::Get()->Log("Файл базы данных не существует: %s", filename_.c_str());
        return;
    }

    std::ifstream file(filename_);
    if (!file) {
        Logger::Get()->Log("Не удалось открыть файл с базой данных: %s", filename_.c_str());
        return;
    }

    answers_.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string question, answer_text, heat_str;
        if (std::getline(ss, question, '\\') &&
            std::getline(ss, answer_text, '\\') &&
            std::getline(ss, heat_str)) {
            try {
                int heat_level = std::stoi(heat_str);
                double heat = convert_heat(heat_level);
                answers_.push_back({question, {answer_text}, heat});
            } catch (const std::exception& e) {
                Logger::Get()->Log("Ошибка при чтении строки: %s", line.c_str());
            }
        }
    }
}

void AnswerDatabase::save_to_file() const noexcept {
    std::ofstream file(filename_);
    if (!file) {
        Logger::Get()->Log("Не удалось открыть файл для записи: %s", filename_.c_str());
        return;
    }

    for (const auto& record : answers_) {
        file << record.question << "\\" << record.answer.text << "\\" << record.heat << '\n';
    }
}

[[nodiscard]] double AnswerDatabase::convert_heat(int heat_level) noexcept {
    static std::mt19937 gen{std::random_device{}()};

    if (heat_level >= 2 && heat_level <= 10) {
        return std::uniform_real_distribution<>(0.7, 1.0)(gen);
    } else if (heat_level == 1) {
        return std::uniform_real_distribution<>(0.4, 0.69)(gen);
    } else if (heat_level == 0) {
        return std::uniform_real_distribution<>(0.1, 1.39)(gen);
    }
    return 0.5; // дефолт, если уровень некорректный
}