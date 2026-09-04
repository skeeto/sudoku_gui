# Win32 Sudoku GUI
CC      := gcc
WINDRES := windres
CFLAGS  := -Wall -Wextra -std=c99 -I include
LDFLAGS := -mwindows
OBJDIR  := build
TARGET  := main.exe

SRCS := src/main.c src/sudoku.c src/sudoku_gui.c
HDRS := include/sudoku.h include/sudoku_gui.h include/config.h include/resource.h
RES  := $(OBJDIR)/sudoku_res.o
OBJS := $(OBJDIR)/main.o $(OBJDIR)/sudoku.o $(OBJDIR)/sudoku_gui.o $(RES)

TEST_SRC := test/test_sudoku.c
TEST_BIN := $(OBJDIR)/test_sudoku.exe

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c $(HDRS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile the .rc (embedded icon + DPI manifest) into a COFF object via windres.
$(OBJDIR)/sudoku_res.o: sudoku.rc include/resource.h images/sudoku_icon.ico app.manifest | $(OBJDIR)
	$(WINDRES) -I include $< -o $@

# Model unit tests (console build, no -mwindows).
$(TEST_BIN): $(TEST_SRC) src/sudoku.c include/sudoku.h | $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) src/sudoku.c

test: $(TEST_BIN)
	./$(TEST_BIN)

run: all
	./$(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(TARGET)
	rm -rf $(OBJDIR)
