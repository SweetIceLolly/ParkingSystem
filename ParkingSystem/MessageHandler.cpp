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
IceTimer::IceTimer(UINT Interval, TimerEvent Event, bool Enabled) {
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
IceButton::IceButton(HWND ParentHwnd, int CtlID, ButtonClickEvent Event) {
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
IceTab::IceTab(HWND ParentHwnd, int CtlID, TabSelectionEvent SelectedEvent) {
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
Description:    Constructor of canvas
Args:           ParentHwnd: The parent window of the canvas
*/
IceCanvas::IceCanvas(HWND ParentHwnd) {
	hWnd = CreateWindowEx(0, L"#32770", L"Canvas", WS_VISIBLE | WS_CHILD,
		0, 0, 100, 100, ParentHwnd, (HMENU)NULL, GetProgramInstance(), NULL);
	//PrevWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)CanvasWndProc);
	SetProp(hWnd, L"PrevWndProc", (HANDLE)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)CanvasWndProc));	//Store previous window procedure and set the new procedure
}

/*
Description:    Canvas child window procedure
Args:           hWnd: Handle to the window
                uMsg: Message code
                wParam, lParam: Extra infos
Return:         Result of message handling
*/
LRESULT CALLBACK IceCanvas::CanvasWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	}

	//Call the default window prodecure
	return CallWindowProc((WNDPROC)GetProp(hWnd, L"PrevWndProc"), hWnd, uMsg, wParam, lParam);
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
				((ButtonClickEvent)(GetProp((HWND)lParam, L"ClickEvent")))();
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