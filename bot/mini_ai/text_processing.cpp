//
// Created by Stas on 11.2.25..
//

#include "text_processing.h"
#include "../../utils/logger.hpp"

#include <locale>
#include <codecvt>
#include <sstream>

std::regex TextProcessing::square_brackets_regex("\\[.*?\\]");
std::regex TextProcessing::replace_chars_regex("[,!@\\\\/()_-]");

const std::unordered_multimap<std::string, std::string> TextProcessing::phonetics = {
        {"о", "а"},
        {"й", "и"},
        {"е", "и"},
        {"ё", "и"},
        {"ы", "и"},
        {"і", "и"},
        {"ї", "и"},
        {"э", "и"},
        {"т", "д"},
        {"з", "с"},
        {"ц", "с"},
        {"ф", "в"},
        {"щ", "ш"},
        {"б", "п"},
        {"г", "х"},
        {"ъ", "ь"},

        {"b", "p"},
        {"d", "t"},
        {"i", "e"},
        {"f", "h"},
        {"g", "j"},
        {"u", "o"},
        {"w", "v"}
};

const std::vector<std::string> TextProcessing::allowed_chars = {
        "0", "1", "2", "3", "4",
        "5", "6", "7", "8", "9",
        " ", ".", "?", "q", "w",
        "e", "r", "t", "y", "u",
        "i", "o", "p", "a", "s",
        "d", "f", "g", "h", "j",
        "k", "l", "z", "x", "c",
        "v", "b", "n", "m", "ї",
        "і", "ё", "й", "ц", "у",
        "к", "е", "н", "г", "ш",
        "щ", "з", "х", "ъ", "ф",
        "ы", "в", "а", "п", "р",
        "о", "л", "д", "ж", "э",
        "я", "ч", "с", "м", "и",
        "т", "ь", "б", "ю"
};

TextProcessing::TextProcessing() noexcept {
    load_synonyms();
}

void TextProcessing::load_synonyms() {
    std::ifstream file(synonyms_filename_);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open synonyms file: " << synonyms_filename_ << std::endl;
        rn;
    }

    std::string line;
    int group_index = 0;

    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string main_word;
        ss >> main_word;
        main_synonyms[group_index] = main_word;

        std::string synonym;
        while (ss >> synonym) {
            synonym_groups[synonym] = group_index;
        }
        synonym_groups[main_word] = group_index;

        ++group_index;
    }

    file.close();
}

void TextProcessing::to_lower_inplace(std::string &str) {
    std::string result;
    std::vector<std::string> utf8_chars = utf8_to_vector(str); // Break string into Unicode code points

    for (const auto &symbol : utf8_chars) {
        std::string lower_symbol = symbol;

        // Use a simple mapping for Cyrillic characters to their lowercase equivalents
        if (symbol >= "А" && symbol <= "Я") {
            // Convert uppercase Cyrillic to lowercase
            lower_symbol = symbol;
            lower_symbol[0] += 32; // UTF-8 offset for uppercase-to-lowercase Cyrillic
        } else if (symbol == "Ё") {
            lower_symbol = "ё"; // Special case for ё
        }
        // Add the lowercase symbol to the result
        result += lower_symbol;
    }

    // Update the original string
    str = result;
}

void TextProcessing::remove_square_brackets_inplace(std::string &str) {
    str = std::regex_replace(str, square_brackets_regex, "");
}

void TextProcessing::replace_chars_inplace(std::string &str) {
    str = std::regex_replace(str, replace_chars_regex, " ");
}

void TextProcessing::filter_chars_inplace(std::string &str) {
    std::string temp;
    std::vector<std::string> chars = utf8_to_vector(str);

    for (const auto& c : chars) {
        if (std::find(allowed_chars.begin(), allowed_chars.end(), c) != allowed_chars.end()) {
            temp += c;
        }
    }
    str = temp;
}

void TextProcessing::remove_duplicates_inplace(std::string &str) {
    if (str.empty()) rn;

    std::vector<std::string> chars = utf8_to_vector(str);
    std::vector<std::string> result;

    if (!chars.empty()) {
        result.push_back(chars[0]);
        for (size_t i = 1; i < chars.size(); ++i) {
            if (chars[i] != chars[i - 1] || chars[i] == " ") {
                result.push_back(chars[i]);
            }
        }
    }
    str.clear();
    for(const auto& s : result) {
        str += s;
    }
}

