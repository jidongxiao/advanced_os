CC = gcc
CFLAGS = -O2 -march=native -Wall -Wextra
LDFLAGS = -lrt

TARGETS = victim attacker

all: $(TARGETS)

victim: victim.c shared.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

attacker: attacker.c shared.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
