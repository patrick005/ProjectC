--  CREATE DATABASE WeatherDB;
grant all privileges on WeatherDB.* to 'myuser'@'%';
USE WeatherDB;

CREATE TABLE weatherData6 (
    
    PRIMARY KEY (tm,stnId), -- 지점과 시간 조합을 기본키로 설정  
    tm DATETIME NOT NULL,  
    wtId VARCHAR(10) NOT NULL,
    wt char(20) NOT NULL
);

grant all privileges on WeatherDB.* to 
FLUSH PRIVILEGES;


CREATE USER 'myuser'@'%' IDENTIFIED BY '0000';
grant all privileges on WeatherDB.* to 'myuser'@'%';
GRANT SELECT ON mysql.user TO 'myuser'@'%';
FLUSH PRIVILEGES;
SELECT * FROM mysql.user WHERE User;a

show columns from weatherData6;
SELECT * FROM weatherData6;

drop DATABASE WeatherData6;