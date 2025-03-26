#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "lcd.h"

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

ScheduleTime schedule = {7, 0, 20, 0};

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

void displayModeOnLCD(const char* msg) {
  lcdClear();
  lcdGotoXY(0, 0);
  lcdPrint((char*)msg);
  _delay_ms(1000);
}

void selectModeFromLCD(char input) {
  switch (input) {
    case 'l':
      current_mode = MODE_LIGHT_SENSOR;
      displayModeOnLCD("Mode: Light Sensor");
      break;
    case 's':
      current_mode = MODE_SCHEDULED_TIME;
      displayModeOnLCD("Mode: Scheduled");
      break;
    case 'm':
      current_mode = MODE_MANUAL_TIME;
      displayModeOnLCD("Mode: Manual Time");
      break;
    default:
      displayModeOnLCD("Invalid Mode");
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
  lcdInit();

  lcdClear();
  lcdGotoXY(0, 0);
  lcdPrint("Select Mode:");
  lcdGotoXY(0, 1);
  lcdPrint("l/s/m (BTN or PC)");

  _delay_ms(1000);

  char input = 'l'; // 기본값, 실제 구현에서는 스위치 또는 UART 입력으로 대체 가능
  selectModeFromLCD(input);

  while (1) {
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
        // 향후 시간 입력 기능 추가 예정
        break;
    }

    _delay_ms(1000);
    current_min++;
    if (current_min >= 60) {
      current_min = 0;
      current_hour = (current_hour + 1) % 24;
    }
  }
}
