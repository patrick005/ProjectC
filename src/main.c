#include "lcd.h"
#include "uart0.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

uint8_t cursor = 0;

volatile uint8_t hours = 0, minutes = 0, seconds = 0;

// ✅ Added line buffers
char weatherStr[17] = "                ";
char timeStr[17]    = "                ";

void timer1Init()
{
    // Set CTC mode (Clear Timer on Compare Match)
    TCCR1B |= (1 << WGM12);

    // Prescaler = 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);

    // Compare match value for 1s interrupt at 16MHz with /1024 prescaler:
    OCR1A = 15624;

    // Enable Timer1 Compare Match A Interrupt
    TIMSK |= (1 << OCIE1A);
}

int main()
{
    uart0Init(); // UART 초기화
    lcdInit();   // LCD 초기화
    timer1Init(); // 타이머 초기화
    sei();       // 전역 인터럽트 허용

    stdin = &INPUT;   // UART 입력 설정
    stdout = &OUTPUT; // UART 출력 설정

    DDRE = 0x02; // Rx(입력), TX(출력)1, SW0~3 입력

    char cData;

    // 초기 LCD 상태 설정
    lcdGotoXY(0, 0);
    lcdPrint(weatherStr);
    lcdGotoXY(0, 1);
    lcdPrint(timeStr);

    while (1)
    {
        // UART 데이터 수신 확인
        if (UCSR0A & (1 << RXC0))
        {
            cData = fgetc(stdin); // UART로부터 문자 읽기

            // ✅ Only modify weatherStr, not timeStr
            switch (cData)
            {
                case '1':
                    snprintf(weatherStr, sizeof(weatherStr), "SUNNY         ");
                    break;
                case '2':
                    snprintf(weatherStr, sizeof(weatherStr), "CLOUDY        ");
                    break;
                case '3':
                    snprintf(weatherStr, sizeof(weatherStr), "RAINY         ");
                    break;
                case '4':
                    snprintf(weatherStr, sizeof(weatherStr), "SNOWY         ");
                    break;
                default:
                    snprintf(weatherStr, sizeof(weatherStr), "Invalid       ");
                    break;
            }

            // ✅ Reprint only line 1
            lcdGotoXY(0, 0);
            lcdPrint(weatherStr);

            _delay_ms(1000); // 출력 유지 시간
        }
    }

    return 0;
}

ISR(TIMER1_COMPA_vect)
{
    seconds++;
    if (seconds >= 60)
    {
        seconds = 0;
        minutes++;
        if (minutes >= 60)
        {
            minutes = 0;
            hours = (hours + 1) % 24;
        }
    }

    // ✅ Update timeStr and reprint only line 2
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d       ", hours, minutes, seconds);
    lcdGotoXY(0, 1);
    lcdPrint(timeStr);
}