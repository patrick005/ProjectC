/* 스마트홈 통합 코드 (조도 기반 블라인드 + LED 제어 + UART 수신 날씨/시간 표시)
   작성일: 2025-03-28 */

   #include <avr/io.h>
   #include <util/delay.h>
   #include <avr/interrupt.h>
   #include <stdbool.h>
   #include <stdio.h>
   #include <string.h>
   #include "uart0.h"
   #include "lcd.h"
   
   // ----- 블라인드 제어 상수 -----
   #define LIGHT_THRESHOLD_OPEN 420
   #define LIGHT_THRESHOLD_CLOSE 200
   #define OPEN_STEPS 8100
   #define CLOSE_STEPS 8100
   #define STEP_DELAY_MS 1
   
   // ----- LED 제어 상수 -----
   #define CDS_THRESHOLD 2
   
   // ----- 핀 정의 -----
   #define CDS_PIN_BLIND PF0
   #define CDS_PIN_LED PF1
   #define PIR_PIN PD0
   
   // ----- 상태 enum -----
   typedef enum { OPEN, CLOSED } BlindState;
   typedef enum { MODE_LIGHT_SENSOR, MODE_MANUAL } BlindMode;
   
   // ----- 전역 변수 -----
   BlindState blind_state = CLOSED;
   BlindMode current_mode = MODE_LIGHT_SENSOR;
   
   uint8_t motor_step = 0;
   const uint8_t step_sequence[8] = {
       0b00001000, 0b00001100, 0b00000100, 0b00000110,
       0b00000010, 0b00000011, 0b00000001, 0b00001001
   };
   
   uint16_t adc_result_blind = 0;
   uint16_t adc_result_led = 0;
   volatile uint16_t timer1count = 0;
   
   char lcdWeatherLine[17] = {0};
   char lcdTimeLine[17] = {0};
   char uartBuffer[16] = {0};
   uint8_t bufIndex = 0;
   
   // ----- 함수 선언 -----
   void initADC();
   uint16_t readADC(uint8_t channel);
   void initMotorPins();
   void stepMotor(int direction, int steps, int speed_ms);
   void initUART();
   char readUART();
   void sendString(const char *str);
   void handleBlindCommand(char input);
   void timer1Init();
   void processUartBuffer();
   
   int main(void) {
       initADC();
       initMotorPins();
       initUART();
       uart0Init();
       lcdInit();
   
       stdin = &INPUT;
       stdout = &OUTPUT;
       DDRE = 0x02;
   
       lcdGotoXY(0, 0);
       lcdPrint("SMART HOME READY");
       sendString("\r\nEnter command: l(light) / m(manual) / o(open) / c(close)\r\n");
   
       DDRF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
       PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
   
       DDRD &= ~(1 << PIR_PIN);
       timer1Init();
       sei();
   
       while (1) {
           if (UCSR0A & (1 << RXC0)) {
               char c = fgetc(stdin);
   
               // 명령 문자 처리
               if (c == 'l' || c == 'm' || c == 'o' || c == 'c') {
                   handleBlindCommand(c);
               } 
               // 문자열 처리
               else {
                   if (c != '\n' && bufIndex < sizeof(uartBuffer) - 1) {
                       uartBuffer[bufIndex++] = c;
                   } else {
                       uartBuffer[bufIndex] = '\0';
                       processUartBuffer();
                       bufIndex = 0;
                       memset(uartBuffer, 0, sizeof(uartBuffer));
                   }
               }
           }
   
           if (current_mode == MODE_LIGHT_SENSOR) {
               adc_result_blind = readADC(CDS_PIN_BLIND);
               if (adc_result_blind > LIGHT_THRESHOLD_OPEN && blind_state == CLOSED) {
                   blind_state = OPEN;
                   stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
                   sendString("\r\n[LIGHT] Bright -> OPEN\r\n");
               } else if (adc_result_blind < LIGHT_THRESHOLD_CLOSE && blind_state == OPEN) {
                   blind_state = CLOSED;
                   stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
                   sendString("\r\n[LIGHT] Dark -> CLOSE\r\n");
               }
               _delay_ms(100);
           }
       }
   }
   
   void processUartBuffer() {
       if (bufIndex == 2 && uartBuffer[0] == 'W') {
           switch (uartBuffer[1]) {
               case '1': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "STORMY        "); break;
               case '2': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "RAINY         "); break;
               case '3': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "SNOWY         "); break;
               case '4': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "FOGGY         "); break;
               case '5': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "YELLOW DUST   "); break;
               case '6': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "SUNNY         "); break;
               case '7': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "PARTLY CLOUDY "); break;
               case '8': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "CLOUDY        "); break;
               case '9': snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "OVERCAST      "); break;
               default: snprintf(lcdWeatherLine, sizeof(lcdWeatherLine), "Invalid       "); break;
           }
           lcdGotoXY(0, 0);
           lcdPrint(lcdWeatherLine);
       } else if ((bufIndex == 14 || bufIndex == 15) && uartBuffer[2] == '-' && uartBuffer[5] == ' ' && uartBuffer[8] == ':' && uartBuffer[11] == ':') {
           strncpy(lcdTimeLine, uartBuffer, 15);
           lcdTimeLine[15] = '\0';
           lcdGotoXY(0, 1);
           lcdPrint(lcdTimeLine);
       }
   }
   //ledpir.c blind.c WetherLCD.c 헤더 합본
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "uart0.h"
#include "lcd.h"

// 블라인드 제어 관련 상수
#define LIGHT_THRESHOLD_OPEN 420
#define LIGHT_THRESHOLD_CLOSE 200
#define OPEN_STEPS 8100 //2바퀴 토크
#define CLOSE_STEPS 8100
#define STEP_DELAY_MS 1

