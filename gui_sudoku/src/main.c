#include <time.h>

#include "sudoku_gui.h"
#include "sudoku.h"

/* Message handler for the single main window. The pointer struct created
 * in main() is attached here so every handler can reach the game + gui. */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    struct PointerStruct *p1 =
        (struct PointerStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg)
    {
    case WM_CREATE:
    {
        CREATESTRUCT *cs = (CREATESTRUCT *)lp;
        struct PointerStruct *pp = (struct PointerStruct *)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pp);

        createFonts(pp->g1);
        setupMenu(hwnd, pp);

        HICON hicon = (HICON)LoadImage(NULL, iconFilePath, IMAGE_ICON,
                                       500, 500, LR_LOADFROMFILE);
        if (hicon)
        {
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicon);
        }

        newGame(pp->s1, pp->g1, 1); /* start on Easy */
        return 0;
    }

    case WM_ERASEBKGND:
        /* The background is drawn in WM_PAINT (double buffered); suppress
           the default erase so the window never flashes blank. */
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;

        /* Render to an offscreen bitmap, then blit it in one shot, so the
           finished frame appears atomically (no mid-draw flicker). */
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
        HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

        HBRUSH bg = CreateSolidBrush(COLOR_BG);
        FillRect(memDC, &rc, bg);
        DeleteObject(bg);

        drawBoard(memDC, p1->s1, p1->g1);
        drawStatus(memDC, p1->s1, p1->g1);

        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBmp);
        DeleteObject(memBmp);
        DeleteDC(memDC);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wp);
        switch (id)
        {
        case IDM_NEW_EASY:
            newGame(p1->s1, p1->g1, 1);
            break;
        case IDM_NEW_MEDIUM:
            newGame(p1->s1, p1->g1, 2);
            break;
        case IDM_NEW_HARD:
            newGame(p1->s1, p1->g1, 3);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            return 0;
        default:
            break;
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }

    case WM_LBUTTONDOWN:
        onGridClick(hwnd, p1->s1, p1->g1, lp);
        return 0;

    case WM_KEYDOWN:
        onKeyPress(hwnd, p1->s1, p1->g1, wp);
        return 0;

    case WM_DESTROY:
        deleteFonts(p1->g1);
        if (p1->g1->menu)
        {
            DestroyMenu(p1->g1->menu);
            p1->g1->menu = NULL;
        }
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}

int main(void)
{
    srand((unsigned)time(NULL));

    struct Game s1 = {0};
    struct Gui g1 = {
        .cell = CELL_SIZE,
        .gridX0 = GRID_X0,
        .gridY0 = GRID_Y0,
    };
    struct PointerStruct p1 = {&s1, &g1};

    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASS wc = {0};
    static const char className[] = "Sudoku";
    wc.lpszClassName = className;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = CreateSolidBrush(COLOR_BG);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window registration failed", "Error",
                   MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    HWND hwnd = CreateWindowEx(0, className, "Sudoku",
                               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                               CW_USEDEFAULT, CW_USEDEFAULT, WIN_W, WIN_H,
                               NULL, NULL, hInstance, &p1);
    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window creation failed", "Error",
                   MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
