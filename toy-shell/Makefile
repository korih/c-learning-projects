CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC = src/main.c src/shell.c src/buildins.c src/parser.c
TARGET = toy-shell

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)