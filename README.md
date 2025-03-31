# 2025-03-23  
Create ProjectC repository.  
Create individual branches for each collaborator.  
Generate project structure based on the MVC model.  

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

+---------------------+
| Tables_in_WeatherDB |
+---------------------+
| weather             |
| weatherData         |
| weatherData2        |
| weatherData3        |
| weatherData5        |
| weatherData6 (사용)  |
+---------------------+

 ### 테이블의 속성 
+-------+-------------+------+-----+---------+-------+
| Field | Type        | Null | Key | Default | Extra |
+-------+-------------+------+-----+---------+-------+
| tm    | datetime    | NO   |     | NULL    |       |
| stnId | varchar(10) | NO   |     | NULL    |       |
| wt    | char(20)    | NO   |     | NULL    |       |
+-------+-------------+------+-----+---------+-------+


# 2025-03-25
아침: 프로젝트 구상 및 계획, 역활 분담 

저녁: 기상청에 접속하여 api 인코딩 확인, api요청 및 출력에 대한 코딩 작성 

# 2025-03-26
아침: 출력 후 mysql에 연결 해 db에 등록되도록 코딩

저녁: 값을 확인 후 내가 원하는 값이 나오도록 코딩 변경


# 2025-03-27
아침: 코딩 오류 수정

저녁: 코딩 내용 정리 및 주석처리 및 README작성

# 2025-03-28
아침: 나의 데이터베이스를 wifi를 통해 다른컴퓨터로 이동  
HOST시 
동글이를 vmware로 연결 후 ifconfig를 통해 나의 ip주소 확인 후
ex)192.168.0.?
mysql 설치 확인 > MySQL이 원격 연결을 허용하도록 설정 변경 > MySQL 서비스 재시작  MySQL 서비스 재시작 > 원격 접속이 가능한 MySQL 사용자 계정 생성 > 방화벽(UFW)에서 MySQL 포트 3306 허용

저녁: 20초마다 데이터 입력과 DB에 정보 저장

# 2025-03-31
기술서 작성, ppt발표 준비