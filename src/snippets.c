#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>


//Important functions:
//  SelectObject -> What does this do?
//  CreateFont
//  SetTextColor
//Reference: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-createfonta
LRESULT CALLBACK WinProc_SetFont(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    switch (message)
    {
    
    
    case WM_PAINT:
        {
        RECT rect;
        HFONT hFontOriginal, hFont1, hFont2, hFont3;
        hdc = BeginPaint(hWnd, &ps);

            
            //Logical units are device dependent pixels, so this will create a handle to a logical font that is 48 pixels in height.
            //The width, when set to 0, will cause the font mapper to choose the closest matching value.
            //The font face name will be Impact.
            hFont1 = CreateFont(48,0,0,0,FW_DONTCARE,FALSE,TRUE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH,TEXT("Impact"));
            hFontOriginal = (HFONT)SelectObject(hdc, hFont1);
            
            //Sets the coordinates for the rectangle in which the text is to be formatted.
            SetRect(&rect, 100,100,700,200);
            SetTextColor(hdc, RGB(255,0,0));
            DrawText(hdc, TEXT("Drawing Text with Impact"), -1,&rect, DT_NOCLIP);
            
            //Logical units are device dependent pixels, so this will create a handle to a logical font that is 36 pixels in height.
            //The width, when set to 20, will cause the font mapper to choose a font which, in this case, is stretched.
            //The font face name will be Times New Roman.  This time nEscapement is at -300 tenths of a degree (-30 degrees)
            hFont2 = CreateFont(36,20,-300,0,FW_DONTCARE,FALSE,TRUE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY, VARIABLE_PITCH,TEXT("Times New Roman"));
            SelectObject(hdc,hFont2);
            
            //Sets the coordinates for the rectangle in which the text is to be formatted.
            SetRect(&rect, 100, 200, 900, 800);
            SetTextColor(hdc, RGB(0,128,0));
            DrawText(hdc, TEXT("Drawing Text with Times New Roman"), -1,&rect, DT_NOCLIP);
                
            //Logical units are device dependent pixels, so this will create a handle to a logical font that is 36 pixels in height.
            //The width, when set to 10, will cause the font mapper to choose a font which, in this case, is compressed. 
            //The font face name will be Arial. This time nEscapement is at 250 tenths of a degree (25 degrees)
            hFont3 = CreateFont(36,10,250,0,FW_DONTCARE,FALSE,TRUE,FALSE,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY, VARIABLE_PITCH,TEXT("Arial"));
            SelectObject(hdc,hFont3);

            //Sets the coordinates for the rectangle in which the text is to be formatted.
            SetRect(&rect, 500, 200, 1400, 600);
            SetTextColor(hdc, RGB(0,0,255));
            DrawText(hdc, TEXT("Drawing Text with Arial"), -1,&rect, DT_NOCLIP);

            SelectObject(hdc,hFontOriginal);
            DeleteObject(hFont1);
            DeleteObject(hFont2);
            DeleteObject(hFont3);

        EndPaint(hWnd, &ps);
        break;
        }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


//We should be able to use this function in reverse to read in bitmap files: https://learn.microsoft.com/en-us/windows/win32/gdi/capturing-an-image
//Important structs:
//  HBITMAP
//  BITMAPFILEHEADER
//  BITMAPINFOHEADER
//Important functions:
//  CreateCompatibleBitmap -> Creates buffer of correct size/type to store bitmap (not sure if relevant for reading file)
//  BitBlt -> Moves a bunch of bits between source->dest device context. Can probably use to render bitmap
//  GetObject -> writes from an object handle to a buffer (e.g. HBITMAP -> BITMAP buffer)
int CaptureAnImage(HWND hWnd)
{
    HDC hdcScreen;
    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;
    DWORD dwBytesWritten = 0;
    DWORD dwSizeofDIB = 0;
    HANDLE hFile = NULL;
    char* lpbitmap = NULL;
    HANDLE hDIB = NULL;
    DWORD dwBmpSize = 0;

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    hdcScreen = GetDC(NULL);
    hdcWindow = GetDC(hWnd);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hdcWindow);

    if (!hdcMemDC)
    {
        MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the client area for size calculation.
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    // This is the best stretch mode.
    SetStretchBltMode(hdcWindow, HALFTONE);

    // The source DC is the entire screen, and the destination DC is the current window (HWND).
    if (!StretchBlt(hdcWindow,
        0, 0,
        rcClient.right, rcClient.bottom,
        hdcScreen,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        SRCCOPY))
    {
        MessageBox(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Create a compatible bitmap from the Window DC.
    hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

    if (!hbmScreen)
    {
        MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(hdcMemDC,
        0, 0,
        rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        hdcWindow,
        0, 0,
        SRCCOPY))
    {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP.
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char*)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer 
    // that's pointed to by lpbitmap.
    GetDIBits(hdcWindow, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap,
        (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // A file is created, this is where we will save the screen capture.
    hFile = CreateFile(L"captureqwsx.bmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    // Add the size of the headers to the size of the bitmap to get the total file size.
    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.

    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    CloseHandle(hFile);

    // Clean up.
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    ReleaseDC(hWnd, hdcWindow);

    return 0;
}