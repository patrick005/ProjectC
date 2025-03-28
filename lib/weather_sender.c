#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <mysql/mysql.h>

#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUD_RATE B115200

int main(void)
{
    // MySQL 정보 설정 
    const char *host = "192.168.0.48";
    const char *user = "myuser";
    const char *pass = "0000";
    const char *db   = "WeatherDB";
    const int   port = 3306;
    
    // MySQL 연결 설정
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, host, user, pass, db, port, NULL, 0)) {
        fprintf(stderr, "MySQL connection failed: %s\n", mysql_error(conn));
        return 1;
    }

    // 시리얼 포트 설정
    int serial = open(SERIAL_PORT, O_WRONLY | O_NOCTTY);
    if (serial < 0) {
        perror("Serial port open failed");
        return 1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serial, &tty) != 0) {
        perror("tcgetattr error");
        return 1;
    }

    cfsetospeed(&tty, BAUD_RATE);
    cfsetispeed(&tty, BAUD_RATE);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tcsetattr(serial, TCSANOW, &tty);


    // 최근 날씨 쿼리해오기
    const char *query = "SELECT wt FROM weatherData6 ORDER BY tm DESC LIMIT 1";
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    // 결과 처리
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "mysql_store_result() failed\n");
        mysql_close(conn);
        return 1;
    }

     // 결과 집합을 받아서 한 줄 추출
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row && row[0]) {
        char *wt = row[0];
        printf("Original weather code from DB: [%s]\n", wt);

        // \r, \n 제거한 후 \n 붙여서 전송
        char cleaned[4] = {0};
        int j = 0;
        for (int i = 0; wt[i] != '\0' && j < 2; i++) {
            if (wt[i] != '\r' && wt[i] != '\n') {
                cleaned[j++] = wt[i];
            }
        }
        cleaned[j] = '\n';

        printf("Sending cleaned: [%s]\n", cleaned);
        write(serial, cleaned, j + 1);  // 날씨 코드를 시리얼 포트로 아트메가128에 전송
    } else {
        fprintf(stderr, "No weather data found\n");
    }

    // DB 연결 종료, 시리얼 포트 닫기
    mysql_free_result(result);
    mysql_close(conn);
    close(serial);
    return 0;
}
