#include <iostream>
#include <string_view>

#include "input.h"
#include "model.h"

int main(int argc, char *argv[]) {
    rawterm::enter_alt_screen();
    rawterm::enable_raw_mode();
    rawterm::enable_signals();

    Input input = {};
    Model model = {};

    if (argc > 1) {
        input.regex_inp = argv[1];
        model.re_inp_cursor = std::string_view(argv[1]).size();
    }

    rawterm::cursor_pipe_blink();

    while (!(model.exit)) {
        model.draw_ui(input);
        model.read_keypress(&input);
    }

    rawterm::exit_alt_screen();
    rawterm::cursor_block();
    return 0;
}
