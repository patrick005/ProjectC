
#include <libserialport.h>
#include <stdio.h>

int main() {
    struct sp_port *port;

    if (sp_get_port_by_name("/dev/ttyUSB0", &port) != SP_OK) {
        printf("포트를 찾을 수 없습니다.\n");
        return 1;
    }

    if (sp_open(port, SP_MODE_READ_WRITE) != SP_OK) {
        printf("포트 열기 실패\n");
        return 1;
    }

    sp_set_baudrate(port, 9600);
    sp_blocking_write(port, "Hello!", 6, 1000);

    sp_close(port);
    sp_free_port(port);

    printf("전송 완료\n");
    return 0;
}
