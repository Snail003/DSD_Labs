// Lab2.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Lab2.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//

const COLORREF backgroundColor = RGB(0, 0, 0);

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = NULL;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(backgroundColor);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH brush; // brush that is used for drawing
    static COLORREF patternColor = backgroundColor;
    static COLORREF backgroundColorBrush = RGB(255 - GetRValue(backgroundColor), 255 - GetGValue(backgroundColor), 255 - GetBValue(backgroundColor));
    static int pattern = 6;
    static RECT rcClient; // client area rectangle 
    static int pointsCounter = 0;
    static POINT pointsPolygon[10];
    static POINT pt;      // x- and y-coordinates of cursor  
    static RECT rcTarget; // rect to receive filled shape  
    static BOOL fSizeEllipse; // TRUE if ellipse is sized  
    static BOOL fDrawEllipse;   // TRUE if ellipse is drawn  
    static BOOL fDrawRectangle; // TRUE if rectangle is drawn  
    static BOOL fSizeRectangle; // TRUE if rectangle is sized  
    static BOOL fSizePolygon; // TRUE if polygon is sized
    static BOOL fDrawPolygon; // TRUE if polygon is drawn

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_FIGURES_RECTANGLE:
                if (fSizePolygon)
                {
                    InvalidateRect(hWnd, NULL, TRUE);
                    fSizePolygon = FALSE;
                }
                fSizeRectangle = TRUE;
                break;
            case ID_FIGURES_ELLIPSE:
                if (fSizePolygon)
                {
                    InvalidateRect(hWnd, NULL, TRUE);
                    fSizePolygon = FALSE;
                }
                fSizeEllipse = TRUE;
                break;
            case ID_POLYGON_ADDPOINTS:
                InvalidateRect(hWnd, NULL, TRUE);
                fSizeRectangle = FALSE;
                fSizeEllipse = FALSE;
                fSizePolygon = TRUE;
                pointsCounter = 0;
                break;
            case ID_POLYGON_DRAW:
                if (fSizePolygon)
                {
                    fSizePolygon = FALSE;
                    fDrawPolygon = TRUE;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                break;
            case ID_FIGURES_CLEAN:
                pointsCounter = 0;
                fDrawPolygon = FALSE;
                fDrawRectangle = FALSE;
                fDrawEllipse = FALSE;
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            case ID_PATTERN_BDIAGONAL:
                pattern = HS_BDIAGONAL;
                break;
            case ID_PATTERN_CROSS:
                pattern = HS_CROSS;
                break;
            case ID_PATTERN_DIAGCROSS:
                pattern = HS_DIAGCROSS;
                break;
            case ID_PATTERN_FDIAGONAL:
                pattern = HS_FDIAGONAL;
                break;
            case ID_PATTERN_HORIZONTAL:
                pattern = HS_HORIZONTAL;
                break;
            case ID_PATTERN_VERTICAL:
                pattern = HS_VERTICAL;
                break;
            case ID_PATTERN_SOLID:
                pattern = 6;
                break;
            case ID_PATTERN_SPECIAL:
                pattern = 7;
                break;
            case ID_PATTERNCOLOR_RED:
                patternColor = RGB(255, 0, 0);
                break;
            case ID_PATTERNCOLOR_ORANGE:
                patternColor = RGB(255, 165, 0);
                break;
            case ID_PATTERNCOLOR_YELLOW:
                patternColor = RGB(255, 255, 0);
                break;
            case ID_PATTERNCOLOR_GREEN:
                patternColor = RGB(0, 255, 0);
                break;
            case ID_PATTERNCOLOR_BLUE:
                patternColor = RGB(0, 0, 255);
                break;
            case ID_PATTERNCOLOR_INDIGO:
                patternColor = RGB(75, 0, 130);
                break;
            case ID_PATTERNCOLOR_VIOLET:
                patternColor = RGB(143, 0, 255);
                break;
            case ID_BACKGROUND_RED:
                backgroundColorBrush = RGB(255, 0, 0);
                break;
            case ID_BACKGROUND_ORANGE:
                backgroundColorBrush = RGB(255, 165, 0);
                break;
            case ID_BACKGROUND_YELLOW:
                backgroundColorBrush = RGB(255, 255, 0);
                break;
            case ID_BACKGROUND_GREEN:
                backgroundColorBrush = RGB(0, 255, 0);
                break;
            case ID_BACKGROUND_BLUE:
                backgroundColorBrush = RGB(0, 0, 255);
                break;
            case ID_BACKGROUND_INDIGO:
                backgroundColorBrush = RGB(75, 0, 130);
                break;
            case ID_BACKGROUND_VIOLET:
                backgroundColorBrush = RGB(143, 0, 255);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            if (pattern == 6)
            {
                brush = CreateSolidBrush(backgroundColorBrush);
            }
            else if (pattern == 7)
            {
                brush = CreatePatternBrush(LoadBitmapA(hInst, (LPCSTR)MAKEINTRESOURCE(IDB_BITMAP1)));
            }
            else
            {
                brush = CreateHatchBrush(pattern, patternColor);
                SetBkColor(ps.hdc, backgroundColorBrush);
            }

            SelectObject(ps.hdc, brush);



            if (fDrawEllipse)
            {
                Ellipse(ps.hdc, rcTarget.left, rcTarget.top,
                    rcTarget.right, rcTarget.bottom);
                fDrawEllipse = FALSE;
                rcTarget.left = rcTarget.right = 0;
                rcTarget.top = rcTarget.bottom = 0;
            }

            if (fDrawRectangle)
            {
                Rectangle(ps.hdc, rcTarget.left, rcTarget.top,
                    rcTarget.right, rcTarget.bottom);
                fDrawRectangle = FALSE;
                rcTarget.left = rcTarget.right = 0;
                rcTarget.top = rcTarget.bottom = 0;
            }

            if (fDrawPolygon)
            {
                Polygon(ps.hdc, pointsPolygon, pointsCounter);
                fDrawPolygon = FALSE;
            }

            DeleteObject(brush);
            EndPaint(hWnd, &ps);
        }
        break;



    case WM_SIZE:
        GetClientRect(hWnd, &rcClient);
        POINT ptClientUL, ptClientLR;
        ptClientUL.x = rcClient.left;
        ptClientUL.y = rcClient.top;
        ptClientLR.x = rcClient.right;
        ptClientLR.y = rcClient.bottom;
        ClientToScreen(hWnd, &ptClientUL);
        ClientToScreen(hWnd, &ptClientLR);
        SetRect(&rcClient, ptClientUL.x + 2, ptClientUL.y + 2,
            ptClientLR.x - 2, ptClientLR.y - 2);
        break;

    case WM_LBUTTONDOWN:
        if (fSizeEllipse || fSizeRectangle)
        {
            ClipCursor(&rcClient);
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
        }
        if (fSizePolygon)
        {
            HDC hdc = GetDC(hWnd);
            LONG cursorX = LOWORD(lParam);
            LONG cursorY = HIWORD(lParam);
            SetPixel(hdc, cursorX, cursorY, RGB(255 - GetRValue(backgroundColor), 255 - GetGValue(backgroundColor), 255 - GetBValue(backgroundColor)));
            pointsPolygon[pointsCounter].x = cursorX;
            pointsPolygon[pointsCounter].y = cursorY;
            pointsCounter = pointsCounter + 1;
            if (pointsCounter > 9)
            {
                fSizePolygon = FALSE;
                fDrawPolygon = TRUE;
               InvalidateRect(hWnd, NULL, TRUE);
            }
            ReleaseDC(hWnd, hdc);
        }
        break;

    case WM_MOUSEMOVE:
        if ((wParam && MK_LBUTTON)
            && (fSizeEllipse || fSizeRectangle))
        {

            HDC hdc = GetDC(hWnd);
            SetROP2(hdc, R2_NOTXORPEN);

            if (!IsRectEmpty(&rcTarget))
            {
                Rectangle(hdc, rcTarget.left, rcTarget.top,
                    rcTarget.right, rcTarget.bottom);
            }

            if ((pt.x < (LONG)LOWORD(lParam)) &&
                (pt.y > (LONG)HIWORD(lParam)))
            {
                SetRect(&rcTarget, pt.x, HIWORD(lParam),
                    LOWORD(lParam), pt.y);
            }

            else if ((pt.x > (LONG)LOWORD(lParam)) &&
                (pt.y > (LONG)HIWORD(lParam)))
            {
                SetRect(&rcTarget, LOWORD(lParam),
                    HIWORD(lParam), pt.x, pt.y);
            }

            else if ((pt.x > (LONG)LOWORD(lParam)) &&
                (pt.y < (LONG)HIWORD(lParam)))
            {
                SetRect(&rcTarget, LOWORD(lParam), pt.y,
                    pt.x, HIWORD(lParam));
            }
            else
            {
                SetRect(&rcTarget, pt.x, pt.y, LOWORD(lParam),
                    HIWORD(lParam));
            } 

            Rectangle(hdc, rcTarget.left, rcTarget.top,
                rcTarget.right, rcTarget.bottom);
            ReleaseDC(hWnd, hdc);
        }
        break;

    case WM_LBUTTONUP:
        if (fSizeEllipse)
        {
            fSizeEllipse = FALSE;
            fDrawEllipse = TRUE;
        }

        if (fSizeRectangle)
        {
            fSizeRectangle = FALSE;
            fDrawRectangle = TRUE;
        }

        if (fDrawEllipse || fDrawRectangle)
        {
            ClipCursor((LPRECT)NULL);
            InvalidateRect(hWnd, &rcTarget, TRUE);
            UpdateWindow(hWnd);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
