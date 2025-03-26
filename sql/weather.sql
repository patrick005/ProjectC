CREATE DATABASE WeatherDB;
grant all privileges on WeatherDB.* to 'myuser'@'%';
USE WeatherDB;

CREATE TABLE weather (
    id INT AUTO_INCREMENT PRIMARY KEY,
    fcstDate VARCHAR(8),
    fcstTime VARCHAR(4),
    category VARCHAR(10),
    fcstValue VARCHAR(10)
);
