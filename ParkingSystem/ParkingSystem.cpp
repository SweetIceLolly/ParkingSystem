/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "FileManager.h"

/* ListView sorting info to be passed to ListViewCompareFunction */
struct lvSortInfo {
	bool						Ascending;									//Ascending or descending
	int							HeaderIndex;								//Header index of ListView
};

/* Control bindings */
shared_ptr<IceEncryptedFile>	LogFile;
shared_ptr<IceEdit>				edPassword;
shared_ptr<IceButton>			btnLogin;
shared_ptr<IceButton>			btnCancelLogin;
shared_ptr<IceListView>			lvLog;
shared_ptr<IceTab>				tabReport;
shared_ptr<IceCanvas>			PositionReportCanvas;
shared_ptr<IceCanvas>			HistoryReportCanvas;
shared_ptr<IceCanvas>			DailyReportCanvas;
shared_ptr<IceCanvas>			MonthlyReportCanvas;
shared_ptr<IceLabel>			labPasswordIcon;
shared_ptr<IceLabel>			labPassword;
shared_ptr<IceLabel>			labWelcome;
shared_ptr<IceLabel>			labPositionLeft;
shared_ptr<IceLabel>			labCarNumber;
shared_ptr<IceLabel>			labPrice;
shared_ptr<IceLabel>			labTime;
shared_ptr<IceEdit>				edCarNumber;
shared_ptr<IceButton>			btnEnterOrExit;
shared_ptr<IceDateTimePicker>	dtpHistoryDate;
shared_ptr<IceDateTimePicker>	dtpHistoryTime;
shared_ptr<IceTimer>			tmrRefreshTime;								//The timer refreshs system time of payment mode
shared_ptr<IceTimer>			tmrRestoreWelcomeText;						//The timer resets welcome text of payment mode after certain seconds
HWND							fraPasswordFrame;							//Password frame control handle

/* Position info */
vector<UINT>					CurrParkedCars;								//Cars currently parked, index of LogFile->FileContent.LogData
bool							ParkingPos[100] = { 0 };					//Available parking positions (true = occupied)

/* History report related */
LogInfo							HistoryParkedCars[100] = { 0 };				//Parked cars record for history report
int								HistoryParkedCarsCount = 0;					//Number of parked cars for history report

/*
Program status identifier
Value		Name				Description
----------------------------------------------------------------------------------------------
-1			Stand by			Program initialization finished
0			Normal mode			Initial status
1			Payment mode		In payment mode, the program requires password to manage
2			Log mode			Viewing log
3			Unlocking mode		In payment mode, trying to unlock the program
4			Locked				System locked, requires password to manage
5			Position Report		Viewing position report
6			History Report		Viewing history Report
7			Daily Report		Viewing	daily Report
8			Monthly Report		Viewing	monthly Report
*/
char							CurrStatus = 0;

/* Misc. */
char							nPasswordTried = 2;							//Password attempt times left
int								TabHeaderHeight;							//Height of tab header

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

	SetWindowLong(GetMainWindowHandle(), GWL_STYLE,							//Make the window sizable
		GetWindowLong(GetMainWindowHandle(), GWL_STYLE) | WS_THICKFRAME | WS_MAXIMIZEBOX);
	InvalidateRect(GetMainWindowHandle(), NULL, TRUE);
}

