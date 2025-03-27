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
    int serial_port = open(SERIAL_PORT, O_WRONLY | O_NOCTTY);
    if (serial_port < 0) {
        perror("Failed to open serial port");
        return 1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serial_port, &tty) != 0) {
        perror("Error from tcgetattr");
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
    tcsetattr(serial_port, TCSANOW, &tty);

    char time_str[9]; // HH:MM:SS + '\0'

    while (1) {
        time_t now = time(NULL);
        struct tm *tm_now = localtime(&now);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_now);

        write(serial_port, time_str, 8);
        write(serial_port, "\n", 1);  // newline as delimiter

        printf("Sent time: %s\n", time_str); // Optional debug
        sleep(1);
    }

    close(serial_port);
    return 0;
}












// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <termios.h>
// #include <time.h>
// #include <mysql/mysql.h>

// #define SERIAL_PORT "/dev/ttyUSB0"
// #define BAUD_RATE B115200

// void send_to_uart(int fd, const char *msg) {
//     write(fd, msg, strlen(msg));
//     usleep(100000); // slight delay
// }

// int main() {
//     int serial_port = open(SERIAL_PORT, O_WRONLY | O_NOCTTY);
//     if (serial_port < 0) {
//         perror("Failed to open serial port");
//         return 1;
//     }

//     struct termios tty;
//     memset(&tty, 0, sizeof tty);
//     tcgetattr(serial_port, &tty);
//     cfsetospeed(&tty, BAUD_RATE);
//     cfsetispeed(&tty, BAUD_RATE);
//     tty.c_cflag |= (CLOCAL | CREAD | CS8);
//     tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
//     tcsetattr(serial_port, TCSANOW, &tty);

//     MYSQL *conn;
//     MYSQL_RES *res;
//     MYSQL_ROW row;

//     conn = mysql_init(NULL);
//     if (!mysql_real_connect(conn, "localhost", "your_user", "your_password", "your_database", 0, NULL, 0)) {
//         fprintf(stderr, "MySQL connection failed: %s\n", mysql_error(conn));
//         return 1;
//     }

//     while (1) {
//         // --- Get weather and date from DB ---
//         if (mysql_query(conn, "SELECT weather, date FROM weather_table ORDER BY id DESC LIMIT 1")) {
//             fprintf(stderr, "SELECT error: %s\n", mysql_error(conn));
//             continue;
//         }
//         res = mysql_store_result(conn);
//         row = mysql_fetch_row(res);

//         char weather_date_str[17] = "                ";
//         if (row && row[0] && row[1]) {
//             snprintf(weather_date_str, sizeof(weather_date_str), "%.6s %.5s", row[0], row[1]); // "SUNNY 03-27"
//         }

//         send_to_uart(serial_port, weather_date_str);  // Line 1
//         mysql_free_result(res);

//         // --- Get current system time ---
//         time_t now = time(NULL);
//         struct tm *tm_now = localtime(&now);
//         char time_str[9];
//         strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_now);

//         send_to_uart(serial_port, time_str); // Line 2

//         sleep(1);
//     }

//     mysql_close(conn);
//     close(serial_port);
//     return 0;
// }
