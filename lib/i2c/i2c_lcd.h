#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include <avr/io.h>

// LCD 명령어
#define LCD_CLEAR           0x01
#define LCD_HOME            0x02
#define LCD_ENTRY_MODE      0x06
#define LCD_DISPLAY_ON      0x0C
#define LCD_FUNCTION_SET    0x28 // 4-bit, 2-line, 5x8 dot
#define LCD_SET_DDRAM_ADDR  0x80

void i2c_lcd_init(uint8_t lcd_addr);
void i2c_lcd_clear(void);
void i2c_lcd_home(void);
void i2c_lcd_goto(uint8_t row, uint8_t col);
void i2c_lcd_puts(const char *str);
void i2c_lcd_write(char data, uint8_t mode);
void i2c_lcd_backlight(uint8_t on);

#endif /* I2C_LCD_H_ */
