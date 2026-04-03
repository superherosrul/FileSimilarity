CC = gcc
CFLAGS = -Wall -Werror -g -std=c99 -D_DEFAULT_SOURCE -fsanitize=address,undefined
LDFLAGS = -lm -fsanitize=address,undefined

TARGET = compare
SRCS = compare.c
OBJS = $(SRCS:.c=.o)
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all clean