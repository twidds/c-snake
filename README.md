# c-snake
Creating snake game using C language and raylib

Environment used to build (you may need to adjust for your own setup):
 * cmake - Version 3.28.1
 * git (for windows) - 2.43.0
 * MinGW - 5.0.0 (GCC 13.2.0)
 * Windows SDK (includs gdi+ SDK) - 10.1.0.0

Controls:
 * esc - exit game
 * s - show/hide stats
 * p - pause
 * i - invincible mode
 * arrow keys - control snake


Build environment setup (MSYS MakeFiles):
1. Download and install cmake: https://cmake.org/download/ (**NOTE**: CMAKE 4.X.X is not supported)
2. Install MSYS2+MinGW following instructions from MSYS2: https://www.msys2.org/
3. Install mingw64 with uct64 using MSYS2: pacman -S mingw-w64-ucrt-x86_64-gcc
4. Install MSYS cmake using MSYS2: pacman -S make
5. Add mingw binary folder to PATH variable. Example: C:\msys64\ucrt64\bin
6. Add msys user binary folder to PATH variable. Example: C:\msys64\usr\bin

Build environment setup (MinGW Makefiles):
1. Download and install cmake: https://cmake.org/download/ (**NOTE**: CMAKE 4.X.X is not supported)
2. Install MSYS2: https://www.msys2.org/
3. Install mingw64 gcc with MSYS2: pacman -S mingw-w64-x86_64-gcc 
4. Install mingw64 make package: pacman -S mingw-w64-x86_64-make
5. Add mingw binary folder to PATH variable. Example: C:\msys64\mingw64\bin

Build (VSCode):
1. Install Visual Studio Code
2. Install the C++ extension
3. Install the CMake Tools Extension
4. Open the project folder in VS Code and select the appropriate build preset (e.g. mingw-build-release)
(https://devblogs.microsoft.com/cppblog/cmake-presets-integration-in-visual-studio-and-visual-studio-code/)
