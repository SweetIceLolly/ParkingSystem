/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.h
*/

#include <Windows.h>
#include <CommCtrl.h>
#include <fstream>
#include <vector>
#include <memory>
#include "resource.h"

/* Procedure declarations */
void RecordProgramInstance(HINSTANCE hInstance);										//This copys hInstance to ProgramInstance
HINSTANCE GetProgramInstance();															//This retrieves hInstance from ProgramInstance
HWND GetMainWindowHandle();																//This retrieves main window handle
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    //Main window procedure
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);        //Button control procedure
LRESULT CALLBACK PasswordEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	//Password editbox control procedure
LRESULT CALLBACK CarNumberEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	//Car number editbox control procedure

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
void MainWindow_Resize(HWND, int, int);			//Window_Resize
void btnLogin_Click();							//Login button click
void btnEnterOrExit_Click();					//Car enter/exit button click

/* Event types */
typedef void(*VOID_EVENT)();					//For void ***() events
typedef void(*MOUSEMOVE_EVENT)(int, int);		//For void ***(int, int) events

/* Description:		Timer class */
class IceTimer {
private:
	UINT_PTR			TimerID;				//Unique timer ID
	bool				bEnabled;				//If the timer is enabled

	static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime);

public:
	VOID_EVENT			TimerEventFunction;		//Timer_Timer() function
	UINT				Interval;				//Timer interval

	IceTimer(UINT Interval, VOID_EVENT Event, bool Enabled = false);
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
	RECT				CtlRect;				//Non realtime-update

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
	template<class ...Args> void SetText(const wchar_t *FormatString, Args&&... FormatParams);
};

/* Description:		Button control class */
class IceButton : public BasicCtl {
public:
	IceButton(HWND ParentHwnd, int CtlID, VOID_EVENT Event);
	void SetCaption(wchar_t *Caption);
};

/* Description:		Edit box class */
class IceEdit : public BasicCtl{
public:
	IceEdit(HWND ParentHwnd, int CtlID, WNDPROC Proc);
	void SetText(wchar_t *Text);
	void GetText(wchar_t *Buffer);
};

/* Description:		ListView class */
class IceListView : public BasicCtl {
public:
	IceListView(HWND ParentHwnd, int CtlID);
	LRESULT AddColumn(wchar_t *Text, int Width = 100, int Index = -1);
	template<class ...Args> LRESULT AddItem(const wchar_t *FormatString, int Index = -1, Args&&... FormatParams);
	template<class ...Args> LRESULT SetItemText(int Index, const wchar_t *FormatString, int SubItemIndex = 0, Args&&... FormatParams);
	void DeleteAllItems();
};

/* Description:		Tab class */
class IceTab : public BasicCtl {
public:
	IceTab(HWND ParentHwnd, int CtlID, VOID_EVENT SelectedEvent);
	LRESULT InsertTab(wchar_t *Text, int Index = -1);
	int GetSel();
	int SetSel(int Index);
};

/* Description:		Painting canvas class */
class IceCanvas : public BasicCtl {
private:
	HBRUSH				ColorBrush;				//Background color brush
	HPEN				CanvasPen = 0;			//Pen of the canvas
	WNDPROC				PervWndProc;			//Previous window procedure

	static LRESULT CALLBACK CanvasWndProc(HWND, UINT, WPARAM, LPARAM);
	void CreateMemoryDC(int Width, int Height);
	void DeleteMemoryDC();

public:
	BITMAPINFO			bi;						//Memory bitmap info strucutre
	HDC					hDC = 0;				//Canvas memory HDC
	HBITMAP				hBmp = 0;				//Canvas memory bitmap
	VOID_EVENT			PaintEventFunction;		//Canvas_Paint() event
	MOUSEMOVE_EVENT		MouseMoveEventFunction;	//Canvas_MouseMove() event

