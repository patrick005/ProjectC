// RGB LED, CdS, PIR 활용한 코드
// RGB LED -> 노란색  CdS -> 210 이상이면 OFF 210 미만이면서 PIR에 동작이 감지 될때 10초 동안 ON
// Timer1만 사용. Timer1은 CTC 모드로 설정되어 100ms마다 인터럽트를 발생 
// 그 인터럽트 서비스 루틴에서 센서 값 읽기, LED 제어, 타이머 카운터 감소 등을 처리.
// string.h 및 util/delay.h 와 같이 불필요한 부분 주석처리
// PIR 로직 부분 Low인 줄 알았으나 High여서 해당 사항에 맞게 수정 및 기존 코드 주석처리

// #define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "uart0.h"

#define PIR_PIN         PD0         // PIR 센서 출력 연결 핀 (이제 감지 시 HIGH)
#define CDS_CHANNEL     0           // CdS 센서가 연결된 ADC 채널 (PF0)
#define CDS_THRESHOLD   210         // 조도 임계치: ADC 값이 210 미만이면 LED ON, 이상이면 LED OFF

// === ADC 초기화 ===
void adcInit(void) {
    ADMUX = (1 << REFS0); // AVCC 기준 전압 사용
    ADCSRA = (1 << ADEN)  // ADC 활성화
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128
}

// === ADC 수동 읽기 ===
uint16_t readADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);  // 채널 선택
    ADCSRA |= (1 << ADSC);                      // 변환 시작
    while (ADCSRA & (1 << ADSC));               // 변환 완료 대기
    return ADC;
}

// === Timer1 초기화 (CTC 모드, 100ms 주기) ===
void timer1Init(void) {
    TCCR1B |= (1 << WGM12);  // CTC 모드 설정
    OCR1A = 1562;            // 16MHz, 프리스케일러 1024, 100ms 주기 계산값
    TIMSK |= (1 << OCIE1A);   // Timer1 비교일치 인터럽트 활성화
    TCCR1B |= (1 << CS12) | (1 << CS10); // 프리스케일러 1024로 타이머 시작
}

// Timer1 비교일치 인터럽트 서비스 루틴
ISR(TIMER1_COMPA_vect) {
    // Modified: 이전 버전에서는 PIR 센서가 감지 시 LOW로 판단하여 아래와 같이 작성했습니다.
    // uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 0 : 1;
    // 이제 PIR 센서가 감지 시 HIGH이므로, 논리를 그대로 사용합니다.
    uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 1 : 0;
    
    uint16_t adcValue = readADC(CDS_CHANNEL);

    // --- LED on/off 제어 (Common Anode 기준) ---
    // 조건: PIR 센서가 감지되고(pirState가 HIGH) CdS 센서 값이 CDS_THRESHOLD 미만(어두움)일 때 LED ON (노란색)
    if (pirState && (adcValue < CDS_THRESHOLD)) {
        PORTF &= ~(1 << PF3); // 빨강 채널 ON (LOW)
        PORTF &= ~(1 << PF4); // 초록 채널 ON (LOW)
        PORTF |=  (1 << PF5); // 파랑 채널 OFF (HIGH)
    } else {
        // 조건 미충족 시 LED OFF (모든 채널 HIGH)
        PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
    }

    // UART 출력
    printf("PIR: %s, CDS ADC: %u, LED: %s\r\n",
           pirState ? "ON" : "OFF",
           adcValue,
           (pirState && (adcValue < CDS_THRESHOLD)) ? "YELLOW" : "OFF");
}

int main(void) {
    uart0Init();
    adcInit();

    // PF3: 빨강, PF4: 초록, PF5: 파랑 제어 (RGB LED)
    DDRF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
    // 초기 LED OFF (공통 애노드 기준: 채널 HIGH → LED 꺼짐)
    PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);

    // PIR 센서 핀(PD0)을 입력 모드로 설정
    DDRD &= ~(1 << PIR_PIN);
    // (필요 시 내부 풀업 사용 가능: PORTD |= (1 << PIR_PIN);)

    timer1Init();   // 타이머 초기화
    sei();          // 전역 인터럽트 활성화

    while (1) {
        // 모든 작업은 Timer1 ISR에서 처리
    }
}
