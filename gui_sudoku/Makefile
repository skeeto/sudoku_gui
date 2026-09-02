# Win32 Sudoku GUI
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99 -I include
LDFLAGS := -mwindows
OBJDIR  := build
TARGET  := main.exe

SRCS := src/main.c src/sudoku.c src/sudoku_gui.c
HDRS := include/sudoku.h include/sudoku_gui.h include/config.h
OBJS := $(OBJDIR)/main.o $(OBJDIR)/sudoku.o $(OBJDIR)/sudoku_gui.o

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c $(HDRS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf $(OBJDIR)
