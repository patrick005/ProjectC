#include "lcd.h"
#include "uart0.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

uint8_t cursor = 0;

int main()
{
    uart0Init(); // UART 초기화
    lcdInit();   // LCD 초기화

    stdin = &INPUT;  // UART 입력 설정
    stdout = &OUTPUT; // UART 출력 설정

    DDRE = 0x02; // Rx(입력), TX(출력)1, SW0~3 입력

    char cData;

    lcdGotoXY(0, 0); // LCD 커서를 초기 위치로 설정
    while (1)
    {
        // UART 데이터 수신 확인
        if (UCSR0A & (1 << RXC0))
        {
            cData = fgetc(stdin); // UART로부터 문자 읽기

            // 입력된 문자에 따라 LCD에 출력
            switch (cData)
            {
                case '1':
                    lcdClear(); // LCD 화면 초기화
                    lcdPrint("SUNNY");
                    break;
                case '2':
                    lcdClear();
                    lcdPrint("CLOUDY");
                    break;
                case '3':
                    lcdClear();
                    lcdPrint("RAINY");
                    break;
                case '4':
                    lcdClear();
                    lcdPrint("SNOWY");
                    break;
                default:
                    lcdClear();
                    lcdPrint("Invalid");
                    break;
            }
            _delay_ms(1000); // 출력 유지 시간
        }
    }
    return 0;
}