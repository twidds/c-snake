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
