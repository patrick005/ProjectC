#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

// 스위치 초기화: PE4, PE5, PE6, PE7를 입력으로 설정하고 내부 풀업 활성화
void initSwitches() {
  DDRE &= ~((1 << PE4) | (1 << PE5) | (1 << PE6) | (1 << PE7)); // 입력으로 설정
  PORTE |= (1 << PE4) | (1 << PE5) | (1 << PE6) | (1 << PE7);     // 내부 풀업 활성화
}

int main(void) {
  // LCD와 스위치 초기화
  lcdInit();
  initSwitches();

  char buffer[17]; // LCD 한 줄에 최대 16글자까지 출력 (null 포함 17)

  while (1) {
    // PINE 레지스터에서 스위치 상태 읽기
    uint8_t switches = PINE;
    
    // 내부 풀업이 활성화되어 있으므로, 버튼이 눌리지 않으면 HIGH(1),
    // 버튼이 눌리면 LOW(0)로 읽힙니다.
    // 각 핀의 상태를 1(안눌림) 또는 0(눌림)으로 표시합니다.
    sprintf(buffer, "PE4:%d PE5:%d", 
            (switches & (1 << PE4)) ? 1 : 0, 
            (switches & (1 << PE5)) ? 1 : 0);
    lcdClear();
    lcdGotoXY(0, 0);
    lcdPrint(buffer);
    
    sprintf(buffer, "PE6:%d PE7:%d", 
            (switches & (1 << PE6)) ? 1 : 0, 
            (switches & (1 << PE7)) ? 1 : 0);
    lcdGotoXY(0, 1);
    lcdPrint(buffer);

    _delay_ms(200); // 200ms 간격으로 업데이트
  }
}
