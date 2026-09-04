/* Unit tests for the pure Sudoku model (src/sudoku.c). Built as a console
 * program by `make test`; exits non-zero if any check fails. */
#include <stdio.h>
#include <string.h>

#include "sudoku.h"

static int checks = 0;
static int failures = 0;

static void check(int cond, const char *msg)
{
    checks++;
    if (!cond)
    {
        failures++;
        printf("FAIL: %s\n", msg);
    }
}

static void clearBoard(int g[SUDOKU_DIM][SUDOKU_DIM])
{
    memset(g, 0, sizeof(int[SUDOKU_DIM][SUDOKU_DIM]));
}

static int countNonZero(const int g[SUDOKU_DIM][SUDOKU_DIM])
{
    int n = 0;
    for (int r = 0; r < SUDOKU_DIM; r++)
        for (int c = 0; c < SUDOKU_DIM; c++)
            if (g[r][c])
                n++;
    return n;
}

/* A fully filled grid with no rule violations is a valid solution. */
static int isFullValid(const int g[SUDOKU_DIM][SUDOKU_DIM])
{
    for (int r = 0; r < SUDOKU_DIM; r++)
        for (int c = 0; c < SUDOKU_DIM; c++)
            if (g[r][c] == 0)
                return 0;
    return !sudoku_has_conflict(g);
}

/* Find a "deadly pattern" rectangle (a b / b a) spanning at most two 3x3
 * boxes, blank its four cells, and return 1. Such a board has exactly two
 * solutions, which lets us test exact solution counting. */
static int blankDeadlyPattern(int g[SUDOKU_DIM][SUDOKU_DIM])
{
    for (int r1 = 0; r1 < SUDOKU_DIM; r1++)
        for (int c1 = 0; c1 < SUDOKU_DIM; c1++)
            for (int r2 = r1 + 1; r2 < SUDOKU_DIM; r2++)
                for (int c2 = c1 + 1; c2 < SUDOKU_DIM; c2++)
                {
                    if (g[r1][c1] == g[r2][c2] && g[r1][c2] == g[r2][c1] &&
                        g[r1][c1] != g[r1][c2])
                    {
                        int sameBoxRow = (r1 / 3 == r2 / 3);
                        int sameBoxCol = (c1 / 3 == c2 / 3);
                        if (sameBoxRow || sameBoxCol)
                        {
                            g[r1][c1] = g[r1][c2] = g[r2][c1] = g[r2][c2] = 0;
                            return 1;
                        }
                    }
                }
    return 0;
}

int main(void)
{
    int g[SUDOKU_DIM][SUDOKU_DIM];

    /* --- sudoku_fill_random: complete, rule-consistent grids --- */
    for (int i = 0; i < 20; i++)
    {
        clearBoard(g);
        check(sudoku_fill_random(g), "fill_random returns true");
        check(isFullValid(g), "fill_random yields a valid full grid");
        check(sudoku_count_solutions(g, 2) == 1,
              "a full valid grid has exactly 1 solution");
    }

    /* --- sudoku_count_solutions --- */
    clearBoard(g);
    check(sudoku_count_solutions(g, 2) == 2,
          "empty board has >=2 solutions (capped at 2)");

    clearBoard(g); /* row 0 = 1..8,0 ; (1,8)=9 -> cell (0,8) has no candidate */
    for (int c = 0; c < 8; c++)
        g[0][c] = c + 1;
    g[1][8] = 9;
    check(sudoku_count_solutions(g, 100) == 0,
          "contradicted board has 0 solutions");

    int deadlyFound = 0;
    for (int attempt = 0; attempt < 50 && !deadlyFound; attempt++)
    {
        clearBoard(g);
        sudoku_fill_random(g);
        deadlyFound = blankDeadlyPattern(g);
    }
    check(deadlyFound, "found a deadly pattern in a random grid");
    if (deadlyFound)
    {
        check(sudoku_count_solutions(g, 100) == 2,
              "deadly-pattern board has exactly 2 solutions");
    }

    /* --- sudoku_generate: unique, consistent, sane clue counts --- */
    double avg[4] = {0, 0, 0, 0};
    int runs = 20;
    for (int level = 1; level <= 3; level++)
    {
        int total = 0;
        for (int i = 0; i < runs; i++)
        {
            int puzzle[SUDOKU_DIM][SUDOKU_DIM];
            int sol[SUDOKU_DIM][SUDOKU_DIM];
            sudoku_generate(level, puzzle, sol);

            check(sudoku_count_solutions(puzzle, 2) == 1,
                  "generated puzzle has a unique solution");
            check(isFullValid(sol), "generated solution is a valid full grid");

            int consistent = 1;
            for (int r = 0; r < SUDOKU_DIM; r++)
                for (int c = 0; c < SUDOKU_DIM; c++)
                    if (puzzle[r][c] != 0 && puzzle[r][c] != sol[r][c])
                        consistent = 0;
            check(consistent, "puzzle givens match the solution");

            int givens = countNonZero(puzzle);
            check(givens >= 17 && givens <= 80, "puzzle clue count in [17,80]");
            total += givens;
        }
        avg[level] = (double)total / runs;
        printf("level %d: avg clues = %.1f\n", level, avg[level]);
    }
    check(avg[1] > avg[3], "easy has more clues than hard on average");

    /* --- sudoku_is_valid --- */
    clearBoard(g);
    check(sudoku_is_valid(g, 0, 0, 5), "empty board: 5 at (0,0) is valid");
    g[0][1] = 5;
    check(!sudoku_is_valid(g, 0, 0, 5), "row duplicate blocks 5 at (0,0)");
    clearBoard(g);
    g[1][0] = 5;
    check(!sudoku_is_valid(g, 0, 0, 5), "column duplicate blocks 5 at (0,0)");
    clearBoard(g);
    g[1][1] = 5;
    check(!sudoku_is_valid(g, 0, 0, 5), "box duplicate blocks 5 at (0,0)");
    clearBoard(g);
    g[0][0] = 7; /* the cell's own value is ignored */
    g[0][1] = 5;
    check(!sudoku_is_valid(g, 0, 0, 5),
          "cell's own value ignored; row 5 still blocks");
    clearBoard(g);
    g[2][2] = 9;
    check(!sudoku_is_valid(g, 0, 0, 9), "same-box 9 blocks 9 at (0,0)");

    /* --- sudoku_has_conflict --- */
    clearBoard(g);
    check(!sudoku_has_conflict(g), "empty board has no conflict");
    g[0][0] = 5;
    g[0][1] = 5;
    check(sudoku_has_conflict(g), "two 5s in a row conflict");
    clearBoard(g);
    g[0][0] = 5;
    g[1][0] = 5;
    check(sudoku_has_conflict(g), "two 5s in a column conflict");
    clearBoard(g);
    g[0][0] = 5;
    g[1][1] = 5;
    check(sudoku_has_conflict(g), "two 5s in a box conflict");
    clearBoard(g);
    sudoku_fill_random(g);
    check(!sudoku_has_conflict(g), "a valid full grid has no conflict");

    /* --- sudoku_cell_conflict --- */
    clearBoard(g);
    g[0][0] = 5;
    g[0][1] = 5;
    check(sudoku_cell_conflict(g, 0, 0), "(0,0) conflicts (row duplicate)");
    check(sudoku_cell_conflict(g, 0, 1), "(0,1) conflicts (row duplicate)");
    clearBoard(g);
    g[0][0] = 5;
    check(!sudoku_cell_conflict(g, 0, 0), "a single 5 does not conflict");
    check(!sudoku_cell_conflict(g, 0, 1), "an empty cell never conflicts");

    printf("\n%d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
