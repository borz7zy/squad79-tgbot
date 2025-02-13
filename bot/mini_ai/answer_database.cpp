//
// Created by Stas on 11.2.25..
//

#include "answer_database.h"
#include "../../utils/original_jaro_winkler.h"

namespace fs = std::filesystem;

AnswerDatabase::AnswerDatabase() noexcept {
    load_from_file();
}

[[nodiscard]] Answer AnswerDatabase::find_best_match(const Message &message) const noexcept {
    constexpr double threshold = 0.7;
    constexpr size_t top_n = 20;

//    std::vector<std::pair<double, const Answer*>> candidates;
    std::vector<AnswerCandidate> candidates;
    candidates.reserve(answers_.size());

    double maxSimilarity = 0.0;

    for (const auto& record : answers_) {
        double distance = JaroWinkler::Get()->similarity(message.text, record.question);

        if (distance >= threshold) {
//            double score = distance * record.heat;
//            candidates.emplace_back(score, &record.answer);
            candidates.emplace_back(&record.answer, distance);
            maxSimilarity = std::max(maxSimilarity, distance);
        }
    }

    if (candidates.empty()) {
        return {"Извини, я не понял тебя!"};
    }

//    std::sort(candidates.begin(), candidates.end(),
//              [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<AnswerCandidate> theBest;
    for(const auto& candidate : candidates) {
        if(candidate.similarity == maxSimilarity) {
            theBest.push_back(candidate);
        }
    }

//    candidates.resize(std::min(candidates.size(), top_n));

//    std::vector<double> weights;
//    weights.reserve(candidates.size());
//    for (const auto& candidate : candidates) {
//        weights.push_back(candidate.first);
//    }

    std::random_device rd;
    std::mt19937 gen(rd());

//    std::discrete_distribution<> d(weights.begin(), weights.end());
    const Answer* best_answer;

//    const Answer* best_answer = candidates[d(gen)].second;

    if(theBest.size() == 1)
        best_answer = theBest[0].answer;
    else{
        std::uniform_int_distribution<> distrib(0, theBest.size() - 1);
        best_answer = theBest[distrib(gen)].answer;
    }

#ifdef DEBUG
    Logger::Get()->Log("Лучший ответ найден: %s", best_answer->text.c_str());
#endif

    const AnswerRecord* best_answer_record = nullptr;
    for (const auto& record : answers_) {
        if (&record.answer == best_answer) {
            best_answer_record = &record;
            break;
        }
    }

    if (best_answer_record) {
        std::cout << "Вопрос: " << best_answer_record->question << std::endl;
        std::cout << "Ответ: " << best_answer->text << std::endl;
    }

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