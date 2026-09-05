#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static void drawGridLines(HDC hdc, int gx, int gy, int cell, double scale)
{
    int size = SUDOKU_DIM * cell;
    int thinW = (int)(1 * scale + 0.5);
    int thickW = (int)(3 * scale + 0.5);
    HPEN thin = CreatePen(PS_SOLID, thinW, COLOR_LINE_THIN);
    HPEN thick = CreatePen(PS_SOLID, thickW, COLOR_LINE_THICK);
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
                           int r, int c, double scale)
{
    int subW = (cr->right - cr->left) / 3;
    int subH = (cr->bottom - cr->top) / 3;
    int dotL = (int)(2 * scale + 0.5);
    int dotR = (int)(3 * scale + 0.5);
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
        Ellipse(hdc, cx - dotL, cy - dotL, cx + dotR, cy + dotR);
    }
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(dot);
}

void drawBoard(HDC hdc, struct Game *s1, struct Gui *g1)
{
    /* Draw text with a transparent background so each digit sits directly on
       its cell (no opaque "box" behind it, even on highlighted cells). */
    SetBkMode(hdc, TRANSPARENT);

    int board[SUDOKU_DIM][SUDOKU_DIM];
    computeBoard(s1, board);
    int cell = g1->cell;
    int gx = g1->gridX0;
    int gy = g1->gridY0;

    HBRUSH bg = CreateSolidBrush(COLOR_WHITE);
    RECT full = { gx, gy, gx + SUDOKU_DIM * cell, gy + SUDOKU_DIM * cell };
    FillRect(hdc, &full, bg);
    DeleteObject(bg);

    int selDigit = (s1->selRow >= 0 && s1->selCol >= 0)
                       ? board[s1->selRow][s1->selCol] : 0;

    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            RECT cr = { gx + c * cell, gy + r * cell,
                        gx + (c + 1) * cell, gy + (r + 1) * cell };

            bool isSel = (r == s1->selRow && c == s1->selCol);
            bool isPeer = s1->selRow >= 0 &&
                          (r == s1->selRow || c == s1->selCol ||
                           (r / 3 == s1->selRow / 3 && c / 3 == s1->selCol / 3));
            bool isMatch = selDigit > 0 && !isSel && board[r][c] == selDigit;

            COLORREF fill = isSel ? COLOR_SEL_FILL
                              : isMatch ? COLOR_MATCH_FILL
                              : isPeer ? COLOR_PEER_FILL : 0;
            if (fill)
            {
                HBRUSH b = CreateSolidBrush(fill);
                FillRect(hdc, &cr, b);
                DeleteObject(b);
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
                drawPencilDots(hdc, &cr, board, r, c, g1->scale);
            }
        }
    }

    drawGridLines(hdc, gx, gy, cell, g1->scale);

    if (s1->selRow >= 0 && s1->selCol >= 0)
    {
        RECT cr = { gx + s1->selCol * cell + 1, gy + s1->selRow * cell + 1,
                    gx + (s1->selCol + 1) * cell - 1, gy + (s1->selRow + 1) * cell - 1 };
        HPEN pen = CreatePen(PS_SOLID, (int)(2 * g1->scale + 0.5), COLOR_SEL_BORDER);
        HPEN oldPen = SelectObject(hdc, pen);
        HBRUSH oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, cr.left, cr.top, cr.right, cr.bottom);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
    }
}

/* Elapsed play time in seconds, frozen once the board is solved. */
static long elapsedSeconds(const struct Game *s1, long now)
{
    if (s1->start_time <= 0)
    {
        return 0;
    }
    long end = s1->solved ? s1->solved_time : now;
    if (end < s1->start_time)
    {
        return 0;
    }
    return end - s1->start_time;
}

/* Write `secs` as MM:SS (or H:MM:SS past one hour) into buf. */
static void formatTime(long secs, char *buf, int n)
{
    if (secs < 0)
    {
        secs = 0;
    }
    long h = secs / 3600;
    long m = (secs % 3600) / 60;
    long s = secs % 60;
    if (h > 0)
    {
        snprintf(buf, (size_t)n, "%ld:%02ld:%02ld", h, m, s);
    }
    else
    {
        snprintf(buf, (size_t)n, "%02ld:%02ld", m, s);
    }
}

