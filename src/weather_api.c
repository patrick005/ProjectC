#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include <mysql/mysql.h>
#include <time.h>
#include <unistd.h>

#define API_KEY "bd051b188f6b1a86175dbb65aa1f5100"
#define CITY_ID "1846095"
#define DB_HOST "localhost"
#define DB_USER "myuser"
#define DB_PASS "0000"
#define DB_NAME "WeatherDB"

struct MemoryStruct {
    char *memory;
    size_t size;
};

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

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    MYSQL *conn;
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL 초기화 실패: %s\n", mysql_error(conn));
        return 1;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
        fprintf(stderr, "MySQL 연결 실패: %s\n", mysql_error(conn));
        return 1;
    }
  
    while(1) {
        if (curl) {



            
            free(chunk.memory);
            chunk.memory = malloc(1);
            chunk.size = 0;
            
            char api_url[256];
            sprintf(api_url, "http://api.openweathermap.org/data/2.5/weather?id=%s&appid=%s&lang=en", CITY_ID, API_KEY);

            curl_easy_setopt(curl, CURLOPT_URL, api_url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
            
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            } else {
                printf("API 응답: %s\n", chunk.memory);
                
                json_error_t error;
                json_t *root = json_loads(chunk.memory, 0, &error);
                if (!root) {
                    fprintf(stderr, "JSON 파싱 실패: %s\n", error.text);
                } else {
                    json_t *weather_array = json_object_get(root, "weather");
                    json_t *dt = json_object_get(root, "dt");
                    if (json_is_array(weather_array) && json_is_integer(dt)) {
                        
                        json_t *weather_obj = json_array_get(weather_array, 0);
                        json_t *id = json_object_get(weather_obj, "id");
                        
                        time_t timestamp = json_integer_value(dt);
                        struct tm *time_info = localtime(&timestamp);
                        char formatted_time[20];
                        strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", time_info);
                        





                        if (json_is_integer(id)) {
                            int weather_id = json_integer_value(id);
                            char weather_condition[20];
                            if (weather_id / 100 == 2) {
                                strcpy(weather_condition, "W1");
                            } else if (weather_id / 100 == 3 || weather_id / 100 == 5) {
                                strcpy(weather_condition, "W2");
                            } else if (weather_id / 100 == 6) {
                                strcpy(weather_condition, "W3");
                            } else if (weather_id == 800) {
                                strcpy(weather_condition, "W6");
                            } else {
                                strcpy(weather_condition, "W10");
                            }
                            char weather_id_str[20];
                            sprintf(weather_id_str, "%d", weather_id);
                            
                            if (insert_weather_data(conn, formatted_time, weather_id_str, weather_condition) != 0) {
                                printf("날씨 데이터 삽입 실패\n");
                            } else {
                                printf("날씨 데이터 삽입 성공\n");
                            }
                        }
                    }
                    json_decref(root);
                }
            }
        }
        sleep(20);
    }
    mysql_close(conn);
    free(chunk.memory);
    curl_global_cleanup();
    return 0;
}
