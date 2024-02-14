#include "model.h"
#include "utils.h"

void Model::draw_ui(Input &inp) {
    rawterm::Pos term_size = rawterm::get_term_size();
    int half = (term_size.horizontal / 2) - 6;

    rawterm::clear_screen();
    rawterm::move_cursor({ 1, 1 });

    std::cout << rawterm::inverse(std::string(half, ' '))
              << rawterm::inverse("regex viewer")
              << rawterm::inverse(std::string(half, ' '));
    if (term_size.horizontal % 2)
        std::cout << rawterm::inverse(" ");

    if (active_view) {
        std::cout << "┌";
        std::cout << repeat(term_size.horizontal - 2, "─");
        std::cout << "┐\r\n";

        unsigned int whitespace =
            term_size.horizontal - 3 - inp.regex_inp.size();
        std::cout << "│ " + inp.regex_inp;
        if (inp.regex_error.size()) {
            if (inp.regex_error.size() > whitespace) {
                inp.regex_error = inp.regex_error.substr(0, whitespace - 1);
            } else if (inp.regex_error.size() < whitespace) {
                whitespace = whitespace - inp.regex_error.size();
                whitespace--;
            }

            std::cout << " \x1b[31m" << inp.regex_error << "\x1b[0m";
        }

        std::cout << std::string(whitespace, ' ') + "│\r\n";

        std::cout << "└";
        std::cout << repeat(term_size.horizontal - 2, "─");
        std::cout << "┘\r\n";
    } else {
        std::cout << rawterm::fg("┌", rawterm::cyan);
        std::cout << rawterm::fg(repeat(term_size.horizontal - 2, "─"),
                                 rawterm::cyan);
        std::cout << rawterm::fg("┐\r\n", rawterm::cyan);

        unsigned int whitespace =
            term_size.horizontal - 3 - inp.regex_inp.size();
        std::cout << rawterm::fg("│ " + inp.regex_inp, rawterm::cyan);
        if (inp.regex_error.size()) {
            if (inp.regex_error.size() > whitespace) {
                inp.regex_error = inp.regex_error.substr(0, whitespace - 1);
            } else if (inp.regex_error.size() < whitespace) {
                whitespace = whitespace - inp.regex_error.size();
                whitespace--;
            }
            std::cout << " \x1b[31m" << inp.regex_error << "\x1b[0m";
        }
        std::cout << std::string(whitespace, ' ') +
                         rawterm::fg("│", rawterm::cyan) + "\r\n";

        std::cout << rawterm::fg("└", rawterm::cyan);
        std::cout << rawterm::fg(repeat(term_size.horizontal - 2, "─"),
                                 rawterm::cyan);
        std::cout << rawterm::fg("┘\r\n", rawterm::cyan);
    }

    std::cout << rawterm::inverse(std::string(term_size.horizontal, ' '));

    if (active_view) {
        std::cout << rawterm::fg("┌", rawterm::cyan);
        std::cout << rawterm::fg(repeat(term_size.horizontal - 2, "─"),
                                 rawterm::cyan);
        std::cout << rawterm::fg("┐\r\n", rawterm::cyan);

        for (unsigned int i = 0; i < inp.raw_test_input.size(); i++) {
            int ws = term_size.horizontal - inp.raw_test_input[i].size() - 3;
            if (inp.rendered) {
                std::cout << rawterm::fg("│ ", rawterm::cyan)
                          << inp.render_test_input[i] + std::string(ws, ' ')
                          << rawterm::fg("│\r\n", rawterm::cyan);
            } else {
                std::cout << rawterm::fg("│ ", rawterm::cyan)
                          << inp.raw_test_input[i] + std::string(ws, ' ')
                          << rawterm::fg("│\r\n", rawterm::cyan);
            }
        }

        int empty_space = term_size.vertical - 8 - inp.raw_test_input.size();
        for (int i = 0; i < empty_space; i++) {
            std::cout << rawterm::fg(
                "│" + std::string(term_size.horizontal - 2, ' ') + "│\r\n",
                rawterm::cyan);
        }

        std::cout << rawterm::fg("└", rawterm::cyan);
        std::cout << rawterm::fg(repeat(term_size.horizontal - 2, "─"),
                                 rawterm::cyan);
        std::cout << rawterm::fg("┘", rawterm::cyan);
    } else {
        std::cout << "┌";
        std::cout << repeat(term_size.horizontal - 2, "─");
        std::cout << "┐\r\n";

        for (unsigned int i = 0; i < inp.raw_test_input.size(); i++) {
            int ws = term_size.horizontal - inp.raw_test_input[i].size() - 3;
            if (inp.rendered) {
                std::cout << "│ " + inp.render_test_input[i] +
                                 std::string(ws, ' ') + "│\r\n";
            } else {
                std::cout << "│ " + inp.raw_test_input[i] +
                                 std::string(ws, ' ') + "│\r\n";
            }
        }

        int empty_space = term_size.vertical - 8 - inp.raw_test_input.size();
        for (int i = 0; i < empty_space; i++) {
            std::cout << "│" + std::string(term_size.horizontal - 2, ' ') +
                             "│\r\n";
        }

        std::cout << "└";
        std::cout << repeat(term_size.horizontal - 2, "─");
        std::cout << "┘";
    }

    std::cout << std::flush;
    set_cursor();
}

