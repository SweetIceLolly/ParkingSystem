/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.h
*/

#include <Windows.h>
#include "resource.h"

void RecordProgramInstance(HINSTANCE hInstance);										//This copys hInstance to ProgramInstance
HINSTANCE GetProgramInstance();															//This retrieves hInstance from ProgramInstance
HWND GetMainWindowHandle();																//This retrieves main window handle
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    //Main window procedure
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);        //Button control procedure

/* Main window menu events */
void mnuLog_Click();
void mnuReport_Click();
void mnuExit_Click();
void mnuEnterPatmentMode_Click();
void mnuStatus_Click();
void mnuParkClosed_Click();
void mnuOptions_Click();
void mnuHowToUse_Click();
void mnuAbout_Click();

/* Event types */
typedef void(*WindowCreateEvent)();				//Window_Create
typedef void(*ButtonClickEvent)();              //Button_Click
typedef void(*MenuClickEvent)();				//Menu_Click

/* Description:		Button control class */
class IceButton {
private:
    char                *Caption;				//Button caption
    HWND                hWnd;					//Button handle

public:
	ButtonClickEvent    ClickEventFunction;		//Button_Click() function
	
	IceButton(HWND ParentHwnd, int CtlID, ButtonClickEvent Event);
	void SetCaption(wchar_t *Caption);
	void SetVisible(bool Visible);
	void SetEnabled(bool Enabled);
};
