#include <stdio.h>
#include <stdlib.h>

#include "sudoku_gui.h"

/* Combine the fixed givens and the user's entries into one board. */
static void computeBoard(const struct Game *s1, int board[SUDOKU_DIM][SUDOKU_DIM])
{
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            board[r][c] = s1->puzzle[r][c] ? s1->puzzle[r][c] : s1->user[r][c];
        }
    }
}

/* True when every cell is filled and no rule is broken. */
static bool boardFullValid(const struct Game *s1)
{
    int board[SUDOKU_DIM][SUDOKU_DIM];
    computeBoard(s1, board);
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            if (board[r][c] == 0)
            {
                return false;
            }
        }
    }
    return !sudoku_has_conflict(board);
}

static void drawGridLines(HDC hdc, int gx, int gy, int cell)
{
    int size = SUDOKU_DIM * cell;
    HPEN thin = CreatePen(PS_SOLID, 1, COLOR_LINE_THIN);
    HPEN thick = CreatePen(PS_SOLID, 3, COLOR_LINE_THICK);
    for (int i = 0; i <= SUDOKU_DIM; i++)
    {
        HPEN pen = (i % 3 == 0) ? thick : thin;
        HPEN old = SelectObject(hdc, pen);
        MoveToEx(hdc, gx + i * cell, gy, NULL);
        LineTo(hdc, gx + i * cell, gy + size);
        MoveToEx(hdc, gx, gy + i * cell, NULL);
        LineTo(hdc, gx + size, gy + i * cell);
        SelectObject(hdc, old);
    }
    DeleteObject(thin);
    DeleteObject(thick);
}

/* Pencil marks: one small dot per digit that still fits in this cell. */
static void drawPencilDots(HDC hdc, const RECT *cr,
                           const int board[SUDOKU_DIM][SUDOKU_DIM],
                           int r, int c)
{
    int subW = (cr->right - cr->left) / 3;
    int subH = (cr->bottom - cr->top) / 3;
    HBRUSH dot = CreateSolidBrush(COLOR_PENCIL);
    HPEN oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    HBRUSH oldBrush = SelectObject(hdc, dot);
    for (int d = 1; d <= SUDOKU_DIM; d++)
    {
        if (!sudoku_is_valid(board, r, c, d))
        {
            continue;
        }
        int dr = (d - 1) / 3;
        int dc = (d - 1) % 3;
        int cx = cr->left + dc * subW + subW / 2;
        int cy = cr->top + dr * subH + subH / 2;
        Ellipse(hdc, cx - 2, cy - 2, cx + 3, cy + 3);
    }
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(dot);
}

void drawBoard(HDC hdc, struct Game *s1, struct Gui *g1)
{
    int board[SUDOKU_DIM][SUDOKU_DIM];
    computeBoard(s1, board);
    int cell = g1->cell;
    int gx = g1->gridX0;
    int gy = g1->gridY0;

    HBRUSH bg = CreateSolidBrush(COLOR_WHITE);
    RECT full = { gx, gy, gx + SUDOKU_DIM * cell, gy + SUDOKU_DIM * cell };
    FillRect(hdc, &full, bg);
    DeleteObject(bg);

    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            RECT cr = { gx + c * cell, gy + r * cell,
                        gx + (c + 1) * cell, gy + (r + 1) * cell };

            if (r == s1->selRow && c == s1->selCol)
            {
                HBRUSH sel = CreateSolidBrush(COLOR_SEL_FILL);
                FillRect(hdc, &cr, sel);
                DeleteObject(sel);
            }

            char buf[4];
            if (s1->puzzle[r][c])
            {
                HFONT old = SelectObject(hdc, g1->fontCell);
                SetTextColor(hdc, COLOR_GIVEN);
                sprintf(buf, "%d", s1->puzzle[r][c]);
                DrawText(hdc, buf, -1, &cr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(hdc, old);
            }
            else if (s1->user[r][c])
            {
                HFONT old = SelectObject(hdc, g1->fontCell);
                SetTextColor(hdc, sudoku_cell_conflict(board, r, c) ? COLOR_CONFLICT : COLOR_USER);
                sprintf(buf, "%d", s1->user[r][c]);
                DrawText(hdc, buf, -1, &cr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(hdc, old);
            }
            else
            {
                drawPencilDots(hdc, &cr, board, r, c);
            }
        }
    }

    drawGridLines(hdc, gx, gy, cell);

    if (s1->selRow >= 0 && s1->selCol >= 0)
    {
        RECT cr = { gx + s1->selCol * cell + 1, gy + s1->selRow * cell + 1,
                    gx + (s1->selCol + 1) * cell - 1, gy + (s1->selRow + 1) * cell - 1 };
        HPEN pen = CreatePen(PS_SOLID, 2, COLOR_SEL_BORDER);
        HPEN oldPen = SelectObject(hdc, pen);
        HBRUSH oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, cr.left, cr.top, cr.right, cr.bottom);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
    }
}

