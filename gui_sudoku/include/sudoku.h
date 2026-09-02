#ifndef SUDOKU_H
#define SUDOKU_H

#include <stdbool.h>

/* Side length of the board. Kept here (not in a Windows header) so the
 * model stays independent of the GUI. */
#define SUDOKU_DIM 9

/* Fill an empty (or partially filled) valid board with a complete,
 * rule-consistent solution using randomized backtracking.
 * Returns true on success. */
bool sudoku_fill_random(int grid[SUDOKU_DIM][SUDOKU_DIM]);

/* Count solutions of the given board, stopping early once `limit` is
 * reached. Returns min(actual_solution_count, limit). Passing limit == 2
 * is enough to tell "unique" apart from "ambiguous". */
int sudoku_count_solutions(const int grid[SUDOKU_DIM][SUDOKU_DIM], int limit);

/* Generate a puzzle with a guaranteed unique solution.
 *
 *   solution : receives the fully solved grid (reference only)
 *   puzzle   : receives the presented puzzle (givens, 0 == empty)
 *   level    : 1 = easy, 2 = medium, 3 = hard
 *
 * Cells are removed one at a time; any removal that would allow more than
 * one solution is immediately rolled back. */
void sudoku_generate(int level,
                     int puzzle[SUDOKU_DIM][SUDOKU_DIM],
                     int solution[SUDOKU_DIM][SUDOKU_DIM]);

/* Would placing `d` at (row, col) break a row / column / 3x3 box rule?
 * The current value in that cell is ignored. */
bool sudoku_is_valid(const int grid[SUDOKU_DIM][SUDOKU_DIM],
                     int row, int col, int d);

/* True if any filled cell duplicates another in its row, column or box. */
bool sudoku_has_conflict(const int grid[SUDOKU_DIM][SUDOKU_DIM]);

/* True if the digit at (row, col) duplicates another in its row, column
 * or box. Empty cells never conflict. */
bool sudoku_cell_conflict(const int grid[SUDOKU_DIM][SUDOKU_DIM],
                          int row, int col);

#endif
