/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.cpp
*/

#include "MessageHandler.h"

/*
Referenced from: https://support.microsoft.com/en-us/help/102589/how-to-use-the-enter-key-from-edit-controls-in-a-dialog-box
Tell the system that this editbox wants all keys
This macro is only used in editbox procedures
*/
#define		EDIT_PROC_DEFAULT_OPERATION				\
	if (uMsg == WM_GETDLGCODE)						\
		return (DLGC_WANTALLKEYS |					\
			CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam));

HINSTANCE	ProgramInstance;						//Instance of the program
HWND		hwndMainWindow;							//Main window handle

//============================================================================
/*
Description:    Constructor of the timer class
Args:           Visible: New visibility status
*/
IceTimer::IceTimer(UINT Interval, VOID_EVENT Event, bool Enabled) {
	TimerEventFunction = Event;															//Set the event function
	IceTimer::Interval = Interval;														//Record the interval
	TimerID = SetTimer(hwndMainWindow, (UINT_PTR)this, Interval, (TIMERPROC)TimerProc);	//Create a timer with ID = pointer to this class
	SetEnabled(Enabled);																//Set the Enabled status
}

/*
Description:    Destructor of the timer class
*/
IceTimer::~IceTimer() {
	KillTimer(NULL, TimerID);															//Kill the timer
}

/*
Description:    Return Enabled property of the timer
Return:			true if the timer is enabled, false otherwise
*/
bool IceTimer::GetEnabled() {
	return bEnabled;
}

/*
Description:    Set Enabled property of the control
Args:           Enabled: New Enabled status
*/
void IceTimer::SetEnabled(bool Enabled) {
	bEnabled = Enabled;
	KillTimer(hwndMainWindow, TimerID);													//Kill the timer anyway
	if (Enabled)																		//If Enabled = true, re-create the timer
		TimerID = SetTimer(hwndMainWindow, (UINT_PTR)this, Interval, (TIMERPROC)TimerProc);
}

/*
Description:	Timer procedure
Args:			hWnd: Handle to the window for timer messages
				uMsg: WM_TIMER message
				idTimer: Timer identifier
				dwTime: Current system time
*/
void CALLBACK IceTimer::TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime) {
	//Invoke Timer_Timer(). Note that idTimer is a ppinter to this class
	((IceTimer*)idTimer)->TimerEventFunction();
}

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
void BasicCtl::Move(int X, int Y) {
	SetWindowPos(hWnd, 0, X, Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

/*
Description:    Set the enabled status of the control
Args:           Width, Height: The new size of the control
*/
void BasicCtl::Size(int Width, int Height) {
	SetWindowPos(hWnd, 0, 0, 0, Width, Height, SWP_NOZORDER | SWP_NOMOVE);
}

/*
Description:    Set the font of the control
Args:           FontSize: New font size
Bold:			Whether the font is bold or not
*/
void BasicCtl::SetFont(int FontSize, bool Bold) {
	if (hFont)																					//Release the font object before a new one is created
		DeleteObject(hFont);

	//Referenced from: https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/nf-wingdi-createfonta
	hFont = CreateFont(FontSize, 0, 0, 0,
		Bold ? FW_BOLD : 0, 0, 0, 0,
		DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, VARIABLE_PITCH, NULL);												//Create the new font
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));							//Apply the new font
}

/*
Description:    Destructor of the control
*/
BasicCtl::~BasicCtl() {
	//Release the font object when the class is being destructed
	DeleteObject(hFont);
}

//============================================================================
/*
Description:    Constructor of the label control
Args:           ParentHwnd: The parent window of the label
				CtlID: Control ID, usually defined in resource.h
*/
IceLabel::IceLabel(HWND ParentHwnd, int CtlID) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	GetWindowRect(hWnd, &CtlRect);
}

//============================================================================
/*
Description:    Constructor of the button control
Args:           ParentHwnd: The parent window of the button
				CtlID: Control ID, usually defined in resource.h
				Event: Button_Click() handler for the button
*/
IceButton::IceButton(HWND ParentHwnd, int CtlID, VOID_EVENT Event) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	GetWindowRect(hWnd, &CtlRect);
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
Description:    Constructor of the editbox control
Args:           ParentHwnd: The parent window of the editbox
				CtlID: Control ID, usually defined in resource.h
