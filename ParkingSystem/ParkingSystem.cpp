/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "FileManager.h"

IceEncryptedFile	*LogFile;
IceEdit				*edPassword;
IceButton			*btnLogin;

/*
Description:    To handle main window resizing event
*/
void MainWindow_Resize(HWND hWnd, WORD Width, WORD Height) {
	
}

/*
Description:	To handle login button event
*/
void btnLogin_Click() {
	//Try to decrypt the file
	wchar_t		Password[20];
	static int	nTry = 2;								//Password attempted times
	
	edPassword->GetText(Password);
	if (LogFile->ReadFile(Password)) {					//Password correct
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
	else {												//Password incorrect
		if (nTry--) {
			MessageBox(GetMainWindowHandle(),
				L"Incorrect password! Please try again...",
				L"Incorrect Password",
				MB_ICONEXCLAMATION);
			SendMessage(edPassword->hWnd, EM_SETSEL, 0, -1);	//Select all text in the textbox
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

	//Bind controls with message handlers
	edPassword = new IceEdit(hWnd, IDC_PASSWORDEDIT, PasswordEditProc);
	btnLogin = new IceButton(hWnd, IDC_LOGIN, btnLogin_Click);

	//Set max. length of password editbox
	SendMessage(edPassword->hWnd, EM_SETLIMITTEXT, 20, 0);

	//Load data file
	LogFile = new IceEncryptedFile(L"Log.dat");

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

		//Release all control bindings
		delete LogFile;
		delete edPassword;
		delete btnLogin;

		//Close the window and exit the program
		DestroyWindow(GetMainWindowHandle());
		PostQuitMessage(0);
	}
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
