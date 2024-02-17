# Makefile for "writer" application

# Compiler settings
ifeq ($(CROSS_COMPILE),)
	CC = gcc
else
	CC = $(CROSS_COMPILE)gcc
endif

CFLAGS = -Wall -Wextra -std=c99

# Source files
SRCS := writer.c

# Object files
OBJS := $(SRCS:.c=.o)

# Rules
%.o : %.c
	$(CC) -c $< $(CFLAGS) -o $@ 

writer : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Clean target
.PHONY: clean
clean:
	rm -f writer $(OBJS)
