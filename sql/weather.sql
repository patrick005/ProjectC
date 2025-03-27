--  CREATE DATABASE WeatherDB;
grant all privileges on WeatherDB.* to 'myuser'@'%';
USE WeatherDB;

CREATE TABLE weatherData6 (
    
    PRIMARY KEY (tm,stnId), -- 지점과 시간 조합을 기본키로 설정  
    tm DATETIME NOT NULL,  
    wtId VARCHAR(10) NOT NULL,
    wt char(20) NOT NULL
);
