#include <catch2/catch_test_macros.hpp>

#include "input.h"

TEST_CASE("Constructor", "[Input]") {
    Input in;

    REQUIRE(in.rendered == false);
    REQUIRE(in.regex_inp == "");
    REQUIRE(in.raw_test_input == std::vector<std::string>{ "" });
    REQUIRE(in.render_test_input == std::vector<std::string>{ "" });
}

TEST_CASE("render regex", "[Input]") {
    Input in;
    in.regex_inp = "[tf]oo";
    in.raw_test_input = { "too", "foo", "boo" };

    in.render();

    REQUIRE(in.render_test_input.size() == 3);
    REQUIRE(in.render_test_input[0] == "\x1b[32mtoo\x1b[0m");
    REQUIRE(in.render_test_input[1] == "\x1b[32mfoo\x1b[0m");
    REQUIRE(in.render_test_input[2] == "boo");
}
