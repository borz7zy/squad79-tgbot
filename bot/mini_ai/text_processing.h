//
// Created by Stas on 11.2.25..
//

#pragma once
#include "../../main.hpp"
#include "../../singletone.hpp"

class TextProcessing : public Singleton<TextProcessing> {
private:
    static std::regex square_brackets_regex;
    static std::regex replace_chars_regex;
    static const std::unordered_multimap<std::string, std::string> phonetics;
    static const std::vector<std::string> allowed_chars;

    std::unordered_map<std::string, int> synonym_groups;
    std::unordered_map<int, std::string> main_synonyms;
    const std::string synonyms_filename_ = "synonyms.txt";

    void load_synonyms();

    std::vector<std::string> utf8_to_vector(const std::string& str);

    void to_lower_inplace(std::string &str);
    void remove_square_brackets_inplace(std::string &str);
    void replace_chars_inplace(std::string &str);
    void filter_chars_inplace(std::string &str);
    void remove_duplicates_inplace(std::string &str);
    void trim_inplace(std::string &str);
    std::string last_sentence(const std::string& str);
    void replace_synonyms_inplace(std::string &str);
    void replace_phonetics_inplace(std::string &str);
public:
    TextProcessing() noexcept;
    ~TextProcessing() = default;

    void textProcessor(std::string &str);
};