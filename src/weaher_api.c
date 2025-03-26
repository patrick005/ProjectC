#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <time.h>
#include <mysql/mysql.h>

#define API_URL "http://apis.data.go.kr/1360000/VilageFcstInfoService_2.0/getVilageFcst"
#define SERVICE_KEY "ca9r8VuUVqipZgh0faFDUCDqEJ9aE0Die5Pukgv9HhJgCNzmre3V7cgTmubPrg1TZXhAO5Ue%2Fu13xqtFRDMIxw%3D%3D"
#define DB_HOST "localhost"
#define DB_USER "myuser"
#define DB_PASS "0000"
#define DB_NAME "WeatherDB"

typedef struct {
    char data[10000];
    size_t length;
} ResponseBuffer;

// 데이터를 받아서 처리하는 콜백 함수
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    ResponseBuffer *response = (ResponseBuffer *)userdata;
    size_t total_size = size * nmemb;

    if (response->length + total_size < sizeof(response->data) - 1) {
        strncat(response->data, (char *)ptr, total_size);
        response->length += total_size;
    }

    return total_size;
}

// 현재 날짜와 시간을 구하는 함수
void get_current_date_time(char *base_date, char *base_time) {
    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);

    // 날짜 (YYYYMMDD)
    strftime(base_date, 9, "%Y%m%d", tm_info);

    // 최근 3시간 단위의 정각 시간 계산
    int hour = tm_info->tm_hour;
    int base_hour = (hour / 3) * 3;

    if (base_hour == 0) {
        base_hour = 21;
        time_t yesterday = now - 86400;
        struct tm *yesterday_info = localtime(&yesterday);
        strftime(base_date, 9, "%Y%m%d", yesterday_info);
    }

    snprintf(base_time, 5, "%02d00", base_hour);
}

// MySQL DB에 데이터 저장 함수
void save_to_database(const char *fcstDate, const char *fcstTime, const char *category, const char *fcstValue) {
    MYSQL *conn;
    char *host = "localhost";
    char *user = "myuser";
    char *pass = "0000";
    char *db = "WeatherDB";
    int port = 3306;
    
    char query[512];

    // MySQL 연결
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\n");
        return;
    }

    if (mysql_real_connect(conn,host,user,pass,db,port,NULL,0) == NULL) {
        fprintf(stderr, "MySQL connection failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    // 데이터 삽입 쿼리 실행
    snprintf(query, sizeof(query), 
             "INSERT INTO weather (fcstDate, fcstTime, category, fcstValue) VALUES ('%s', '%s', '%s', '%s')", 
             fcstDate, fcstTime, category, fcstValue);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "MySQL insert error: %s\n", mysql_error(conn));
    } else {
        printf("Data inserted into DB: %s %s %s %s\n", fcstDate, fcstTime, category, fcstValue);
    }

    // MySQL 연결 종료
    mysql_close(conn);
}

int main() {
    CURL *curl;
    CURLcode res;
    char url[1024];
    ResponseBuffer response = { .data = "", .length = 0 };

    char base_date[9];
    char base_time[5];

    // 현재 날짜와 시간 구하기
    get_current_date_time(base_date, base_time);

    // API 요청 URL 구성
    snprintf(url, sizeof(url),
             "%s?serviceKey=%s&numOfRows=10&pageNo=1&base_date=%s&base_time=%s&nx=66&ny=106&dataType=JSON", 
             API_URL, SERVICE_KEY, base_date, base_time);

    // curl 초기화
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // 요청 설정
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // 요청 실행
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // 응답 데이터 출력
            printf("Response: \n%s\n", response.data);

            // JSON 파싱
            struct json_object *parsed_json;
            struct json_object *response_body;
            struct json_object *body;
            struct json_object *items;

            parsed_json = json_tokener_parse(response.data);
            if (!json_object_object_get_ex(parsed_json, "response", &response_body)) {
                printf("Error: 'response' not found in JSON\n");
                return 1;
            }
            if (!json_object_object_get_ex(response_body, "body", &body)) {
                printf("Error: 'body' not found in JSON\n");
                return 1;
            }
            if (!json_object_object_get_ex(body, "items", &items)) {
                printf("Error: 'items' not found in JSON\n");
                return 1;
            }

            // items 배열인지 확인
            if (json_object_get_type(items) != json_type_array) {
                printf("Error: 'items' is not an array\n");
                return 1;
            }

            // 각 아이템(날씨 데이터) 저장
            for (size_t i = 0; i < json_object_array_length(items); i++) {
                struct json_object *item = json_object_array_get_idx(items, i);
                struct json_object *fcstDate, *fcstTime, *category, *fcstValue;

                json_object_object_get_ex(item, "fcstDate", &fcstDate);
                json_object_object_get_ex(item, "fcstTime", &fcstTime);
                json_object_object_get_ex(item, "category", &category);
                json_object_object_get_ex(item, "fcstValue", &fcstValue);

                // MySQL DB에 데이터 저장
                save_to_database(json_object_get_string(fcstDate),
                                 json_object_get_string(fcstTime),
                                 json_object_get_string(category),
                                 json_object_get_string(fcstValue));
            }
        }

        // curl 종료
        curl_easy_cleanup(curl);
    }

    // curl 전역 종료
    curl_global_cleanup();

    return 0;
}