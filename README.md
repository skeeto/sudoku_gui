# Sudoku
This is a basic beginner Sudoku project made with the C language using Windows API.
It has a GUI and you can play Sudoku puzzles with custom difficulty.

# Prerequisites
- GCC compiler 
- Windows operating system

# How to run 
```
make
```
If you don't have make installed run directly from `gui_sudoku/`
```
gcc -I include src/main.c src/sudoku.c src/sudoku_gui.c -o main.exe -mwindows
./main.exe
```

# Screenshots

![Screenshots](gui_sudoku/images/sudoku_levels.png)

![Screenshots](gui_sudoku/images/sudoku_game.png)
