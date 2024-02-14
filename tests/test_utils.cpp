#include <catch2/catch_test_macros.hpp>

#include "utils.h"

TEST_CASE("Repeat string") {
    REQUIRE(repeat(3, "0") == "000");
    REQUIRE(repeat(3, "-") == "---");
}
