#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "uart0.h"

#define PIR_PIN         PD0       // PIR 센서 핀
#define CDS_CHANNEL     0         // CdS 센서 ADC 채널
#define CDS_THRESHOLD   200     // CdS 센서 임계값

#define LED_RED_PIN     PA4       // RGB LED 빨강 핀
#define LED_GREEN_PIN   PA5       // RGB LED 초록 핀
#define LED_BLUE_PIN    PA6       // RGB LED 파랑 핀

volatile uint16_t timerCounter = 0;

// === ADC 초기화 ===
void adcInit(void) {
    ADMUX = (1 << REFS0); // AVCC를 기준 전압으로 설정
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 프리스케일러 128 설정
}

// === ADC 수동 읽기 ===
uint16_t readADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);  // 채널 선택
    ADCSRA |= (1 << ADSC);                      // 변환 시작
    while (ADCSRA & (1 << ADSC));               // 변환 완료 대기
    return ADC;
}

// === Timer1 초기화 ===
void timer1Init(void) {
    TCCR1B |= (1 << WGM12);                    // CTC 모드 설정
    OCR1A = 15624;                             // 16MHz, 프리스케일러 1024, 100ms 주기
    TIMSK |= (1 << OCIE1A);                   // Timer1 비교일치 인터럽트 활성화
    TCCR1B |= (1 << CS12) | (1 << CS10);       // 프리스케일러 1024로 타이머 시작
}

// === Timer1 비교일치 인터럽트 서비스 루틴 ===
ISR(TIMER1_COMPA_vect) {
    uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 1 : 0;
    uint16_t adcValue = readADC(CDS_CHANNEL);

    if (pirState && (adcValue < CDS_THRESHOLD)) {
        timerCounter = 100; // 10초 유지
        PORTA &= ~(1 << LED_RED_PIN);   // 빨강 ON (LOW)
        PORTA &= ~(1 << LED_GREEN_PIN); // 초록 ON (LOW)
        PORTA |=  (1 << LED_BLUE_PIN);  // 파랑 OFF (HIGH)
    } else {
        if (timerCounter > 0) timerCounter--;  // 100ms마다 감소
        if (timerCounter == 0) {
            PORTA |= (1 << LED_RED_PIN) | (1 << LED_GREEN_PIN) | (1 << LED_BLUE_PIN); // LED OFF
        }
    }

    printf("PIR: %s, CDS ADC: %u, LED: %s\r\n",
           pirState ? "ON" : "OFF",
           adcValue,
           (pirState && (adcValue < CDS_THRESHOLD)) ? "YELLOW" : "OFF");
}

int main(void) {
    uart0Init();
    adcInit();

    DDRA |= (1 << LED_RED_PIN) | (1 << LED_GREEN_PIN) | (1 << LED_BLUE_PIN);
    PORTA |= (1 << LED_RED_PIN) | (1 << LED_GREEN_PIN) | (1 << LED_BLUE_PIN); // 초기 LED OFF

    DDRD &= ~(1 << PIR_PIN); // PIR 입력 설정

    timer1Init();
    sei();

    while (1) {
        // 모든 작업은 Timer1 ISR에서 처리
    }
}