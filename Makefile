CC      = gcc
CFLAGS  = -Wall -Wextra -O2

all: log_receiver

log_receiver: log_receiver.c
	$(CC) $(CFLAGS) -o log_receiver log_receiver.c

clean:
	rm -f log_receiver
