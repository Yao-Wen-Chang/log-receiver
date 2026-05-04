CC      = gcc

all: log_receiver

log_receiver: log_receiver.c
	$(CC) -o log_receiver log_receiver.c

clean:
	rm -f log_receiver

clean-logs:
	rm -f *.log

clean-all:
	rm -f *.log log_receiver