void Model::read_keypress(Input *ptr) {
    while (true) {
        rawterm::Key k = rawterm::process_keypress();
        rawterm::Mod modifier = k.getMod();

        if (k.code == 'q' && modifier == rawterm::Mod::Control) {
            exit = true;
            return;
        }

        if (active_view == 0) {
            // regex view
            if (modifier == rawterm::Mod::Tab) {
                ptr->rendered = false;
                active_view = 1;
            } else if (modifier == rawterm::Mod::Enter) {
                ptr->render();
            } else if (modifier == rawterm::Mod::Arrow) {
                switch (k.code) {
                case 'C': // Right
                    if (re_inp_cursor <
                        static_cast<int>(ptr->regex_inp.size())) {
                        rawterm::move_cursor_right();
                        re_inp_cursor++;
                    }
                    break;
                case 'D': // Left
                    if (re_inp_cursor) {
                        rawterm::move_cursor_left();
                        re_inp_cursor--;
                    }
                    break;
                }
            } else if (modifier == rawterm::Mod::Backspace) {
                if (ptr->regex_inp.size()) {
                    if (re_inp_cursor > 0) {
                        ptr->regex_inp.erase(re_inp_cursor - 1, 1);
                        // rawterm::move_cursor_left();
                        re_inp_cursor--;
                    }
                }
            } else if (k.isCharInput()) {
                ptr->regex_inp.insert(re_inp_cursor, 1, k.code);
                rawterm::move_cursor_right();
                re_inp_cursor++;
            }

            return;
        } else if (active_view == 1) {
            // test view

            if (modifier == rawterm::Mod::Tab) {
                ptr->regex_error = "";
                re_inp_cursor = ptr->regex_inp.size();
                active_view = 0;
            } else if (modifier == rawterm::Mod::Enter) {
                if (test_cur.horizontal == 0) {
                    ptr->raw_test_input.insert(
                        ptr->raw_test_input.begin() + test_cur.vertical, "");
                    test_cur.vertical++;
                    test_cur.horizontal = 0;
                    return;
                }

                if (test_cur.horizontal ==
                    ptr->raw_test_input[test_cur.vertical].size()) {
                    ptr->raw_test_input.insert(ptr->raw_test_input.begin() +
                                                   test_cur.vertical + 1,
                                               "");
                    test_cur.vertical++;
                    test_cur.horizontal = 0;
                    return;
                }

                std::string &line_ref = ptr->raw_test_input[test_cur.vertical];
                std::string l1 = line_ref.substr(0, test_cur.horizontal);
                std::string l2 =
                    line_ref.substr(test_cur.horizontal, line_ref.size());

                ptr->raw_test_input[test_cur.vertical] = l1;
                ptr->raw_test_input.insert(
                    ptr->raw_test_input.begin() + test_cur.vertical + 1, l2);

                test_cur.vertical++;
                test_cur.horizontal = 0;

            } else if (modifier == rawterm::Mod::Arrow) {
                switch (k.code) {
                case 'A': // Up
                    if (test_cur.vertical) {
                        test_cur.vertical--;
                    }
                    break;
                case 'B': // Down
                    if (test_cur.vertical < ptr->raw_test_input.size()) {
                        test_cur.vertical++;
                    }
                    break;
                case 'C': // Right
                    if (test_cur.horizontal <
                        ptr->raw_test_input[test_cur.vertical].size()) {
                        test_cur.horizontal++;
                    }
                    break;
                case 'D': // Left
                    if (test_cur.horizontal) {
                        test_cur.horizontal--;
                    }
                    break;
                }
            } else if (modifier == rawterm::Mod::Backspace) {
                if (ptr->raw_test_input[test_cur.vertical].size()) {
                    if (test_cur.horizontal > 0) {
                        ptr->raw_test_input[test_cur.vertical].erase(
                            test_cur.horizontal - 1, 1);
                        test_cur.horizontal--;
                    }
                }
            } else if (modifier == rawterm::Mod::Space) {
                ptr->raw_test_input[test_cur.vertical].insert(
                    test_cur.horizontal, 1, ' ');
                test_cur.horizontal++;
            } else if (k.isCharInput()) {
                ptr->raw_test_input[test_cur.vertical].insert(
                    test_cur.horizontal, 1, k.code);
                test_cur.horizontal++;
            }
            return;
        }
    }
}

int Model::set_cursor() {
    if (active_view) {
        // test input
        rawterm::move_cursor({ static_cast<size_t>(7 + test_cur.vertical),
                               static_cast<size_t>(3 + test_cur.horizontal) });

        return 1;
    } else {
        // regex input
        rawterm::move_cursor({ 3, static_cast<size_t>(3 + re_inp_cursor) });
        return 0;
    }
}