	IceCanvas(HWND ParentHwnd, COLORREF BackColor = 0xffffff,
		VOID_EVENT PaintEvent = NULL, MOUSEMOVE_EVENT MouseMoveEvent = NULL);
	~IceCanvas();
	void DestroyCanvas();
	template <class ...Args> void Print(int X, int Y, const wchar_t *FormatString, Args&&... FormatParams);
	void Cls();
	void SetPenProps(int Width = 1, COLORREF PenColor = 0, int PenStyle = PS_SOLID);
	void DrawLine(int FromX, int FromY, int ToX, int ToY);
	void DrawRect(int X1, int Y1, int X2, int Y2);
};

/* Description:		Date/Time picker control class */
class IceDateTimePicker : public BasicCtl {
public:
	IceDateTimePicker(HWND ParentHwnd, int CtlID, VOID_EVENT ChangedEvent);
	bool GetTime(SYSTEMTIME *lpTime);
};

/* ==================================================================================================
   ======================================= Template functions ======================================= */

/*
Description:    Set the caption of the label control
Args:           FormatString: Format string of new caption
				FormatParams: String parameters
*/
template<class ...Args>
void IceLabel::SetText(const wchar_t *FormatString, Args&&... FormatParams) {
	wchar_t buf[255];
	swprintf_s(buf, FormatString, std::forward<Args>(FormatParams)...);
	SetWindowText(hWnd, buf);
}

/*
Description:    Set the text of an existing item
Args:           Index: The index of the item
				FormatString: Format string of new caption
				SubItemIndex: The index of the subitem of the specified item. 0 means the main item. Default = 0
				FormatParams: String parameters
Return:			TRUE if successful, or FALSE otherwise
*/
template<class ...Args>
LRESULT IceListView::SetItemText(int Index, const wchar_t *FormatString, int SubItemIndex, Args&&... FormatParams) {
	LVITEM		lvi = { 0 };					//ListView item info
	wchar_t		buf[255];

	swprintf_s(buf, FormatString, std::forward<Args>(FormatParams)...);
	lvi.iSubItem = SubItemIndex;
	lvi.mask = LVIF_TEXT;																		//Specific text
	lvi.cchTextMax = lstrlenW(buf);
	lvi.pszText = buf;
	return SendMessage(hWnd, LVM_SETITEMTEXT, Index, (LPARAM)&lvi);
}

/*
Description:    Add a new list item to the listview
Args:           FormatString: Format string of the text of the new item
				Index: The index of the new column. -1 means at the end of the list. Default = -1
				FormatParams: String parameters
Return:			Index to of the new item if successful, or -1 otherwise
*/
template<class ...Args>
LRESULT IceListView::AddItem(const wchar_t *FormatString, int Index, Args&&... FormatParams) {
	LVITEM		lvi = { 0 };					//ListView item info
	wchar_t		buf[255];

	if (Index == -1)																			//Add the item to the end
		lvi.iItem = SendMessage(hWnd, LVM_GETITEMCOUNT, 0, 0);
	else																						//Add the item to the specified index
		lvi.iItem = Index;
	swprintf_s(buf, FormatString, std::forward<Args>(FormatParams)...);
	lvi.mask = LVIF_TEXT | LVIF_PARAM;															//Specific text
	lvi.cchTextMax = lstrlenW(buf);
	lvi.pszText = buf;
	return SendMessage(hWnd, LVM_INSERTITEM, 0, (LPARAM)&lvi);
}

/*
Description:    Print a text on canvas
Args:           X, Y: Text output position
				FormatString: Format string of the output text
				FormatParams: String parameters
*/
template <class ...Args>
void IceCanvas::Print(int X, int Y, const wchar_t *FormatString, Args&&... FormatParams) {
	wchar_t		buf[255];

	swprintf_s(buf, FormatString, std::forward<Args>(FormatParams)...);
	TextOut(hDC, X, Y, buf, lstrlenW(buf));
}