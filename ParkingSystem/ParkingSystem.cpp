/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "FileManager.h"

/* Control bindings */
shared_ptr<IceEncryptedFile>	LogFile;
shared_ptr<IceEdit>				edPassword;
shared_ptr<IceButton>			btnLogin;
shared_ptr<IceButton>			btnCancelLogin;
shared_ptr<IceListView>			lvLog;
shared_ptr<IceLabel>			labPasswordIcon;
shared_ptr<IceLabel>			labPassword;
shared_ptr<IceLabel>			labWelcome;
shared_ptr<IceLabel>			labPositionLeft;
shared_ptr<IceLabel>			labCarNumber;
shared_ptr<IceLabel>			labPrice;
shared_ptr<IceLabel>			labTime;
shared_ptr<IceEdit>				edCarNumber;
shared_ptr<IceButton>			btnEnterOrExit;
shared_ptr<IceTimer>			tmrRefreshTime;								//The timer refreshs system time of payment mode
shared_ptr<IceTimer>			tmrRestoreWelcomeText;						//The timer resets welcome text of payment mode after certain seconds
HWND							fraPasswordFrame;							//Password frame control handle

vector<UINT>					CurrParkedCars;								//Cars currently parked, index of LogFile->FileContent.LogData
bool							ParkingPos[100] = { 0 };					//Available parking positions (true = occupied)

/*
Program status identifier
Value		Name				Description
----------------------------------------------------------------------------------------------
0			Normal mode			Initial status
1			Payment mode		In payment mode, the program requires password to manage
2			Log mode			Viewing log
3			Unlocking mode		In payment mode, trying to unlock the program
*/
char							CurrStatus = 0;

/*
Description:    Hide password-related controls
*/
void HidePasswordFrame() {
	btnLogin->SetVisible(false);
	btnCancelLogin->SetVisible(false);
	edPassword->SetVisible(false);
	labPasswordIcon->SetVisible(false);
	labPassword->SetVisible(false);
	ShowWindow(fraPasswordFrame, SW_HIDE);
	
	//Restore password field and prompt text
	edPassword->SetText(L"");
	labPassword->SetText(L"Enter system password:\n(Default: 123)");
}

/*
Description:    Show password-related controls
*/
void ShowPasswordFrame() {
	btnLogin->SetVisible(true);
	btnCancelLogin->SetVisible(true);
	edPassword->SetVisible(true);
	labPasswordIcon->SetVisible(true);
	labPassword->SetVisible(true);
	ShowWindow(fraPasswordFrame, SW_SHOW);
}

/*
Description:    Hide payment-related controls
*/
void HidePaymentFrame() {
	labWelcome->SetVisible(false);
	labPositionLeft->SetVisible(false);
	labCarNumber->SetVisible(false);
	labPrice->SetVisible(false);
	labTime->SetVisible(false);
	edCarNumber->SetVisible(false);
	btnEnterOrExit->SetVisible(false);
}

/*
Description:    Show payment-related controls
*/
void ShowPaymentFrame() {
	labWelcome->SetVisible(true);
	labPositionLeft->SetVisible(true);
	labCarNumber->SetVisible(true);
	labPrice->SetVisible(true);
	labTime->SetVisible(true);
	edCarNumber->SetVisible(true);
	btnEnterOrExit->SetVisible(true);
}

