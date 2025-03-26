#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LIGHT_THRESHOLD 400  // 조도 기준값 (밝기 경계)

#define IN1 PC0
#define IN2 PC1
#define IN3 PC2
#define IN4 PC3

uint16_t adc_result = 0;
uint8_t motor_step = 0;
int motor_direction = 1; // 1: 정방향, -1: 역방향

const uint8_t step_sequence[8] = {
  0b00001000, // IN4
  0b00001100, // IN3 + IN4
  0b00000100, // IN3
  0b00000110, // IN2 + IN3
  0b00000010, // IN2
  0b00000011, // IN1 + IN2
  0b00000001, // IN1
  0b00001001  // IN1 + IN4
};

void initADC() {
  ADMUX = (1 << REFS0); // AVCC 기준 전압, ADC0 사용
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // ADC 인에이블, 프리스케일러 64
}

uint16_t readADC(uint8_t channel) {
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  ADCSRA |= (1 << ADSC); // 변환 시작
  while (ADCSRA & (1 << ADSC)); // 변환 대기
  return ADC;
}

void initMotorPins() {
  DDRC |= 0x0F; // PC0~PC3 출력 설정
}

void stepMotor(int direction, int steps, int speed_ms) {
  for (int i = 0; i < steps; i++) {
    motor_step = (motor_step + direction + 8) % 8;
    PORTC = (PORTC & 0xF0) | (step_sequence[motor_step] & 0x0F);
    _delay_ms(speed_ms);
  }
}

int main(void) {
  initADC();
  initMotorPins();

  while (1) {
    adc_result = readADC(0); // PF0 (ADC0) 조도센서 입력

    if (adc_result > LIGHT_THRESHOLD) {
      // 밝으면 블라인드 내리기 (정방향)
      stepMotor(1, 100, 3);
    } else {
      // 어두우면 블라인드 올리기 (역방향)
      stepMotor(-1, 100, 3);
    }

    _delay_ms(1000); // 1초마다 확인
  }
}
       