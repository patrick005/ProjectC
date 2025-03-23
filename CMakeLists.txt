cmake_minimum_required(VERSION 3.00)
project(project1)

set(SOURCES
    src/project1.c
    src/main.c
    )

add_executable(project1 ${SOURCES})
target_include_directories(project1 PRIVATE include)
# target_include_directories(project1 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

target_include_directories(project1 PRIVATE /usr/include/mysql)
target_link_directories(project1 PRIVATE /usr/lib/mysql)
target_link_libraries(project1 PRIVATE mysqlclient)

# message(STATUS "Using compiler: ${CMAKE_C_COMPILER}")
# message(STATUS "Include directories: /usr/include/mysql")
# message(STATUS "Library directories: /usr/lib/mysql")
# 컴파일 디버그용 메시지 출력