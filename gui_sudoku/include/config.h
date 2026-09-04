#ifndef CONFIG_H
#define CONFIG_H

#include <windows.h>

/* Window / layout */
#define WIN_W 500
#define WIN_H 560
#define CELL_SIZE 44
#define GRID_X0 50
#define GRID_Y0 65

/* Fonts */
#define TITLE_FONT "Segoe UI"
#define CELL_FONT "Segoe UI"

/* Level labels, indexed 1..LEVEL_COUNT */
#define LEVEL_LABELS { "Easy", "Medium", "Hard" }
#define LEVEL_COUNT 3

/* Save file (auto-resume) */
#define saveFilePath "sudoku.save"

/* Palette */
#define COLOR_BG         RGB(145, 171, 199)
#define COLOR_WHITE      RGB(255, 255, 255)
#define COLOR_BLACK      RGB(0, 0, 0)
#define COLOR_GIVEN      RGB(20, 20, 20)
#define COLOR_USER       RGB(40, 90, 180)
#define COLOR_CONFLICT   RGB(200, 30, 30)
#define COLOR_PENCIL     RGB(150, 170, 200)
#define COLOR_SEL_FILL   RGB(226, 236, 248)
#define COLOR_SEL_BORDER RGB(70, 130, 200)
#define COLOR_LINE_THIN  RGB(180, 180, 180)
#define COLOR_LINE_THICK RGB(80, 80, 80)
#define COLOR_TITLE      RGB(255, 255, 255)
#define COLOR_STATUS     RGB(255, 255, 255)
#define COLOR_SOLVED     RGB(225, 248, 228)

#endif
