/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.cpp
*/

#include "MessageHandler.h"

HINSTANCE ProgramInstance;                      //Instance of the program

/*
Description:    This copys hInstance to ProgramInstance
Args:           hInstance: Program hInstance
*/
void RecordProgramInstance(HINSTANCE hInstance) {
	ProgramInstance = hInstance;
}

/*
Description:    This retrieves hInstance from ProgramInstance
Return:			Program hInstance
*/
HINSTANCE GetProgramInstance(HINSTANCE hInstance) {
	return ProgramInstance;
}

/*
Description:    Re-register a window class
Args:           PrevClassName: Previous window class name
                NewClassName: New window class name
                lpfnPrevWndProc: WNDPROC typed variable to store previous window procedure
                lpfnNewWindowProc: New window procedure
*/
void ReregisterClass(LPCWSTR PrevClassName, LPCWSTR NewClassName,
                     WNDPROC *lpfnPrevWndProc, WNDPROC lpfnNewWindowProc) {
    WNDCLASSEXW ctlClass = { 0 };
    GetClassInfoEx(ProgramInstance, PrevClassName, &ctlClass);
    *lpfnPrevWndProc = ctlClass.lpfnWndProc;
    ctlClass.lpfnWndProc = lpfnNewWindowProc;
    ctlClass.lpszClassName = NewClassName;
    ctlClass.cbSize = sizeof(WNDCLASSEX);
    RegisterClassEx(&ctlClass);
}

/*
Description:    Main window procedure
Args:           hWnd: Handle to the window
                uMsg: Message code
                wParam, lParam: Extra infos
Return:         Result of message handling
*/
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
//LRESULT CALLBACK ButtonProc()