#include "sudoku.h"

#include <stdlib.h>
#include <time.h>

/* Return true if `d` already appears in the row, column or 3x3 box that
 * contains (row, col). The target cell itself is skipped. */
static bool occupies(const int grid[SUDOKU_DIM][SUDOKU_DIM],
                     int row, int col, int d)
{
    for (int i = 0; i < SUDOKU_DIM; i++)
    {
        if (i != col && grid[row][i] == d)
        {
            return true;
        }
        if (i != row && grid[i][col] == d)
        {
            return true;
        }
    }

    int br = (row / 3) * 3;
    int bc = (col / 3) * 3;
    for (int r = br; r < br + 3; r++)
    {
        for (int c = bc; c < bc + 3; c++)
        {
            if ((r != row || c != col) && grid[r][c] == d)
            {
                return true;
            }
        }
    }
    return false;
}

bool sudoku_is_valid(const int grid[SUDOKU_DIM][SUDOKU_DIM],
                     int row, int col, int d)
{
    return d >= 1 && d <= SUDOKU_DIM && !occupies(grid, row, col, d);
}

bool sudoku_has_conflict(const int grid[SUDOKU_DIM][SUDOKU_DIM])
{
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            if (grid[r][c] != 0 && sudoku_cell_conflict(grid, r, c))
            {
                return true;
            }
        }
    }
    return false;
}

bool sudoku_cell_conflict(const int grid[SUDOKU_DIM][SUDOKU_DIM],
                          int row, int col)
{
    int d = grid[row][col];
    if (d == 0)
    {
        return false;
    }
    return occupies(grid, row, col, d);
}

/* Fisher-Yates shuffle of a 1..9 digit array. */
static void shuffle_digits(int nums[SUDOKU_DIM])
{
    for (int i = SUDOKU_DIM - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int t = nums[i];
        nums[i] = nums[j];
        nums[j] = t;
    }
}

static bool fill_from(int grid[SUDOKU_DIM][SUDOKU_DIM], int pos)
{
    if (pos == SUDOKU_DIM * SUDOKU_DIM)
    {
        return true;
    }
    int row = pos / SUDOKU_DIM;
    int col = pos % SUDOKU_DIM;

    int nums[SUDOKU_DIM];
    for (int i = 0; i < SUDOKU_DIM; i++)
    {
        nums[i] = i + 1;
    }
    shuffle_digits(nums);

    for (int i = 0; i < SUDOKU_DIM; i++)
    {
        int d = nums[i];
        if (occupies(grid, row, col, d))
        {
            continue;
        }
        grid[row][col] = d;
        if (fill_from(grid, pos + 1))
        {
            return true;
        }
        grid[row][col] = 0;
    }
    return false;
}

bool sudoku_fill_random(int grid[SUDOKU_DIM][SUDOKU_DIM])
{
    return fill_from(grid, 0);
}

static int count_from(int grid[SUDOKU_DIM][SUDOKU_DIM], int pos, int limit)
{
    while (pos < SUDOKU_DIM * SUDOKU_DIM && grid[pos / SUDOKU_DIM][pos % SUDOKU_DIM] != 0)
    {
        pos++;
    }
    if (pos == SUDOKU_DIM * SUDOKU_DIM)
    {
        return 1; /* a complete board is one solution */
    }

    int row = pos / SUDOKU_DIM;
    int col = pos % SUDOKU_DIM;
    int total = 0;

    for (int d = 1; d <= SUDOKU_DIM && total < limit; d++)
    {
        if (occupies(grid, row, col, d))
        {
            continue;
        }
        grid[row][col] = d;
        total += count_from(grid, pos + 1, limit - total);
        grid[row][col] = 0;
    }
    return total;
}

int sudoku_count_solutions(const int grid[SUDOKU_DIM][SUDOKU_DIM], int limit)
{
    if (limit <= 0)
    {
        return 0;
    }
    int work[SUDOKU_DIM][SUDOKU_DIM];
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            work[r][c] = grid[r][c];
        }
    }
    return count_from(work, 0, limit);
}

/* How many cells to remove for each difficulty. Hard stays comfortably
 * below the point where a unique solution can no longer be guaranteed. */
static int target_removals(int level)
{
    switch (level)
    {
    case 1:
        return 30;
    case 2:
        return 45;
    default:
        return 52;
    }
}

void sudoku_generate(int level,
                     int puzzle[SUDOKU_DIM][SUDOKU_DIM],
                     int solution[SUDOKU_DIM][SUDOKU_DIM])
{
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            solution[r][c] = 0;
            puzzle[r][c] = 0;
        }
    }

    sudoku_fill_random(solution);

    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            puzzle[r][c] = solution[r][c];
        }
    }

    int order[SUDOKU_DIM * SUDOKU_DIM];
    for (int i = 0; i < SUDOKU_DIM * SUDOKU_DIM; i++)
    {
        order[i] = i;
    }
    /* Shuffle the removal order. */
    for (int i = SUDOKU_DIM * SUDOKU_DIM - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int t = order[i];
        order[i] = order[j];
        order[j] = t;
    }

    int target = target_removals(level);
    int removed = 0;

    for (int i = 0; i < SUDOKU_DIM * SUDOKU_DIM && removed < target; i++)
    {
        int pos = order[i];
        int row = pos / SUDOKU_DIM;
        int col = pos % SUDOKU_DIM;
        int saved = puzzle[row][col];

        puzzle[row][col] = 0;
        if (sudoku_count_solutions(puzzle, 2) > 1)
        {
            puzzle[row][col] = saved; /* would become ambiguous; keep it */
        }
        else
        {
            removed++;
        }
    }
}
