CC = gcc
CFLAGS = -Wall -Wextra

TARGET = program
LIB = libmathutils.a
LIB_OBJ = math_utils.o

all: $(TARGET)

$(LIB): $(LIB_OBJ)
	ar rcs $(LIB) $(LIB_OBJ)

$(LIB_OBJ): math_utils.c math_utils.h
	$(CC) $(CFLAGS) -c math_utils.c -o math_utils.o

main.o: main.c math_utils.h
	$(CC) $(CFLAGS) -c main.c -o main.o

$(TARGET): main.o $(LIB)
	$(CC) main.o ./$(LIB) -o $(TARGET)

clean:
	rm -f $(TARGET) main.o $(LIB) $(LIB_OBJ)