// LED 제어 관련 상수
#define CDS_THRESHOLD 2

// 핀 정의
#define CDS_PIN_BLIND PF0
#define CDS_PIN_LED PF1
#define PIR_PIN PD0

// 블라인드 상태 및 모드
typedef enum { OPEN, CLOSED } BlindState;
BlindState blind_state = CLOSED;
typedef enum { MODE_LIGHT_SENSOR, MODE_MANUAL } BlindMode;
BlindMode current_mode = MODE_LIGHT_SENSOR;

// 모터 제어 관련 변수
uint8_t motor_step = 0;
const uint8_t step_sequence[8] = {
    0b00001000, 0b00001100, 0b00000100, 0b00000110,
    0b00000010, 0b00000011, 0b00000001, 0b00001001
};

// ADC 관련 변수
uint16_t adc_result_blind = 0;
uint16_t adc_result_led = 0;

// 날씨 관련 LCD출력 문자열
char weatherStr[17] = {0};  
char timeStr[17]    = {0};

// 타이머 관련 변수
volatile uint16_t timer1count = 0;

// 함수 선언
void initADC();
uint16_t readADC(uint8_t channel);
void initMotorPins();
void stepMotor(int direction, int steps, int speed_ms);
void initUART();
char readUART();
void sendString(const char *str);
void handleUARTCommand(char input);
void timer1Init();


int main(void) {
    initADC();
    initMotorPins();
    initUART(); // UART 초기화
    uart0Init();
    lcdInit();   // LCD 초기화

    lcdGotoXY(0, 0);
    stdin = &INPUT;
    stdout = &OUTPUT;

    sendString("\r\nEnter command: l(light) / m(manual) / o(open) / c(close)\r\n");
    
    // Rx 입력, Tx 출력
    DDRE = 0x02;

    char buf[16] = {0};
    uint8_t bufIndex = 0; //UART 수신 데이터를 저장할 버퍼
    
    // RGB LED 설정
    DDRF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
    PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);

    // PIR 센서 설정
    DDRD &= ~(1 << PIR_PIN);

    timer1Init();
    sei();

    while (1) {
        if (UCSR0A & (1 << RXC0)) {
            char input = readUART();
            handleUARTCommand(input);
        }

        if (current_mode == MODE_LIGHT_SENSOR) {
            adc_result_blind = readADC(CDS_PIN_BLIND);

            if (adc_result_blind > LIGHT_THRESHOLD_OPEN && blind_state == CLOSED) {
                blind_state = OPEN; //디버깅용 선출력메시지
                stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
                // blind_state = OPEN;
                sendString("\r\n[LIGHT] Bright → Blind OPENED\r\n");
            } else if (adc_result_blind < LIGHT_THRESHOLD_CLOSE && blind_state == OPEN) {
                blind_state = CLOSED;  //디버깅용 선출력메시지
                stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
                // blind_state = CLOSED;
                sendString("\r\n[LIGHT] Dark → Blind CLOSED\r\n");
            }

            _delay_ms(100);
        }
    }
}

// ADC 초기화
void initADC() {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

// ADC 값 읽기 (멀티플렉싱)
uint16_t readADC(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

// 모터 핀 초기화
void initMotorPins() {
    DDRA |= 0x0F;
}

// 스테핑 모터 제어
void stepMotor(int direction, int steps, int speed_ms) {
    for (int i = 0; i < steps; i++) {
        motor_step = (motor_step + direction + 8) % 8;
        PORTA = (PORTA & 0xF0) | (step_sequence[motor_step] & 0x0F);
        _delay_ms(speed_ms);
    }
}

// UART 초기화
void initUART() {
    UBRR0H = 0;
    UBRR0L = 8;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// UART 문자 읽기
char readUART() {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

// UART 문자열 전송
void sendString(const char *str) {
    while (*str) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *str++;
    }
}

// UART 명령 처리
void handleUARTCommand(char input) {
    lcdDataWrite(input);

    switch (input) {
        case 'l': current_mode = MODE_LIGHT_SENSOR; sendString("\r\n[MODE] Light sensor mode selected\r\n"); break;
        case 'm': current_mode = MODE_MANUAL; sendString("\r\n[MODE] Manual control mode selected\r\n"); break;
        case 'o': if (blind_state == CLOSED) { stepMotor(1, OPEN_STEPS, STEP_DELAY_MS); blind_state = OPEN; sendString("\r\nBlind OPENED\r\n"); } else { sendString("\r\nBlind is already open\r\n"); } break;
        case 'c': if (blind_state == OPEN) { stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS); blind_state = CLOSED; sendString("\r\nBlind CLOSED\r\n"); } else { sendString("\r\nBlind is already closed\r\n"); } break;
        default: sendString("\r\n[ERROR] Unknown command. Use l/m/o/c\r\n"); break;
    }
}

// 타이머 1 초기화
void timer1Init(void) {
    TCCR1B |= (1 << WGM12);
    OCR1A = 1562;
    TIMSK |= (1 << OCIE1A);
    TCCR1B |= (1 << CS12) | (1 << CS10);
}

// 타이머 1 인터럽트 서비스 루틴
ISR(TIMER1_COMPA_vect) {
    uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 1 : 0;
    adc_result_led = readADC(CDS_PIN_LED);

    if (!(pirState) && (adc_result_led < CDS_THRESHOLD)) {
        timer1count = 1;
        if (timer1count < 100) {
            PORTF &= ~(1 << PF3);
            PORTF &= ~(1 << PF4);
            PORTF |= (1 << PF5);
            timer1count++;
        } else {
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
}
   // 이하 함수들은 기존 코드 동일: initADC, readADC, initMotorPins, stepMotor, initUART, readUART, sendString, handleBlindCommand, timer1Init, ISR
   