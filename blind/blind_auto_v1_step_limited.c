#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define LIGHT_THRESHOLD 400      // 조도 기준값
#define OPEN_STEPS 3000          // 블라인드가 완전히 열리는 데 필요한 스텝 수
#define CLOSE_STEPS 3000         // 블라인드가 완전히 닫히는 데 필요한 스텝 수
#define STEP_DELAY_MS 1

#define IN1 PC0
#define IN2 PC1
#define IN3 PC2
#define IN4 PC3

// 블라인드 상태 정의
enum BlindState { OPEN, CLOSED };
enum BlindState blind_state = CLOSED;

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
  while (ADCSRA & (1 << ADSC)); // 변환 완료 대기
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

    if (adc_result > LIGHT_THRESHOLD && blind_state == CLOSED) {
      // 밝고 현재 닫힌 상태 → 블라인드 열기
      stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
      blind_state = OPEN;
    } else if (adc_result <= LIGHT_THRESHOLD && blind_state == OPEN) {
      // 어둡고 현재 열린 상태 → 블라인드 닫기
      stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
      blind_state = CLOSED;
    }

    _delay_ms(1000); // 1초 간격으로 조도 체크
  }
}
