cmake_minimum_required(VERSION 3.10)
project(serial_pc_example C)

# C 언어 표준 설정
set(CMAKE_C_STANDARD 99)

# 빌드할 실행 파일과 소스 지정 (여기선 main.c)
add_executable(serial_pc main.c)

# libserialport 라이브러리 찾기
find_library(SERIALPORT_LIB serialport)

# 못 찾았을 경우 에러 출력
if (NOT SERIALPORT_LIB)
    message(FATAL_ERROR "libserialport not found!")
endif()

# 실행 파일에 libserialport 연결
target_link_libraries(serial_pc ${SERIALPORT_LIB})
