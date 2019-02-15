/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.h
*/

#include <Windows.h>

void RecordProgramInstance(HINSTANCE hInstance);										//This copys hInstance to ProgramInstance
HINSTANCE GetProgramInstance();															//This retrieves hInstance from ProgramInstance
HWND GetMainWindowHandle();
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    //Main window procedure
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);        //Button control procedure

/* Event types */
typedef void(*WindowCreateEvent)();				//Window_Create
typedef void(*ButtonClickEvent)();              //Button_Click

/* Description:		Button control class */
class IceButton {
private:
    char                *Caption;				//Button caption
    HWND                hWnd;					//Button handle

public:
	ButtonClickEvent    ClickEventFunction;		//Button_Click() function

	/* Description: This removes the default constructor and = operator of the class */
	/*IceButton(const IceButton&) = delete;
	void operator=(const IceButton&) = delete;*/
	
	IceButton(HWND ParentHwnd, int CtlID, ButtonClickEvent Event);

	void SetCaption(wchar_t *Caption);

	void SetVisible(bool Visible);

	void SetEnable(bool Enabled);
	
	wchar_t* GetCaption();
};