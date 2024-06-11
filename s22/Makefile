# Makefile for compiling and linking main, main2, and main3

CC = gcc
CFLAGS = -Wall -Wextra -Werror
OBJDIR = obj

# Source files
SRCS = main.c main2.c main3.c graphe.c algos.c utils.c
# Object files
OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

# Executables
EXECS = main main2 main3

# Default target
all: $(EXECS)

# Rule for creating object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Rules for creating object files
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rules for linking executables
main: $(OBJDIR)/main.o $(OBJDIR)/graphe.o $(OBJDIR)/algos.o $(OBJDIR)/utils.o
	$(CC) $(CFLAGS) $^ -o $@

main2: $(OBJDIR)/main2.o $(OBJDIR)/graphe.o $(OBJDIR)/algos.o $(OBJDIR)/utils.o
	$(CC) $(CFLAGS) $^ -o $@

main3: $(OBJDIR)/main3.o $(OBJDIR)/graphe.o $(OBJDIR)/algos.o $(OBJDIR)/utils.o
	$(CC) $(CFLAGS) $^ -o $@

# Clean rule
clean:
	rm -rf $(OBJDIR) $(EXECS)

.PHONY: all clean
