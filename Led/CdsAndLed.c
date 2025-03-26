#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "uart0.h"

#define CDS_CHANNEL    0       // CdS 센서가 연결된 ADC 채널 (PF0)
#define CDS_THRESHOLD  210     // 센서 값이 210 이상이면 LED OFF, 미만이면 LED ON

// === ADC 초기화 ===
void adcInit(void) {
    ADMUX = (1 << REFS0); // AVCC 기준 전압 사용
    ADCSRA = (1 << ADEN)  // ADC 활성화
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128
}

// === ADC 수동 읽기 ===
uint16_t readADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
    ADCSRA |= (1 << ADSC); // 변환 시작
    while (ADCSRA & (1 << ADSC)); // 변환 완료 대기
    return ADC;
}

int main(void) {
    uart0Init();
    lcdInit();
    adcInit();

    // PF3: 빨강, PF4: 초록, PF5: 파랑 제어 (RGB LED)
    DDRF |= (1 << PF3) | (1 << PF4) | (1 << PF5);

    stdin  = &INPUT;
    stdout = &OUTPUT;

    lcdGotoXY(0, 0);
    lcdPrintData("Light Sensor", 12);

    char buf[32];

    while (1) {
        uint16_t adcValue = readADC(CDS_CHANNEL);

        // --- LED on/off 제어 (Common Anode 가정) ---
        if (adcValue < CDS_THRESHOLD) {
            // 센서값 210 미만이면 LED ON → 노란색 (빨강+초록 켜짐, 파랑 꺼짐)
            PORTF &= ~(1 << PF3); // 빨강 ON (LOW)
            PORTF &= ~(1 << PF4); // 초록 ON (LOW)
            PORTF |=  (1 << PF5); // 파랑 OFF (HIGH)
        } else {
            // 센서값 210 이상이면 LED OFF (모든 채널 off)
            PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
        }

        // LCD 출력
        lcdGotoXY(0, 1);
        sprintf(buf, "CDS: %u LED: %s", adcValue, (adcValue < CDS_THRESHOLD) ? "ON" : "OFF");
        lcdPrintData(buf, strlen(buf));

        // UART 출력
        printf("CDS ADC: %u, LED: %s\r\n", adcValue, (adcValue < CDS_THRESHOLD) ? "ON" : "OFF");

        _delay_ms(100);
    }
}