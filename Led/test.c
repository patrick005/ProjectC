#define F_CPU 16000000UL
#include <avr/io.h>#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "lcd.h"
#include "uart0.h"

#define CDS_CHANNEL 0       // ADC0 (PF0)
#define CDS_THRESHOLD 150   // 기준 조도값

// === ADC 초기화 ===
void adcInit() {
    ADMUX = (1 << REFS0); // AVCC 기준 전압
    ADCSRA = (1 << ADEN)  // ADC 활성화
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128
}

// === ADC 수동 읽기 ===
uint16_t readADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // 채널 설정
    ADCSRA |= (1 << ADSC); // 변환 시작
    while (ADCSRA & (1 << ADSC)); // 완료 대기
    return ADC;
}

// === PWM 초기화 (Fast PWM 8bit, Timer3) ===
void pwmInit() {
    // Fast PWM 8bit, 비반전 모드
    TCCR3A = (1 << COM3A1) | (1 << COM3B1) | (1 << WGM30);
    TCCR3B = (1 << WGM32) | (1 << CS31); // 분주비 8

    // 핀 출력 설정: R (PF3), G (PF4), B (PF5 - 일반 출력으로 끔)
    DDRF |= (1 << PF3) | (1 << PF4) | (1 << PF5);

    // 파란색 LED 완전히 끄기
    OCR3C = 0;
    PORTF &= ~(1 << PF5);
}

// === 밝기 설정: 노란색 = R + G, B 끔 ===
void setYellowPWM(uint8_t brightness) {
    OCR3A = brightness; // R
    OCR3B = brightness; // G
    OCR3C = 0;          // B 끔
}

int main(void) {
    uart0Init();
    lcdInit();
    adcInit();
    pwmInit();

    stdin = &INPUT;
    stdout = &OUTPUT;

    lcdGotoXY(0, 0);
    lcdPrintData("Light Sensor", 12);

    char buf[32];

    while (1) {
        uint16_t adcValue = readADC(CDS_CHANNEL);

        // 조건: 151 이상 → 밝게 / 150 이하 → 꺼짐
        if (adcValue > CDS_THRESHOLD) {
            setYellowPWM(200); // 밝게 켜기
        } else {
            setYellowPWM(0); // 꺼짐
        }

        // LCD + UART 출력
        lcdGotoXY(0, 1);
        sprintf(buf, "CDS: %u", adcValue);
        lcdPrintData(buf, strlen(buf));
        printf("CDS ADC_data : %u\r\n", adcValue);

        _delay_ms(100);
    }
}
