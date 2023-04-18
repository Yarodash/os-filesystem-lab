cmake_minimum_required(VERSION 3.0.0)
project(filesystem VERSION 0.1.0)

add_subdirectory(src)

add_executable(filesystem main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(filesystem drive_library)
