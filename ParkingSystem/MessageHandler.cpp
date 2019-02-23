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
Description:    Set the visiblility of the control
Args:           Visible: New visibility status
*/
void BasicCtl::SetVisible(bool Visible) {
	if (Visible)
		ShowWindow(hWnd, SW_SHOW);
	else
		ShowWindow(hWnd, SW_HIDE);
}

/*
Description:    Set the enabled status of the control
Args:           Enabled: New enabled status
*/
void BasicCtl::SetEnabled(bool Enabled) {
	EnableWindow(hWnd, Enabled);
}

/*
Description:    Set the position of the control
Args:           X, Y: The position of the control
*/
void BasicCtl::Move(WORD X, WORD Y) {
	SetWindowPos(hWnd, 0, X, Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

/*
Description:    Set the enabled status of the control
Args:           Width, Height: The new size of the control
*/
void BasicCtl::Size(WORD Width, WORD Height) {
	SetWindowPos(hWnd, 0, 0, 0, Width, Height, SWP_NOZORDER | SWP_NOREPOSITION);
}

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
HINSTANCE GetProgramInstance() {
	return ProgramInstance;
}

HWND GetMainWindowHandle() {
	return hwndMainWindow;
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
		//Invoke MainWindow_Create()
		((void(*)())lParam)();
		break;

	case WM_COMMAND:															//Control commands
		if (lParam != 0) {															//Notification from a control
			switch (HIWORD(wParam)) {													//Control notification code
			case BN_CLICKED:																//Button clicked
				//Invoke Button_Click()
				((ButtonClickEvent)(GetProp((HWND)lParam, L"ClickEvent")))();
				break;
			}
		}
		else {
			if (HIWORD(wParam) == 0) {													//Notification from a menu
				switch (LOWORD(wParam)) {													//Get menu ID
				case ID_FILE_EXITSYSTEM:														//Exit System
					mnuExit_Click();
				}
			}
		}
		break;

	case WM_SIZE:																//Window resizing
		//Invoke Window_Resize()
		MainWindow_Resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:																//Window closing
		mnuExit_Click();
		return TRUE;
        
	default:
		return 0;
    }
    return 0;
}