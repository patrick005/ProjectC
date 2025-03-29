#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "uart0.h"
#include "lcd.h"
#include <string.h> 

// === BLINDS + LED SETTINGS ===
#define CDS_THRESHOLD 1
#define LIGHT_THRESHOLD_OPEN 700
#define LIGHT_THRESHOLD_CLOSE 200
#define OPEN_STEPS 8100
#define CLOSE_STEPS 8100
#define STEP_DELAY_MS 1

#define CDS_PIN_BLIND PF0
#define CDS_PIN_LED PF1
#define PIR_PIN PD0

// === STRUCTURES ===
typedef enum { OPEN, CLOSED } BlindState;
BlindState blind_state = CLOSED;
typedef enum { MODE_LIGHT_SENSOR, MODE_MANUAL } BlindMode;
BlindMode current_mode = MODE_LIGHT_SENSOR;

// === MOTOR ===
uint8_t motor_step = 0;
const uint8_t step_sequence[8] = {
    0b00001000, 0b00001100, 0b00000100, 0b00000110,
    0b00000010, 0b00000011, 0b00000001, 0b00001001
};

// === GLOBALS ===
uint16_t adc_result_blind = 0;
uint16_t adc_result_led = 0;
volatile uint16_t timer1count = 0;

char weatherStr[17] = {0};

void initADC();
uint16_t readADC(uint8_t channel);
void initMotorPins();
void stepMotor(int direction, int steps, int speed_ms);
void initUART();
void sendString(const char *str);
void handleUARTCommand(char input);
void processWeatherCommand(char weatherCode);
void timer1Init();

int main(void) {
    initADC();
    initMotorPins();
    initUART();
    uart0Init();
    lcdInit();

    stdin = &INPUT;
    stdout = &OUTPUT;

    lcdGotoXY(0, 0);
    lcdPrint("Waiting...");

    DDRF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
    PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);

    DDRD &= ~(1 << PIR_PIN);

    timer1Init();
    sei();

    while (1) {
        // UART 입력을 바로 처리 (버퍼에 누적하지 않음)
        if (UCSR0A & (1 << RXC0)) {
            char c = fgetc(stdin);
            // weather 명령: 'W' 다음에 숫자 한 문자를 읽음
            if (c == 'W') {  
                while (!(UCSR0A & (1 << RXC0)));
                char code = fgetc(stdin);
                processWeatherCommand(code);
            } else {
                // 나머지 명령은 한 문자 입력 시 즉시 처리
                handleUARTCommand(c);
            }
        }

        // Light sensor 모드일 때만 센서값을 읽음
        if (current_mode == MODE_LIGHT_SENSOR) {
            adc_result_blind = readADC(CDS_PIN_BLIND);
            if (adc_result_blind > LIGHT_THRESHOLD_OPEN && blind_state == CLOSED) {
                blind_state = OPEN;
                stepMotor(1, OPEN_STEPS, STEP_DELAY_MS);
                sendString("\r\n[LIGHT] Bright -> Blinds OPENED\r\n");
            } else if (adc_result_blind < LIGHT_THRESHOLD_CLOSE && blind_state == OPEN) {
                blind_state = CLOSED;
                stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS);
                sendString("\r\n[LIGHT] Dark -> Blinds CLOSED\r\n");
            }
            _delay_ms(100);
        }
    }
}

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

void sendString(const char *str) {
    while (*str) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *str++;
    }
}

void handleUARTCommand(char input) {
    switch (input) {
        case 'l': 
            current_mode = MODE_LIGHT_SENSOR; 
            sendString("\r\n[MODE] Light sensor mode\r\n"); 
            break;
        case 'm': 
            current_mode = MODE_MANUAL;       
            sendString("\r\n[MODE] Manual mode\r\n"); 
            break;
        case 'o': 
            if (blind_state == CLOSED) { 
                stepMotor(1, OPEN_STEPS, STEP_DELAY_MS); 
                blind_state = OPEN; 
                sendString("\r\nBlind OPENED\r\n"); 
            }
            break;
        case 'c': 
            if (blind_state == OPEN) { 
                stepMotor(-1, CLOSE_STEPS, STEP_DELAY_MS); 
                blind_state = CLOSED; 
                sendString("\r\nBlind CLOSED\r\n"); 
            }
            break;
        default: 
            sendString("\r\n[ERROR] Unknown command\r\n"); 
            break;
    }
}

void processWeatherCommand(char weatherCode) {
    switch (weatherCode) {
        case '1': snprintf(weatherStr, sizeof(weatherStr), "STORMY        "); break;
        case '2': snprintf(weatherStr, sizeof(weatherStr), "RAINY         "); break;
        case '3': snprintf(weatherStr, sizeof(weatherStr), "SNOWY         "); break;
        case '4': snprintf(weatherStr, sizeof(weatherStr), "FOGGY         "); break;
        case '5': snprintf(weatherStr, sizeof(weatherStr), "YELLOW DUST   "); break;
        case '6': snprintf(weatherStr, sizeof(weatherStr), "SUNNY         "); break;
        case '7': snprintf(weatherStr, sizeof(weatherStr), "PARTLY CLOUDY "); break;
        case '8': snprintf(weatherStr, sizeof(weatherStr), "CLOUDY        "); break;
        case '9': snprintf(weatherStr, sizeof(weatherStr), "OVERCAST      "); break;
        default:  snprintf(weatherStr, sizeof(weatherStr), "Invalid       "); break;
    }
    lcdGotoXY(0, 0);
    lcdPrint(weatherStr);
}

void timer1Init(void) {
    TCCR1B |= (1 << WGM12);
    OCR1A = 1562;
    TIMSK |= (1 << OCIE1A);
    TCCR1B |= (1 << CS12) | (1 << CS10);
}

ISR(TIMER1_COMPA_vect) {
    uint8_t pirState = (PIND & (1 << PIR_PIN)) ? 1 : 0;
    adc_result_led = readADC(CDS_PIN_LED);
    if (!pirState && (adc_result_led < CDS_THRESHOLD)) {
        timer1count = 1;
        if (timer1count < 100) {
            PORTF &= ~(1 << PF3);
            PORTF &= ~(1 << PF4);
            PORTF |=  (1 << PF5);
            timer1count++;
        } else {
            PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
            timer1count = 0;
        }
    } else {
        if (timer1count > 0 && timer1count < 100) {
            timer1count++;
        } else if (timer1count == 100) {
            PORTF |= (1 << PF3) | (1 << PF4) | (1 << PF5);
            timer1count = 0;
        }
    }
}
