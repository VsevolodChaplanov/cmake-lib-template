#include <fmt/core.h> // for format
#include <my_project/my_project.hpp>

namespace my_project {
    auto get_message() -> std::string { return fmt::format("{}\n", "hello world!"); }
} // namespace my_project
