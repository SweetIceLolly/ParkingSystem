/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.h
*/

#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"

void RecordProgramInstance(HINSTANCE hInstance);										//This copys hInstance to ProgramInstance
HINSTANCE GetProgramInstance();															//This retrieves hInstance from ProgramInstance
HWND GetMainWindowHandle();																//This retrieves main window handle
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    //Main window procedure
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);        //Button control procedure
LRESULT CALLBACK PasswordEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	//Password editbox control procedure

/* Main window menu events */
void mnuLog_Click();
void mnuReport_Click();
void mnuLock_Click();
void mnuExit_Click();
void mnuEnterPaymentMode_Click();
void mnuStatus_Click();
void mnuParkClosed_Click();
void mnuOptions_Click();
void mnuHowToUse_Click();
void mnuAbout_Click();

/* Main window events */
void MainWindow_Resize(HWND, int, int);		//Window_Resize
void btnLogin_Click();							//Login button click

/* Event types */
typedef void(*ButtonClickEvent)();              //Button_Click
typedef void(*MenuClickEvent)();				//Menu_Click
typedef void(*TimerEvent)();					//Timer_Timer

/* Description:		Timer class */
class IceTimer {
private:
	UINT_PTR			TimerID;				//Unique timer ID
	bool				bEnabled;				//If the timer is enabled

	static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime);

public:
	TimerEvent			TimerEventFunction;		//Timer_Timer() function
	UINT				Interval;				//Timer interval

	IceTimer(UINT Interval, TimerEvent Event, bool Enabled = false);
	~IceTimer();
	bool GetEnabled();
	void SetEnabled(bool Enabled);
};

/*
Description:		Basic control class
					This class contains basic functions for all controls
*/
class BasicCtl {
protected:
	HFONT				hFont = NULL;			//Font of the control

public:
	HWND				hWnd;					//Control handle
	RECT				CtlRect;				//non realtime-update

	~BasicCtl();
	void SetVisible(bool Visible);
	void SetEnabled(bool Enabled);
	void Move(int X, int Y);
	void Size(int Width, int Height);
	void SetFont(int FontSize, bool Bold);
};

/* Description:		Label (Static) control class */
class IceLabel : public BasicCtl {
public:
	IceLabel(HWND ParentHwnd, int CtlID);
	void SetText(wchar_t *Text);
};

/* Description:		Button control class */
class IceButton : public BasicCtl {
public:
	ButtonClickEvent    ClickEventFunction;		//Button_Click() function
	
	IceButton(HWND ParentHwnd, int CtlID, ButtonClickEvent Event);
	void SetCaption(wchar_t *Caption);
};

/* Description:		Edit box class */
class IceEdit : public BasicCtl{
public:
	IceEdit(HWND ParentHwnd, int CtlID, WNDPROC Proc);
	void SetText(wchar_t *Text);
	void GetText(wchar_t *Buffer);
};

/* Description:		Listview class */
class IceListView : public BasicCtl {
public:
	IceListView(HWND ParentHwnd, int CtlID);
	LRESULT AddColumn(wchar_t *Text, int Width = 100, int Index = -1);
	LRESULT AddItem(wchar_t *Text, int Index = -1);
	LRESULT SetItemText(int Index, wchar_t *Text, int SubItemIndex);
};