*/
IceEdit::IceEdit(HWND ParentHwnd, int CtlID, WNDPROC Proc) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	if (Proc)																					//If a window procedure is specified
		SetProp(hWnd, L"PrevWndProc", (HANDLE)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)Proc));		//Store previous window procedure and set the new procedure
	GetWindowRect(hWnd, &CtlRect);
}

/*
Description:    Set the text of the button control
Args:           Caption: New text
*/
void IceEdit::SetText(wchar_t *Text) {
	SetWindowText(hWnd, Text);
}

/*
Description:    Set the caption of the button control
Args:           Caption: New caption
*/
void IceEdit::GetText(wchar_t *Buffer) {
	GetWindowText(hWnd, Buffer, 255);
}

//============================================================================
/*
Description:    Constructor of the listview control
Args:           ParentHwnd: The parent window of the listview
				CtlID: Control ID, usually defined in resource.h
*/
IceListView::IceListView(HWND ParentHwnd, int CtlID) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	SendMessage(hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);											//Add full-row select style for the listview
	GetWindowRect(hWnd, &CtlRect);
}

/*
Description:    Add a new column to the listview
Args:           Text: The text of the new column
				Width: The width of the new column. Default = 100
				Index: The index of the new column. -1 means at the end of column headers. Default = -1
Return:			Index to of the new column if successful, or -1 otherwise
*/
LRESULT IceListView::AddColumn(wchar_t *Text, int Width, int Index) {
	LVCOLUMN	lvCol = { 0 };																	//Listview column info

	lvCol.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_FMT;												//Specific width, text and text alignment
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.cchTextMax = lstrlenW(Text);
	lvCol.pszText = Text;
	lvCol.cx = Width;
	if (Index == -1)																			//Add the column header to the end
		return SendMessage(hWnd, LVM_INSERTCOLUMN,
			SendMessage(																				//Get the total number of column headers
				(HWND)SendMessage(hWnd, LVM_GETHEADER, 0, 0),												//Get the handle to the column header
				HDM_GETITEMCOUNT, 0, 0),
			(LPARAM)&lvCol);
	else																						//Add the column header to the specified index
		return SendMessage(hWnd, LVM_INSERTCOLUMN, Index, (LPARAM)&lvCol);
}

/*
Description:    Clear all list items in the listview
*/
void IceListView::DeleteAllItems() {
	SendMessage(hWnd, LVM_DELETEALLITEMS, 0, 0);
}

//============================================================================
/*
Description:    Constructor of the tab control
Args:           ParentHwnd: The parent window of the tab
				CtlID: Control ID, usually defined in resource.h
				SelectedEvent: Tab_SelectedTab() handler for the tab
*/
IceTab::IceTab(HWND ParentHwnd, int CtlID, VOID_EVENT SelectedEvent) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	GetWindowRect(hWnd, &CtlRect);
	SetProp(hWnd, L"SelectedTabEvent", (HANDLE)SelectedEvent);
}

/*
Description:    Add a new item to the tab
Args:           Text: The text of the new item
				Index: The index of the new item. -1 means at the end. Default = -1
Return:			Index to of the new item if successful, or -1 otherwise
*/
LRESULT IceTab::InsertTab(wchar_t *Text, int Index) {
	TCITEM tci = { 0 };																			//Tab control item info
	tci.mask = TCIF_TEXT;
	tci.cchTextMax = 255;
	tci.pszText = Text;																			//Specific text
	if (Index == -1)																			//Add the column header to the end
		Index = SendMessage(hWnd, TCM_GETITEMCOUNT, 0, 0);											//Get total item count of the tab
	return (int)SendMessage(hWnd, TCM_INSERTITEM, Index, (LPARAM)&tci);
}

//============================================================================
/*
Description:	Create memory DC & bitmap for canvas
Args:			Width, Height: Memory DC size
*/
void IceCanvas::CreateMemoryDC(int Width, int Height) {
	//Set bitmap info
	memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biBitCount = 32;																	//32-bit color
	bi.bmiHeader.biSize = sizeof(bi);
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biWidth = Width;
	bi.bmiHeader.biHeight = Height;
	bi.bmiHeader.biSizeImage = Width * Height * 32 / 8;												//Calculate bitmap image size

	hDC = CreateCompatibleDC(NULL);																	//Create canvas memory DC
	hBmp = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, NULL, NULL, 0);								//Create canvas memory bitmap
	SelectObject(hDC, hBmp);																		//Bind DC & bitmap
}

/*
Description:	Delete memory DC & bitmap
*/
void IceCanvas::DeleteMemoryDC() {
	if (hDC)
		DeleteDC(hDC);
	if (hBmp)
		DeleteObject(hBmp);
}

/*
Description:    Constructor of canvas
Args:           ParentHwnd: The parent window of the canvas
				BackColor: Background color of the canvas. Default is 0xffffff (white)
*/
IceCanvas::IceCanvas(HWND ParentHwnd, COLORREF BackColor,
	MOUSEMOVE_EVENT ResizeEvent, MOUSEMOVE_EVENT MouseMoveEvent) {
	ColorBrush = CreateSolidBrush(BackColor);														//Create background brush
	ResizeEventFunction = ResizeEvent;																//Record event functions
	MouseMoveEventFunction = MouseMoveEvent;
	hWnd = CreateWindowEx(0, L"#32770", L"Canvas", WS_VISIBLE | WS_CHILD,
		0, 0, 100, 100, ParentHwnd, (HMENU)NULL, GetProgramInstance(), NULL);
	CreateMemoryDC(100, 100);																		//Create initial memory DC
	Cls();
	SetProp(hWnd, L"Object", (HANDLE)this);															//Save a pointer to this class
	PervWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)CanvasWndProc);					//Store previous window procedure and set the new procedure
	SetBkMode(hDC, TRANSPARENT);																	//Make output text background be transparent
}

/*
Description:    Destructor of canvas class
*/
IceCanvas::~IceCanvas() {
	DestroyCanvas();
}

/*
Description:    Clean canvas
*/
void IceCanvas::Cls() {
	RECT	rc = { 0, 0, bi.bmiHeader.biWidth, bi.bmiHeader.biHeight };
	FillRect(hDC, &rc, ColorBrush);
	InvalidateRect(hWnd, NULL, TRUE);
}

/*
Description:    Set canvas pen properties
Args:           Width: Pen width, default = 1
				PenColor: Pen color, default = 0 (black)
				PenStyle: Pen style, default = PS_SOLID (solid lines)
*/
void IceCanvas::SetPenProps(int Width = 1, COLORREF PenColor = 0, int PenStyle = PS_SOLID) {
	if (CanvasPen)																					//Delete previous pen
		DeleteObject(CanvasPen);
	CreatePen(PenStyle, Width, PenColor);															//Create a new pen with specified style
	SelectObject(hDC, CanvasPen);																	//Bind the new pen with memory DC
}

/*
Description:    Draw a line on canvas
Args:           FromX, FromY: Position of one of the endpoints of the line
				ToX, ToY: Position of the other endpoint of the line
*/
void IceCanvas::DrawLine(int FromX, int FromY, int ToX, int ToY) {
	MoveToEx(hDC, FromX, FromY, NULL);																//Move the pen to (FromX, FromY)
	LineTo(hDC, ToX, ToY);																			//Line from (FromX, FromY) to (ToX, ToY)
}

/*
Description:    Draw a rectangle on canvas
Args:           X1, Yi: Position of left-up endpoint of the rectangle
				X2, Y2: Position of right-down endpoint of the rectangle
*/
void IceCanvas::DrawRect(int X1, int Y1, int X2, int Y2) {
	Rectangle(hDC, X1, Y1, X2, Y2);
}

/*
Description:    Canvas child window procedure
Args:           hWnd: Handle to the window
                uMsg: Message code
                wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK IceCanvas::CanvasWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	IceCanvas*	ThisCanvas = (IceCanvas*)GetProp(hWnd, L"Object");									//Retrieve a pointer to this class

	switch (uMsg) {
	case WM_SIZE:
		//Remember current DC handle & bitmap handle
		HDC			PrevHDC;
		HBITMAP		PrevBmp;

		PrevHDC = ThisCanvas->hDC;
		PrevBmp = ThisCanvas->hBmp;

		//Re-create memory DC & bitmap with the new size
		int Width, Height;

		Width = LOWORD(lParam);
		Height = HIWORD(lParam);
		ThisCanvas->CreateMemoryDC(Width, Height);

		//Paint image from previous memory DC to the new memory DC
		BitBlt(ThisCanvas->hDC, 0, 0, Width, Height, PrevHDC, 0, 0, SRCCOPY);

		//Delete previous memory DC & bitmap
		DeleteDC(PrevHDC);
		DeleteObject(PrevBmp);

		//Invoke resize event
		ThisCanvas->ResizeEventFunction(LOWORD(lParam), HIWORD(lParam));

		break;

	case WM_ERASEBKGND:
		//Paint contents from memory DC
		//Note that for this message, wParam is hDC of the window
		BitBlt((HDC)wParam, 0, 0,
			ThisCanvas->bi.bmiHeader.biWidth,
			ThisCanvas->bi.bmiHeader.biHeight,
			ThisCanvas->hDC, 0, 0, SRCCOPY);
		return 0;

	case WM_MOUSEMOVE:
		ThisCanvas->MouseMoveEventFunction(LOWORD(lParam), HIWORD(lParam)); 
		return 0;
	}

	//Call the default window prodecure
	return CallWindowProc(ThisCanvas->PervWndProc, hWnd, uMsg, wParam, lParam);
}

/*
Description:    Close canvas child window
*/
void IceCanvas::DestroyCanvas() {
	DeleteObject(ColorBrush);																		//Release background color brush
	if (CanvasPen)																					//Release canvas pen
		DeleteObject(CanvasPen);
	DeleteMemoryDC();																				//Release memory DC & bitmap
	DestroyWindow(hWnd);																			//Close canvas window
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

/*
Description:    This retrieves handle to the main window from hwndMainWindow
Return:			Handle to the main window
*/
HWND GetMainWindowHandle() {
	return hwndMainWindow;
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
	case WM_INITDIALOG:															//Window created
		hwndMainWindow = hWnd;														//Record the window handle
		//Invoke MainWindow_Create()
		((void(*)(HWND))lParam)(hWnd);
		break;

	case WM_COMMAND:															//Control commands
		if (lParam != 0) {															//Notification from a control
			switch (HIWORD(wParam)) {													//Control notification code
			case BN_CLICKED:																//Button clicked
				//Invoke Button_Click()
				((VOID_EVENT)(GetProp((HWND)lParam, L"ClickEvent")))();
				break;
			}
		}
		else {
			if (HIWORD(wParam) == 0) {													//Notification from a menu
				switch (LOWORD(wParam)) {													//Get menu ID
				case ID_FILE_PARKINGLOG:														//Show parking log
					mnuLog_Click();
					break;

				case ID_FILE_REPORT:															//Show parking report
					mnuReport_Click();
					break;

				case ID_FILE_LOCKSYSTEM:														//Lock system
					mnuLock_Click();
					break;
				
				case ID_FILE_EXITSYSTEM:														//Exit system
					mnuExit_Click();
					break;

				case ID_STATUS_ENTER:															//Enter payment mode
					mnuEnterPaymentMode_Click();
					break;

				case ID_STATUS_OPTIONS:															//System options
					mnuOptions_Click();
					break;
				}
			}
		}
		break;

	case WM_SIZE:																//Window resizing
		//Invoke Window_Resize()
		MainWindow_Resize(hWnd, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:																//Window closing
		mnuExit_Click();
		return TRUE;
        
	default:
		return 0;
    }
	return 0;
}

/*
Description:    Password editbox procedure
Args:           hWnd: Handle to the window
                uMsg: Message code
                wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK PasswordEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	EDIT_PROC_DEFAULT_OPERATION
	
	if(uMsg == WM_CHAR) {														//Key pressed message
		if (wParam == VK_RETURN) {													//Enter key
			btnLogin_Click();
			return 0;
		}
		else if (wParam == VK_ESCAPE) {												//Esc key
			mnuExit_Click();
			return 0;
		}
	}
	
	//Call the default window prodecure of the editbpx
	return CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam);
}

/*
Description:    Car number editbox procedure
Args:           hWnd: Handle to the window
				uMsg: Message code
				wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK CarNumberEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	EDIT_PROC_DEFAULT_OPERATION
	
	if(uMsg == WM_CHAR) {													//Key pressed message
		if (wParam == VK_RETURN) {												//Enter key
			btnEnterOrExit_Click();
			return 0;
		}
		else if ((wParam > 'z' || wParam < 'a') &&
			(wParam > 'Z' || wParam < 'A') &&
			(wParam > '9' || wParam < '0') &&
			(wParam != VK_DELETE) &&
			(wParam != VK_BACK))												//Accept numbers, letters and delete keys only, reject other characters

			return 0;
	}
	else if (uMsg == WM_PASTE)												//Block paste message
		return 0;

	//Call the default window prodecure of the editbpx
	return CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam);
}