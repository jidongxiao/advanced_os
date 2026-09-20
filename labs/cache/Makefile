CC = gcc
CFLAGS = -O2 -march=native -Wall -Wextra
LDFLAGS = -lrt

TARGETS = attacker victim

all: $(TARGETS)

attacker: attacker.c
	$(CC) $(CFLAGS) $< -o $@

victim: victim.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
