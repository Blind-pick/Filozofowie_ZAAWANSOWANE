cmake_minimum_required(VERSION 3.30)
project(Filozofowie_ZAAWANSOWANE)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG        yaml-cpp-0.7.0
)

FetchContent_MakeAvailable(yaml-cpp)

add_executable(Filozofowie_ZAAWANSOWANE
        main.cpp
        philosopher.cpp
        philosopher.h
        waiter.cpp
        waiter.h
        cook.cpp
        cook.h
        kitchen.cpp
        kitchen.h
        ConfigLoader.cpp
        ConfigLoader.h
        display.cpp
        display.h)

# Podlinkuj yaml-cpp do swojego projektu
target_link_libraries(Filozofowie_ZAAWANSOWANE PRIVATE yaml-cpp)
