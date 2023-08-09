#EXECBIN = httpserver
#SOURCES = $(wildcard *.c)
#OBJECTS = $(SOURCES:%.c=%.o)
#FORMATS = $(SOURCES:%.c=%.fmt)
#
#CC = clang
#FORMAT = clang-format
#CFLAGS = -Wall -Wpedantic -Werror -Wextra
#
#.PHONY: all clean format
#
#all: $(EXECBIN)
#
#$(EXECBIN): $(OBJECTS)
#	$(CC) -o $@ $^
#
#%.o : %.c
#	$(CC) $(CFLAGS) -c $<
#
#clean:
#	rm -f $(EXECBIN) $
#
#format: $(FORMATS)
#
#%.fmt: %.c
#	$(FORMAT) -i $<
#	touch $@
#
#####
CC = clang
CFLAGS = -Wall -Wpedantic -Werror -Wextra

.PHONY: all format clean

all: httpserver

httpserver: httpserver.o asgn2_helper_funcs.a 
	$(CC) $(CFLAGS) -g -o httpserver httpserver.o asgn2_helper_funcs.a

#httpserver.o: httpserver.c asgn2_helper_funcs.h
#	$(CC) $(CFLAGS) -c httpserver.c
#
#asgn2_helper_funcs.o: asgn2_helper_funcs.a asgn2_helper_funcs.h
#	$(CC) $(CFLAGS) -c asgn2_helper_funcs.a

clean:
	rm -f httpserver *.o 

format:
	clang-format -i -style=file *.[ch]
