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


# 2025-03-25
아침: 프로젝트 구상 및 계획, 역활 분담 
저녁: api에 대한 정보 검색 및 코딩 작성

# 2025-03-26
아침: api에 대한 정보 검색 및 코딩 작성
저녁: db에 코딩 등록

# 2025-03-27
아침: db에 코딩 등록
저녁: 코딩 내용 정리 및 주석처리 및 README작성

# 2025-03-28
아침:
저녁: