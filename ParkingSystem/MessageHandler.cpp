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
Description:    Constructor of tooltip class
*/
IceToolTip::IceToolTip() {
	//Create a tooltip window
	hWndToolTip = CreateWindowEx(0, L"tooltips_class32", L"", WS_POPUP | TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0, GetProgramInstance(), 0);
	SendMessage(hWndToolTip, TTM_SETMAXTIPWIDTH, 0, 1000);
}

/*
Description:    Destructor of tooltip class
*/
IceToolTip::~IceToolTip() {
	//Destroy the tooltip window
	DestroyWindow(hWndToolTip);
}

/*
Description:	Set tooltip for a control
Args:			hWndTargetCtl: Handle to the target control
				ToolTip: Tooltip string
*/
void IceToolTip::SetToolTip(HWND hWndTargetCtl, wchar_t *ToolTipText) {
	TTTOOLINFO	ti = { 0 };																		//Tooltip info

	ti.cbSize = sizeof(ti);
	ti.hwnd = hWndTargetCtl;																	//Set target control handle
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;													//Set tooltip style
	ti.uId = (UINT_PTR)hWndTargetCtl;
	ti.lpszText = ToolTipText;																	//Set tooltip text
	SendMessage(hWndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);										//Bind tooltip with control
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
				Bold: Whether the font is bold or not
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
Description:	Constructor of the checkbox control
Args:           ParentHwnd: The parent window of the control
				CtlID: Control ID, usually defined in resource.h
				Event: CheckBox_Click() handler for the button
*/
IceCheckBox::IceCheckBox(HWND ParentHwnd, int CtlID, VOID_EVENT Event) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	GetWindowRect(hWnd, &CtlRect);
	SetProp(hWnd, L"ClickEvent", (HANDLE)Event);
}

/*
Description:    Get checked state of the checkbox control
Return:			true if the chekbox is checked, false otherwise
*/
bool IceCheckBox::GetChecked() {
	return (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

//============================================================================
/*
Description:	Constructor of the combobox control
Args:           ParentHwnd: The parent window of the control
				CtlID: Control ID, usually defined in resource.h
*/
IceComboBox::IceComboBox(HWND ParentHwnd, int CtlID) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	GetWindowRect(hWnd, &CtlRect);
}

/*
Description:    Get index of selected item of the combobox control
Return:			Index of the selected item
*/
int IceComboBox::GetSelItem() {
	return (int)SendMessage(hWnd, CB_GETCURSEL, 0, 0);
}

/*
Description:    Set index of selected item of the combobox control
Args:			Index: Index of the item you want to be selected
*/
void IceComboBox::SetSelItem(int Index) {
	SendMessage(hWnd, CB_SETCURSEL, (WPARAM)Index, 0);
}

/*
Description:    Get index of selected item of the combobox control
Args:			Index: Index of the item you want to be selected
Return:			Index of the selected item
*/
int IceComboBox::AddItem(wchar_t *Text) {
	return (int)SendMessage(hWnd, CB_INSERTSTRING, -1, (LPARAM)Text);
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
	LVCOLUMN	lvCol = { 0 };																	//ListView column info

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

/*
Description:    Get the selected tab index
Return:			Index to of the selected tab
*/
int IceTab::GetSel() {
	return (int)SendMessage(hWnd, TCM_GETCURSEL, 0, 0);
}

/*
Description:    Set the selected tab index
Args:			Index: Specific tab index
Return:			The previously selected tab, or -1 otherwise
*/
int IceTab::SetSel(int Index) {
	return SendMessage(hWnd, TCM_SETCURSEL, Index, 0);
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

	SelectObject(hDC, GetStockObject(NULL_BRUSH));													//Make rectangles background be transparent
	SetBkMode(hDC, TRANSPARENT);																	//Make output text background be transparent
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
	VOID_EVENT PaintEvent, MOUSEMOVE_EVENT MouseMoveEvent, VOID_EVENT DblClickEvent) {
	ColorBrush = CreateSolidBrush(BackColor);														//Create background brush
	PaintEventFunction = PaintEvent;																//Record event functions
	MouseMoveEventFunction = MouseMoveEvent;
	DblClickEventFunction = DblClickEvent;
	hWnd = CreateWindowEx(0, L"#32770", L"Canvas", WS_VISIBLE | WS_CHILD,
		0, 0, 100, 100, ParentHwnd, (HMENU)NULL, GetProgramInstance(), NULL);
	GetWindowRect(hWnd, &CtlRect);
	CreateMemoryDC(100, 100);																		//Create initial memory DC
	Cls();
	SetProp(hWnd, L"Object", (HANDLE)this);															//Save a pointer to this class
	PervWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)CanvasWndProc);					//Store previous window procedure and set the new procedure
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
}

/*
Description:    Set canvas pen properties
Args:           Width: Pen width, default = 1
				PenColor: Pen color, default = 0 (black)
				PenStyle: Pen style, default = PS_SOLID (solid lines)
*/
void IceCanvas::SetPenProps(int Width, COLORREF PenColor, int PenStyle) {
	if (CanvasPen)																					//Delete previous pen
		DeleteObject(CanvasPen);
	CanvasPen = CreatePen(PenStyle, Width, PenColor);												//Create a new pen with specified style
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
	case WM_SIZE:																//Window resizing
		//Re-create memory DC & bitmap with the new size
		int Width, Height;

		Width = LOWORD(lParam);
		Height = HIWORD(lParam);
		ThisCanvas->CreateMemoryDC(Width, Height);
		InvalidateRect(ThisCanvas->hWnd, NULL, TRUE);

		//Invoke paint event
		if (ThisCanvas->PaintEventFunction)
			ThisCanvas->PaintEventFunction();

		break;

	case WM_ERASEBKGND:															//Window redraw
		//Paint contents from memory DC
		//Note that for this message, wParam is hDC of the window
		BitBlt((HDC)wParam, 0, 0,
			ThisCanvas->bi.bmiHeader.biWidth,
			ThisCanvas->bi.bmiHeader.biHeight,
			ThisCanvas->hDC, 0, 0, SRCCOPY);

		if (ThisCanvas->PaintEventFunction)											//Invoke paint event
			ThisCanvas->PaintEventFunction();
		return 0;

	case WM_LBUTTONDBLCLK:														//Left button double clicked
		if (ThisCanvas->DblClickEventFunction)										//Invoke double click event
			ThisCanvas->DblClickEventFunction();
		break;

	case WM_MOUSEMOVE:															//Mouse move
		if (ThisCanvas->MouseMoveEventFunction)
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
Description:    Constructor of the date/time picker control
Args:           ParentHwnd: The parent window of the control
				CtlID: Control ID, usually defined in resource.h
				ChangedEvent: TimePicker_TimeChanged() handler
*/
IceDateTimePicker::IceDateTimePicker(HWND ParentHwnd, int CtlID, VOID_EVENT ChangedEvent) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	GetWindowRect(hWnd, &CtlRect);
	SetProp(hWnd, L"ChangedEvent", (HANDLE)ChangedEvent);
}

/*
Description:    Get time value of the control
Args:           lpTime: Pointer to a SYSTEMTIME variable to store the time value
Return:			true if successful, false otherwise
*/
bool IceDateTimePicker::GetTime(SYSTEMTIME *lpTime) {
	if (lpTime)																						//Check if lpTime is NULL pointer
		return !SendMessage(hWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)lpTime);								//The message returns GDT_VALID (0) if successful
	else
		return false;
}

/*
Description:    Set time value of the control
Args:           lpTime: Pointer to a SYSTEMTIME variable that stores the time value
Return:			true if successful, false otherwise
*/
bool IceDateTimePicker::SetTime(SYSTEMTIME *lpTime) {
	if (lpTime)
		return SendMessage(hWnd, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)lpTime) != 0;
	else
		return SendMessage(hWnd, DTM_SETSYSTEMTIME, GDT_NONE, (LPARAM)lpTime) != 0;
}

//============================================================================
/*
Description:    Constructor of the slider control
Args:           ParentHwnd: The parent window of the control
				CtlID: Control ID, usually defined in resource.h
				ChangedEvent: Slider_ValueChanged() handler
*/
IceSlider::IceSlider(HWND ParentHwnd, int CtlID, VOID_EVENT ChangedEvent) {
	hWnd = GetDlgItem(ParentHwnd, CtlID);
	GetWindowRect(hWnd, &CtlRect);
	SetProp(hWnd, L"ChangedEvent", (HANDLE)ChangedEvent);
	SetProp(hWnd, L"PrevWndProc", (HANDLE)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)SliderWndProc));	//Store previous window procedure and set the new procedure
}

/*
Description:    Slider window procedure
Args:           hWnd: Handle to the window
				uMsg: Message code
				wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK IceSlider::SliderWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == SBM_SETSCROLLINFO) {																//Slider value changed
		((VOID_EVENT)(GetProp(hWnd, L"ChangedEvent")))();
	}
	else if (uMsg == WM_MOUSEWHEEL) {																//Using mouse wheel to change slider value
		int nPos = SendMessage(hWnd, TBM_GETPOS, 0, 0);													//Get current value

		if ((short)HIWORD(wParam) > 0)																	//Modify value according to scroll direction
			nPos += SendMessage(hWnd, TBM_GETLINESIZE, 0, 0);
		else
			nPos -= SendMessage(hWnd, TBM_GETLINESIZE, 0, 0);
		
		SendMessage(hWnd, TBM_SETPOS, TRUE, nPos);														//Set new value
		SetScrollPos(hWnd, SB_CTL, nPos, TRUE);															//Raise value changed event
		return 0;																						//Don't use system default procedure
	}
	else if (uMsg == WM_KEYDOWN) {																	//Using keyboard to change slider value
		//Invert some keys to make it more user-friendly
		if (wParam == VK_UP)																			//Invert Up key and Down key
			wParam = VK_DOWN;
		else if (wParam == VK_DOWN)
			wParam = VK_UP;
		else if (wParam == VK_PRIOR)																	//Invert PageUp key and PageDown key
			wParam = VK_NEXT;
		else if (wParam == VK_NEXT)
			wParam = VK_PRIOR;
	}
	
	//Call the default window prodecure
	return CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam);
}

/*
Description:    Set tick frequency (i.e. the space between every tick) of the slider
Args:           TickFreq: Tick frequency
*/
void IceSlider::SetTickFreq(int TickFreq) {
	SendMessage(hWnd, TBM_SETTICFREQ, (WPARAM)TickFreq, 0);
}

/*
Description:    Set max value of the slider
Args:           Max: Max value
*/
void IceSlider::SetMax(int Max) {
	SendMessage(hWnd, TBM_SETRANGEMAX, TRUE, Max);
}

/*
Description:    Set change in value per small change
Args:           SmallChange: Change in value per small changes
*/
void IceSlider::SetSmallChange(int SmallChange) {
	SendMessage(hWnd, TBM_SETLINESIZE, 0, SmallChange);
}

/*
Description:    Set change in value per large change
Args:           SmallChange: Change in value per large changes
*/
void IceSlider::SetLargeChange(int LargeChange) {
	SendMessage(hWnd, TBM_SETPAGESIZE, 0, LargeChange);
}

/*
Description:    Set position of the slider
Args:           Pos: New position
*/
void IceSlider::SetPos(int Pos) {
	SendMessage(hWnd, TBM_SETPOS, TRUE, Pos);
}

/*
Description:    Get position of the slider
Return:			Current position
*/
int IceSlider::GetPos() {
	return SendMessage(hWnd, TBM_GETPOS, 0, 0);
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
		((void(*)(HWND))lParam)(hWnd);												//Invoke MainWindow_Create()
		break;

	case WM_NOTIFY:																//Control notifications
		switch (((NMHDR*)lParam)->code) {
		case TCN_SELCHANGE:																//Tab selected
			//Invoke Tab_SelectedTab()
			((VOID_EVENT)(GetProp(((NMHDR*)lParam)->hwndFrom, L"SelectedTabEvent")))();
			break;

		case HDN_ITEMCLICK:																//ListView header clicked
			if (((NMHEADER*)lParam)->iButton == 0)											//Left button clicked
				//Invoke ListView_HeaderClicked()
				((void(*)(int))(GetProp(((NMHDR*)lParam)->hwndFrom, L"HeaderClickEvent")))(((NMHEADER*)lParam)->iItem);
			break;

		case DTN_DATETIMECHANGE:														//TimePicker changed
			//Invoke TimePicker_DateTimeChanged()
			VOID_EVENT lpfnChangedEvent = (VOID_EVENT)GetProp(((NMHDR*)lParam)->hwndFrom, L"ChangedEvent");
			if (lpfnChangedEvent)
				lpfnChangedEvent();
			break;
		}
		break;

	case WM_HSCROLL:
	case WM_VSCROLL:															//Slider position changed
		SetScrollPos((HWND)lParam, SB_CTL, HIWORD(wParam), TRUE);					//Update slider position
		break;

	case WM_COMMAND:															//Control commands
		if (lParam != 0) {															//Notification from a control
			switch (HIWORD(wParam)) {
			case BN_CLICKED:															//Button clicked
				((VOID_EVENT)(GetProp((HWND)lParam, L"ClickEvent")))();						//Invoke Button_Click() or CheckBox_Click()
				break;

			case CBN_EDITCHANGE:														//Combobox selection changed
			case CBN_SELCHANGE:
				SetFocus(GetDlgItem(hWnd, IDC_SEARCHHOURSEDIT));
				break;
			}
		}
		else {
			if (HIWORD(wParam) == 0) {													//Notification from a menu
				switch (LOWORD(wParam)) {													//Get menu ID
				case ID_FILE_PARKINGLOG:														//Show parking log
					mnuLog_Click();
					break;

				case ID_FILE_SEARCHLOG:															//Search for parking log
					mnuSearchLog_Click();
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

				case ID_HELP_HOWTOUSE:															//How to use
					mnuHowToUse_Click();
					break;

				case ID_HELP_ABOUT:																//About
					mnuAbout_Click();
					break;
				}
			}
		}
		break;

	case WM_SIZE:																//Window resizing
		//Invoke Window_Resize()
		MainWindow_Resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CTLCOLORSTATIC:														//Redrawing background of static controls
		if ((int)GetMenu((HWND)lParam) == IDC_HISTORYTIMESLIDER)					//Handle this message for the slider control only
			return (LRESULT)GetStockObject(WHITE_BRUSH);								//Return a white brush to draw the background
		else
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
Description:    Settings window procedure
Args:           hWnd: Handle to the window
				uMsg: Message code
				wParam, lParam: Extra infos
Return:         Result of message handling
*/
INT_PTR CALLBACK SettingsWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:															//Window created
		((void(*)(HWND))lParam)(hWnd);												//Invoke MainWindow_Create()

	case WM_COMMAND:															//Control commands
		if (lParam != 0) {															//Notification from a control
			if (HIWORD(wParam) == BN_CLICKED) {											//Button clicked notification code															/
				((VOID_EVENT)(GetProp((HWND)lParam, L"ClickEvent")))();						//Invoke Button_Click()
			}
		}
		break;

	case WM_CLOSE:																//Window closing
		EnableWindow(hwndMainWindow, TRUE);											//Enable main window
		DestroyWindow(hWnd);														//Close the window
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
			(wParam != VK_DELETE) && (wParam != VK_BACK))						//Accept numbers, letters and delete keys only, reject other characters

			return 0;
	}
	else if (uMsg == WM_PASTE)												//Block paste message
		return 0;

	//Call the default window prodecure of the editbpx
	return CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam);
}

/*
Description:    Search car number editbox procedure
Args:           hWnd: Handle to the window
				uMsg: Message code
				wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK SearchCarNumberEditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	EDIT_PROC_DEFAULT_OPERATION

	if (uMsg == WM_CHAR) {													//Key pressed message
		if (wParam == VK_RETURN) {												//Enter key
			GetDlgItem(hwndMainWindow, IDC_SEARCHBUTTON);
			return 0;
		}
		else if ((wParam > 'z' || wParam < 'a') &&
			(wParam > 'Z' || wParam < 'A') &&
			(wParam > '9' || wParam < '0') &&
			(wParam != VK_DELETE) && (wParam != VK_BACK) &&
			(wParam != '#') && (wParam != '@') &&
			(wParam != '*') && (wParam != '?'))									//Only accept numbers, letters, delete keys, #, @, *, ?

			return 0;
	}
	else if (uMsg == WM_PASTE)												//Block paste message
		return 0;

	//Call the default window prodecure of the editbpx
	return CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam);
}

/*
Description:    Window procedure for editboxes in settings window. When the user presses Enter,
				input focus should be changed to next control automacally
Args:           hWnd: Handle to the window
				uMsg: Message code
				wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK SettingsWindowEditBoxWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	EDIT_PROC_DEFAULT_OPERATION

	if (uMsg == WM_CHAR) {													//Key pressed message
		Editbox_KeyPressed(wParam, (int)GetMenu(hWnd));							//Handle the pressed key
		return 0;
	}

	//Call the default window prodecure of the editbpx
	return CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam);;
}