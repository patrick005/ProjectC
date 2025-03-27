CREATE DATABASE WeatherDB;
grant all privileges on WeatherDB.* to 'myuser'@'%';
USE WeatherDB;

CREATE TABLE weatherData5 (
    
    PRIMARY KEY (tm,stnId) -- 지점과 시간 조합을 기본키로 설정  // PRIMARY KEY (tm, stnId)
    tm DATETIME NOT NULL,  
    stnId VARCHAR(4) NOT NULL,
    wt char(20) NOT NULL
);
