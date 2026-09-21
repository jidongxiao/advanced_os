CC = gcc
CFLAGS = -Wall -Wextra
PICFLAGS = -fPIC

TARGET = program
LIB = libmathutils.so

all: $(TARGET)

math_utils.o: math_utils.c math_utils.h
	$(CC) $(CFLAGS) $(PICFLAGS) -c math_utils.c -o math_utils.o

$(LIB): math_utils.o
	$(CC) -shared math_utils.o -o $(LIB)

main.o: main.c math_utils.h
	$(CC) $(CFLAGS) -c main.c -o main.o

$(TARGET): main.o $(LIB)
	$(CC) main.o ./$(LIB) -o $(TARGET)

run: $(TARGET)
	LD_LIBRARY_PATH=. ./$(TARGET)

clean:
	rm -f $(TARGET) main.o math_utils.o $(LIB)
