# Lumon Bouncing Logo Screensaver

A simple screensaver that displays a bouncing Lumon logo. Works as a regular window application on Linux and MacOS, and can be installed as a screensaver on Windows.

## Prerequisites

### Windows
1. Install MinGW-w64 from: https://www.mingw-w64.org/
2. Add MinGW-w64's bin directory to your PATH
3. Install the Windows SDK for GDI+ headers

### Linux
1. Install g++: `sudo apt-get install build-essential` (Ubuntu/Debian) or `sudo dnf install gcc-c++` (Fedora)
2. Install X11 development files: `sudo apt-get install libx11-dev` (Ubuntu/Debian) or `sudo dnf install libX11-devel` (Fedora)

### MacOS
1. Install Xcode Command Line Tools: `xcode-select --install`
2. Install Homebrew: https://brew.sh/
3. Install required libraries: `brew install pkg-config`

## Building

### Windows
```bash
g++ -o LumonScreensaver.exe LumonScreensaver.cpp resources.rc -lgdiplus -lgdi32 -luser32 -mwindows
```
To install as screensaver:
1. Copy LumonScreensaver.exe to LumonScreensaver.scr
2. Move LumonScreensaver.scr to C:\Windows\System32
3. Right-click and select "Install"

### Linux
```bash
g++ -o LumonScreensaver LumonScreensaver.cpp -lX11
```

### MacOS
```bash
g++ -o LumonScreensaver LumonScreensaver.cpp -framework Cocoa -framework CoreGraphics
```

## Note on Cross-Platform Usage
- On Windows: Works as a proper screensaver (.scr)
- On Linux/MacOS: Runs as a regular window application showing the bouncing logo
- Make sure lumon_logo.png is in the same directory as the executable

## Development

The application uses:
- Windows: Win32 API and GDI+ for rendering
- Linux: X11 for window management
- MacOS: Cocoa and Core Graphics

The logo bounces off the screen edges with simple physics simulation.
