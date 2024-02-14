#ifndef MODEL_H
#define MODEL_H

#include "rawterm/rawterm.h"

#include "input.h"

struct Model {
    int active_view = 0;
    bool exit = false;

    int re_inp_cursor = 0;
    rawterm::Pos test_cur = {};

    Model(){};
    void draw_ui(Input &);
    void read_keypress(Input *);
    int set_cursor();
};

#endif // MODEL_H
