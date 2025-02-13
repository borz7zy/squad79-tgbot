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
};