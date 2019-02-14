/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.h
*/

#include <Windows.h>

void RecordProgramInstance(HINSTANCE hInstance);										//This copys hInstance to ProgramInstance
HINSTANCE GetProgramInstance();															//This retrieves hInstance from ProgramInstance
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    //Main window procedure
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);        //Button control procedure

/* Event types */
typedef void(*WindowCreateEvent)();				//Window_Create
typedef void(*ButtonClickEvent)();              //Button_Click

/* Description:    Button control class */
class IceButton {
private:
    char                *Caption;				//Button caption
    HWND                hWnd;					//Button handle

public:
	ButtonClickEvent    ClickEventFunction;		//Button_Click function

	/*
	Description:    Constructor of the button control
	Args:           ParentHwnd: The parent window of the button
					CtlID: Control ID, usually defined in resource.h
					Event: Button_Click() handler for the button
	*/
    IceButton(HWND ParentHwnd, int CtlID, ButtonClickEvent Event) {
		ClickEventFunction = Event;
		hWnd = GetDlgItem(ParentHwnd, CtlID);
		SetProp(hWnd, L"Event", (HANDLE)Event);
    }

    /*
    Description:    Set the caption of the button control
    Args:           Caption: New caption
    */
    void SetCaption(wchar_t *Caption) {
        SetWindowText(hWnd, Caption);
    }

    /*
    Description:    Set the visiblility of the button control
    Args:           Visible: New visibility status
    */
    void SetVisible(bool Visible) {
        if (Visible)
            ShowWindow(hWnd, SW_SHOW);
        else
            ShowWindow(hWnd, SW_HIDE);
    }

    /*
    Description:    Set the enabled status of the button control
    Args:           Visible: New enabled status
    */
    void SetEnable(bool Enabled) {
        EnableWindow(hWnd, Enabled);
    }

	/*
	Description:    Get the caption of the button control
	Return:			Button caption
	Note:			For captions that less than 255 bytes only
	*/
	wchar_t* GetCaption() {
		wchar_t buffer[255];
		GetWindowText(hWnd, buffer, 255);
		return buffer;
	}
};