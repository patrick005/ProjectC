# 변수 설정
CC = gcc
CFLAGS = -Wall -g
LIBS = -lcurl -ljansson -lmysqlclient

# 최종 타겟 실행 파일
TARGET = weather_program

# 소스 파일
SRC = main.c

# 오브젝트 파일
OBJ = main.o

# Makefile 규칙
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)



# clean 규칙: 빌드된 파일을 삭제
clean:
	rm -f $(TARGET) $(OBJ)

# 디버그 모드로 컴파일하려면 `make debug`를 실행
debug:
	$(CC) $(CFLAGS) -DDEBUG -o $(TARGET) $(SRC) $(LIBS)