cmake_minimum_required(VERSION 3.29)
project(ALL_training)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost COMPONENTS filesystem system)

add_executable(${PROJECT_NAME} main.cpp ATN.cpp DFA.cpp Grammar.cpp Parser.cpp StrUtils.cpp)
target_link_libraries(${PROJECT_NAME} boost_system)
