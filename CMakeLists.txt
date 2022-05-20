project(proton)

cmake_minimum_required(VERSION 3.10)

set(PROTON_MAJOR_VERSION 1)
set(PROTON_MINOR_VERSION 0)
set(PROTON_PATCH_VERSION 0)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PROTON_BUILD_DEBUG 1)
else()
    set(PROTON_BUILD_RELEASE 1)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

if (UNIX)
    set(ENABLE_PROFILING OFF CACHE BOOL "")
    set(ENABLE_SANITIZER OFF CACHE BOOL "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wfatal-errors -fvisibility=hidden -pedantic -Wl,--no-undefined -fPIC -Wall -Wextra -Wconversion -Werror=return-type")

    if (ENABLE_PROFILING)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -O3")
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} -g -O0")

    if (ENABLE_SANITIZER)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fsanitize=address")
    endif()
endif()

configure_file("source/cmake.h.in" "__include/cmake.h")
add_subdirectory(3rd-party/protop)

add_executable(neutron
    "source/neutron.cc"
    "source/main.cc")

add_executable(proton
    "source/neutron.cc"
    "source/generator.cc")
target_include_directories(proton
    PRIVATE "${CMAKE_BINARY_DIR}/__include")
target_link_libraries(proton libprotop)
add_dependencies(proton process_neutron_hh)

add_executable(template "source/template.cc")
target_include_directories(template
    PRIVATE "${CMAKE_BINARY_DIR}/__include")

add_custom_target(process_neutron_hh
    ${CMAKE_BINARY_DIR}/template ${CMAKE_SOURCE_DIR}/source/proton.hh ${CMAKE_BINARY_DIR}/__include/proton.hh.cc
    DEPENDS template
)