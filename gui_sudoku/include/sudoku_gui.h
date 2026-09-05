#ifndef SUDOKU_GUI_H
#define SUDOKU_GUI_H

#include <windows.h>
#include "sudoku.h"
#include "config.h"

/* Menu command IDs */
#define IDM_NEW_EASY   101
#define IDM_NEW_MEDIUM 102
#define IDM_NEW_HARD   103
#define IDM_EXIT       104

/* Timer id that drives the once-per-second clock repaint. */
#define IDT_CLOCK      1

/* All game/model state lives here. The puzzle (givens) is fixed; the user
 * fills the rest. `solution` is kept only as a reference and is never used
 * to judge input. */
struct Game
{
    int puzzle[SUDOKU_DIM][SUDOKU_DIM];
    int user[SUDOKU_DIM][SUDOKU_DIM];
    int solution[SUDOKU_DIM][SUDOKU_DIM];
    int level;
    long score;
    bool solved;
    int selRow; /* -1 == nothing selected */
    int selCol;
    long start_time; /* epoch seconds when play began; 0 == none */
    long solved_time; /* epoch seconds when solved; 0 == unsolved */
};

/* Presentation-only state. */
struct Gui
{
    int cell;
    int gridX0;
    int gridY0;
    double scale; /* device-pixels-per-96dpi, from system DPI */
    HFONT fontCell;
    HFONT fontStatus;
    HFONT fontWin; /* large heading used by the "Solved!" overlay */
    HMENU menu;
};

/* Bundled so it can be handed to CreateWindowEx / retrieved in WndProc. */
struct PointerStruct
{
    struct Game *s1;
    struct Gui *g1;
};

void setupMenu(HWND hwnd, struct PointerStruct *p1);
void createFonts(struct Gui *g1);
void deleteFonts(struct Gui *g1);
void newGame(struct Game *s1, struct Gui *g1, int level);
void drawBoard(HDC hdc, struct Game *s1, struct Gui *g1);
void drawStatus(HDC hdc, struct Game *s1, struct Gui *g1);
void drawSolvedOverlay(HDC hdc, struct Game *s1, struct Gui *g1);
void onGridClick(HWND hwnd, struct Game *s1, struct Gui *g1, LPARAM lp);
void onKeyPress(HWND hwnd, struct Game *s1, struct Gui *g1, WPARAM key);

void saveGame(const struct Game *s1);
bool loadGame(struct Game *s1);

#endif
