#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define BAUD 115200
#define MYUBRR (F_CPU / 16 / BAUD - 1)

// UART 초기화 함수
void USART_Init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);               // 수신, 송신 활성화
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);             // 8비트 데이터
}

// 1바이트 송신
void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

// 1바이트 수신
unsigned char USART_Receive(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

// 문자열 송신
void USART_SendString(const char* str) {
    while (*str) {
        USART_Transmit(*str++);
    }
}

// LED 관련 함수 (예: PC0 핀에 LED 연결)
void LED_Init(void) {
    DDRC |= 0x0F;  // PORTC의 하위 4비트를 출력으로 설정
}

void LED_On(void) {
    PORTC = 0x0F;  // 하위 4비트 모두 1: LED 켜기
}

void LED_Off(void) {
    PORTC = 0x00;  // 하위 4비트 모두 0: LED 끄기
}

int main(void) {
    char buffer[100];
    int i = 0;

    LED_Init();
    LED_Off();  // 초기 상태: LED 끔
    USART_Init(MYUBRR);
    USART_SendString("ATmega128 Ready\n");

    while (1) {
        unsigned char ch = USART_Receive();
        if (ch == '\n' || ch == '\r') {
            buffer[i] = '\0';  // 문자열 종료

            if (strcmp(buffer, "LED_ON") == 0) {
                LED_On();
                USART_SendString("LED turned ON\n");
            } else if (strcmp(buffer, "LED_OFF") == 0) {
                LED_Off();
                USART_SendString("LED turned OFF\n");
            } else {
                USART_SendString("Unknown Command\n");
            }
            i = 0;
        } else {
            if (i < (int)(sizeof(buffer) - 1)) {
                buffer[i++] = ch;
            }
        }
    }
    return 0;
}
