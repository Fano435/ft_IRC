#include "server.h"

std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end;

    while ((end = input.find(delimiter, start)) != std::string::npos) {
        if (end > start) {
            result.push_back(input.substr(start, end - start));
        }
        start = end + 1;
    }

    if (start < input.size()) {
        result.push_back(input.substr(start));
    }

    return result;
}

bool isValidNickname(const std::string& nick) {
    if (nick.empty())
        return false;

    char first = nick[0];

    if (first == ':' || first == '#' || std::isdigit(first))
        return false;

    for (size_t i = 0; i < nick.length(); ++i) {
        char c = nick[i];

        if (std::isalnum(c))
            continue;

        if (c == '[' || c == ']' ||
            c == '{' || c == '}' ||
            c == '\\' || c == '|')
            continue;

        if (std::isspace(c))
            return false;

        return false;
    }

    return true;
}