#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>

#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUD_RATE B115200

int main() {
    int serial_port = open(SERIAL_PORT, O_WRONLY | O_NOCTTY); // 시리얼 포트를 쓰기 전용으로 엶
    if (serial_port < 0) {
        perror("Failed to open serial port");
        return 1;
    } // 시리얼 포트 열기 실패 시 에러 문자 출력

    struct termios tty;
    memset(&tty, 0, sizeof tty); // 시리얼 통신의 설정을 저장할 구조체를 생성하고 전부 0으로 초기화 
    if (tcgetattr(serial_port, &tty) != 0) {
        perror("Error from tcgetattr");
        return 1;
    } // 시리얼 포트의 설정이 초기화가 안 됐을 시 에러 문자 출력

    cfsetospeed(&tty, BAUD_RATE); // 시리얼 출력 속도 설정
    cfsetispeed(&tty, BAUD_RATE); // 시리얼 입력 속도 설정
    // 시리얼 통신 규칙 설정
    tty.c_cflag |= (CLOCAL | CREAD);  // 로컬 연결, 수신 가능
    tty.c_cflag &= ~CSIZE; // 데이터 크기 초기화
    tty.c_cflag |= CS8; // 데이터 비트 8개
    tty.c_cflag &= ~PARENB; // 패리티 없음
    tty.c_cflag &= ~CSTOPB; // 스톱비트 1개
    tty.c_cflag &= ~CRTSCTS; // 하드웨어 흐름제어 없음
    tcsetattr(serial_port, TCSANOW, &tty); // 설정한 규칙을 시리얼 포트에 적용

    char time_str[20]; // 시간 문자열을 HH:MM:SS + '\0' 의 형태로 저장

    while (1) {
        time_t now = time(NULL); // 현재시간 초 단위로 가져오기
        struct tm *tm_now = localtime(&now);
        strftime(time_str, sizeof(time_str), "%m-%d %H:%M:%S", tm_now);

        write(serial_port, time_str, strlen(time_str));
        write(serial_port, "\n", 1);  // 시리얼 포트로 시간 전송

        printf("Sent time: %s\n", time_str); // 디버깅용 출력
        sleep(1);
    }

    close(serial_port);
    return 0;
}