/*
Description:    Show password-related controls
*/
void ShowPasswordFrame() {
	nPasswordTried = 2;														//Restore password attempt times
	btnLogin->SetVisible(true);
	btnCancelLogin->SetVisible(true);
	edPassword->SetVisible(true);
	labPasswordIcon->SetVisible(true);
	labPassword->SetVisible(true);
	ShowWindow(fraPasswordFrame, SW_SHOW);

	SetWindowLong(GetMainWindowHandle(), GWL_STYLE,							//Make the window not sizable
		GetWindowLong(GetMainWindowHandle(), GWL_STYLE) & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
	InvalidateRect(GetMainWindowHandle(), NULL, TRUE);
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
	//For initial status, resize all controls
	//That's why I don't add 'else' before 'if'
	if (CurrStatus == 1 || CurrStatus == 0) {								//Payment mode
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
	if (CurrStatus == 2 || CurrStatus == 0)									//Log viewing mode
		lvLog->Size(Width, Height);												//Change listview size
	if (CurrStatus == 3 || CurrStatus == 0) {								//Unlocking mode
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
	if (CurrStatus == 5 || CurrStatus == 0) {								//Viewing position report
		tabReport->Size(Width, Height);
		PositionReportCanvas->Size(Width, Height - TabHeaderHeight);
	}
	if (CurrStatus == 6 || CurrStatus == 0) {								//Viewing history report
		tabReport->Size(Width, Height);
		HistoryReportCanvas->Size(Width, Height - TabHeaderHeight);
		dtpHistoryDate->Move((HistoryReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2 + 45, 30);
		dtpHistoryTime->Move((HistoryReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2 +
			60 + dtpHistoryDate->CtlRect.right - dtpHistoryDate->CtlRect.left, 30);
	}
	if (CurrStatus == 7 || CurrStatus == 0) {								//Viewing daily report
		tabReport->Size(Width, Height);
		DailyReportCanvas->Size(Width, Height - TabHeaderHeight);
	}
	if (CurrStatus == 8 || CurrStatus == 0) {								//Viewing monthly report
		tabReport->Size(Width, Height);
		MonthlyReportCanvas->Size(Width, Height - TabHeaderHeight);
	}
}

/*
Description:	To handle login button event
*/
void btnLogin_Click() {
	//Try to decrypt the file
	wchar_t		Password[20];
	
	edPassword->GetText(Password);
	if (CurrStatus == 0) {													//Login to enter system
		if (LogFile->ReadFile(Password) || LogFile->WithoutFile) {				//Password correct
			HidePasswordFrame();

			//Show the main menu
			HMENU hMenu = LoadMenu(GetProgramInstance(), MAKEINTRESOURCE(IDR_MAINWINDOW_MENU));
			SetMenu(GetMainWindowHandle(), hMenu);
			DestroyMenu(hMenu);

			//Redraw the window to apply new style
			InvalidateRect(GetMainWindowHandle(), NULL, TRUE);

			//Add parking cars to the list
			for (UINT i = 0; i < LogFile->FileContent.ElementCount; i++) {
				if (LogFile->FileContent.LogData[i].LeaveTime.wYear == 0) {			//If the car hasn't left
					CurrParkedCars.push_back(i);										//Add it to the parked list
					ParkingPos[LogFile->FileContent.LogData[i].CarPos] = true;			//Mark the parking position as occupied
				}
			}
			labPositionLeft->SetText(L"Position Left: %i", 100 - CurrParkedCars.size());

			//Update program status
			CurrStatus = -1;
		}
		else {																	//Password incorrect
			if (nPasswordTried--) {														//Decrease attempt times
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
	else if (CurrStatus == 3) {												//Login to exit payment mode
		if (!lstrcmpW(Password, LogFile->FileContent.Password)) {				//Password correct
			HidePasswordFrame();

			//Show the main menu
			HMENU hMenu = LoadMenu(GetProgramInstance(), MAKEINTRESOURCE(IDR_MAINWINDOW_MENU));
			SetMenu(GetMainWindowHandle(), hMenu);
			DestroyMenu(hMenu);
			CurrStatus = -1;
		}
		else {																	//Password incorrect
			if (nPasswordTried--) {
				labPassword->SetText(L"Incorrect password!\nTry again.");
				SendMessage(edPassword->hWnd, EM_SETSEL, 0, -1);						//Select all text in the textbox
				SetFocus(edPassword->hWnd);
			}
			else {
				HidePasswordFrame();
				ShowPaymentFrame();

				//Show login failed message
				labWelcome->SetText(L"Failed to exit payment mode!");
				tmrRestoreWelcomeText->SetEnabled(true);
				SetFocus(edCarNumber->hWnd);
				CurrStatus = 1;
			}
		}
	}
	else if (CurrStatus == 4) {												//Login to unlock the system
		if (!lstrcmpW(Password, LogFile->FileContent.Password)) {				//Password correct
			HidePasswordFrame();

			//Show the main menu
			HMENU hMenu = LoadMenu(GetProgramInstance(), MAKEINTRESOURCE(IDR_MAINWINDOW_MENU));
			SetMenu(GetMainWindowHandle(), hMenu);
			DestroyMenu(hMenu);
			CurrStatus = -1;
		}
		else {																	//Password incorrect
			labPassword->SetText(L"Incorrect password!\nTry again.");
			SendMessage(edPassword->hWnd, EM_SETSEL, 0, -1);						//Select all text in the textbox
			SetFocus(edPassword->hWnd);
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
Description:	To calculate required fee with specified time info
Args:           EnterTime: Enter time of the car
				LeaveTime: Leave time of the car
				OutHourDifference: Return value of HourDifference. Default = NULL (means the value won't be returned)
Return:			Required fee
*/
float CalcFee(SYSTEMTIME *EnterTime, SYSTEMTIME *LeaveTime, int *OutHourDifference = NULL) {
	int			HourDifference;												//No. of hours between EnterTime and CurrTime

	if (LeaveTime->wDay < EnterTime->wDay) {
		if (LeaveTime->wMonth == 2) {											//February
			if ((LeaveTime->wYear % 4 == 0 && LeaveTime->wYear % 100 != 0) ||
				(LeaveTime->wYear % 400 == 0))										//For leap years, 29 days in Feb

				LeaveTime->wDay += 29;
			else																	//For normal years, 28 days in Feb
				LeaveTime->wDay += 28;
		}
		else if (LeaveTime->wMonth == 5 || LeaveTime->wMonth == 7 || LeaveTime->wMonth == 10 || LeaveTime->wMonth == 12)
			LeaveTime->wDay += 30;													//30-days months
		else
			LeaveTime->wDay += 31;													//31-days months
		LeaveTime->wMonth--;													//Month reduced by 1 cuz we just added the days to wDay
	}

	if (LeaveTime->wMonth < EnterTime->wMonth) {							//Not a full year
		LeaveTime->wMonth += 12;
		LeaveTime->wYear--;
	}

	HourDifference = (LeaveTime->wYear - EnterTime->wYear +
		LeaveTime->wMonth - EnterTime->wMonth +
		LeaveTime->wDay - EnterTime->wDay) * 24 +
		LeaveTime->wHour - EnterTime->wHour;								//Calculate date difference in hours
	if (LeaveTime->wMinute > 0)													//Less than 1 hour = 1 hour
		HourDifference++;

	if (OutHourDifference)													//If the user wants HourDifference to be returned
		*OutHourDifference = HourDifference;
	if (HourDifference > 5)														//20% off for >5hrs parking
		return (float)HourDifference * LogFile->FileContent.FeePerHour * 0.8;
	else
		return (float)HourDifference * LogFile->FileContent.FeePerHour;
}

/*
Description:	To handle enter & exit button event
*/
void btnEnterOrExit_Click() {
	wchar_t		CarNumber[20];												//Car number buffer
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
			int	HourDifference;
			LogFile->FileContent.LogData[CurrParkedCars[i]].Fee = CalcFee(
				&(LogFile->FileContent.LogData[CurrParkedCars[i]].EnterTime), &CurrTime, &HourDifference);

			//Display parking hours and fee
			labWelcome->SetText(L"Hours Parked: %ihr, Fee: $%.2f",
				HourDifference, LogFile->FileContent.LogData[CurrParkedCars[i]].Fee);

			CurrParkedCars.erase(CurrParkedCars.begin() + i);							//Remove the car from the parked cars list
			LogFile->SaveFile();
			labPositionLeft->SetText(L"Position Left: %i", 100 - CurrParkedCars.size());

			//Clean the window
			edCarNumber->SetText(L"");
			SetFocus(edCarNumber->hWnd);
			tmrRestoreWelcomeText->SetEnabled(true);									//Restore welcome text after few seconds

			return;
		}
	}

	//No matched result, means the car is entering
	//Allocate a car position
	bool PositionAllocated = false;											//If a position is allocated
	for (int i = 0; i < 100; i++) {											//Find an unoccupied position
		if (!ParkingPos[i]) {
			ParkingPos[i] = true;												//Mark the position as occupied
			labWelcome->SetText(L"Welcome! Your Car Position: %i", i + 1);		//Show the position for the user
			LogFile->AddLog(CarNumber, CurrTime, { 0 }, i, 0);					//Add car enter log
			CurrParkedCars.push_back(LogFile->FileContent.ElementCount - 1);	//Add the log index to the parked cars list
			labPositionLeft->SetText(L"Position Left: %i", 100 - CurrParkedCars.size());
			PositionAllocated = true;											//Mark that a position is allocated

			break;
		}
	}
	if (!PositionAllocated)													//No position allocated (Park fulled)
		labWelcome->SetText(L"Sorry, No Position Left.");

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
Description:	To handle paint event of position report canvas
*/
void PositionReportCanvas_Paint() {
	//Calculate width and height of each position box
	int BoxW = (PositionReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2 / 10,
		BoxH = (PositionReportCanvas->bi.bmiHeader.biHeight - 60) / 10;

	//Paint
	RECT	BoxPos;
	PositionReportCanvas->Cls();
	if (BoxW > 0 && BoxH > 0) {												//Make sure the window is large enough to draw everything
		for (int i = 0; i < 10; i++) {											//Rows
			for (int j = 0; j < 10; j++) {											//Columns
				BoxPos.top = 30 + i * BoxH;												//Calculate the position of box
				BoxPos.left = 30 + j * BoxW;
				BoxPos.right = BoxPos.left + BoxW;
				BoxPos.bottom = BoxPos.top + BoxH;
				
				if (ParkingPos[i * 10 + j])	{											//Position occupied
					//Draw gray background with a cross
					FillRect(PositionReportCanvas->hDC, &BoxPos, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
					PositionReportCanvas->DrawLine(BoxPos.left, BoxPos.top, BoxPos.right, BoxPos.bottom);
					PositionReportCanvas->DrawLine(BoxPos.right, BoxPos.top, BoxPos.left, BoxPos.bottom);
				}

				//Draw border
				PositionReportCanvas->DrawRect(BoxPos.left, BoxPos.top, BoxPos.right, BoxPos.bottom);

				//Print number of position
				PositionReportCanvas->Print(BoxPos.left, BoxPos.top, L"%i", i * 10 + j + 1);
			}
		}
	}
}

/*
Description:	To handle mouse move event of position report canvas
Args:			X, Y: Position of cursor
*/
void PositionReportCanvas_MouseMove(int X, int Y) {
	//Calculate width and height of each position box
	int PositionAreaWidth = (PositionReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2;
	int BoxW = PositionAreaWidth / 10,
		BoxH = (PositionReportCanvas->bi.bmiHeader.biHeight - 60) / 10;

	//Calculate the car position under the cursor
	int SelPosX = (X - 30) / BoxW;
	int SelPosY = (Y - 30) / BoxH;
	static int PrevPos = -2;												//Remember the previous selected car position to reduce CPU usage

	if (SelPosX > 9 || SelPosY > 9 || X < 30 || Y < 30) {					//If cursor moved out of the position area
		if (PrevPos != -1) {													//If the cursor is in the position area previously
			PrevPos = -1;
			PositionReportCanvas->Print(PositionAreaWidth + 45, 30,
				L"Occupied Positions: %i/100", CurrParkedCars.size());				//Show number of occupied positions
			InvalidateRect(PositionReportCanvas->hWnd, NULL, TRUE);					//Refresh canvas
		}
		return;
	}

	if (SelPosX + SelPosY * 10 != PrevPos) {								//If cursor moved from one position to another
		PrevPos = SelPosX + SelPosY * 10;										//Remember the current position
		if (ParkingPos[PrevPos]) {												//Position occupied
			PositionReportCanvas->Print(PositionAreaWidth + 45, 30,
				L"Parking Position #%i:", PrevPos + 1);
			PositionReportCanvas->Print(PositionAreaWidth + 45, 50,
				L"Status: Occupied");
			
			//Find out the number of occupied position, which is the index of CurrParkedCars
			int nOccupiedPos = 0;
			for (int i = PrevPos - 1; i >= 0; i--) {
				if (ParkingPos[i])
					nOccupiedPos++;
			}
			PositionReportCanvas->Print(PositionAreaWidth + 45, 70,
				L"Car Number: %s", LogFile->FileContent.LogData[CurrParkedCars[nOccupiedPos]].CarNumber);
			
			//Show enter time & est. fee info
			SYSTEMTIME stEnter = LogFile->FileContent.LogData[CurrParkedCars[nOccupiedPos]].EnterTime;
			SYSTEMTIME stNow;
			int HourDifference;

			PositionReportCanvas->Print(PositionAreaWidth + 45, 90,
				L"Enter Time: %04u-%02u-%02u %02u:%02u:%02u",
				stEnter.wYear, stEnter.wMonth, stEnter.wDay, stEnter.wHour, stEnter.wMinute, stEnter.wSecond);
			GetLocalTime(&stNow);
			PositionReportCanvas->Print(PositionAreaWidth + 45, 130,
				L"Estimated Fee (Until Now): $%.2f", CalcFee(&stEnter, &stNow, &HourDifference));
			PositionReportCanvas->Print(PositionAreaWidth + 45, 110,
				L"Hours Parked (Until Now): %i", HourDifference);
		}
		else {																	//Position unoccupied
			PositionReportCanvas->Print(PositionAreaWidth + 45, 30,
				L"Parking Position #%i:", PrevPos + 1);
			PositionReportCanvas->Print(PositionAreaWidth + 45, 50,
				L"Status: Unoccupied");
		}
		InvalidateRect(PositionReportCanvas->hWnd, NULL, TRUE);					//Refresh canvas
	}
}

/*
Description:	To handle paint event of history report canvas
*/
void HistoryReportCanvas_Paint() {
	//Calculate width and height of each position box
	int BoxW = (HistoryReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2 / 10,
		BoxH = (HistoryReportCanvas->bi.bmiHeader.biHeight - 60) / 10;

	//Paint
	RECT	BoxPos;
	HistoryReportCanvas->Cls();
	
	if (BoxW > 0 && BoxH > 0) {												//Make sure the window is large enough to draw everything
		for (int i = 0; i < 10; i++) {											//Rows
			for (int j = 0; j < 10; j++) {											//Columns
				BoxPos.top = 30 + i * BoxH;												//Calculate the position of box
				BoxPos.left = 30 + j * BoxW;
				BoxPos.right = BoxPos.left + BoxW;
				BoxPos.bottom = BoxPos.top + BoxH;

				if (HistoryParkedCars[i * 10 + j].EnterTime.wYear) {					//Position occupied
					//Draw gray background with a cross
					FillRect(HistoryReportCanvas->hDC, &BoxPos, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
					HistoryReportCanvas->DrawLine(BoxPos.left, BoxPos.top, BoxPos.right, BoxPos.bottom);
					HistoryReportCanvas->DrawLine(BoxPos.right, BoxPos.top, BoxPos.left, BoxPos.bottom);
				}

				//Draw border
				HistoryReportCanvas->DrawRect(BoxPos.left, BoxPos.top, BoxPos.right, BoxPos.bottom);

				//Print number of position
				HistoryReportCanvas->Print(BoxPos.left, BoxPos.top, L"%i", i * 10 + j + 1);
			}
		}
	}
}

/*
Description:	To handle time changed event of date/time picker of history report
*/
void dtpHistoryDate_DateTimeChanged() {
	SYSTEMTIME	stSelectedTime;													//The time user selected
	SYSTEMTIME	stTmp;															//The time of the control
	LogInfo		CarInfo;														//Info of current car

	dtpHistoryDate->GetTime(&stTmp);											//Get selected date from date picker
	stSelectedTime.wYear = stTmp.wYear;
	stSelectedTime.wMonth = stTmp.wMonth;
	stSelectedTime.wDay = stTmp.wDay;
	dtpHistoryTime->GetTime(&stTmp);											//Get selected time from time picker
	stSelectedTime.wHour = stTmp.wHour;
	stSelectedTime.wMinute = stTmp.wMinute;
	stSelectedTime.wSecond = stTmp.wSecond;

	memset(HistoryParkedCars, 0, sizeof(LogInfo) * 100);						//Initialize history parked cars array
	HistoryParkedCarsCount = 0;													//Reset number of parked cars
	for (UINT i = 0; i < LogFile->FileContent.ElementCount; i++) {				//Find all cars match the specified time
		CarInfo = LogFile->FileContent.LogData[i];									//Get info of current car
		
		//If Enter Time <= Selected Time <= Leave Time,
		//the car is in the park at the specified time
		//Note that (wYear == 0) means the car is still parking
		if (((UINT)(CarInfo.EnterTime.wYear * 365 * 24 * 3600 + CarInfo.EnterTime.wMonth * 31 * 24 * 3600 + CarInfo.EnterTime.wDay * 24 * 3600 +
			CarInfo.EnterTime.wHour * 3600 + CarInfo.EnterTime.wMinute * 60 + CarInfo.EnterTime.wSecond) <=
			(UINT)(stSelectedTime.wYear * 365 * 24 * 3600 + stSelectedTime.wMonth * 31 * 24 * 3600 + stSelectedTime.wDay * 24 * 3600 +
			stSelectedTime.wHour * 3600 + stSelectedTime.wMinute * 60 + stSelectedTime.wSecond)) && (
			((UINT)CarInfo.LeaveTime.wYear * 365 * 24 * 3600 + CarInfo.LeaveTime.wMonth * 31 * 24 * 3600 + CarInfo.LeaveTime.wDay * 24 * 3600 +
			CarInfo.LeaveTime.wHour * 3600 + CarInfo.LeaveTime.wMinute * 60 + CarInfo.LeaveTime.wSecond >=
			(UINT)(stSelectedTime.wYear * 365 * 24 * 3600 + stSelectedTime.wMonth * 31 * 24 * 3600 + stSelectedTime.wDay * 24 * 3600 +
			stSelectedTime.wHour * 3600 + stSelectedTime.wMinute * 60 + stSelectedTime.wSecond)) || (CarInfo.LeaveTime.wYear == 0))) {
			
			HistoryParkedCars[CarInfo.CarPos] = CarInfo;								//Record car info
			HistoryParkedCarsCount++;													//Number of parked cars + 1
		}
	}

	HistoryReportCanvas_Paint();												//Invoke canvas redraw
	InvalidateRect(HistoryReportCanvas->hWnd, NULL, TRUE);						//Refresh canvas
}

/*
Description:	To handle mouse move event of history report canvas
Args:			X, Y: Position of cursor
*/
void HistoryReportCanvas_MouseMove(int X, int Y) {
	//Calculate width and height of each position box
	int HistoryAreaWidth = (HistoryReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2;
	int BoxW = HistoryAreaWidth / 10,
		BoxH = (HistoryReportCanvas->bi.bmiHeader.biHeight - 60) / 10;

	//Calculate the car position under the cursor
	int SelPosX = (X - 30) / BoxW;
	int SelPosY = (Y - 30) / BoxH;
	static int PrevPos = -2;												//Remember the previous selected car position to reduce CPU usage

	if (SelPosX > 9 || SelPosY > 9 || X < 30 || Y < 30) {					//If cursor moved out of the position area
		if (PrevPos != -1) {													//If the cursor is in the position area previously
			PrevPos = -1;
			HistoryReportCanvas->Print(HistoryAreaWidth + 45, 70,
				L"Occupied Positions: %i/100", HistoryParkedCarsCount);				//Show number of occupied positions
			InvalidateRect(HistoryReportCanvas->hWnd, NULL, TRUE);					//Refresh canvas
		}
		return;
	}

	if (SelPosX + SelPosY * 10 != PrevPos) {								//If cursor moved from one position to another
		PrevPos = SelPosX + SelPosY * 10;										//Remember the current position
		if (HistoryParkedCars[PrevPos].EnterTime.wYear) {						//Position occupied
			HistoryReportCanvas->Print(HistoryAreaWidth + 45, 70,
				L"Parking Position #%i:", PrevPos + 1);
			HistoryReportCanvas->Print(HistoryAreaWidth + 45, 90,
				L"Status: Occupied");

			//Find out the number of occupied position, which is the index of CurrParkedCars
			int nOccupiedPos = 0;
			for (int i = PrevPos - 1; i >= 0; i--) {
				if (ParkingPos[i])
					nOccupiedPos++;
			}
			HistoryReportCanvas->Print(HistoryAreaWidth + 45, 110,
				L"Car Number: %s", HistoryParkedCars[PrevPos].CarNumber);

			//Show enter time & est. fee info
			SYSTEMTIME stEnter = HistoryParkedCars[PrevPos].EnterTime;
			HistoryReportCanvas->Print(HistoryAreaWidth + 45, 130,
				L"Enter Time: %04u-%02u-%02u %02u:%02u:%02u",
				stEnter.wYear, stEnter.wMonth, stEnter.wDay, stEnter.wHour, stEnter.wMinute, stEnter.wSecond);
		}
		else {																	//Position unoccupied
			HistoryReportCanvas->Print(HistoryAreaWidth + 45, 70,
				L"Parking Position #%i:", PrevPos + 1);
			HistoryReportCanvas->Print(HistoryAreaWidth + 45, 90,
				L"Status: Unoccupied");
		}
		InvalidateRect(HistoryReportCanvas->hWnd, NULL, TRUE);					//Refresh canvas
	}
}

/*
Description:	To handle paint event of daily report canvas
*/
void DailyReportCanvas_Paint() {

}

/*
Description:	To handle mouse move event of daily report canvas
Args:			X, Y: Position of cursor
*/
void DailyReportCanvas_MouseMove(int X, int Y) {

}

/*
Description:	To handle paint event of monthly report canvas
*/
void MonthlyReportCanvas_Paint() {

}

/*
Description:	To handle mouse move event of monthly report canvas
Args:			X, Y: Position of cursor
*/
void MonthlyReportCanvas_MouseMove(int X, int Y) {

}

/*
Description:	Comparison function of ListView item sorting
Args:			lParam1: Current index of the first item
				lParam2: Current index of the second item
				lParamSort: Additional info specified by wParam of LVM_SORTITEMSEX message (Pointer to a lvSortInfo structure)
Return:			A negative value if the first item should precede the second,
				a positive value if the first item should follow the second,
				or zero if the two items are equivalent
*/
int CALLBACK ListViewCompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	LVITEM	lvi1 = { 0 }, lvi2 = { 0 };										//ListView item info
	wchar_t	buf1[255], buf2[255];											//Text buffer

	lvi1.cchTextMax = lvi2.cchTextMax = 255;								//Set buffer text size
	lvi1.iSubItem = lvi2.iSubItem = ((lvSortInfo*)lParamSort)->HeaderIndex;	//Set sub item index
	lvi1.pszText = buf1;													//Set buffer address
	lvi2.pszText = buf2;
	SendMessage(lvLog->hWnd, LVM_GETITEMTEXT, lParam1, (LPARAM)&lvi1);
	SendMessage(lvLog->hWnd, LVM_GETITEMTEXT, lParam2, (LPARAM)&lvi2);

	if (((lvSortInfo*)lParamSort)->Ascending)								//Return value depends on sorting direction
		return lstrcmpW(buf1, buf2);
	else
		return -lstrcmpW(buf1, buf2);
}

/*
Description:	To handle log listview header clicked event
*/
void lvLog_HeaderClicked(int Index) {
	static bool Ascending[5] = { true, true, true, true, true };			//Ascending or descending (initial = ascending)

	Ascending[Index] = !Ascending[Index];									//Reverse sorting direction
	lvSortInfo	si = { Ascending[Index], Index };							//Set sorting info
	SendMessage(lvLog->hWnd, LVM_SORTITEMSEX, (WPARAM)&si, (LPARAM)ListViewCompareFunction);
}

/*
Description:	To handle tab selected event for report tab control
*/
void tabReport_TabSelected() {
	switch (tabReport->GetSel()) {											//Get the index of selected tab
	case 0:																	//Position report
		CurrStatus = 5;
		PositionReportCanvas->SetVisible(true);
		HistoryReportCanvas->SetVisible(false);
		DailyReportCanvas->SetVisible(false);
		MonthlyReportCanvas->SetVisible(false);
		PositionReportCanvas_Paint();											//Invoke canvas redraw
		InvalidateRect(PositionReportCanvas->hWnd, NULL, TRUE);					//Refresh canvas
		PositionReportCanvas->Print((PositionReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2 + 45, 30,
			L"Occupied Positions: %i/100", CurrParkedCars.size());				//Show number of occupied positions
		break;

	case 1:																	//History report
		CurrStatus = 6;
		PositionReportCanvas->SetVisible(false);
		HistoryReportCanvas->SetVisible(true);
		DailyReportCanvas->SetVisible(false);
		MonthlyReportCanvas->SetVisible(false);
		dtpHistoryDate_DateTimeChanged();
		HistoryReportCanvas_Paint();											//Invoke canvas redraw
		HistoryReportCanvas->Print((HistoryReportCanvas->bi.bmiHeader.biWidth - 60) / 3 * 2 + 45, 70,
			L"Occupied Positions: %i/100", HistoryParkedCarsCount);				//Show number of occupied positions
		InvalidateRect(HistoryReportCanvas->hWnd, NULL, TRUE);					//Refresh canvas
		break;

	case 2:																	//Daily report
		CurrStatus = 7;
		PositionReportCanvas->SetVisible(false);
		HistoryReportCanvas->SetVisible(false);
		DailyReportCanvas->SetVisible(true);
		MonthlyReportCanvas->SetVisible(false);
		DailyReportCanvas_Paint();												//Invoke canvas redraw
		break;

	case 3:																	//Monthly report
		CurrStatus = 8;
		PositionReportCanvas->SetVisible(false);
		HistoryReportCanvas->SetVisible(false);
		DailyReportCanvas->SetVisible(false);
		MonthlyReportCanvas->SetVisible(true);
		MonthlyReportCanvas_Paint();											//Invoke canvas redraw
		break;
	}

	RECT	MainWindowSize;
	GetClientRect(GetMainWindowHandle(), &MainWindowSize);					//Get window size
	MainWindow_Resize(GetMainWindowHandle(),
		MainWindowSize.right - MainWindowSize.left,
		MainWindowSize.bottom - MainWindowSize.top);						//Invoke window resize event to resize the canvas
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
	tabReport = make_shared<IceTab>(hWnd, IDC_REPORTTAB, tabReport_TabSelected);
	PositionReportCanvas = make_shared<IceCanvas>(tabReport->hWnd, 0xffffff, PositionReportCanvas_Paint, PositionReportCanvas_MouseMove);
	HistoryReportCanvas = make_shared<IceCanvas>(tabReport->hWnd, 0xffffff, HistoryReportCanvas_Paint, HistoryReportCanvas_MouseMove);
	DailyReportCanvas = make_shared<IceCanvas>(tabReport->hWnd, 0xffffff, DailyReportCanvas_Paint, DailyReportCanvas_MouseMove);
	MonthlyReportCanvas = make_shared<IceCanvas>(tabReport->hWnd, 0xffffff, MonthlyReportCanvas_Paint, MonthlyReportCanvas_MouseMove);
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
	dtpHistoryDate = make_shared<IceDateTimePicker>(hWnd, IDC_HISTORYDATEPICKER, (VOID_EVENT)dtpHistoryDate_DateTimeChanged);
	dtpHistoryTime = make_shared<IceDateTimePicker>(hWnd, IDC_HISTORYTIMEPICKER, (VOID_EVENT)dtpHistoryDate_DateTimeChanged);
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
	tabReport->Move(0, 0);													//Set the position of report tab
	SendMessage(edPassword->hWnd, EM_SETLIMITTEXT, 20, 0);					//Max length of password editbox
	SendMessage(edCarNumber->hWnd, EM_SETLIMITTEXT, 10, 0);					//Max length of car number editbox
	lvLog->AddColumn(L"#", 40);												//Add columns to log listview
	lvLog->AddColumn(L"Car Number", 120);
	lvLog->AddColumn(L"Enter Time (YYYY-MM-DD HH:MM:SS)", 145);
	lvLog->AddColumn(L"Leave Time (YYYY/MM/DD HH:MM:SS)", 145);
	lvLog->AddColumn(L"Position", 80);
	lvLog->AddColumn(L"Fee", 70);
	tabReport->InsertTab(L"Position Info");									//Add tabs to tab control
	tabReport->InsertTab(L"History");
	tabReport->InsertTab(L"Daily Report");
	tabReport->InsertTab(L"Monthly Report");
	SetParent(dtpHistoryDate->hWnd, HistoryReportCanvas->hWnd);				//Set history date/time pickers as child window of history report canvas
	SetParent(dtpHistoryTime->hWnd, HistoryReportCanvas->hWnd);
	SetProp(FindWindowEx(lvLog->hWnd, NULL, L"SysHeader32", NULL), L"HeaderClickEvent", (HANDLE)lvLog_HeaderClicked);

	//Set canvas positions
	//There's a updown control in the tab control, so we can determine
	//the height of tab header by retrieving the height of updown control
	RECT	UpDownCtlRect;
	GetWindowRect(FindWindowEx(tabReport->hWnd, NULL, L"msctls_updown32", NULL), &UpDownCtlRect);
	TabHeaderHeight = UpDownCtlRect.bottom - UpDownCtlRect.top + 4;
	PositionReportCanvas->Move(0, TabHeaderHeight);
	HistoryReportCanvas->Move(0, TabHeaderHeight);
	HistoryReportCanvas->SetVisible(false);
	DailyReportCanvas->Move(0, TabHeaderHeight);
	DailyReportCanvas->SetVisible(false);
	MonthlyReportCanvas->Move(0, TabHeaderHeight);
	MonthlyReportCanvas->SetVisible(false);

	//Load data file
	LogFile = make_shared<IceEncryptedFile>(L"Log.dat");

	//Set window focus to the password editbox
	SetFocus(edPassword->hWnd);
}

/*
Description:	To lock the system
*/
void mnuLock_Click() {
	CurrStatus = 4;															//Change status to locked mode
	SetMenu(GetMainWindowHandle(), NULL);									//Remove window menu
	lvLog->SetVisible(false);												//Hide unrelated controls
	tabReport->SetVisible(false);
	ShowPasswordFrame();
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

		SetFocus(edCarNumber->hWnd);										//Let the car number editbox has focus
	}
	else if (CurrStatus == 4)											//Cannot exit the system when the system is locked
		return;
	else {																//Normal mode, user is goint to exit the system
		//Show a prompt when exiting
		if (1)/*ToDo: uncomment (MessageBox(GetMainWindowHandle(),
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
	tabReport->SetVisible(false);
	ShowPaymentFrame();
	SetMenu(GetMainWindowHandle(), NULL);									//Remove window menu
	SetFocus(edCarNumber->hWnd);											//Set input focus to the car number textbox
	CurrStatus = 1;															//Change status to payment mode

	RECT	MainWindowSize;
	GetClientRect(GetMainWindowHandle(), &MainWindowSize);					//Get window size
	MainWindow_Resize(GetMainWindowHandle(),
		MainWindowSize.right - MainWindowSize.left,
		MainWindowSize.bottom - MainWindowSize.top);						//Invoke window resize payment controls
}

/*
Description:	To handle show Log menu event
*/
void mnuLog_Click() {
	lvLog->DeleteAllItems();												//Clear log listview
	for (UINT i = 0; i < LogFile->FileContent.ElementCount; i++) {			//Add all log info to the listview
		//Index
		lvLog->AddItem(L"%i", -1, i + 1);

		//Car number
		lvLog->SetItemText(i, LogFile->FileContent.LogData[i].CarNumber, 1);

		//Enter time
		lvLog->SetItemText(i, L"%04u-%02u-%02u %02u:%02u:%02u", 2,
			LogFile->FileContent.LogData[i].EnterTime.wYear,
			LogFile->FileContent.LogData[i].EnterTime.wMonth,
			LogFile->FileContent.LogData[i].EnterTime.wDay,
			LogFile->FileContent.LogData[i].EnterTime.wHour,
			LogFile->FileContent.LogData[i].EnterTime.wMinute,
			LogFile->FileContent.LogData[i].EnterTime.wSecond);

		if (LogFile->FileContent.LogData[i].LeaveTime.wYear) {					//The car has left
			//Leave time
			lvLog->SetItemText(i, L"%04u-%02u-%02u %02u:%02u:%02u", 3,
				LogFile->FileContent.LogData[i].LeaveTime.wYear,
				LogFile->FileContent.LogData[i].LeaveTime.wMonth,
				LogFile->FileContent.LogData[i].LeaveTime.wDay,
				LogFile->FileContent.LogData[i].LeaveTime.wHour,
				LogFile->FileContent.LogData[i].LeaveTime.wMinute,
				LogFile->FileContent.LogData[i].LeaveTime.wSecond);

			//Fee
			lvLog->SetItemText(i, L"$%.2f", 5, LogFile->FileContent.LogData[i].Fee);
		}
		else																	//The car is still parking
			lvLog->SetItemText(i, L"Still Parking", 3);

		//Car position
		lvLog->SetItemText(i, L"%i", 4, LogFile->FileContent.LogData[i].CarPos + 1);
	}
	tabReport->SetVisible(false);											//Hide report tab
	lvLog->SetVisible(true);												//Show log listview
	CurrStatus = 2;															//Change status to log mode

	RECT	MainWindowSize;
	GetClientRect(GetMainWindowHandle(), &MainWindowSize);					//Get window size
	MainWindow_Resize(GetMainWindowHandle(),
		MainWindowSize.right - MainWindowSize.left,
		MainWindowSize.bottom - MainWindowSize.top);						//Invoke window resize event to resize listview
}

/*
Description:	To handle show report menu event
*/
void mnuReport_Click() {
	lvLog->SetVisible(false);
	tabReport->SetVisible(true);
	tabReport->SetSel(0);													//Set selected tab
	tabReport_TabSelected();												//Invoke tab selected event to show canvas

	RECT	MainWindowSize;
	GetClientRect(GetMainWindowHandle(), &MainWindowSize);					//Get window size
	MainWindow_Resize(GetMainWindowHandle(),
		MainWindowSize.right - MainWindowSize.left,
		MainWindowSize.bottom - MainWindowSize.top);						//Invoke window resize event to resize tab control
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