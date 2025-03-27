#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include <mysql/mysql.h>
#include <time.h> // 시간을 처리하기 위한 헤더

#define API_KEY "bd051b188f6b1a86175dbb65aa1f5100" // OpenWeatherMap API 키를 입력하세요.
#define CITY_ID "1846095" // 세종시 ID (OpenWeatherMap에서 확인 가능)
#define DB_HOST "localhost"
#define DB_USER "myuser"
#define DB_PASS "0000"
#define DB_NAME "WeatherDB"

// API 응답 데이터를 저장할 구조체
struct MemoryStruct {
    char *memory;
    size_t size;
};

// API 응답 데이터를 메모리에 저장하는 콜백 함수
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        printf("메모리 할당 실패!\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// MySQL에 날씨 데이터를 삽입하는 함수
int insert_weather_data(MYSQL *conn, const char *tm, const char *stnId, const char *wt) {
    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO weatherData6 (tm, stnId, wt) VALUES ('%s', '%s', '%s')",
        tm, stnId, wt);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "INSERT 실패: %s\n", mysql_error(conn));
        return 1;
    }
    return 0;
}

int main() {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    // libcurl 초기화
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    // MySQL 초기화
    MYSQL *conn;
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL 초기화 실패: %s\n", mysql_error(conn));
        return 1;
    }

    // MySQL 연결
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "MySQL 연결 실패: %s\n", mysql_error(conn));
        return 1;
    }

    if (curl) {
        // API 요청 URL 생성
        char api_url[256];
        sprintf(api_url, "http://api.openweathermap.org/data/2.5/weather?id=%s&appid=%s&lang=en", CITY_ID, API_KEY);

        curl_easy_setopt(curl, CURLOPT_URL, api_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // API 요청 실행
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // JSON 파싱
            json_error_t error;
            json_t *root = json_loads(chunk.memory, 0, &error);

            if (!root) {
                fprintf(stderr, "JSON 파싱 실패: %s\n", error.text);
            } else {
                // 날씨 데이터 추출
                json_t *weather_array = json_object_get(root, "weather");
                json_t *dt = json_object_get(root, "dt"); // UNIX 타임스탬프 추출
                if (json_is_array(weather_array) && json_is_integer(dt)) {
                    size_t array_size = json_array_size(weather_array);
                    for (size_t i = 0; i < array_size; i++) {
                        json_t *weather_obj = json_array_get(weather_array, i);
                        if (json_is_object(weather_obj)) {
                            // 날씨 ID, 주요 정보, 설명 추출
                            json_t *id = json_object_get(weather_obj, "id");
                      
                            // UNIX 타임스탬프 변환
                            time_t timestamp = json_integer_value(dt);
                            struct tm *time_info = localtime(&timestamp); // UTC 시간으로 변환
                            char formatted_time[20]; // 시간 문자열 저장
                      
                            strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", time_info);
                     
                            // 날씨 상태 출력
                            if (json_is_integer(id)) {
                                int weather_id = json_integer_value(id);  // 날씨 ID를 정수로 저장

                                // 날씨 ID로 조건문 처리           

                                // 날씨 정보를 MySQL에 삽입하는 부분 수정
                                    char weather_condition[20];  // 날씨 상태를 저장할 변수

                                    if (weather_id / 100 == 2) {
                                        strcpy(weather_condition, "뇌우");
                                    } else if (weather_id / 100 == 3 || weather_id / 100 == 5) {
                                        strcpy(weather_condition, "비");
                                    } else if (weather_id / 100 == 6) {
                                        strcpy(weather_condition, "눈");
                                    } else if (weather_id == 701 || weather_id == 711 || weather_id == 721 || weather_id == 741) {
                                        strcpy(weather_condition, "안개");
                                    } else if (weather_id == 731 || weather_id == 751) {
                                        strcpy(weather_condition, "황사");
                                    } else if (weather_id == 800) {
                                        strcpy(weather_condition, "맑음");
                                    } else if (weather_id == 801) {
                                        strcpy(weather_condition, "약간 흐림");
                                    } else if (weather_id == 802 || weather_id == 803) {
                                        strcpy(weather_condition, "흐림");
                                    } else if (weather_id == 804) {
                                        strcpy(weather_condition, "많이 흐림");
                                    }

                                    // 날씨 정보 출력
                                    printf("%d\n", weather_id);
                                    printf("%s\n", formatted_time);
                                    printf("날씨: %s\n", weather_condition);


                                // 날씨 정보를 MySQL에 삽입
                                if (insert_weather_data(conn, formatted_time, "weather_id", weather_condition) != 0) {
                                    printf("날씨 데이터 삽입 실패\n");
                                } else {
                                    printf("날씨 데이터 삽입 성공\n");
                                }
                            }
                        }
                    }
                }
                json_decref(root);
            }
        }
        curl_easy_cleanup(curl);
    }

    // MySQL 연결 종료
    mysql_close(conn);
    free(chunk.memory);
    curl_global_cleanup();
    return 0;
}
