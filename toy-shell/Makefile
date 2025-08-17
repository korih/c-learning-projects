CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = toy-shell

all: $(TARGET)

$(TARGET): shell.c
	$(CC) $(CFLAGS) -o $(TARGET) shell.c

clean:
	rm -f $(TARGET)