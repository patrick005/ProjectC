# 작업 내용

| 날짜   | 시간  | 참여자   | 작업 내용                                                                                           |
|--------|--------|----------|-----------------------------------------------------------------------------------------------------|
| 25일   | 오전  | 전체 팀  | 아이디어 구상 및 작업 분배                                                                         |
| 25일   | 오후  | 전체 팀  | 각 파트별 개발 구상 및 자료 조사                                                                   |
| 26일   | 오전  | 임윤진   | CDS 값을 받아 OLED를 주황색 기본으로 출력 조정 및 코드 최적화 / PIR 센서를 접목한 LED 출력 제어 시도 중 |
| 26일   | 오전  | 정유진   | Git fork로 인해 작업장소 꼬인 것 해결, AVR 헤더에 대한 참조 시도 중                              |
| 26일   | 오전  | 송우림   | Git fork로 인해 작업장소 꼬인 것 해결, CDS 값을 기반으로 한 스텝 모터 회전 구현 / 정밀도 설정 시도 중 |
| 26일   | 오전  | 정영재   | PPT 제작(과정, 결과 제외), 팀 구성원 Git 설정, 디버깅, 센서 원리 설명, 기본 구현 회로 자료 배포   |
| 26일   | 오전  | 정석준   | API 받아오기 성공 / SQL에 넣는 것 구현 시도 중                                                    |
| 26일   | 오후  |         |                                                                                                        |


## File Tree

ProjectC  
├── CMakeLists.txt  
├── LICENSE  
├── README.md  
├── include  
├── sql  
│   └── table.sql  
├── src  
│   └── main.c  
└── web  

---

### 데이터베이스 내 테이블 목록  

| Tables_in_UserInfo |  
|--------------------|  
| ID                 |  
| name               |  
| UserId             |  
| password           |  
| Email              |  


 ### 테이블의 속성 
| CalumName | Type     | Size | UNIQUEable | NULLable |  
|-----------|----------|------|------------|----------|  
| name      | varchar  | 50   |            | NOT NULL |  
| UserId    | varchar  | 50   | UNIQUE     | NOT NULL |  
| password  | varchar  | 255  |            | NOT NULL |  
| Email     | varchar  | 255  | UNIQUE     | NOT NULL |  
| etc       | -        | -    | -          | -        |  