void drawStatus(HDC hdc, struct Game *s1, struct Gui *g1)
{
    SetBkMode(hdc, TRANSPARENT);

    const char *labels[] = LEVEL_LABELS;
    const char *lvl = (s1->level >= 1 && s1->level <= LEVEL_COUNT)
                          ? labels[s1->level - 1] : "";
    char timeStr[16];
    formatTime(elapsedSeconds(s1, (long)time(NULL)), timeStr, (int)sizeof(timeStr));

    char line[96];
    if (s1->solved)
    {
        sprintf(line, "Solved in %s!   Score: %ld", timeStr, s1->score);
        SetTextColor(hdc, COLOR_SOLVED);
    }
    else
    {
        sprintf(line, "Level: %s    Time: %s    Score: %ld    click a cell, type 1-9",
                lvl, timeStr, s1->score);
        SetTextColor(hdc, COLOR_STATUS);
    }
    HFONT old = SelectObject(hdc, g1->fontStatus);
    RECT sr = { 0, 0, (int)(WIN_W * g1->scale), g1->gridY0 };
    DrawText(hdc, line, -1, &sr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
}

/* Celebration shown over the board once it is solved. */
void drawSolvedOverlay(HDC hdc, struct Game *s1, struct Gui *g1)
{
    if (!s1->solved)
    {
        return;
    }
    int boardL = g1->gridX0;
    int boardT = g1->gridY0;
    int boardR = g1->gridX0 + SUDOKU_DIM * g1->cell;
    int boardB = g1->gridY0 + SUDOKU_DIM * g1->cell;
    int boardW = boardR - boardL;
    int boardH = boardB - boardT;

    int panelW = (int)(boardW * 0.72 + 0.5);
    int panelH = (int)(boardH * 0.34 + 0.5);
    int pl = (boardL + boardR) / 2 - panelW / 2;
    int pt = (boardT + boardB) / 2 - panelH / 2;
    int rad = (int)(panelH * 0.12 + 0.5);

    HBRUSH fill = CreateSolidBrush(COLOR_SOLVED);
    HPEN pen = CreatePen(PS_SOLID, (int)(2 * g1->scale + 0.5), COLOR_SOLVED_BORDER);
    HBRUSH oldBrush = SelectObject(hdc, fill);
    HPEN oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, pl, pt, pl + panelW, pt + panelH, rad, rad);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(fill);
    DeleteObject(pen);

    SetBkMode(hdc, TRANSPARENT);

    const char *labels[] = LEVEL_LABELS;
    const char *lvl = (s1->level >= 1 && s1->level <= LEVEL_COUNT)
                         ? labels[s1->level - 1] : "";
    char timeStr[16];
    formatTime(elapsedSeconds(s1, (long)time(NULL)), timeStr, (int)sizeof(timeStr));
    char detail[96];
    sprintf(detail, "%s    Time: %s    Score: %ld", lvl, timeStr, s1->score);

    int pad = (int)(panelW * 0.06 + 0.5);
    int y1 = pt + (int)(panelH * 0.46);
    int y2 = pt + (int)(panelH * 0.74);
    RECT rHead = { pl + pad, pt, pl + panelW - pad, y1 };
    RECT rDet = { pl + pad, y1, pl + panelW - pad, y2 };
    RECT rHint = { pl + pad, y2, pl + panelW - pad, pt + panelH };

    HFONT old = SelectObject(hdc, g1->fontWin);
    SetTextColor(hdc, COLOR_SOLVED_TEXT);
    DrawText(hdc, "Solved!", -1, &rHead, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, g1->fontStatus);
    SetTextColor(hdc, COLOR_GIVEN);
    DrawText(hdc, detail, -1, &rDet, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SetTextColor(hdc, COLOR_SOLVED_HINT);
    DrawText(hdc, "Press N for a new game", -1, &rHint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
    g1->fontCell = makeFont((int)(24 * g1->scale + 0.5), FW_BOLD, CELL_FONT);
    g1->fontStatus = makeFont((int)(16 * g1->scale + 0.5), FW_NORMAL, TITLE_FONT);
    g1->fontWin = makeFont((int)(34 * g1->scale + 0.5), FW_BOLD, CELL_FONT);
}

void deleteFonts(struct Gui *g1)
{
    if (g1->fontCell)
        DeleteObject(g1->fontCell);
    if (g1->fontStatus)
        DeleteObject(g1->fontStatus);
    if (g1->fontWin)
        DeleteObject(g1->fontWin);
}

void newGame(struct Game *s1, struct Gui *g1, int level)
{
    (void)g1;
    s1->level = level;
    s1->score = 0;
    s1->solved = false;
    s1->selRow = -1;
    s1->selCol = -1;
    s1->start_time = (long)time(NULL);
    s1->solved_time = 0;
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
    if (s1->solved)
    {
        return; /* board is finished; ignore clicks */
    }
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
    if (s1->solved)
    {
        /* Board is finished: only a "new game" key does anything. */
        if (key == 'N' || key == 'n' || key == VK_RETURN || key == VK_SPACE)
        {
            newGame(s1, g1, s1->level);
            saveGame(s1);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return;
    }
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
    bool wasSolved = s1->solved;
    s1->solved = boardFullValid(s1);
    if (s1->solved && !wasSolved)
    {
        s1->solved_time = (long)time(NULL);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

/* Auto-resume: persist the current game to a small text file. */
void saveGame(const struct Game *s1)
{
    FILE *f = fopen(saveFilePath, "w");
    if (!f)
    {
        return;
    }
    fprintf(f, "sudoku_save 2\n");
    fprintf(f, "%d %ld %d %d %d %ld %ld\n", s1->level, s1->score, s1->solved,
            s1->selRow, s1->selCol, s1->start_time, s1->solved_time);
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
            fprintf(f, "%d ", s1->puzzle[r][c]);
        fputc('\n', f);
    }
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
            fprintf(f, "%d ", s1->user[r][c]);
        fputc('\n', f);
    }
    fclose(f);
}

/* Returns true (and fills s1) when a valid save file was read. */
bool loadGame(struct Game *s1)
{
    FILE *f = fopen(saveFilePath, "r");
    if (!f)
    {
        return false;
    }
    char magic[32] = {0};
    int ver = 0, lvl = 0, solved = 0, selRow = -1, selCol = -1;
    long score = 0, startTime = 0, solvedTime = 0;
    int pz[SUDOKU_DIM * SUDOKU_DIM];
    int us[SUDOKU_DIM * SUDOKU_DIM];
    bool ok = fscanf(f, "%31s", magic) == 1 && strcmp(magic, "sudoku_save") == 0 &&
              fscanf(f, "%d", &ver) == 1 && ver == 2 &&
              fscanf(f, "%d %ld %d %d %d %ld %ld",
                     &lvl, &score, &solved, &selRow, &selCol,
                     &startTime, &solvedTime) == 7;
    for (int i = 0; i < SUDOKU_DIM * SUDOKU_DIM && ok; i++)
        ok = fscanf(f, "%d", &pz[i]) == 1;
    for (int i = 0; i < SUDOKU_DIM * SUDOKU_DIM && ok; i++)
        ok = fscanf(f, "%d", &us[i]) == 1;
    fclose(f);
    if (!ok)
    {
        return false;
    }
    if (lvl < 1 || lvl > LEVEL_COUNT)
    {
        return false;
    }
    for (int i = 0; i < SUDOKU_DIM * SUDOKU_DIM; i++)
    {
        if (pz[i] < 0 || pz[i] > 9 || us[i] < 0 || us[i] > 9)
        {
            return false;
        }
    }
    s1->level = lvl;
    s1->score = score;
    s1->solved = solved != 0;
    s1->selRow = (selRow >= 0 && selRow < SUDOKU_DIM) ? selRow : -1;
    s1->selCol = (selCol >= 0 && selCol < SUDOKU_DIM) ? selCol : -1;
    s1->start_time = startTime;
    s1->solved_time = solvedTime;
    for (int r = 0; r < SUDOKU_DIM; r++)
    {
        for (int c = 0; c < SUDOKU_DIM; c++)
        {
            s1->puzzle[r][c] = pz[r * SUDOKU_DIM + c];
            s1->user[r][c] = us[r * SUDOKU_DIM + c];
        }
    }
    return true;
}
