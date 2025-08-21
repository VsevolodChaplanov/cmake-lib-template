#ifndef my_project_my_project_hpp
#define my_project_my_project_hpp

#include <my_project/exports/exports.hpp>

namespace my_project {
    MY_PROJECT_EXPORT auto get_message() -> std::string;
}
#endif // my_project_my_project_hpp