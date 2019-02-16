/*
Description:    Handle file creation, modification and
                other operations for the whole system
Author:         Hanson
File:           FileManager.cpp
*/

#include "FileManager.h"

/*
Description:    Constructor of encrypted file class
Args:			FilePath: Log file path
*/
IceEncryptedFile::IceEncryptedFile(wchar_t *FilePath) {
	_wfopen_s(&hFile, FilePath, L"r+");													//Open log file
	if (!hFile) {
		_wfopen_s(&hFile, FilePath, L"w+");													//Create log file if it doesn't exist
		if (!hFile) {																		//Path not accessible
			if (MessageBox(GetMainWindowHandle(),
				L"Failed to access log file! Continue without log file?",
				L"Error",
				MB_ICONERROR | MB_YESNO) == IDNO) {

				DestroyWindow(GetMainWindowHandle());											//Quit the system
				PostQuitMessage(0);
			}
		}
	}
}

/*
Description:    Add a new record to the file
Args:			CarNumber: Car number
				EnterTime: Enter time of the car
				LeaveTime: Leave time of the car
				CarPos: Parked position
				Fee: Fee paid
Return:			true if succeed, false otherwise
*/
bool IceEncryptedFile::AddLog(wchar_t *CarNumber, SYSTEMTIME EnterTime, SYSTEMTIME LeaveTime, int CarPos, int Fee) {
	if (!hFile)																					//Continued without log file
		return false;

	LogInfo	info;
	lstrcpyW(info.CarNumber, CarNumber);
	info.EnterTime = EnterTime;
	info.LeaveTime = LeaveTime;
	info.CarPos = CarPos;
	info.Fee = Fee;

	LogData.push_back(info);
	return true;
}

bool IceEncryptedFile::SaveFile() {
	if (!hFile)																					//Continued without log file
		return false;
	return true;
}