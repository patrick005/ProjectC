#include "lcd.h"
#include "uart0.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

char weatherStr[17] = "                ";
char timeStr[17]    = "                ";

int main()
{
    uart0Init(); // UART 초기화
    lcdInit();   // LCD 초기화
    sei();       // 전역 인터럽트 허용

    stdin = &INPUT;
    stdout = &OUTPUT;

    DDRE = 0x02; // Rx 입력, Tx 출력

    lcdGotoXY(0, 0);
    lcdPrint(weatherStr);
    lcdGotoXY(0, 1);
    lcdPrint(timeStr);
    char cData;
char timeBuf[9];
uint8_t tIndex = 0;
uint8_t expectWeather = 0;

while (1)
{
    if (UCSR0A & (1 << RXC0))
    {
        cData = fgetc(stdin);

        if (expectWeather)
        {
            expectWeather = 0;

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

            lcdGotoXY(0, 0);
            lcdPrint(weatherStr);
        }
        else if (cData == 'W')
        {
            // Next char will be a weather command
            expectWeather = 1;
        }
        else if ((cData >= '0' && cData <= '9') || cData == ':')
        {
            if (tIndex < 8)
                timeBuf[tIndex++] = cData;
            else
                tIndex = 0; // overflow protection
        }
        else if (cData == '\n' && tIndex == 8)
        {
            timeBuf[8] = '\0';
            snprintf(timeStr, sizeof(timeStr), "%s       ", timeBuf);
            lcdGotoXY(0, 1);
            lcdPrint(timeStr);
            tIndex = 0;
        }
        else
        {
            tIndex = 0; // reset on unknown
        }
    }
}

    

    return 0;
}




// #include "lcd.h"
// #include "uart0.h"
// #include <avr/interrupt.h>
// #include <avr/io.h>
// #include <stdio.h>
// #include <util/delay.h>

// uint8_t cursor = 0;

// volatile uint8_t hours = 0, minutes = 0, seconds = 0;


// char weatherStr[17] = "                ";
// char timeStr[17]    = "                "; //버퍼 주고 처음 공백 채워서 LCD 빈 화면 출력

// void timer1Init()
// {
//     TCCR1B |= (1 << WGM12); // 클리어 타임 비교매치로 설정 
//     TCCR1B |= (1 << CS12) | (1 << CS10);// 1024 분주비 설정
//     OCR1A = 15624; // 16MHz/1024 = 15624 = 1초에 한 번 비교일치 타임 인트럽트 발생
//     TIMSK |= (1 << OCIE1A); //타이머1의 비교일치1 활성화함 
// }

// int main()
// {
//     uart0Init(); // UART 초기화
//     lcdInit();   // LCD 초기화
//     timer1Init(); // 타이머 초기화
//     sei();       // 전역 인터럽트 허용

//     stdin = &INPUT;   // UART 입력 설정
//     stdout = &OUTPUT; // UART 출력 설정

//     DDRE = 0x02; // Rx(입력), TX(출력)1, SW0~3 입력

//     char cData;

//     // 초기 LCD 상태 설정
//     lcdGotoXY(0, 0);
//     lcdPrint(weatherStr);
//     lcdGotoXY(0, 1);
//     lcdPrint(timeStr);

//     while (1)
//     {
//         // UART 데이터 수신 확인
//         if (UCSR0A & (1 << RXC0))
//         {
//             cData = fgetc(stdin); // UART로부터 문자 읽기

    
//             switch (cData)
//             {
//                 case '1':
//                     snprintf(weatherStr, sizeof(weatherStr), "SUNNY         ");
//                     break;
//                 case '2':
//                     snprintf(weatherStr, sizeof(weatherStr), "CLOUDY        ");
//                     break;
//                 case '3':
//                     snprintf(weatherStr, sizeof(weatherStr), "RAINY         ");
//                     break;
//                 case '4':
//                     snprintf(weatherStr, sizeof(weatherStr), "SNOWY         ");
//                     break;
//                 default:
//                     snprintf(weatherStr, sizeof(weatherStr), "Invalid       ");
//                     break;
//             }

            
//             lcdGotoXY(0, 0);
//             lcdPrint(weatherStr);

//             _delay_ms(1000); // 출력 유지 시간
//         }
//     }

//     return 0;
// }

// ISR(TIMER1_COMPA_vect)
// {
//     seconds++;
//     if (seconds >= 60)
//     {
//         seconds = 0;
//         minutes++;
//         if (minutes >= 60)
//         {
//             minutes = 0;
//             hours = (hours + 1) % 24;
//         }
//     }

   
//     snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d       ", hours, minutes, seconds);
//     lcdGotoXY(0, 1);
//     lcdPrint(timeStr);
// }
