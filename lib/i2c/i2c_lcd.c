#include "i2c_lcd.h"
#include <util/delay.h>

#define TWI_START   0x08
#define TWI_REP_START 0x10
#define TWI_MT_SLA_ACK 0x18
#define TWI_MT_DATA_ACK 0x28

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE    0x04
#define LCD_RS        0x01

static uint8_t lcd_addr;
static uint8_t backlight_flag = LCD_BACKLIGHT;

void i2c_start(void) {
    TWCR = (1<<TWSTA)|(1<<TWEN)|(1<<TWINT);
    while (!(TWCR & (1<<TWINT)));
}

void i2c_stop(void) {
    TWCR = (1<<TWSTO)|(1<<TWEN)|(1<<TWINT);
    _delay_us(10);
}

void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWEN)|(1<<TWINT);
    while (!(TWCR & (1<<TWINT)));
}

void lcd_write_4bits(uint8_t data) {
    i2c_start();
    i2c_write(lcd_addr << 1);
    i2c_write(data | LCD_ENABLE | backlight_flag);
    _delay_us(1);
    i2c_write((data & ~LCD_ENABLE) | backlight_flag);
    i2c_stop();
}

void i2c_lcd_write(char data, uint8_t mode) {
    uint8_t high = data & 0xF0;
    uint8_t low = (data << 4) & 0xF0;
    lcd_write_4bits(high | (mode ? LCD_RS : 0));
    lcd_write_4bits(low  | (mode ? LCD_RS : 0));
    _delay_us(50);
}

void i2c_lcd_cmd(uint8_t cmd) {
    i2c_lcd_write(cmd, 0);
}

void i2c_lcd_init(uint8_t addr) {
    lcd_addr = addr;
    
    TWSR = 0x00;   // prescaler = 1
    TWBR = 72;     // 100kHz @ 16MHz
    TWCR = (1<<TWEN); // Enable TWI

    _delay_ms(50);

    lcd_write_4bits(0x30);
    _delay_ms(5);
    lcd_write_4bits(0x30);
    _delay_us(150);
    lcd_write_4bits(0x30);
    _delay_us(50);
    lcd_write_4bits(0x20); // 4-bit mode
    _delay_us(50);

    i2c_lcd_cmd(LCD_FUNCTION_SET);
    i2c_lcd_cmd(LCD_DISPLAY_ON);
    i2c_lcd_cmd(LCD_CLEAR);
    i2c_lcd_cmd(LCD_ENTRY_MODE);
    _delay_ms(2);
}

void i2c_lcd_clear(void) {
    i2c_lcd_cmd(LCD_CLEAR);
    _delay_ms(2);
}

void i2c_lcd_home(void) {
    i2c_lcd_cmd(LCD_HOME);
    _delay_ms(2);
}

void i2c_lcd_goto(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    addr += col;
    i2c_lcd_cmd(LCD_SET_DDRAM_ADDR | addr);
}

void i2c_lcd_puts(const char *str) {
    while (*str) {
        i2c_lcd_write(*str++, 1);
    }
}

void i2c_lcd_backlight(uint8_t on) {
    backlight_flag = on ? LCD_BACKLIGHT : 0x00;
    // Force re-write to update backlight
    lcd_write_4bits(0x00);
}
