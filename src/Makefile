CC='/usr/bin/gcc'
GENERAL_LIBRARY = library
CFLAGS=-Wextra -Wall -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code -c
MAIN_PROGRAM = daemon
MONITOR_PROGRAM = monitor
FILE_TRANSFER_PROGRAM = file_transfer

all: build

build: $(MAIN_PROGRAM) $(MONITOR_PROGRAM) $(FILE_TRANSFER_PROGRAM)
	@kill $(pgrep $(MAIN_PROGRAM)) 2> /dev/null || true
	$(CC) -g -c $(GENERAL_LIBRARY).c -o $(GENERAL_LIBRARY).o $(CFLAGS)
	$(CC) -g -c $(MAIN_PROGRAM).c -o $(MAIN_PROGRAM).o $(CFLAGS)
	# add new .h file into the following line
	$(CC) $(GENERAL_LIBRARY).o $(MAIN_PROGRAM).o -o $(MAIN_PROGRAM)

	$(CC) -g -c $(GENERAL_LIBRARY).c -o $(GENERAL_LIBRARY).o $(CFLAGS)
	$(CC) -g -c $(MONITOR_PROGRAM).c -o $(MONITOR_PROGRAM).o $(CFLAGS)
	$(CC) $(GENERAL_LIBRARY).o $(MONITOR_PROGRAM).o -o $(MONITOR_PROGRAM)

	$(CC) -g -c $(GENERAL_LIBRARY).c -o $(GENERAL_LIBRARY).o $(CFLAGS)
	$(CC) -g -c $(FILE_TRANSFER_PROGRAM).c -o $(FILE_TRANSFER_PROGRAM).o $(CFLAGS)
	$(CC) $(GENERAL_LIBRARY).o $(FILE_TRANSFER_PROGRAM).o -o $(FILE_TRANSFER_PROGRAM)

clean:
	rm $(FILE_TRANSFER_PROGRAM).o $(MAIN_PROGRAM).o $(MONITOR_PROGRAM).o $(GENERAL_LIBRARY).o $(MONITOR_PROGRAM) $(MAIN_PROGRAM) $(BACKUP_PROGRAM) $(FILE_TRANSFER_PROGRAM)