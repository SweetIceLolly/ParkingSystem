/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.cpp
*/

#include "MessageHandler.h"

/*
Description:    Re-register a window class
Args:           PrevClassName: Previous window class name
                NewClassName: New window class name
                lpfnPrevWndProc: WNDPROC typed variable to store previous window procedure
                lpfnNewWindowProc: New window procedure
*/
void ReregisterClass(LPCSTR PrevClassName, LPCSTR NewClassName,
                     WNDPROC *lpfnPrevWndProc, WNDPROC lpfnNewWindowProc) {
    WNDCLASSEX ctlClass = { 0 };
    GetClassInfoEx(ProgramInstance, PrevClassName, &ctlClass);
    *lpfnPrevWndProc = ctlClass.lpfnWndProc;
    ctlClass.lpfnWndProc = lpfnNewWindowProc;
    ctlClass.lpszClassName = NewClassName;
    ctlClass.cbSize = sizeof(WNDCLASSEX);
    RegisterClass(&ctlClass);
}


/*
Description:    Main window procedure
Args:           hWnd: Handle to the window
                uMsg: Message code
                wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:                              //Window closing
            DestroyWindow(hWnd);                        //Close the window and exit the program
            PostQuitMessage(0);
            break;
        
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

/*
Description:    Button control procedure
Args:           hWnd: Handle to the window
                uMsg: Message code
                wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK ButtonProc()