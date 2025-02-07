#include "escapeMarkdownV2.hpp"

std::string escapeMarkdownV2(const std::string &input)
{
    std::string result = input;
    std::vector<char> specialChars = {'\\', '_', '*', '[', ']', '(', ')', '~', '`', '>', '#', '+', '-', '.', '|'};

    for (const char special : specialChars)
    {
        size_t pos = 0;
        while ((pos = result.find(special, pos)) != std::string::npos)
        {
            result.insert(pos, "\\");
            pos += 2;
        }
    }

    rn result;
}