/*
Description:    To handle main window resizing event
*/
void MainWindow_Resize(HWND hWnd, int Width, int Height) {
	if (CurrStatus == 1) {													//Payment mode
		//Change payment mode controls positions & sizes & fonts by ratio
		labWelcome->Size(Width, Height / 3);									//Welcome label
		labWelcome->SetFont(Width / 25, false);
		btnEnterOrExit->Move(Width / 1.3, Height / 1.5);						//Enter/Exit button
		btnEnterOrExit->Size(Width / 5, Height / 12);
		btnEnterOrExit->SetFont(Width / 48, false);
		labTime->Move(Width / 1.65, Height / 1.1);								//Time label
		labTime->Size(Width, Height);
		labTime->SetFont(Width / 35, false);
		labPositionLeft->Move(15, Height / 1.85);								//Position left label
		labPositionLeft->Size(Width / 2.5, Height / 12);
		labPositionLeft->SetFont(Width / 35, false);
		labCarNumber->Move(15, Height / 1.5);									//Car number label
		labCarNumber->Size(Width / 2.5, Height / 12);
		labCarNumber->SetFont(Width / 35, false);
		labPrice->Move(15, Height / 1.25);										//Price label
		labPrice->Size(Width / 1.35, Height / 12);
		labPrice->SetFont(Width / 35, false);
		edCarNumber->Move(Width / 2.2, Height / 1.5);							//Car number editbox
		edCarNumber->Size(Width / 3.5, Height / 12);
		edCarNumber->SetFont(Width / 35, false);
	}
	else if (CurrStatus == 2) {												//Log viewing mode
		lvLog->Size(Width, Height);												//Change listview size
	}
	else if (CurrStatus == 3) {												//Unlocking mode
		//Change unlocking mode controls positions by ratio
		RECT	PasswordFrameRect;
		POINT	PasswordFramePos;
		GetWindowRect(fraPasswordFrame, &PasswordFrameRect);					//Get password frame size
		SetWindowPos(fraPasswordFrame, HWND_BOTTOM,
			Width / 2 - (PasswordFrameRect.right - PasswordFrameRect.left) / 2,
			Height / 2 - (PasswordFrameRect.bottom - PasswordFrameRect.top) / 2,
			0, 0, SWP_NOSIZE);
		GetWindowRect(fraPasswordFrame, &PasswordFrameRect);					//Get the new password frame position
		PasswordFramePos.x = PasswordFrameRect.left;
		PasswordFramePos.y = PasswordFrameRect.top;
		ScreenToClient(GetMainWindowHandle(), &PasswordFramePos);				//Get appearant position of password frame
		labPasswordIcon->Move(PasswordFramePos.x + 15, PasswordFramePos.y + 30);
		labPassword->Move(PasswordFramePos.x + 60, PasswordFramePos.y + 30);
		edPassword->Move(PasswordFramePos.x + 195, PasswordFramePos.y + 30);
		btnLogin->Move(PasswordFramePos.x + 105, PasswordFramePos.y + 75);
		btnCancelLogin->Move(PasswordFramePos.x + 190, PasswordFramePos.y + 75);
	}
}

/*
Description:	To handle login button event
*/
void btnLogin_Click() {
	//Try to decrypt the file
	wchar_t		Password[20];
	static int	nTry = 2;													//Password attempted times
	
	edPassword->GetText(Password);
	if (CurrStatus == 0) {													//Login to enter system
		if (LogFile->ReadFile(Password) || LogFile->WithoutFile) {				//Password correct
			//Change window style to sizable
			SetWindowLong(GetMainWindowHandle(), GWL_STYLE,
				GetWindowLong(GetMainWindowHandle(), GWL_STYLE) | WS_THICKFRAME | WS_MAXIMIZEBOX);

			//Hide password frame
			edPassword->SetText(L"");
			btnLogin->SetVisible(false);
			btnCancelLogin->SetVisible(false);
			edPassword->SetVisible(false);
			labPasswordIcon->SetVisible(false);
			labPassword->SetVisible(false);
			ShowWindow(fraPasswordFrame, SW_HIDE);

			//Show the main menu
			HMENU hMenu = LoadMenu(GetProgramInstance(), MAKEINTRESOURCE(IDR_MAINWINDOW_MENU));
			SetMenu(GetMainWindowHandle(), hMenu);
			DestroyMenu(hMenu);

			//Hide and show the window to apply new style
			ShowWindow(GetMainWindowHandle(), SW_HIDE);
			ShowWindow(GetMainWindowHandle(), SW_SHOW);

			//Add parking cars to the list
			for (UINT i = 0; i < LogFile->FileContent.ElementCount; i++) {
				if (LogFile->FileContent.LogData[i].LeaveTime.wYear == 0) {			//If the car is not left
					CurrParkedCars.push_back(i);										//Add it to the parked list
					ParkingPos[LogFile->FileContent.LogData[i].CarPos] = true;			//Mark the parking position as occupied
				}
			}
		}
		else {																	//Password incorrect
			if (nTry--) {															//Decrease attempt times
				MessageBox(GetMainWindowHandle(),
					L"Incorrect password! Please try again...",
					L"Incorrect Password",
					MB_ICONEXCLAMATION);
				SendMessage(edPassword->hWnd, EM_SETSEL, 0, -1);						//Select all text in the textbox
				SetFocus(edPassword->hWnd);
			}
			else {																	//Ran out of attempt times
				MessageBox(GetMainWindowHandle(),
					L"Too many incorrect password attempts! Exiting...",
					L"Incorrect Password",
					MB_ICONEXCLAMATION);
				mnuExit_Click();
			}
		}
	}
	else {																	//Login to exit payment mode
		if (!lstrcmpW(Password, LogFile->FileContent.Password)) {				//Password correct
			HidePasswordFrame();

			SetWindowLong(GetMainWindowHandle(), GWL_STYLE,						//Make the window sizable
				GetWindowLong(GetMainWindowHandle(), GWL_STYLE) | WS_THICKFRAME | WS_MAXIMIZEBOX);

			//Show the main menu
			HMENU hMenu = LoadMenu(GetProgramInstance(), MAKEINTRESOURCE(IDR_MAINWINDOW_MENU));
			SetMenu(GetMainWindowHandle(), hMenu);
			DestroyMenu(hMenu);
			CurrStatus = 0;
		}
		else {																	//Password incorrect
			if (nTry--) {
				labPassword->SetText(L"Incorrect password!\nTry again.");
				SendMessage(edPassword->hWnd, EM_SETSEL, 0, -1);						//Select all text in the textbox
				SetFocus(edPassword->hWnd);
			}
			else {
				HidePasswordFrame();
				ShowPaymentFrame();

				SetWindowLong(GetMainWindowHandle(), GWL_STYLE,							//Make the window sizable
					GetWindowLong(GetMainWindowHandle(), GWL_STYLE) | WS_THICKFRAME | WS_MAXIMIZEBOX);

				//Show login failed message
				labWelcome->SetText(L"Failed to exit payment mode!");
				tmrRestoreWelcomeText->SetEnabled(true);
				SetFocus(edCarNumber->hWnd);
				CurrStatus = 1;
			}
		}
	}
}

/*
Description:	To handle login cancel button event
*/
void btnCancelLogin_Click() {
	mnuExit_Click();
}

