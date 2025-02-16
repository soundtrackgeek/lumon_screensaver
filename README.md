# Lumon Bouncing Logo Screensaver

A simple screensaver that displays a bouncing Lumon logo. Works as a regular window application on Linux and MacOS, and can be installed as a screensaver on Windows.

## Prerequisites

### Windows
1. Install MSYS2 from: https://www.msys2.org/
2. Open MSYS2 MINGW64 terminal and run:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb mingw-w64-x86_64-windows-default-manifest
   ```
3. Add `C:\msys64\mingw64\bin` to your PATH

### Linux
1. Install g++: `sudo apt-get install build-essential` (Ubuntu/Debian) or `sudo dnf install gcc-c++` (Fedora)
2. Install X11 development files: `sudo apt-get install libx11-dev` (Ubuntu/Debian) or `sudo dnf install libX11-devel` (Fedora)

### MacOS
1. Install Xcode Command Line Tools: `xcode-select --install`
2. Install Homebrew: https://brew.sh/
3. Install required libraries: `brew install pkg-config`

## Building

### Windows
First, compile the resource file:
```bash
windres resources.rc -O coff -o resources.res
```
Then compile the program:
```bash
g++ -o LumonScreensaver.exe LumonScreensaver.cpp resources.res -lgdiplus -lgdi32 -luser32 -lole32 -loleaut32 -luuid -mwindows -DUNICODE -D_UNICODE
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
- Windows: Win32 API and GDI+ for rendering (with COM for image loading)
- Linux: X11 for window management
- MacOS: Cocoa and Core Graphics

The logo bounces off the screen edges with simple physics simulation.

## Troubleshooting

### Windows
If you get compiler errors about missing headers or undefined types:
1. Make sure you installed MSYS2 and ran the pacman command listed above
2. Make sure you're using the MSYS2 MINGW64 version of g++
3. Verify your PATH includes C:\msys64\mingw64\bin
4. If using PowerShell/Command Prompt, run `g++ --version` to confirm you're using the MinGW version

### Resource Compilation
If you get errors about the resource file:
1. Make sure to compile resources.rc using windres first
2. Check that lumon_logo.png exists in the same directory as resources.rc
3. If windres fails, verify the PNG file is a valid image file
