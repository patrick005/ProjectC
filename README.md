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

데이터베이스 내 테이블 목록
+-------------------+
| Tables_in_bowling |
+-------------------+
| score             |
| visit             |
+-------------------+

score 테이블의 속성
+-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+---------------+---------+------------+
| Table | Non_unique | Key_name | Seq_in_index | Column_name | Collation | Cardinality | Sub_part | Packed | Null | Index_type | Comment | Index_comment | Visible | Expression |
+-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+---------------+---------+------------+
| userID |          0 | PRIMARY  |            1 | id          | A         |          40 |     NULL |   NULL |      | BTREE      |         |               | YES     | NULL       |
+-------+------------+----------+--------------+-------------+-----------+-------------+----------+--------+------+------------+---------+---------------+---------+------------+



