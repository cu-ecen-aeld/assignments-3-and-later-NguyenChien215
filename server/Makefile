# Edited by: NguyenChien215
# Date: 2nd, March, 2023

CC = $(CROSS_COMPILE)gcc
TARGET = aesdsocket
CFLAGS  = -g -Wall -Werror

# Make all rule
default: all
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean
clean:
	rm -f $(TARGET)
