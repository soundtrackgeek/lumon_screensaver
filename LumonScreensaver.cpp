#ifdef _WIN32
    #include <objidl.h>  // For IStream
    #include <ole2.h>    // For COM interfaces
    #include <wchar.h>   // For wtoll
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <gdiplus.h>
    #pragma comment(lib, "gdiplus.lib")
    #pragma comment(lib, "ole32.lib")
    #ifndef SPI_SETSCREENSAVERRUNNING
        #define SPI_SETSCREENSAVERRUNNING 97
    #endif
#elif __APPLE__
    #include <Cocoa/Cocoa.h>
    #include <CoreGraphics/CoreGraphics.h>
#else // Linux
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
#endif

#include <cstdlib>
#include <ctime>
#include <string>
#include <memory>

#ifdef _WIN32
    #include "resource.h"
#endif

// Common state structure for all platforms
struct ScreenSaverState {
    int x;
    int y;
    int dx;
    int dy;
    int width;
    int height;
    int screenWidth;
    int screenHeight;
    #ifdef _WIN32
        std::unique_ptr<Gdiplus::Image> logo;
    #endif
};

ScreenSaverState g_state;

#ifdef _WIN32
    ULONG_PTR g_gdiplusToken;

    // Function declarations
    LRESULT WINAPI ScreenSaverProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void InitScreenSaver(HWND hwnd);
    void CleanupScreenSaver();
    void UpdateFrame(HWND hwnd);
    void RenderFrame(HWND hwnd);
    
    // Add global variable for screensaver state
    bool g_isScreenSaver = false;

    // Entry point
    extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
        // Parse command line using Unicode
        LPWSTR cmdLine = GetCommandLineW();
        bool isPreview = false;
        bool isConfig = false;
        bool isScreenSaver = false;
        HWND hwndParent = NULL;

        // Initialize GDI+ first to catch any initialization errors
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        if (status != Gdiplus::Ok) {
            MessageBoxW(NULL, L"Failed to initialize GDI+", L"Error", MB_OK | MB_ICONERROR);
            return 1;
        }

        // Skip the program name in command line
        bool inQuotes = false;
        LPWSTR p = cmdLine;
        while (*p) {
            if (*p == L'"') inQuotes = !inQuotes;
            else if (*p == L' ' && !inQuotes) {
                p++;
                break;
            }
            p++;
        }

        // Now p points to arguments after program name
        if (*p && *(p + 1)) {  // Need at least /X
            switch (*(p + 1)) {
                case L'p':
                case L'P':
                    isPreview = true;
                    if (*(p + 2)) {  // Has window handle
                        hwndParent = (HWND)(ULONG_PTR)wtoll(p + 3);
                    }
                    break;
                case L'c':
                case L'C':
                    isConfig = true;
                    if (*(p + 2)) {  // Has window handle
                        hwndParent = (HWND)(ULONG_PTR)wtoll(p + 3);
                    }
                    MessageBoxW(NULL, L"No configuration options available.", L"Lumon Screensaver", MB_OK | MB_ICONINFORMATION);
                    Gdiplus::GdiplusShutdown(gdiplusToken);
                    return 0;
                case L's':
                case L'S':
                    isScreenSaver = true;
                    break;
                default:
                    // If no valid flag, run in test mode (windowed)
                    break;
            }
        }

        // Tell system that screensaver is running
        if (isScreenSaver) {
            g_isScreenSaver = true;
            BOOL dummy;
            SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, TRUE, &dummy, 0);
        }

        // Register the screensaver window class
        WNDCLASSW wc = {};
        wc.lpfnWndProc = (WNDPROC)ScreenSaverProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"LumonScreensaverClass";
        wc.hCursor = isScreenSaver ? NULL : LoadCursor(NULL, IDC_ARROW);
        
        if (!RegisterClassW(&wc)) {
            MessageBoxW(NULL, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
            Gdiplus::GdiplusShutdown(gdiplusToken);
            return 1;
        }

        // Get the primary monitor's resolution
        int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);  // Changed from SM_CXSCREEN
        int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN); // Changed from SM_CYSCREEN
        int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);    // Add this
        int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);    // Add this

        DWORD windowStyle;
        int windowX, windowY, windowWidth, windowHeight;

        if (isPreview) {
            if (!hwndParent) {
                MessageBoxW(NULL, L"Preview mode requires a parent window", L"Error", MB_OK | MB_ICONERROR);
                Gdiplus::GdiplusShutdown(gdiplusToken);
                return 1;
            }
            RECT rect;
            GetClientRect(hwndParent, &rect);
            windowX = 0;
            windowY = 0;
            windowWidth = rect.right;
            windowHeight = rect.bottom;
            windowStyle = WS_CHILD | WS_VISIBLE;
        } else if (isScreenSaver) {
            windowX = screenLeft;      // Changed from 0
            windowY = screenTop;       // Changed from 0
            windowWidth = screenWidth;
            windowHeight = screenHeight;
            windowStyle = WS_POPUP;    // Remove WS_VISIBLE
        } else {
            // Test mode - windowed
            windowX = (screenWidth - 800) / 2;  // Center the window
            windowY = (screenHeight - 600) / 2;
            windowWidth = 800;
            windowHeight = 600;
            windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        }

        // Create the window
        HWND hwnd = CreateWindowW(
            L"LumonScreensaverClass",
            L"Lumon Screensaver",
            windowStyle,
            windowX, windowY,
            windowWidth, windowHeight,
            hwndParent, NULL, hInstance, NULL
        );

        if (!hwnd) {
            DWORD error = GetLastError();
            wchar_t errorMsg[256];
            swprintf(errorMsg, 256, L"Failed to create window. Error code: %lu", error);
            MessageBoxW(NULL, errorMsg, L"Error", MB_OK | MB_ICONERROR);
            Gdiplus::GdiplusShutdown(gdiplusToken);
            return 1;
        }

        // Store GDI+ token in global variable for cleanup
        g_gdiplusToken = gdiplusToken;

        // Hide cursor in screensaver mode
        if (isScreenSaver) {
            ShowCursor(FALSE);
        }

        // Force window to foreground in screensaver mode
        if (isScreenSaver) {
            SetForegroundWindow(hwnd);
        }

        // After creating the window in screensaver mode, make it visible and topmost
        if (isScreenSaver) {
            ShowWindow(hwnd, SW_SHOWMAXIMIZED);  // Add this line
            SetWindowPos(hwnd, HWND_TOPMOST, screenLeft, screenTop, screenWidth, screenHeight, 
                        SWP_SHOWWINDOW);  // Modified
        }

        // Message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Show cursor before exit if we hid it
        if (isScreenSaver) {
            ShowCursor(TRUE);
        }

        // Cleanup
        if (isScreenSaver) {
            BOOL dummy;
            SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, FALSE, &dummy, 0);
        }

        UnregisterClassW(L"LumonScreensaverClass", hInstance);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return msg.wParam;
    }

    LRESULT WINAPI ScreenSaverProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        static POINT s_lastMousePos = { 0, 0 };
        
        switch (message) {
            case WM_CREATE:
                if (g_isScreenSaver) {
                    // Save initial mouse position
                    GetCursorPos(&s_lastMousePos);
                }
                InitScreenSaver(hwnd);
                SetTimer(hwnd, 1, 16, NULL); // ~60 FPS
                return 0;

            case WM_MOUSEMOVE:
                if (g_isScreenSaver) {
                    POINT currentPos;
                    GetCursorPos(&currentPos);
                    
                    // Exit if mouse moved more than 3 pixels in any direction
                    if (abs(currentPos.x - s_lastMousePos.x) > 3 ||
                        abs(currentPos.y - s_lastMousePos.y) > 3) {
                        DestroyWindow(hwnd);
                    }
                }
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
                ShowCursor(TRUE);  // Make sure cursor is shown when exiting
                PostQuitMessage(0);
                return 0;

            // Add input handling to exit screensaver
            case WM_KEYDOWN:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
                DestroyWindow(hwnd);
                return 0;

            // Add system key handling
            case WM_SYSCOMMAND:
                if (g_isScreenSaver) {
                    switch (wParam) {
                        case SC_CLOSE:
                        case SC_MAXIMIZE:
                        case SC_MINIMIZE:
                        case SC_MOVE:
                        case SC_SIZE:
                            return 0;
                    }
                }
                return DefWindowProc(hwnd, message, wParam, lParam);

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

#elif __APPLE__
@interface ScreenSaverView : NSView
@end

@implementation ScreenSaverView
- (void)drawRect:(NSRect)rect {
    NSRect bounds = [self bounds];
    [[NSColor blackColor] set];
    NSRectFill(bounds);
    
    // Draw logo (placeholder rectangle for now)
    [[NSColor whiteColor] set];
    NSRectFill(NSMakeRect(g_state.x, g_state.y, g_state.width, g_state.height));
}
@end

#else // Linux
Display* display;
Window window;
GC gc;
#endif

void InitScreenSaver() {
    g_state.width = 100;  // Logo dimensions
    g_state.height = 100;
    g_state.dx = 5;
    g_state.dy = 5;
    
    srand(time(NULL));
    
    #ifdef _WIN32
        // ... existing Windows initialization ...
    #elif __APPLE__
        NSScreen* screen = [NSScreen mainScreen];
        NSRect screenRect = [screen frame];
        g_state.screenWidth = screenRect.size.width;
        g_state.screenHeight = screenRect.size.height;
    #else // Linux
        Screen* screen = DefaultScreenOfDisplay(display);
        g_state.screenWidth = screen->width;
        g_state.screenHeight = screen->height;
    #endif
    
    g_state.x = rand() % (g_state.screenWidth - g_state.width);
    g_state.y = rand() % (g_state.screenHeight - g_state.height);
}

void UpdateFrame() {
    // Update position
    g_state.x += g_state.dx;
    g_state.y += g_state.dy;

    // Bounce off edges
    if (g_state.x < 0 || g_state.x + g_state.width > g_state.screenWidth) {
        g_state.dx = -g_state.dx;
    }
    if (g_state.y < 0 || g_state.y + g_state.height > g_state.screenHeight) {
        g_state.dy = -g_state.dy;
    }
}

#ifndef _WIN32 // Main for Linux and MacOS
int main() {
    #ifdef __APPLE__
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        NSApplication* app = [NSApplication sharedApplication];
        NSWindow* window = [[NSWindow alloc] 
            initWithContentRect:NSMakeRect(0, 0, 800, 600)
            styleMask:NSWindowStyleMaskTitled
            backing:NSBackingStoreBuffered
            defer:NO];
        
        ScreenSaverView* view = [[ScreenSaverView alloc] initWithFrame:[window frame]];
        [window setContentView:view];
        [window makeKeyAndOrderFront:nil];
        
        InitScreenSaver();
        
        [NSTimer scheduledTimerWithTimeInterval:0.016 // ~60 FPS
                                        target:view
                                      selector:@selector(setNeedsDisplay:)
                                      userInfo:nil
                                       repeats:YES];
        
        [app run];
        [pool release];
    #else // Linux
        display = XOpenDisplay(NULL);
        if (!display) return 1;
        
        Window root = DefaultRootWindow(display);
        window = XCreateSimpleWindow(display, root, 0, 0, 800, 600, 1, 0, 0);
        
        XSelectInput(display, window, ExposureMask | KeyPressMask);
        XMapWindow(display, window);
        
        gc = XCreateGC(display, window, 0, NULL);
        
        InitScreenSaver();
        
        while (1) {
            XEvent event;
            while (XPending(display)) {
                XNextEvent(display, &event);
                if (event.type == KeyPress) return 0;
            }
            
            UpdateFrame();
            
            XClearWindow(display, window);
            XFillRectangle(display, window, gc, g_state.x, g_state.y, 
                          g_state.width, g_state.height);
            XFlush(display);
            
            struct timespec ts = {0, 16666667}; // ~60 FPS
            nanosleep(&ts, NULL);
        }
        
        XCloseDisplay(display);
    #endif
    return 0;
}
#endif