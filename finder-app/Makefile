# Author: NguyenChien215
# Makefile for "writer" application
CFLAGS = -Wall -Wextra -std=c99
TARGET = writer

# Compiler settings
ifeq ($(CROSS_COMPILE),)
	CC = gcc
else
	CC = $(CROSS_COMPILE)gcc
endif

# Source files
SRCS := writer.c

# Object files
OBJS := $(SRCS:.c=.o)

# Default target
.PHONY: all
all: $(TARGET)

#Rules
%.o : %.c
	$(CC) -c $< $(CFLAGS) -o $@ 

writer : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)
	


#Clean target
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)


