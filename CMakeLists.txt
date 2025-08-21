cmake_minimum_required(VERSION 3.28)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(prelude) # prevent in-source builds

# cmake-format: off
# ---- Enable CPM or conan packagers 
# ---- CPM - cmake based package manager 
# include(cmake/cpm.cmake) 
# ---- Install conan eps with cmake configure command may increase configuration time and not recommended
# include(cmake/conan_provider.cmake)
# cmake-format: on

# conan search for project name and version and bases on this formatting
# cmake-format: off
project(my_project
        VERSION 0.1.0
        DESCRIPTION "Short description"
        HOMEPAGE_URL "https://example.com/"
        LANGUAGES CXX)
# cmake-format: on

include(vsenvironment) # setup msvc environment

run_vcvarsall()

include(CTest)
include(GNUInstallDirs)
include(GenerateExportHeader)

include(options.cmake)
include(dependencies.cmake)

include(variables)
include(ccache)
include(ipo)
include(coverage)
include(utility)
include(folders)
include(static-analyzers)
include(memory)

if(${PROJECT_NAME}_ENABLE_CCACHE)
    enable_ccache()
endif()

enable_ipo()

# ---- Project options

add_library(${PROJECT_NAME}-options INTERFACE)
add_library(${PROJECT_NAME}::options ALIAS ${PROJECT_NAME}-options)

# ---- Compile options

target_compile_options(${PROJECT_NAME}-options INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)

# ---- Set cxx standard project wide

target_compile_features(${PROJECT_NAME}-options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

export(
    TARGETS ${PROJECT_NAME}-options
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Options.cmake)

# ---- Declare library precompiled headers

add_library(${PROJECT_NAME}-pch INTERFACE) # declare library if pch is off prevent install rule error for unknown target
add_library(${PROJECT_NAME}::pch ALIAS ${PROJECT_NAME}-pch)
if(${PROJECT_NAME}_ENABLE_PCH)
    target_precompile_headers(${PROJECT_NAME}-pch INTERFACE <string> <vector>)
    target_link_libraries(${PROJECT_NAME}-pch INTERFACE $<BUILD_INTERFACE:${PROJECT_NAME}::options>)
endif()

# ---- Declare library ----

if(${PROJECT_NAME}_BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_library(${PROJECT_NAME} STATIC)
endif()
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# ---- Generate export header

generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME
                       ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/exports/exports.hpp)

# ---- Collect sources

file(GLOB LIBRARY_INCLUDES ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}/*.hpp)
file(GLOB LIBRARY_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# ---- Set target sources

# cmake-format: off
target_sources(
    ${PROJECT_NAME}
    PUBLIC FILE_SET ${PROJECT_NAME}_Headers
           TYPE HEADERS
           BASE_DIRS ${PROJECT_SOURCE_DIR}/include
           FILES ${LIBRARY_INCLUDES}
    PUBLIC FILE_SET ${PROJECT_NAME}_GeneratedHeaders
           TYPE HEADERS
           BASE_DIRS ${PROJECT_BINARY_DIR}/include
           FILES ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/exports/exports.hpp
    PRIVATE ${LIBRARY_SOURCES})
# cmake-format: on

# ---- Set target include dirs

target_include_directories(
    ${PROJECT_NAME} ${warning_guard}
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ---- Add libraries

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC fmt::fmt
    PRIVATE $<BUILD_INTERFACE:${PROJECT_NAME}::pch> $<BUILD_INTERFACE:${PROJECT_NAME}::options>)

# ---- Properties

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES DEBUG_POSTFIX ${${PROJECT_NAME}_DEBUG_POSTFIX}
               VERSION ${PROJECT_VERSION}
               SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

# ---- Development export

export(
    TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-pch ${PROJECT_NAME}-options
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)

# ---- Clang-tidy target

if(${PROJECT_NAME}_ENABLE_CLANGTIDY)
    target_clangtidy(TARGET ${PROJECT_NAME} WARNINGS_AS_ERRORS ${${PROJECT_NAME}_WARNINGS_AS_ERRORS})
endif()

# ---- Cppcheck target

if(${PROJECT_NAME}_ENABLE_CPPCHECK)
    target_cppcheck(TARGET ${PROJECT_NAME} WARNINGS_AS_ERRORS ${${PROJECT_NAME}_WARNINGS_AS_ERRORS})
endif()

# ---- IWYU target

if(${PROJECT_NAME}_ENABLE_IWYU)
    target_include_what_you_use(TARGET ${PROJECT_NAME})
endif()

# ---- Add coverage support

if(${PROJECT_NAME}_ENABLE_COVERAGE)
    target_add_coverage_flags(${PROJECT_NAME})
endif()

# ---- Build examples

if(${PROJECT_NAME}_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

# ---- Build tests

include(CTest)
if(BUILD_TESTING AND ${PROJECT_NAME}_BUILD_TESTING)
    add_subdirectory(test)
endif()

# ---- Configure install rules

if(NOT CMAKE_SKIP_INSTALL_RULES AND NOT ${PROJECT_NAME}_SKIP_INSTALL_RULES)
    # cmake-format: off
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME COMPONENT Runtime
        LIBRARY COMPONENT Runtime NAMELINK_COMPONENT Development
        ARCHIVE COMPONENT Development
        FILE_SET ${PROJECT_NAME}_Headers COMPONENT Development
        FILE_SET ${PROJECT_NAME}_GeneratedHeaders COMPONENT Development)
    # cmake-format: on
endif()

# ---- Install rules for project interface library

include(install-rules)

# ---- Developer mode ----

if(NOT ${PROJECT_NAME}_DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(AUTHOR_WARNING "Developer mode is intended for developers of ${PROJECT_NAME}")
endif()

include(dev-mode)
