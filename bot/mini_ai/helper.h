//
// Created by Stas on 11.2.25..
//

#pragma once

#include <string>
#include <chrono>

struct Message {
    std::string text;
};

struct Answer {
    std::string text;
};

struct AnswerRecord {
    std::string question;
    Answer answer;
    double heat;
    mutable std::chrono::steady_clock::time_point last_used;

    AnswerRecord(std::string q, Answer a, double h)
            : question(std::move(q)), answer(std::move(a)), heat(h),
              last_used(std::chrono::steady_clock::now()) {}
};