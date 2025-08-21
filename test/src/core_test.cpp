#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <my_project/my_project.hpp>

TEST_CASE("Simple test") { CHECK_THAT(my_project::get_message(), Catch::Matchers::Equals("hello world!")); }