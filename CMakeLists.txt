cmake_minimum_required(VERSION 3.00)
project(projectC)

# AVR-GCC 툴체인 설정
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

# AVR 마이크로컨트롤러 모델 설정
set(MCU_FLAG "-mmcu=atmega128")
add_compile_options(${MCU_FLAG})

# avr-libc 라이브러리 연결
find_library(AVR_LIBC avr-libc)
if (AVR_LIBC)
    target_link_libraries(projectC ${AVR_LIBC})
endif()

set(SOURCES
    src/main.c
)

add_executable(projectC ${SOURCES})
target_include_directories(projectC PRIVATE include)

# MySQL 관련 설정 (필요한 경우)
target_include_directories(projectC PRIVATE /usr/include/mysql)
target_link_directories(projectC PRIVATE /usr/lib/mysql)
target_link_libraries(projectC PRIVATE mysqlclient)