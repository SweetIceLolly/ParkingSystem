/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "FileManager.h"

shared_ptr<IceEncryptedFile>	LogFile;
shared_ptr<IceEdit>				edPassword;
shared_ptr<IceButton>			btnLogin;
shared_ptr<IceListView>			lvLog;

/*
Description:    To handle main window resizing event
*/
void MainWindow_Resize(HWND hWnd, WORD Width, WORD Height) {
	lvLog->Size(Width, Height);
}

/*
Description:	To handle login button event
*/
void btnLogin_Click() {
	//Try to decrypt the file
	wchar_t		Password[20];
	static int	nTry = 2;													//Password attempted times
	
	edPassword->GetText(Password);
	if (LogFile->ReadFile(Password) || LogFile->WithoutFile) {				//Password correct
		//Change window style to sizable
		SetWindowLong(GetMainWindowHandle(), GWL_STYLE,
			GetWindowLong(GetMainWindowHandle(), GWL_STYLE) | WS_THICKFRAME | WS_MAXIMIZEBOX);

		//Hide password frame
		btnLogin->SetVisible(false);
		edPassword->SetVisible(false);
		ShowWindow(GetDlgItem(GetMainWindowHandle(), IDC_PASSWORDFRAME), SW_HIDE);
		ShowWindow(GetDlgItem(GetMainWindowHandle(), IDC_PASSWORDICON), SW_HIDE);
		ShowWindow(GetDlgItem(GetMainWindowHandle(), IDC_PASSWORDLABEL), SW_HIDE);

		//Show the main menu and welcome text
		HMENU hMenu = LoadMenu(GetProgramInstance(), MAKEINTRESOURCE(IDR_MAINWINDOW_MENU));
		SetMenu(GetMainWindowHandle(), hMenu);
		DestroyMenu(hMenu);

		//Hide and show the window to apply new style
		ShowWindow(GetMainWindowHandle(), SW_HIDE);
		ShowWindow(GetMainWindowHandle(), SW_SHOW);
	}
	else {																	//Password incorrect
		if (nTry--) {
			MessageBox(GetMainWindowHandle(),
				L"Incorrect password! Please try again...",
				L"Incorrect Password",
				MB_ICONEXCLAMATION);
			SendMessage(edPassword->hWnd, EM_SETSEL, 0, -1);						//Select all text in the textbox
			SetFocus(edPassword->hWnd);
		}
		else {
			MessageBox(GetMainWindowHandle(),
				L"Too many incorrect password attempts! Exiting...",
				L"Incorrect Password",
				MB_ICONEXCLAMATION);
			mnuExit_Click();
		}
	}
}

/*
Description:    To handle main window creation event
*/
void MainWindow_Create(HWND hWnd) {
	//Set the icon of the window
	HICON hIcon = (HICON)LoadImage(GetProgramInstance(),
		MAKEINTRESOURCE(IDI_MAINICON),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON), 0);
	if (hIcon)
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//Bind controls with their corresponding classes
	edPassword = make_shared<IceEdit>(hWnd, IDC_PASSWORDEDIT, PasswordEditProc);
	btnLogin = make_shared<IceButton>(hWnd, IDC_LOGIN, btnLogin_Click);
	lvLog = make_shared<IceListView>(hWnd, IDC_LISTVIEW_LOG);
	
	//Set control properties
	SendMessage(edPassword->hWnd, EM_SETLIMITTEXT, 20, 0);					//Max.length of password editbox
	lvLog->AddColumn(L"#", 40);												//Add columns to the log listview
	lvLog->AddColumn(L"Car Number", 120);
	lvLog->AddColumn(L"Enter Time (YYYY-MM-DD HH:MM:SS)", 145);
	lvLog->AddColumn(L"Leave Time (YYYY/MM/DD HH:MM:SS)", 145);
	lvLog->AddColumn(L"Position", 80);
	lvLog->AddColumn(L"Fee", 50);

	//Load data file
	LogFile = make_shared<IceEncryptedFile>(L"Log.dat");

	//Set window focus to the password editbox
	SetFocus(edPassword->hWnd);
}

/*
Description:	To handle Exit menu event
*/
void mnuExit_Click() {
	if (1)/*(MessageBox(GetMainWindowHandle(),
		L"Exit Parking System?",
		L"Confirm",
		MB_YESNO | MB_ICONQUESTION) == IDYES)*/ {

		//Close the window and exit the program
		DestroyWindow(GetMainWindowHandle());
		PostQuitMessage(0);
	}
}

/*
Description:	To handle Show Log menu event
*/
void mnuLog_Click() {
	wchar_t buffer[20];

	lvLog->SetVisible(true);												//Show log listview
	for (UINT i = 0; i < LogFile->FileContent.ElementCount; i++) {			//Add all log info to the listview
		//Index
		_itow_s(i + 1, buffer, 10);													
		lvLog->AddItem(buffer);

		//Car number
		lvLog->SetItemText(i, LogFile->FileContent.LogData[i].CarNumber, 1);

		//Enter time
		wsprintf(buffer, L"%04u-%02u-%02u %02u:%02u:%02u",
			LogFile->FileContent.LogData[i].EnterTime.wYear,
			LogFile->FileContent.LogData[i].EnterTime.wMonth,
			LogFile->FileContent.LogData[i].EnterTime.wDay,
			LogFile->FileContent.LogData[i].EnterTime.wHour,
			LogFile->FileContent.LogData[i].EnterTime.wMinute,
			LogFile->FileContent.LogData[i].EnterTime.wSecond);
		lvLog->SetItemText(i, buffer, 2);

		//Leave time
		wsprintf(buffer, L"%04u-%02u-%02u %02u:%02u:%02u",
			LogFile->FileContent.LogData[i].LeaveTime.wYear,
			LogFile->FileContent.LogData[i].LeaveTime.wMonth,
			LogFile->FileContent.LogData[i].LeaveTime.wDay,
			LogFile->FileContent.LogData[i].LeaveTime.wHour,
			LogFile->FileContent.LogData[i].LeaveTime.wMinute,
			LogFile->FileContent.LogData[i].LeaveTime.wSecond);
		lvLog->SetItemText(i, buffer, 3);

		//Car position
		_itow_s(LogFile->FileContent.LogData[i].CarPos, buffer, 10);
		lvLog->SetItemText(i, buffer, 4);

		//Fee
		wsprintf(buffer, L"$%u", LogFile->FileContent.LogData[i].Fee);
		lvLog->SetItemText(i, buffer, 5);
	}
}

/*
Description:	To handle Options menu event
*/
void mnuOptions_Click() {

}

/*
Description:    The entry point of the program
Args:           All parameters are unused
Return:         Result of DialogBox
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	//Record program instance
	RecordProgramInstance(hInstance);

	//Create the main window and pass MainWindow_Create() to the lParam of its WM_INITDIALOG message
	return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAINWINDOW), NULL,
		MainWindowProc, (LPARAM)MainWindow_Create);
}
