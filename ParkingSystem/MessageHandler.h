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

/* Main window events */
void MainWindow_Resize(WORD, WORD);				//Window_Resize

/* Event types */
typedef void(*ButtonClickEvent)();              //Button_Click
typedef void(*MenuClickEvent)();				//Menu_Click

/*
Description:		Basic control class
					This class contains basic functions for all controls
*/
class BasicCtl {
public:
	HWND				hWnd;

	void SetVisible(bool Visible);
	void SetEnabled(bool Enabled);
	void Move(WORD X, WORD Y);
	void Size(WORD Width, WORD Height);
};

/* Description:		Button control class */
class IceButton : public BasicCtl {
private:
    char                *Caption;				//Button caption

public:
	ButtonClickEvent    ClickEventFunction;		//Button_Click() function
	
	IceButton(HWND ParentHwnd, int CtlID, ButtonClickEvent Event);
	void SetCaption(wchar_t *Caption);
};

/* Description:		Edit box class */
class IceEdit : public BasicCtl{
public:
	IceEdit(HWND ParentHwnd, int CtlID);
	void SetText(wchar_t *Text);
	void GetText(wchar_t *Buffer);
};