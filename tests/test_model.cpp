#include <catch2/catch_test_macros.hpp>

#include "model.h"

TEST_CASE("Constructor", "[Model]") {
    Model m;

    REQUIRE(m.active_view == 0);
    REQUIRE(m.exit == false);
    REQUIRE(m.re_inp_cursor == 0);
    REQUIRE(m.test_cur == rawterm::Pos{});
}

TEST_CASE("set cursor", "[Model]") {
    Model m;

    REQUIRE(m.set_cursor() == 0);
    REQUIRE(m.active_view == 0);
}