/*
Description:	To handle enter & exit button event
*/
void btnEnterOrExit_Click() {
	wchar_t		CarNumber[10];												//Car number buffer
	SYSTEMTIME	CurrTime = { 0 };											//Current time

	GetLocalTime(&CurrTime);												//Get current system time
	edCarNumber->GetText(CarNumber);

	//Detect empty text
	if (lstrlenW(CarNumber) == 0) {
		SetFocus(edCarNumber->hWnd);
		return;
	}

	//Determine whether the car is entering or leaving
	for (UINT i = 0; i < CurrParkedCars.size(); i++) {						//Search for the car number in the parked cars list
		//Matched, means the car is leaving
		if (!lstrcmpW(CarNumber, LogFile->FileContent.LogData[CurrParkedCars[i]].CarNumber)) {
			labWelcome->SetText(L"Leave");
			LogFile->FileContent.LogData[CurrParkedCars[i]].LeaveTime = CurrTime;		//Record leave time of the car
			ParkingPos[LogFile->FileContent.LogData[CurrParkedCars[i]].CarPos] = false;	//Mark the parking position as unoccupied
			
			//Calculate fee when the car is leaving
			int				HourDifference;												//No. of hours between EnterTime and CurrTime
			SYSTEMTIME		*EnterTime =
				&(LogFile->FileContent.LogData[CurrParkedCars[i]].EnterTime);			//Get a pointer to EnterTime of current log for convenience

			if (CurrTime.wDay < EnterTime->wDay) {
				if (CurrTime.wMonth == 2) {												//February
					if ((CurrTime.wYear % 4 == 0 && CurrTime.wYear % 100 != 0) ||
						(CurrTime.wYear % 400 == 0))										//For leap years, 29 days in Feb

						CurrTime.wDay += 29;
					else																	//For normal years, 28 days in Feb
						CurrTime.wDay += 28;
				}
				else if (CurrTime.wMonth == 5 || CurrTime.wMonth == 7 || CurrTime.wMonth == 10 || CurrTime.wMonth == 12)
					CurrTime.wDay += 30;													//30-days months
				else
					CurrTime.wDay += 31;													//31-days months
				CurrTime.wMonth--;														//Month reduced by 1 cuz we just added the days to wDay
			}

			if (CurrTime.wMonth < EnterTime->wMonth) {								//Not a full year
				CurrTime.wMonth += 12;
				CurrTime.wYear--;
			}

			HourDifference = (CurrTime.wYear - EnterTime->wYear +
				CurrTime.wMonth - EnterTime->wMonth +
				CurrTime.wDay - EnterTime->wDay) * 24 +
				CurrTime.wHour - EnterTime->wHour;									//Calculate date difference in hours
			if (CurrTime.wMinute > 0)													//Less than 1 hour = 1 hour
				HourDifference++;

			LogFile->FileContent.LogData[CurrParkedCars[i]].Fee = (float)HourDifference * LogFile->FileContent.FeePerHour;
			if (HourDifference > 5)														//20% off for >5hrs parking
				LogFile->FileContent.LogData[CurrParkedCars[i]].Fee *= 0.8;

			//Display parking hours and fee
			labWelcome->SetText(L"Hours Parked: %ihr, Fee: $%.2f",
				HourDifference, LogFile->FileContent.LogData[CurrParkedCars[i]].Fee);

			CurrParkedCars.erase(CurrParkedCars.begin() + i);							//Remove the car from the parked cars list
			LogFile->SaveFile();

			//labPositionLeft->SetText(bufStr);

			//Clean the window
			edCarNumber->SetText(L"");
			SetFocus(edCarNumber->hWnd);
			tmrRestoreWelcomeText->SetEnabled(true);									//Restore welcome text after few seconds

			return;
		}
	}

	//No matched result, means the car is entering
	//Allocate a car position
	for (int i = 0; i < 100; i++) {											//Find an unoccupied position
		if (!ParkingPos[i]) {
			ParkingPos[i] = true;												//Mark the position as occupied
			labWelcome->SetText(L"Welcome! Your Car Position: %i", i);			//Show the position for the user
			LogFile->AddLog(CarNumber, CurrTime, { 0 }, i, 0);					//Add car enter log
			CurrParkedCars.push_back(LogFile->FileContent.ElementCount - 1);	//Add the log index to the parked cars list

			break;
		}
	}

	//Clean the window
	edCarNumber->SetText(L"");
	SetFocus(edCarNumber->hWnd);
	tmrRestoreWelcomeText->SetEnabled(true);								//Restore welcome text after few seconds
}

