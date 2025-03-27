#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "uart0.h"
#include "lcd.h"

#define OPEN_THRESHOLD 520
#define CLOSE_THRESHOLD 480
#define OPEN_STEPS 3000
#define CLOSE_STEPS 3000
#define STEP_DELAY_MS 1

// 쿨다운 시간 (모터 작동 후 다음 작동까지 대기)
#define COOLDOWN_TICKS 3000  // 약 3초 대기

typedef enum { OPEN, CLOSED } BlindState;
typedef enum { MODE_LIGHT_SENSOR, MODE_MANUAL } BlindMode;

BlindState blind_state = CLOSED;
BlindMode current_mode = MODE_LIGHT_SENSOR;

uint16_t adc_result = 0;
uint8_t motor_step = 0;
uint32_t cooldown_counter = 0;

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
  UBRR0L = 8;
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

char readUART() {
  while (!(UCSR0A & (1 << RXC0)));
  return UDR0;
}

void displayMessage(const char* msg) {
  lcdClear();
  lcdGotoXY(0, 0);
  lcdPrint(msg);
}

void handleUARTCommand(char cmd) {
  switch (cmd) {
    case 'l':
      current_mode = MODE_LIGHT_SENSOR;
      displayMessage("Mode: LightSensor");
      break;
    case 'm':
      current_mode = MODE_MANUAL;
      displayMessage("Mode: Manual");
      break;
    case 'o':
      if (current_mode == MODE_MANUAL && blind_state == CLOSED) {
        stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
        blind_state = OPEN;
        displayMessage("Blind OPENED");
      }
      break;
    case 'c':
      if (current_mode == MODE_MANUAL && blind_state == OPEN) {
        stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
        blind_state = CLOSED;
        displayMessage("Blind CLOSED");
      }
      break;
    default:
      displayMessage("Invalid Command");
      break;
  }
}

int main(void) {
  initADC();
  initMotorPins();
  initUART();
  uart0Init();
  lcdInit();

  stdin = &INPUT;
  displayMessage("Mode: LightSensor");

  while (1) {
    if (UCSR0A & (1 << RXC0)) {
      char cmd = readUART();
      handleUARTCommand(cmd);
      getchar();  // 개행문자 제거
    }

    if (cooldown_counter > 0) {
      cooldown_counter--;
      _delay_ms(1);
      continue;
    }

    if (current_mode == MODE_LIGHT_SENSOR) {
      adc_result = readADC(0);

      if (blind_state == CLOSED && adc_result > OPEN_THRESHOLD) {
        stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
        blind_state = OPEN;
        cooldown_counter = COOLDOWN_TICKS;
        displayMessage("Opened (light)");
      }
      else if (blind_state == OPEN && adc_result < CLOSE_THRESHOLD) {
        stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
        blind_state = CLOSED;
        cooldown_counter = COOLDOWN_TICKS;
        displayMessage("Closed (dark)");
      }
    }

    _delay_ms(1);
  }
}
