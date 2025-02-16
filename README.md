# Lumon Bouncing Logo Screensaver

A simple Windows screensaver that displays a bouncing Lumon logo.

## Building

1. Open the solution in Visual Studio
2. Make sure you have the Windows SDK installed
3. Build the solution in Release mode
4. Rename the output .exe file to .scr
5. Right-click the .scr file and choose "Install" to install it as a screensaver

## Installation

After building:
1. Copy the .scr file to `C:\Windows\System32`
2. Right-click the file and select "Install"
3. The screensaver will appear in Windows screensaver settings

## Development

The screensaver uses Win32 API and GDI+ for rendering. The logo bounces off the screen edges with simple physics simulation.