/*
Description:	Refresh system time
*/
void tmrRefreshTime_Timer() {
	SYSTEMTIME	st = { 0 };													//Retrieved system time

	GetLocalTime(&st);														//Get current system time
	labTime->SetText(L"Time: %04i-%02i-%02i %02i:%02i:%02i",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

/*
Description:	Restore welome text few seconds after a car leaves
*/
void tmrRestoreWelcomeText_Timer() {
	labWelcome->SetText(L"Welcome to Shar Carpark!");						//Restore the welcome text
	tmrRestoreWelcomeText->SetEnabled(false);								//Disable the timer
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
	btnCancelLogin = make_shared<IceButton>(hWnd, IDC_CANCELLOGIN, btnCancelLogin_Click);
	lvLog = make_shared<IceListView>(hWnd, IDC_LISTVIEW_LOG);
	labPasswordIcon = make_shared<IceLabel>(hWnd, IDC_PASSWORDICON);
	labPassword = make_shared<IceLabel>(hWnd, IDC_PASSWORDLABEL);
	labWelcome = make_shared<IceLabel>(hWnd, IDC_WELCOMELABEL);
	labPositionLeft = make_shared<IceLabel>(hWnd, IDC_POSITIONCOUNTLABEL);
	labCarNumber = make_shared<IceLabel>(hWnd, IDC_CARNUMBERLABEL);
	labPrice = make_shared<IceLabel>(hWnd, IDC_PRICELABEL);
	labTime = make_shared<IceLabel>(hWnd, IDC_SYSTEMTIMELABEL);
	edCarNumber = make_shared<IceEdit>(hWnd, IDC_CARNUMBEREDIT, CarNumberEditProc);
	btnEnterOrExit = make_shared<IceButton>(hWnd, IDC_ENTEROREXITBUTTON, btnEnterOrExit_Click);
	tmrRefreshTime = make_shared<IceTimer>(1000, tmrRefreshTime_Timer, true);
	tmrRestoreWelcomeText = make_shared<IceTimer>(5000, tmrRestoreWelcomeText_Timer, false);
	fraPasswordFrame = GetDlgItem(GetMainWindowHandle(), IDC_PASSWORDFRAME);
	
	//Set control properties
	labWelcome->SetVisible(false);											//Hide unrelated controls
	labPositionLeft->SetVisible(false);
	labCarNumber->SetVisible(false);
	labPrice->SetVisible(false);
	labTime->SetVisible(false);
	edCarNumber->SetVisible(false);
	btnEnterOrExit->SetVisible(false);
	labWelcome->Move(0, 20);												//Set the position of welcome label
	lvLog->Move(0, 0);														//Set the position of listview
	SendMessage(edPassword->hWnd, EM_SETLIMITTEXT, 20, 0);					//Max length of password editbox
	SendMessage(edCarNumber->hWnd, EM_SETLIMITTEXT, 10, 0);					//Max length of car number editbox
	lvLog->AddColumn(L"#", 40);												//Add columns to log listview
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
	if (CurrStatus == 1) {												//Payment mode, user is trying to exit payment mode
		ShowPasswordFrame();
		HidePaymentFrame();
		CurrStatus = 3;														//Enter unlocking mode

		RECT	MainWindowSize;
		SetWindowLong(GetMainWindowHandle(), GWL_STYLE,						//Make the window not sizable
			GetWindowLong(GetMainWindowHandle(), GWL_STYLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
		GetClientRect(GetMainWindowHandle(), &MainWindowSize);				//Get window size
		MainWindow_Resize(GetMainWindowHandle(),
			MainWindowSize.right - MainWindowSize.left,
			MainWindowSize.bottom - MainWindowSize.top);					//Invoke window resize event to center password frame

		SetFocus(edPassword->hWnd);											//Let the password editbox has focus
	}
	else if (CurrStatus == 3) {											//Unlocking mode, user cancelled exiting payment mode
		HidePasswordFrame();
		ShowPaymentFrame();
		CurrStatus = 1;														//Return to payment mode

		RECT	MainWindowSize;
		SetWindowLong(GetMainWindowHandle(), GWL_STYLE,						//Make the window sizable
			GetWindowLong(GetMainWindowHandle(), GWL_STYLE) | WS_THICKFRAME | WS_MAXIMIZEBOX);
		GetClientRect(GetMainWindowHandle(), &MainWindowSize);				//Get window size
		MainWindow_Resize(GetMainWindowHandle(),
			MainWindowSize.right - MainWindowSize.left,
			MainWindowSize.bottom - MainWindowSize.top);					//Invoke window resize event to resize payment controls

		SetFocus(edCarNumber->hWnd);										//Let the car number editbox has focus
	}
	else {																//Normal mode, user is goint to exit the system
		//Show a prompt when exiting
		if (1)/*(MessageBox(GetMainWindowHandle(),
			L"Exit Parking System?",
			L"Confirm",
			MB_YESNO | MB_ICONQUESTION) == IDYES)*/ {

			//Close the window and exit the program
			DestroyWindow(GetMainWindowHandle());
			PostQuitMessage(0);
		}
	}
}

/*
Description:	To handle Enter Payment Mode menu event
*/
void mnuEnterPaymentMode_Click() {
	lvLog->SetVisible(false);												//Hide unrelated controls
	ShowPaymentFrame();
	SetMenu(GetMainWindowHandle(), NULL);									//Remove window menu
	SetFocus(edCarNumber->hWnd);											//Set input focus to the car number textbox
	CurrStatus = 1;															//Change status to payment mode
	
	RECT	MainWindowSize;
	GetClientRect(GetMainWindowHandle(), &MainWindowSize);					//Get window size
	MainWindow_Resize(GetMainWindowHandle(),
		MainWindowSize.right - MainWindowSize.left,
		MainWindowSize.bottom - MainWindowSize.top);						//Invoke window resize event to center password frame
}

/*
Description:	To handle Show Log menu event
*/
void mnuLog_Click() {
	wchar_t buffer[20];

	lvLog->DeleteAllItems();												//Clear log listview
	for (UINT i = 0; i < LogFile->FileContent.ElementCount; i++) {			//Add all log info to the listview
		//Index
		_itow_s(i + 1, buffer, 10);													
		lvLog->AddItem(buffer);

		//Car number
		lvLog->SetItemText(i, LogFile->FileContent.LogData[i].CarNumber, 1);

		//Enter time
		swprintf_s(buffer, L"%04u-%02u-%02u %02u:%02u:%02u",
			LogFile->FileContent.LogData[i].EnterTime.wYear,
			LogFile->FileContent.LogData[i].EnterTime.wMonth,
			LogFile->FileContent.LogData[i].EnterTime.wDay,
			LogFile->FileContent.LogData[i].EnterTime.wHour,
			LogFile->FileContent.LogData[i].EnterTime.wMinute,
			LogFile->FileContent.LogData[i].EnterTime.wSecond);
		lvLog->SetItemText(i, buffer, 2);

		//Leave time
		if (LogFile->FileContent.LogData[i].LeaveTime.wYear) {					//The car has left
			swprintf_s(buffer, L"%04u-%02u-%02u %02u:%02u:%02u",
				LogFile->FileContent.LogData[i].LeaveTime.wYear,
				LogFile->FileContent.LogData[i].LeaveTime.wMonth,
				LogFile->FileContent.LogData[i].LeaveTime.wDay,
				LogFile->FileContent.LogData[i].LeaveTime.wHour,
				LogFile->FileContent.LogData[i].LeaveTime.wMinute,
				LogFile->FileContent.LogData[i].LeaveTime.wSecond);
			//Fee
			swprintf_s(buffer, L"$%.2f", LogFile->FileContent.LogData[i].Fee);
			lvLog->SetItemText(i, buffer, 5);
		}
		else																	//The car is still parking
			lstrcpyW(buffer, L"Still Parking");
		lvLog->SetItemText(i, buffer, 3);

		//Car position
		_itow_s(LogFile->FileContent.LogData[i].CarPos, buffer, 10);
		lvLog->SetItemText(i, buffer, 4);
	}
	lvLog->SetVisible(true);												//Show log listview
	CurrStatus = 2;															//Change status to log mode
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
