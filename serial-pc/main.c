#include <libserialport.h>
#include <stdio.h>
#include <string.h>

int main() {
    struct sp_port *port;

    // 포트 이름을 실제 연결된 장치로 수정하세요 (예: /dev/ttyUSB0 또는 /dev/ttyUSB1)
    if (sp_get_port_by_name("/dev/ttyUSB0", &port) != SP_OK) {
        printf("포트를 찾을 수 없습니다.\n");
        return 1;
    }

    if (sp_open(port, SP_MODE_READ_WRITE) != SP_OK) {
        printf("포트를 열 수 없습니다.\n");
        sp_free_port(port);
        return 1;
    }

    sp_set_baudrate(port, 115200);
    sp_set_bits(port, 8);
    sp_set_parity(port, SP_PARITY_NONE);
    sp_set_stopbits(port, 1);

    char data[100];
    printf("보낼 명령 입력 (예: LED_ON 또는 LED_OFF): ");
    fgets(data, sizeof(data), stdin);

    // 입력받은 문자열에서 개행 문자를 제거한 후, 다시 '\n'을 추가
    data[strcspn(data, "\n")] = '\0';
    strcat(data, "\n");

    sp_blocking_write(port, data, strlen(data), 1000);
    printf("전송 완료: %s\n", data);

    char buf[100];
    int len = sp_blocking_read(port, buf, sizeof(buf) - 1, 2000);
    if (len > 0) {
        buf[len] = '\0';
        printf("받은 응답: %s\n", buf);
    } else {
        printf("응답 없음 (2초 내 수신 실패)\n");
    }

    sp_close(port);
    sp_free_port(port);
    return 0;
}