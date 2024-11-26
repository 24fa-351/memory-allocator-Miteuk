CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = test

default: $(TARGET)
$(TARGET): test_malloc.c mymalloc.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)