#include "lcd.h"
#include "uart0.h"
#include <avr/interrupt.h>#include "lcd.h"
#include "uart0.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

char weatherStr[17] = "                ";
char timeStr[17]    = "                ";

int main()
{
    uart0Init(); // UART 초기화
    lcdInit();   // LCD 초기화

    stdin = &INPUT;
    stdout = &OUTPUT;

    DDRE = 0x02; // Rx 입력, Tx 출력

    lcdGotoXY(0, 0);
    lcdPrint("Waiting...");
    lcdGotoXY(0, 1);
    lcdPrint("00:00:00"); //시리얼 통신과 LCD 초기 설정

    char buf[16] = {0};
    uint8_t bufIndex = 0; //UART 수신 데이터를 저장할 버퍼

    while (1)
    {
        if (UCSR0A & (1 << RXC0))
        {
            char c = fgetc(stdin);

            // 개행문자가 수신되기 전까지는 계속 버퍼에 수신된 문자를 저장
            if (c != '\n' && bufIndex < sizeof(buf) - 1)
            {
                buf[bufIndex++] = c;
            }
            else
            {
                buf[bufIndex] = '\0'; // 개행문자가 수신되면 하나의 데이터로 간주 

                // 버퍼에 수신된 데이터가 2글자이고 W로 시작하면 날씨로 LCD 첫 줄에 출력
                if (bufIndex == 2 && buf[0] == 'W')
                {
                    switch (buf[1])
                    {
                        case '1': snprintf(weatherStr, sizeof(weatherStr), "STORMY        "); break;
                        case '2': snprintf(weatherStr, sizeof(weatherStr), "RAINY         "); break;
                        case '3': snprintf(weatherStr, sizeof(weatherStr), "SNOWY         "); break;
                        case '4': snprintf(weatherStr, sizeof(weatherStr), "FOGGY         "); break;
                        case '5': snprintf(weatherStr, sizeof(weatherStr), "YELLOW DUST   "); break;
                        case '6': snprintf(weatherStr, sizeof(weatherStr), "SUNNY         "); break;
                        case '7': snprintf(weatherStr, sizeof(weatherStr), "CLEAR         "); break;
                        case '8': snprintf(weatherStr, sizeof(weatherStr), "PARTLY CLOUDY "); break;
                        case '9': snprintf(weatherStr, sizeof(weatherStr), "OVERCAST      "); break;
                        default:  snprintf(weatherStr, sizeof(weatherStr), "Invalid       "); break;
                    }

                    lcdGotoXY(0, 0);
                    lcdPrint(weatherStr);
                }

                // 총 8글자고 HH:MM:SS의 형식이면 시간으로 LCD 둘째 줄에 출력
                else if (bufIndex == 8 && buf[2] == ':' && buf[5] == ':')
                {
                    snprintf(timeStr, sizeof(timeStr), "%s       ", buf);
                    lcdGotoXY(0, 1);
                    lcdPrint(timeStr);
                }

                // 버퍼 초기화
                bufIndex = 0;
                for (int i = 0; i < sizeof(buf); i++) buf[i] = 0;
            }
        }
    }

    return 0;
}
