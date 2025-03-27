#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "uart0.h"
#include "lcd.h"


#define LIGHT_THRESHOLD 400 // 조도 센서(숫자 낮을 수록 민감)
#define OPEN_STEPS 3000     // 모터 회전 수
#define CLOSE_STEPS 3000    // 모터 회전 수(역방향)
#define STEP_DELAY_MS 1     // 모터 회전 속도(숫자 낮을 수록 빠름)


// 블라인드 상태 정의
typedef enum { OPEN, CLOSED } BlindState; // OPEN = 0, CLOSED = 1
BlindState blind_state = CLOSED;          // 블라인드 상태 기본값 = CLOSED

// 작동 모드 정의
typedef enum {
  MODE_LIGHT_SENSOR,
  MODE_SCHEDULED_TIME,
  MODE_MANUAL_TIME
} BlindMode;

BlindMode current_mode = MODE_LIGHT_SENSOR; // 초기 모드

// 스케줄 시간 구조체
typedef struct {
  uint8_t open_hour;  // 7
  uint8_t open_min;   // 0
  uint8_t close_hour; // 20
  uint8_t close_min;  // 0
} ScheduleTime;

ScheduleTime schedule = {7, 0, 20, 0}; // 예시 시간: 07:00, 20:00

// 현재 시간 (나중에 UART로 받아서 갱신)
uint8_t current_hour = 7;
uint8_t current_min = 0;

uint16_t adc_result = 0;
uint8_t motor_step = 0;

// 모터 스텝 시퀀스 (전압 순차적으로 걸어서 모터 회전)
const uint8_t step_sequence[8] = {
  0b00001000, 0b00001100, 0b00000100, 0b00000110,
  0b00000010, 0b00000011, 0b00000001, 0b00001001
};

// ADC초기화 함수
void initADC() {
  ADMUX = (1 << REFS0); 
  // (01000000) 기준 전압 AVCC (보통 5V), 입력 채널 : ADC0 (PORTF0 핀)
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); 
  // (10000110) ADC 사용 활성화, 분주비 ÷8 -> ADC 클럭 = 시스템 클럭 ÷8 = 2MHz (시스템클럭 : 16MHz)
}

uint16_t readADC(uint8_t channel) {
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); 
  // ADMUX 의 상위 4개비트 설정 유지 및 채널 하위 4개비트 설정 유지
  
  ADCSRA |= (1 << ADSC); 
  // ADSC = bit 6 (아날로그 데이터 변환 시작)
  
  while (ADCSRA & (1 << ADSC)); 
  // ADSSC가 1인 동안 변환중(while문 실행), 0이 되면 (변환 종료) 탈출
  
  return ADC; 
  // 변환된 ADC 결과값(10비트) 리턴 (함수를 호출한 쪽에서 변수로 받아 저장)
}

// 모터와 연결된 PORTC 의 하위 4개 비트 출력방향으로 설정
void initMotorPins() {
  DDRC |= 0x0F;
}

// 모터 작동 제어를 위한 함수
void stepMotor(int direction, int steps, int speed_ms) {
  for (int i = 0; i < steps; i++) {
    motor_step = (motor_step + direction + 8) % 8;
    PORTC = (PORTC & 0xF0) | (step_sequence[motor_step] & 0x0F);
    _delay_ms(speed_ms);
  }
}
// direction : 회전방향 (1 : 시계방향, -1 : 반시계방향)
// steps : 몇 번 회전할 지
// speed_ms : 회전 속도 (지연 시간 ms 단위)

void initUART() {
  UBRR0H = 0;
  UBRR0L = 8; // 115200bps
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
  lcdDataWrite(input);
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
  
  lcdGotoXY(0, 0);
  
  stdin = &INPUT;
  // stdout = &OUTPUT;

  sendString("\r\nSelect mode: l (light)  s (schedule)  m (manual)\r\n");

  while (1) {
    // UART 입력이 있으면 모드 전환 시도
    if (UCSR0A & (1 << RXC0)) {
      selectModeFromUART();
      getchar();
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
