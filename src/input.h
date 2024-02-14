#ifndef INPUT_H
#define INPUT_H

#include <regex>
#include <string>
#include <vector>

#include <iostream>

struct Input {
    bool rendered = false;
    std::string regex_inp = "";
    std::string regex_error;
    std::vector<std::string> raw_test_input = { "" };
    std::vector<std::string> render_test_input = { "" };

    void render() {
        std::regex re;
        try {
            re = std::regex(regex_inp);
        } catch (const std::regex_error &e) {
            regex_error = std::string(e.what());
            return;
        }

        std::vector<std::string> tmp_lines;

        for (auto &&line : raw_test_input) {
            std::string tmp;
            std::smatch m;

            std::regex_search(line, m, re);
            tmp_lines.push_back(
                std::regex_replace(line, re, "\x1b[32m$0\x1b[0m"));
        }

        render_test_input = tmp_lines;
        rendered = true;
        return;
    }
};

#endif // INPUT_H
