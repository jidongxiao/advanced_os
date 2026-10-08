CC = gcc
CFLAGS = -O2 -Wall -Wextra

USER_PROGRAMS = cache_time flush_reload page_cache_mincore \
                sender receiver victim_secret monitor_secret

CHANNEL_FILES = bit0.bin bit1.bin bit2.bin bit3.bin \
                bit4.bin bit5.bin bit6.bin bit7.bin

PAGE_CACHE_FILE = lookup_table.bin

all: $(USER_PROGRAMS) channel_files page_cache_file


cache_time: cache_time.c
	$(CC) $(CFLAGS) -o $@ $<

flush_reload: flush_reload.c
	$(CC) $(CFLAGS) -o $@ $<

page_cache_mincore: page_cache_mincore.c
	$(CC) $(CFLAGS) -o $@ $<

sender: sender.c
	$(CC) $(CFLAGS) -o $@ $<

receiver: receiver.c
	$(CC) $(CFLAGS) -o $@ $<

victim_secret: victim_secret.c
	$(CC) $(CFLAGS) -o $@ $<

monitor_secret: monitor_secret.c
	$(CC) $(CFLAGS) -o $@ $<


channel_files:
	for i in 0 1 2 3 4 5 6 7; do \
		dd if=/dev/zero of=bit$$i.bin bs=4096 count=1 status=none; \
	done


page_cache_file:
	dd if=/dev/zero of=$(PAGE_CACHE_FILE) bs=4096 count=2049 status=none


clean:
	rm -f $(USER_PROGRAMS) $(CHANNEL_FILES) $(PAGE_CACHE_FILE)


.PHONY: all clean channel_files page_cache_file
