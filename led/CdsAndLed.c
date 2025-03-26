// RGB LED, CdS, PIR 활용한 코드
// RGB LED -> 노란색  CdS -> 210 이상이면 OFF 210 미만이면서 PIR에 동작이 감지 될때 10초 동안 ON
// Timer1만 사용. Timer1은 CTC 모드로 설정되어 100ms마다 인터럽트를 발생 
// 그 인터럽트 서비스 루틴에서 센서 값 읽기, LED 제어, 타이머 카운터 감소 등을 처리.


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "uart0.h"

#define CDS_CHANNEL    0       // CdS 센서가 연결된 ADC 채널 (PF0)
#define CDS_THRESHOLD  210     // CDS 센서 값이 210 이상이면 LED OFF, 209 이하이면 조건에 따라 LED ON
#define PIR_PIN        PD0     // PIR 센서가 연결된 핀

// LED가 켜져있어야 하는 시간을 10초 (100 x 100ms)로 정의
#define LED_ON_DURATION 100   

// LED ON 시간을 유지하기 위한 카운터 (100ms 단위)
volatile uint8_t ledOnTimeCounter = 0;

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

// === Timer1 초기화 (CTC 모드, 100ms 주기) ===
void timer1Init(void) {
    // CTC 모드 설정
    TCCR1B |= (1 << WGM12);
    // 16MHz, 프리스케일러 1024, 100ms 주기: OCR1A ≈ 1562
    OCR1A = 1562;
    // Timer1 비교 일치 인터럽트 활성화 (타겟 MCU에 따라 TIMSK 또는 TIMSK1 사용)
    TIMSK |= (1 << OCIE1A);
    // 프리스케일러 1024로 타이머 시작 (CS12, CS10 설정)
    TCCR1B |= (1 << CS12) | (1 << CS10);
}

// Timer1 비교일치 인터럽트 서비스 루틴
ISR(TIMER1_COMPA_vect) {
    uint16_t adcValue = readADC(CDS_CHANNEL);
    // PIR 센서 읽기 (동작 감지 시 HIGH라고 가정)
    uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 1 : 0;

    // PIR 동작 감지 및 CDS 값이 209 이하인 경우 LED를 켜고 타이머를 리셋
    if (pirState && (adcValue < CDS_THRESHOLD)) {
        ledOnTimeCounter = LED_ON_DURATION; // 10초 동안 유지
    } else if (ledOnTimeCounter > 0) {
        ledOnTimeCounter--;  // LED 유지 타이머 감소
    }

    // --- LED 제어 (Common Anode 기준) ---
    // LED ON 타이머가 남아있으면 LED 켜기, 아니면 LED OFF
    if (ledOnTimeCounter > 0) {
        // LED ON: 노란색 (빨강, 초록 채널 ON (LOW), 파랑 OFF (HIGH))
        PORTF &= ~(1 << PF3); // 빨강 ON (LOW)
        PORTF &= ~(1 << PF4); // 초록 ON (LOW)
        PORTF |=  (1 << PF5); // 파랑 OFF (HIGH)
    } else {
        // LED OFF: 모든 채널 HIGH
        PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
    }

    // UART 출력: PIR 상태, CDS ADC 값, LED 상태, 남은 LED 유지 시간
    printf("PIR: %s, CDS ADC: %u, LED: %s, Timer: %u\r\n", 
           (pirState ? "ON" : "OFF"), 
           adcValue, 
           (ledOnTimeCounter > 0) ? "ON" : "OFF",
           ledOnTimeCounter);
}

int main(void) {
    uart0Init();
    adcInit();

    // PIR 센서가 연결된 핀(PD0) 입력 설정
    DDRD &= ~(1 << PIR_PIN);
    // 필요 시 내부 풀업 활성화: PORTD |= (1 << PIR_PIN);

    // PF3: 빨강, PF4: 초록, PF5: 파랑 (RGB LED) 출력 설정
    DDRF |= (1 << PF3) | (1 << PF4) | (1 << PF5);

    // 표준 입출력 리디렉션 (UART 사용)
    stdin  = &INPUT;
    stdout = &OUTPUT;

    timer1Init();   // 타이머 초기화
    sei();          // 전역 인터럽트 활성화

    while (1) {
        // 메인 루프는 인터럽트에서 처리.
    }
}
