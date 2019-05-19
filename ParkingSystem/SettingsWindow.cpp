/*
Description:    Handle settings window events
Author:         Hanson
File:           SettingsWindow.cpp
*/

#include "FileManager.h"

/* Control bindings */
shared_ptr<IceEdit>				edCurrPassword;
shared_ptr<IceEdit>				edNewPassword;
shared_ptr<IceEdit>				edConfirmPassword;
shared_ptr<IceEdit>				edFeePerHour;
shared_ptr<IceButton>			cmdOK;
shared_ptr<IceButton>			cmdCancel;
IceEncryptedFile				*LogFile;
HWND							SettingsWindowHandle;

/*
Description:	To handle cancel button clicked event
*/
void cmdCancel_Click() {
	EnableWindow(GetMainWindowHandle(), TRUE);										//Enable main window
	DestroyWindow(SettingsWindowHandle);											//Close the window
}

/*
Description:	To handle OK button clicked event
*/
void cmdOK_Click() {
	wchar_t	PasswordBuffer[20];														//Buffer to store new password and current password
	wchar_t ConfirmPasswordBuffer[20];												//Buffer to store confirm password
	wchar_t	FeeBuffer[10];															//Buffer to store fee string
	float	NewFee;																	//New fee

	edCurrPassword->GetText(PasswordBuffer);										//Get entered current password
	if (lstrlenW(PasswordBuffer) != 0) {											//If user entered current password, it means the user wants to change the password
		if (!lstrcmpW(PasswordBuffer, LogFile->FileContent.Password)) {					//Password match
			edNewPassword->GetText(PasswordBuffer);
			edConfirmPassword->GetText(ConfirmPasswordBuffer);
			if (!lstrcmpW(PasswordBuffer, ConfirmPasswordBuffer)) {							//New password equals confirm password
				lstrcpyW(LogFile->FileContent.Password, PasswordBuffer);						//Store the new password
			}
			else {
				MessageBox(SettingsWindowHandle, L"New password and confirm password are not the same!",
					L"Failed to Change Password", MB_OK | MB_ICONEXCLAMATION);
				SendMessage(edNewPassword->hWnd, EM_SETSEL, 0, -1);								//Select all text in the textbox
				SetFocus(edNewPassword->hWnd);
				return;
			}
		}
		else {																			//Password unmatch
			MessageBox(SettingsWindowHandle, L"Incorrect password! Settings are not saved.",
				L"Failed to Change Password", MB_OK | MB_ICONEXCLAMATION);
			SendMessage(edCurrPassword->hWnd, EM_SETSEL, 0, -1);							//Select all text in the textbox
			SetFocus(edCurrPassword->hWnd);
			return;
		}
	}
	edFeePerHour->GetText(FeeBuffer);												//Get entered fee per hour
	NewFee = _wtof(FeeBuffer);														//Convert fee string to float
	if (NewFee > 0) {																//Check if the value is valid
		LogFile->FileContent.FeePerHour = NewFee;										//Store the new fee value
	}
	else {																			//Value is invalid
		MessageBox(SettingsWindowHandle, L"Invalid fee value! Please enter again.",
			L"Failed to Change Fee", MB_OK | MB_ICONEXCLAMATION);
		SendMessage(edFeePerHour->hWnd, EM_SETSEL, 0, -1);								//Select all text in the textbox
		SetFocus(edFeePerHour->hWnd);
		return;
	}
	if (!LogFile->SaveFile()) {														//Save data file
		MessageBox(SettingsWindowHandle, L"Settings applied, but failed saving settings to \"Log.dat\"! Please check if the file can be accessed.",
			L"Failed to Save Settings", MB_OK | MB_ICONERROR);
	}
	cmdCancel_Click();																//Close the window
}

/*
Description:    To handle settings window creation event
*/
void SettingsWindow_Create(HWND hWnd) {
	SettingsWindowHandle = hWnd;

	//Set the icon of the window
	HICON hIcon;
	hIcon = (HICON)LoadImage(GetProgramInstance(),
		MAKEINTRESOURCE(IDI_MAINICON),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON), 0);
	if (hIcon)
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//Bind controls with their corresponding classes
	edCurrPassword = make_shared<IceEdit>(hWnd, IDC_CURRENTPASSWORDEDIT, (WNDPROC)NULL);
	edNewPassword = make_shared<IceEdit>(hWnd, IDC_NEWPASSWORDEDIT, (WNDPROC)NULL);
	edConfirmPassword = make_shared<IceEdit>(hWnd, IDC_CONFIRMPASSWORDEDIT, (WNDPROC)NULL);
	edFeePerHour = make_shared<IceEdit>(hWnd, IDC_FEEPERHOUREDIT, (WNDPROC)NULL);
	cmdOK = make_shared<IceButton>(hWnd, IDC_OKBUTTON, cmdOK_Click);
	cmdCancel = make_shared<IceButton>(hWnd, IDC_CANCELBUTTON, cmdCancel_Click);

	//Set control properties
	wchar_t	FeeStr[10];														//Buffer to store converted fee value
	
	SendMessage(edFeePerHour->hWnd, EM_SETLIMITTEXT, 8, 0);					//Max length of fee per hour editbox
	SendMessage(edCurrPassword->hWnd, EM_SETLIMITTEXT, 20, 0);				//Max length of password editboxes
	SendMessage(edNewPassword->hWnd, EM_SETLIMITTEXT, 20, 0);
	SendMessage(edConfirmPassword->hWnd, EM_SETLIMITTEXT, 20, 0);

	LogFile = (IceEncryptedFile*)GetLogFilePtr();							//Get a pointer to LogFile
	swprintf_s(FeeStr, L"%.2f", LogFile->FileContent.FeePerHour);			//Get fee per hour
	edFeePerHour->SetText(FeeStr);
}