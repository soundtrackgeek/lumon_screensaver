#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gdiplus.h>
#include <memory>
#include "resource.h"

#pragma comment(lib, "gdiplus.lib")

// Screensaver state
struct ScreenSaverState {
    int x;
    int y;
    int dx;
    int dy;
    int width;
    int height;
    std::unique_ptr<Gdiplus::Image> logo;
};

ScreenSaverState g_state;

// Initialize GDI+
ULONG_PTR g_gdiplusToken;

// Function declarations
LRESULT WINAPI ScreenSaverProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitScreenSaver(HWND hwnd);
void CleanupScreenSaver();
void UpdateFrame(HWND hwnd);
void RenderFrame(HWND hwnd);

// Entry point
extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

    // Register the screensaver window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = (WNDPROC)ScreenSaverProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("LumonScreensaverClass");
    RegisterClass(&wc);

    // Create the screensaver window
    CreateWindow(
        TEXT("LumonScreensaverClass"),
        TEXT("Lumon Screensaver"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup GDI+
    Gdiplus::GdiplusShutdown(g_gdiplusToken);
    return msg.wParam;
}

LRESULT WINAPI ScreenSaverProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            InitScreenSaver(hwnd);
            SetTimer(hwnd, 1, 16, NULL); // ~60 FPS
            return 0;

        case WM_TIMER:
            UpdateFrame(hwnd);
            return 0;

        case WM_PAINT:
            RenderFrame(hwnd);
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, 1);
            CleanupScreenSaver();
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

void InitScreenSaver(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    // Load the logo from resources
    HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(IDR_LOGO), TEXT("PNG"));
    if (hResource) {
        HGLOBAL hGlobal = LoadResource(NULL, hResource);
        if (hGlobal) {
            void* resourceData = LockResource(hGlobal);
            DWORD resourceSize = SizeofResource(NULL, hResource);
            
            IStream* stream = NULL;
            if (CreateStreamOnHGlobal(NULL, TRUE, &stream) == S_OK) {
                ULONG bytesWritten;
                stream->Write(resourceData, resourceSize, &bytesWritten);
                g_state.logo.reset(Gdiplus::Image::FromStream(stream));
                stream->Release();
            }
        }
    }

    // Initialize position and movement
    g_state.width = g_state.logo ? g_state.logo->GetWidth() : 100;
    g_state.height = g_state.logo ? g_state.logo->GetHeight() : 100;
    g_state.x = rand() % (rect.right - g_state.width);
    g_state.y = rand() % (rect.bottom - g_state.height);
    g_state.dx = 5;
    g_state.dy = 5;
}

void CleanupScreenSaver() {
    g_state.logo.reset();
}

void UpdateFrame(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);

    // Update position
    g_state.x += g_state.dx;
    g_state.y += g_state.dy;

    // Bounce off edges
    if (g_state.x < 0) {
        g_state.x = 0;
        g_state.dx = -g_state.dx;
    }
    else if (g_state.x + g_state.width > rect.right) {
        g_state.x = rect.right - g_state.width;
        g_state.dx = -g_state.dx;
    }

    if (g_state.y < 0) {
        g_state.y = 0;
        g_state.dy = -g_state.dy;
    }
    else if (g_state.y + g_state.height > rect.bottom) {
        g_state.y = rect.bottom - g_state.height;
        g_state.dy = -g_state.dy;
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

void RenderFrame(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    // Create a GDI+ graphics object
    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    // Clear the background
    graphics.Clear(Gdiplus::Color::Black);

    // Draw the logo
    if (g_state.logo) {
        graphics.DrawImage(g_state.logo.get(), g_state.x, g_state.y, 
            g_state.width, g_state.height);
    }

    EndPaint(hwnd, &ps);
}