void drawStatus(HDC hdc, struct Game *s1, struct Gui *g1)
{
    SetBkMode(hdc, TRANSPARENT);

    const char *labels[] = LEVEL_LABELS;
    const char *lvl = (s1->level >= 1 && s1->level <= LEVEL_COUNT)
                          ? labels[s1->level - 1] : "";
    char line[96];
    if (s1->solved)
    {
        sprintf(line, "Solved!   Score: %ld   (Game > New to play again)", s1->score);
        SetTextColor(hdc, COLOR_SOLVED);
    }
    else
    {
        sprintf(line, "Level: %s    Score: %ld    click a cell, type 1-9", lvl, s1->score);
        SetTextColor(hdc, COLOR_STATUS);
    }
    HFONT old = SelectObject(hdc, g1->fontStatus);
    RECT sr = { 0, 0, WIN_W, g1->gridY0 };
    DrawText(hdc, line, -1, &sr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
}

void setupMenu(HWND hwnd, struct PointerStruct *p1)
{
    HMENU menu = CreateMenu();
    HMENU game = CreatePopupMenu();
    AppendMenu(game, MF_STRING, IDM_NEW_EASY, "New - Easy");
    AppendMenu(game, MF_STRING, IDM_NEW_MEDIUM, "New - Medium");
    AppendMenu(game, MF_STRING, IDM_NEW_HARD, "New - Hard");
    AppendMenu(menu, MF_POPUP, (UINT_PTR)game, "Game");
    SetMenu(hwnd, menu);
    p1->g1->menu = menu;
}

/* Grayscale anti-aliased fonts: no ClearType subpixel fringing, so text
 * stays crisp on any background colour. */
static HFONT makeFont(int height, int weight, const char *face)
{
    LOGFONT lf = {0};
    lf.lfHeight = height;
    lf.lfWeight = weight;
    lf.lfQuality = ANTIALIASED_QUALITY;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpynA(lf.lfFaceName, face, LF_FACESIZE);
    return CreateFontIndirectA(&lf);
}

void createFonts(struct Gui *g1)
{
    g1->fontCell = makeFont(24, FW_BOLD, CELL_FONT);
    g1->fontStatus = makeFont(16, FW_NORMAL, TITLE_FONT);
}

void deleteFonts(struct Gui *g1)
{
    if (g1->fontCell)
        DeleteObject(g1->fontCell);
    if (g1->fontStatus)
        DeleteObject(g1->fontStatus);
}

void newGame(struct Game *s1, struct Gui *g1, int level)
{
    (void)g1;
    s1->level = level;
    s1->score = 0;
    s1->solved = false;
    s1->selRow = -1;
    s1->selCol = -1;
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            s1->user[r][c] = 0;
        }
    }

    sudoku_generate(level, s1->puzzle, s1->solution);

    bool found = false;
    for (int r = 0; r < SUDOKU_DIM && !found; r++)
    {
        for (int c = 0; c < SUDOKU_DIM && !found; c++)
        {
            if (s1->puzzle[r][c] == 0)
            {
                s1->selRow = r;
                s1->selCol = c;
                found = true;
            }
        }
    }
}

void onGridClick(HWND hwnd, struct Game *s1, struct Gui *g1, LPARAM lp)
{
    int x = LOWORD(lp) - g1->gridX0;
    int y = HIWORD(lp) - g1->gridY0;
    if (x < 0 || y < 0 || x >= SUDOKU_DIM * g1->cell || y >= SUDOKU_DIM * g1->cell)
    {
        s1->selRow = -1;
        s1->selCol = -1;
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }
    s1->selCol = x / g1->cell;
    s1->selRow = y / g1->cell;
    InvalidateRect(hwnd, NULL, TRUE);
}

void onKeyPress(HWND hwnd, struct Game *s1, struct Gui *g1, WPARAM key)
{
    (void)g1;
    int r = s1->selRow;
    int c = s1->selCol;

    if (r < 0)
    {
        bool found = false;
        for (int i = 0; i < SUDOKU_DIM && !found; i++)
        {
            for (int j = 0; j < SUDOKU_DIM && !found; j++)
            {
                if (s1->puzzle[i][j] == 0)
                {
                    r = i;
                    c = j;
                    found = true;
                }
            }
        }
        if (!found)
        {
            r = 0;
            c = 0;
        }
        s1->selRow = r;
        s1->selCol = c;
    }

    int nr = r;
    int nc = c;
    switch (key)
    {
    case VK_UP:
        if (r > 0)
            nr = r - 1;
        break;
    case VK_DOWN:
        if (r < SUDOKU_DIM - 1)
            nr = r + 1;
        break;
    case VK_LEFT:
        if (c > 0)
            nc = c - 1;
        break;
    case VK_RIGHT:
        if (c < SUDOKU_DIM - 1)
            nc = c + 1;
        break;
    default:
        break;
    }

    if (nr != r || nc != c)
    {
        s1->selRow = nr;
        s1->selCol = nc;
        InvalidateRect(hwnd, NULL, TRUE);
        return;
    }

    if (s1->solved)
    {
        return;
    }
    if (s1->puzzle[r][c] != 0)
    {
        return; /* givens are fixed */
    }

    int newVal = 0;
    if (key >= '1' && key <= '9')
    {
        newVal = key - '0';
    }
    else if (key == '0' || key == VK_BACK || key == VK_DELETE || key == VK_SPACE)
    {
        newVal = 0;
    }
    else
    {
        return;
    }

    int old = s1->user[r][c];
    if (newVal == old)
    {
        return;
    }

    if (old == 0 && newVal != 0)
    {
        s1->score += 10;
    }
    else if (old != 0 && newVal == 0)
    {
        s1->score -= 10;
        if (s1->score < 0)
            s1->score = 0;
    }

    s1->user[r][c] = newVal;
    s1->solved = boardFullValid(s1);
    InvalidateRect(hwnd, NULL, TRUE);
}
