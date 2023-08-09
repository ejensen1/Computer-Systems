##CC = clang
##CFLAGS = -Wall -Wpedantic -Werror -Wextra
##
##.PHONY: all format clean
##
##all: queue_test
##
##queue_test: queue_test.o queue.o
##	$(CC) $(CFLAGS) -g -o queue_test queue_test.o queue.o
##
##queue_test.o: queue_test.c queue.h
##	$(CC) $(CFLAGS) -c queue.c queue_test.c
##
##    
##queue: queue.o
##	$(CC) $(CFLAGS) -c queue.c
##
##
##clean:
##	rm -f queue_test *.o
##
##format:
##	clang-format -i -style=file *.[ch]

###


CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra

.PHONY: all format clean

all: queue

#queue_test: queue_test.o queue.o
#	$(CC) $(CFLAGS) -g -o queue_test queue_test.o queue.o

#queue_test.o: queue_test.c queue.h
#	$(CC) $(CFLAGS) -c queue.c queue_test.c

    
queue: queue.o
	$(CC) $(CFLAGS) -c queue.c


clean:
	rm -f *.o

format:
	clang-format -i -style=file *.[ch]
