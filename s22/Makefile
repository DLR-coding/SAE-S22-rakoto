CC=gcc
CFLAGS=-Wall -Wextra -g
LDFLAGS=

# List of source files
SRCS=graphe.c algos.c s22.c main2.c

# List of object files
OBJS=$(SRCS:.c=.o)

# Name of the executable
EXEC=main2

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