void TextProcessing::trim_inplace(std::string &str) {
    size_t start = str.find_first_not_of(' ');
    if (start == std::string::npos) {
        str.clear();
        rn;
    }
    size_t end = str.find_last_not_of(' ');
    str = str.substr(start, end - start + 1);
}

std::string TextProcessing::last_sentence(const std::string &str) {
    if (str.empty()) {
        rn "";
    }

    std::string trimmed = str;
    while (!trimmed.empty() && std::isspace(trimmed.back())) {
        trimmed.pop_back();
    }

    if (trimmed.find_first_not_of(".!?") == std::string::npos) {
        rn "";
    }

    size_t last_pos = trimmed.find_last_of(".!?");
    if (last_pos == std::string::npos) {
        rn trimmed;
    }

    size_t start_pos = trimmed.find_last_of(".!?", last_pos - 1);
    if (start_pos == std::string::npos) {
        rn trimmed.substr(0, last_pos + 1);
    }

    rn trimmed.substr(start_pos + 1, last_pos - start_pos);
}

void TextProcessing::replace_synonyms_inplace(std::string& str) {
    std::string result;
    std::regex word_regex(R"([^\s.,!?;:\"'\(\)\[\]{}<>]+)");
    std::sregex_iterator words_begin(str.begin(), str.end(), word_regex);
    std::sregex_iterator words_end;

    size_t last_pos = 0;

    for (std::sregex_iterator it = words_begin; it != words_end; ++it) {
        std::smatch match = *it;
        std::string word = match.str();
        size_t word_pos = match.position();

        result += str.substr(last_pos, word_pos - last_pos);

        std::string lower_word = word;
        std::transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);

        if (synonym_groups.count(lower_word)) {
            int group_index = synonym_groups[lower_word];
            result += main_synonyms[group_index];
        } else {
            result += word;
        }

        last_pos = word_pos + word.length();
    }

    result += str.substr(last_pos);

    str = result;
}

void TextProcessing::replace_phonetics_inplace(std::string &str) {
    std::vector<std::string> chars = utf8_to_vector(str);
    str.clear();
    for (auto& ch : chars) {
        auto it = phonetics.find(ch);
        if (it != phonetics.end()) {
            ch = it->second;
        }
        str += ch;
    }
}

std::vector<std::string> TextProcessing::utf8_to_vector(const std::string &str) {
    std::vector<std::string> result;

    for (size_t i = 0; i < str.size();) {
        unsigned char c = str[i];

        // UTF-8 uses the MSB to determine the number of bytes per character
        size_t char_size = 0;
        if ((c & 0x80) == 0x00) {
            char_size = 1; // Single-byte character (ASCII)
        } else if ((c & 0xE0) == 0xC0) {
            char_size = 2; // Two-byte character
        } else if ((c & 0xF0) == 0xE0) {
            char_size = 3; // Three-byte character
        } else if ((c & 0xF8) == 0xF0) {
            char_size = 4; // Four-byte character
        } else {
            throw std::runtime_error("Invalid UTF-8 encoding");
        }

        result.emplace_back(str.substr(i, char_size));
        i += char_size; // Advance position
    }

    rn result;
}

void TextProcessing::textProcessor(std::string &str) {
    //std::cout<<str<<std::endl;
    to_lower_inplace(str);
    //std::cout<<str<<std::endl;
    remove_square_brackets_inplace(str);
    //std::cout<<str<<std::endl;
    replace_chars_inplace(str);
    //std::cout<<str<<std::endl;
    filter_chars_inplace(str);
    //std::cout<<str<<std::endl;
    remove_duplicates_inplace(str);
    //std::cout<<str<<std::endl;
    trim_inplace(str);
    //std::cout<<str<<std::endl;
    str = last_sentence(str);
    //std::cout<<str<<std::endl;
    trim_inplace(str);
    //std::cout<<str<<std::endl;
    replace_synonyms_inplace(str);
    //std::cout<<str<<std::endl;

//    replace_phonetics_inplace(str);

    //std::cout<<str<<std::endl;
}