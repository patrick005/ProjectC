--db생성

-- grant all privileges on mydb.* to myuser @ %;
-- grant all privileges on mydb.* to 'myuser' @'%'; /터미널에선 % 가려줘야함
use mydb;
create table UserInfo(
    -- ID INT AUTO_INCREMENT integer primary key,
    name varchar(50) NOT NULL,
    UserId varchar(50) UNIQUE NOT NULL,
    password varchar(255) NOT NULL,
    Email varchar(255) UNIQUE NOT NULL,
    -- price integer 추후 추가
);

-- show databases;
