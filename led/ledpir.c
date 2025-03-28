// ledpir.c
// #define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <stdio.h>

#include "uart0.h"
   
#define CDS_PIN PF1 // CdS 센서 연결 핀
#define PIR_PIN PD0 // PIR 센서 연결 핀
#define CDS_THRESHOLD   2         // 조도 임계치: ADC 값이 210 미만이면 LED ON, 이상이면 LED OFF
//센서 여러개달면 수치 바뀜 최종 확인 필요

volatile uint16_t timer1count = 0; 

// === ADC 초기화 ===
void adcInit(void) { 
    ADMUX = (1 << REFS0); // AVCC 기준 전압 사용
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC 초기화 및 분주비 128
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

ISR(TIMER1_COMPA_vect);


int main(void) {
    
    adcInit();
    // uart0Init(); // UART 초기화
    // stdout = &OUTPUT; //UART printf 설정
    // fflush(stdout);//UART printf 초기화
	
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


// Timer1 비교일치 인터럽트 서비스 루틴
ISR(TIMER1_COMPA_vect) {
    // PIR 센서가 감지 시 HIGH 논리
    // uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 0 : 1;
    uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 1 : 0;
    
    uint16_t adcValue = readADC(CDS_PIN);

    // --- LED on/off 제어 (Common Anode 기준) ---
    // 조건: PIR 센서가 감지되고(pirState가 HIGH) CdS 센서 값이 CDS_THRESHOLD 미만(어두움)일 때 LED ON (노란색)
    
    // if (timer1count < 100) { // 10초 (가정: 100이 1초)
    //     if (!(pirState) && (adcValue < CDS_THRESHOLD)) {
    //         PORTF &= ~(1 << PF3); // 빨강 채널 ON (LOW)
    //         PORTF &= ~(1 << PF4); // 초록 채널 ON (LOW)
    //         PORTF |= (1 << PF5);  // 파랑 채널 OFF (HIGH)
    //     } //else {
    //         // 조건 미충족 시 LED OFF 및 타이머 초기화
    //         // PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
    //         // timer1count = 0;
    //     //}
    //     timer1count++;
    // } else {
    //     // 10초 경과 후 LED OFF 및 타이머 초기화
    //     PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
    //     timer1count = 0;
    // }
    if (!(pirState) && (adcValue < CDS_THRESHOLD)) {
        timer1count = 1;
        if (timer1count < 100) {
            PORTF &= ~(1 << PF3); // 빨강 채널 ON (LOW)
            PORTF &= ~(1 << PF4); // 초록 채널 ON (LOW)
            PORTF |= (1 << PF5);  // 파랑 채널 OFF (HIGH)
            timer1count++;
        } else {
            // 10초 경과 후 LED OFF 및 타이머 초기화
            PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
            timer1count = 0;
        }
    } else{
        if(timer1count > 0 && timer1count < 100){
            timer1count++;
        }else if(timer1count == 100){
            PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
            timer1count = 0;
        }
    }
	// //patrick's debugging

    // // // UART 출력
	// printf("CDS ADC: %u, LED: %s\r\n", adcValue,(pirState && (adcValue < CDS_THRESHOLD)) ? "YELLOW" : "OFF");
		
    // // PIR 센서 상태에 따라 메시지 출력
    // if (pirState) {printf("나님 퇴장!\r\n"); // 0일때 감지 안됨
    // } else {printf("나님 등장!\r\n"); } // 1일때 감지됨
}
