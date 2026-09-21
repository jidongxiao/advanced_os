CC = gcc
CFLAGS = -Wall -Wextra

TARGET = program
SOURCES = main.c math_utils.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
