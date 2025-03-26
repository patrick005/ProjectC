#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <time.h>

#define API_URL "http://apis.data.go.kr/1360000/VilageFcstInfoService_2.0/getVilageFcst"
#define SERVICE_KEY "ca9r8VuUVqipZgh0faFDUCDqEJ9aE0Die5Pukgv9HhJgCNzmre3V7cgTmubPrg1TZXhAO5Ue%2Fu13xqtFRDMIxw%3D%3D" // 기상청에서 발급받은 API 키

// 데이터를 받아서 처리하는 콜백 함수
size_t write_callback(void *ptr, size_t size, size_t nmemb, char *data) {
    strcat(data, ptr);
    return size * nmemb;
}

// 현재 날짜와 시간을 구하는 함수
void get_current_date_time(char *base_date, char *base_time) {
    time_t now;
    struct tm *tm_info;
    char buffer[20];

    time(&now);
    tm_info = localtime(&now);

    // 날짜 (YYYYMMDD)
    strftime(buffer, sizeof(buffer), "%Y%m%d", tm_info);
    strcpy(base_date, buffer);

    // 시간 (HH00) - 기상청 데이터는 정각으로 제공
    strftime(buffer, sizeof(buffer), "%H00", tm_info);
    strcpy(base_time, buffer);
}

int main() {
    CURL *curl;
    CURLcode res;
    char url[1024];
    char response[10000] = "";

    char base_date[9];
    char base_time[5];

    // 현재 날짜와 시간 구하기
    get_current_date_time(base_date, base_time);

    // 기상청 API 요청 URL을 작성
    snprintf(url, sizeof(url),
             "%s?serviceKey=%s&numOfRows=10&pageNo=1&base_date=%s&base_time=%s&nx=66&ny=106", 
             API_URL, SERVICE_KEY, base_date, base_time);

    // curl 초기화
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // 요청 URL 설정
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        // 요청 실행
        res = curl_easy_perform(curl);

        // 요청이 성공했는지 확인
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // 응답을 출력
            printf("Response: \n%s\n", response);

            // JSON 응답 파싱
            struct json_object *parsed_json;
            struct json_object *response_body;
            struct json_object *body;
            struct json_object *items;

            parsed_json = json_tokener_parse(response);
            json_object_object_get_ex(parsed_json, "response", &response_body);
            json_object_object_get_ex(response_body, "body", &body);
            json_object_object_get_ex(body, "items", &items);

            // 각 아이템(날씨 데이터) 출력
            for (size_t i = 0; i < json_object_array_length(items); i++) {
                struct json_object *item = json_object_array_get_idx(items, i);
                struct json_object *fcstDate, *fcstTime, *category, *fcstValue;

                json_object_object_get_ex(item, "fcstDate", &fcstDate);
                json_object_object_get_ex(item, "fcstTime", &fcstTime);
                json_object_object_get_ex(item, "category", &category);
                json_object_object_get_ex(item, "fcstValue", &fcstValue);

                printf("Date: %s, Time: %s, Category: %s, Value: %s\n",
                       json_object_get_string(fcstDate),
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