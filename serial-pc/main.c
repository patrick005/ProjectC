#include <libserialport.h>
#include <stdio.h>
#include <string.h>

int main() {
    struct sp_port *port;
    sp_get_port_by_name("/dev/ttyUSB0", &port);
    sp_open(port, SP_MODE_READ_WRITE);

    sp_set_baudrate(port, 9600);
    sp_set_bits(port, 8);
    sp_set_parity(port, SP_PARITY_NONE);
    sp_set_stopbits(port, 1);

    char msg[] = "Hello ATmega!";
    sp_blocking_write(port, msg, strlen(msg), 1000);

    sp_close(port);
    sp_free_port(port);
    return 0;
}
