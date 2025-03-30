
#include <avr/io.h>
#include <stdio.h>
#include <avr/delay.h>
#include "uart0.h"

#define CDS_PIN PF3 // CdS 센서 연결 핀

void adcInit(void) {
    ADMUX = (1 << REFS0); // AVCC 기준 전압 사용
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 분주비 128
}

uint16_t readADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // 채널 선택
    ADCSRA |= (1 << ADSC); // 변환 시작
    while (ADCSRA & (1 << ADSC)); // 변환 완료 대기
    return ADC;
}

int main(void) {
    uart0Init();
    adcInit();
    stdout = &OUTPUT; // printf 설정

    while (1) {
        uint16_t adcValue = readADC(CDS_PIN);
        // 상수 A, B, C는 실험 또는 데이터시트를 통해 결정해야 합니다.
        float r_cds = 100000.0 / (adcValue + 10.0) - 1000.0;

        printf("CDS ADC: %u\r\n", adcValue);
        _delay_ms(1000);
    }
}
