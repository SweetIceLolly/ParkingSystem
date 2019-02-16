/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.cpp
*/

#include "MessageHandler.h"

HINSTANCE	ProgramInstance;						//Instance of the program
HWND		hwndMainWindow;							//Main window handle

//============================================================================
/*
Description:    Constructor of the button control
Args:           ParentHwnd: The parent window of the button
				CtlID: Control ID, usually defined in resource.h
				Event: Button_Click() handler for the button
*/
IceButton::IceButton(HWND ParentHwnd, int CtlID, ButtonClickEvent Event) {
	ClickEventFunction = Event;
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	SetProp(hWnd, L"ClickEvent", (HANDLE)Event);
}

/*
Description:    Set the caption of the button control
Args:           Caption: New caption
*/
void IceButton::SetCaption(wchar_t *Caption) {
	SetWindowText(hWnd, Caption);
}

/*
Description:    Set the visiblility of the button control
Args:           Visible: New visibility status
*/
void IceButton::SetVisible(bool Visible) {
	if (Visible)
		ShowWindow(hWnd, SW_SHOW);
	else
		ShowWindow(hWnd, SW_HIDE);
}

/*
Description:    Set the enabled status of the button control
Args:           Enabled: New enabled status
*/
void IceButton::SetEnable(bool Enabled) {
	EnableWindow(hWnd, Enabled);
}

/*
Description:    Get the caption of the button control
Return:			Button caption
Note:			For captions that less than 255 bytes only
Todo:			Maybe delete?
*/
wchar_t* IceButton::GetCaption() {
	wchar_t buffer[255];
	GetWindowText(hWnd, buffer, 255);
	return buffer;
}

//============================================================================
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

HWND GetMainWindowHandle() {
	return hwndMainWindow;
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

//============================================================================
/*
Description:    Main window procedure
Args:           hWnd: Handle to the window
                uMsg: Message code
                wParam, lParam: Extra infos
Return:         Result of message handling
*/
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
	case WM_INITDIALOG:															//Window created
		hwndMainWindow = hWnd;														//Record the window handle
		((WindowCreateEvent)lParam)();													//Invoke MainWindow_Create()
		break;

	case WM_COMMAND:															//Control commands
		if (lParam != 0) {															//Notification from a control
			switch (HIWORD(wParam)) {													//Control notification code
			case BN_CLICKED:																//Button clicked
				((ButtonClickEvent)(GetProp((HWND)lParam, L"ClickEvent")))();					//Invoke Button_Click()
				break;
			}
		}
		else {
			if (HIWORD(wParam) == 0) {													//Notification from a menu
				
			}
		}
		break;

	case WM_CLOSE:																//Window closing
		DestroyWindow(hWnd);														//Close the window and exit the program
		PostQuitMessage(0);
		return TRUE;
        
	default:
		return 0;
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