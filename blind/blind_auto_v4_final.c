#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "uart0.h"
#include "lcd.h"

#define LIGHT_THRESHOLD_OPEN 600  // 조도 높을 때 여는 임계값
#define LIGHT_THRESHOLD_CLOSE 400 // 조도 낮을 때 닫는 임계값
#define OPEN_STEPS 3000
#define CLOSE_STEPS 3000
#define STEP_DELAY_MS 1

typedef enum { OPEN, CLOSED } BlindState;
BlindState blind_state = CLOSED;

typedef enum {
  MODE_LIGHT_SENSOR,
  MODE_MANUAL
} BlindMode;

BlindMode current_mode = MODE_LIGHT_SENSOR;

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
  DDRA |= 0x0F;
}

void stepMotor(int direction, int steps, int speed_ms) {
  for (int i = 0; i < steps; i++) {
    motor_step = (motor_step + direction + 8) % 8;
    PORTA = (PORTA & 0xF0) | (step_sequence[motor_step] & 0x0F);
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

void sendString(const char *str) {
  while (*str) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = *str++;
  }
}

void handleUARTCommand(char input) {
  lcdDataWrite(input);

  switch (input) {
    case 'l':
      current_mode = MODE_LIGHT_SENSOR;
      sendString("\r\n[MODE] Light sensor mode selected\r\n");
      break;

    case 'm':
      current_mode = MODE_MANUAL;
      sendString("\r\n[MODE] Manual control mode selected\r\n");
      break;

    case 'o':
      if (blind_state == CLOSED) {
        stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
        blind_state = OPEN;
        sendString("\r\nBlind OPENED\r\n");
      } else {
        sendString("\r\nBlind is already open\r\n");
      }
      break;

    case 'c':
      if (blind_state == OPEN) {
        stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
        blind_state = CLOSED;
        sendString("\r\nBlind CLOSED\r\n");
      } else {
        sendString("\r\nBlind is already closed\r\n");
      }
      break;

    default:
      sendString("\r\n[ERROR] Unknown command. Use l/m/o/c\r\n");
      break;
  }
}

int main(void) {
  initADC();
  initMotorPins();
  initUART();
  uart0Init();

  lcdGotoXY(0, 0);
  stdin = &INPUT;

  sendString("\r\nEnter command: l(light) / m(manual) / o(open) / c(close)\r\n");

  while (1) {
    // UART 명령 입력 확인
    if (UCSR0A & (1 << RXC0)) {
      char input = readUART();
      handleUARTCommand(input);
      getchar(); // 개행문자 제거
    }

    // 조도센서 모드
    if (current_mode == MODE_LIGHT_SENSOR) {
      adc_result = readADC(0);

      if (adc_result > LIGHT_THRESHOLD_OPEN && blind_state == CLOSED) {
        stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
        blind_state = OPEN;
        sendString("\r\n[LIGHT] Bright → Blind OPENED\r\n");

      } else if (adc_result < LIGHT_THRESHOLD_CLOSE && blind_state == OPEN) {
        stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
        blind_state = CLOSED;
        sendString("\r\n[LIGHT] Dark → Blind CLOSED\r\n");
      }

      _delay_ms(500); // 너무 자주 움직이지 않도록 약간의 딜레이
    }
  }
}
