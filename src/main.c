#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "uart0.h"

#define LIGHT_THRESHOLD 400
#define OPEN_STEPS 3000
#define CLOSE_STEPS 3000
#define STEP_DELAY_MS 1

#define IN1 PC0
#define IN2 PC1
#define IN3 PC2
#define IN4 PC3

// 블라인드 상태 정의
typedef enum { OPEN, CLOSED } BlindState;
BlindState blind_state = CLOSED;

// 작동 모드 정의
typedef enum {
  MODE_LIGHT_SENSOR,
  MODE_SCHEDULED_TIME,
  MODE_MANUAL_TIME
} BlindMode;

BlindMode current_mode = MODE_LIGHT_SENSOR; // 초기 모드

// 스케줄 시간 구조체
typedef struct {
  uint8_t open_hour;
  uint8_t open_min;
  uint8_t close_hour;
  uint8_t close_min;
} ScheduleTime;

ScheduleTime schedule = {7, 0, 20, 0}; // 예시 시간: 07:00, 20:00

// 현재 시간 (나중에 UART로 받아서 갱신)
uint8_t current_hour = 7;
uint8_t current_min = 0;

uint16_t adc_result = 0;
uint8_t motor_step = 0;

const uint8_t step_sequence[8] = {
  0b00001000, 0b00001100, 0b00000100, 0b00000110,
  0b00000010, 0b00000011, 0b00000001, 0b00001001
};

void initADC() {
  ADMUX = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

uint16_t readADC(uint8_t channel) {
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

void initMotorPins() {
  DDRC |= 0x0F;
}

void stepMotor(int direction, int steps, int speed_ms) {
  for (int i = 0; i < steps; i++) {
    motor_step = (motor_step + direction + 8) % 8;
    PORTC = (PORTC & 0xF0) | (step_sequence[motor_step] & 0x0F);
    _delay_ms(speed_ms);
  }
}

void initUART() {
  UBRR0H = 0;
  UBRR0L = 207; // 9600bps @ 16MHz
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8bit, 1 stop bit
}

char readUART() {
  while (!(UCSR0A & (1 << RXC0)));
  return UDR0;
}

void sendString(const char *str) {
  while (*str) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = *str++;
  }
}

void selectModeFromUART() {
  char input = readUART();
  switch (input) {
    case 'l':
      current_mode = MODE_LIGHT_SENSOR;
      sendString("\r\n[MODE] Light sensor mode selected\r\n");
      break;
    case 's':
      current_mode = MODE_SCHEDULED_TIME;
      sendString("\r\n[MODE] Scheduled time mode selected\r\n");
      break;
    case 'm':
      current_mode = MODE_MANUAL_TIME;
      sendString("\r\n[MODE] Manual time mode selected\r\n");
      break;
    default:
      sendString("\r\n[ERROR] Unknown mode. Use l/s/m\r\n");
      break;
  }
}

void checkScheduleAndControlBlind() {
  if (current_hour == schedule.open_hour && current_min == schedule.open_min && blind_state == CLOSED) {
    stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
    blind_state = OPEN;
  }
  if (current_hour == schedule.close_hour && current_min == schedule.close_min && blind_state == OPEN) {
    stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
    blind_state = CLOSED;
  }
}

int main(void) {
  initADC();
  initMotorPins();
  initUART();
  uart0Init();
  
  stdin = &INPUT;
  stdout = &OUTPUT;

  printf("\r\nSelect mode: l (light)  s (schedule)  m (manual)\r\n");

  while (1) {
    // UART 입력이 있으면 모드 전환 시도
    if (UCSR0A & (1 << RXC0)) {
      selectModeFromUART();
    }

    switch (current_mode) {
      case MODE_LIGHT_SENSOR:
        adc_result = readADC(0);
        if (adc_result > LIGHT_THRESHOLD && blind_state == CLOSED) {
          stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
          blind_state = OPEN;
        } else if (adc_result <= LIGHT_THRESHOLD && blind_state == OPEN) {
          stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
          blind_state = CLOSED;
        }
        break;

      case MODE_SCHEDULED_TIME:
        checkScheduleAndControlBlind();
        break;

      case MODE_MANUAL_TIME:
        // 추후 구현 예정
        break;
    }

    // 시간 시뮬레이션 (테스트용)
    _delay_ms(1000);
    current_min++;
    if (current_min >= 60) {
      current_min = 0;
      current_hour = (current_hour + 1) % 24;
    }
  }
}
