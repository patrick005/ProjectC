CREATE DATABASE WeatherDB;
grant all privileges on WeatherDB.* to 'myuser'@'%';
-- USE WeatherDB;

-- CREATE TABLE weather (
--     id INT AUTO_INCREMENT PRIMARY KEY,
--     fcstDate VARCHAR(8),
--     fcstTime VARCHAR(4),
--     category VARCHAR(10),
--     fcstValue VARCHAR(10)
-- );
USE WeatherDB;

CREATE TABLE weather (
    id INT AUTO_INCREMENT PRIMARY KEY, --자동 증가하는 정수형 기본 키로, 레코드의 고유 식별자 역할
    systime TIMESTAMP DEFAULT CURRENT_TIMESTAMP, -- 레코드 생성 시 시스템 시간을 자동으로 기록하는 타임스탬프
    api_time TIMESTAMP, -- API 응답 시간을 저장하는 타임스탬프
    api_id VARCHAR(3),  -- API 응답 데이터의 ID 값을 문자열 형태로 저장
    condition VARCHAR(20) -- API 응답 데이터의 ID 값을 기반으로 생성된 날씨 조건 문자열("맑음", "흐림" 등)을 저장
);
-- SQL 테이블 생성 시 id와 systime 컬럼은 자동으로 값이 입력되도록 설정
-- api_time, api_id, condition 컬럼의 값은 C 코드를 통해 직접 삽입