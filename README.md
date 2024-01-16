# c-snake
Creating snake game using C language and WinAPI (GDI+)

Environment used to build (you may need to adjust for your own setup):
 * cmake - Version 3.28.1
 * git (for windows) - 2.43.0
 * MinGW - 5.0.0 (GCC 13.2.0)
 * Windows SDK (includs gdi+ SDK) - 10.1.0.0

Controls:
 * esc - exit game
 * s - show/hide stats
 * p - pause
 * arrow keys - control snake

References:
 * http://www.winprog.org/tutorial/bitmaps.html
 * https://learn.microsoft.com/en-us/windows/win32/gdi/windows-gdi
 * https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-gdi-start


TODO:
 * Add spritesheet support
 * Add rendering of game objects (tie sprites to world objects)
 * Add movement
 * Add collision handling
 * Add end game screen
 * Add command line parsing (allow variable x/y for grid and window)
 * Add smooth animations

 maybe TODO:
  * Move to GDI instead of GDI+ (would likely be huge performance gain) -> would probably fork project
  * Add better wrapping around graphics/window handlers (abstract away from OS level)
  * make game code platform independent
  * Add linux support