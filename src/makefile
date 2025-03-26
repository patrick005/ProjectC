CC = gcc
CFLAGS = -Wall -g

# 사용되는 라이브러리 설정
LIBS = -lcurl -ljson-c

# 실행 파일 이름
TARGET = weather_api

# 소스 파일
SRC = weather_api.c

# 오브젝트 파일
OBJ = $(SRC:.c=.o)

# 기본 목표
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

# .o 파일 생성 규칙
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 실행 파일 삭제 규칙
clean:
	rm -f $(TARGET) $(OBJ)