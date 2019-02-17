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
	//Open log file
	_wfopen_s(&lpFile, FilePath, L"r+");
	if (!lpFile) {
		_wfopen_s(&lpFile, FilePath, L"w+");												//Create log file if it doesn't exist
		if (!lpFile) {																		//Path not accessible
			if (MessageBox(GetMainWindowHandle(),
				L"Failed to access log file! Continue without log file?",
				L"Error",
				MB_ICONERROR | MB_YESNO) == IDNO) {

				DestroyWindow(GetMainWindowHandle());											//Quit the system
				PostQuitMessage(0);
				return;
			}
			
			if (!SaveFile()) {																//Initalize the file content
				if (MessageBox(GetMainWindowHandle(),
					L"Failed to initalize the log file content! Please make sure the file is writable! Continue without log file?",
					L"Error",
					MB_ICONERROR | MB_YESNO) == IDNO) {

					DestroyWindow(GetMainWindowHandle());											//Quit the system
					PostQuitMessage(0);
				}
				return;
			}
		}
	}

	SYSTEMTIME a = { 0 };
	lstrcpyW(FileContent.Password, L"I Like Emily");
	AddLog(L"123456s", (SYSTEMTIME)a, (SYSTEMTIME)a, 15, 48);
	AddLog(L"1abcde", (SYSTEMTIME)a, (SYSTEMTIME)a, 385, 786);
	AddLog(L"aefsad", (SYSTEMTIME)a, (SYSTEMTIME)a, 42, 345);

	//Read log file contents
	fseek(lpFile, 0, SEEK_END);
	int szFile = ftell(lpFile);																//Get file size
	rewind(lpFile);
	if (szFile < sizeof(FileContent)) {														//Invalid file size
		if (MessageBox(GetMainWindowHandle(),
			L"Invalid file content! Create a new file to replace current file?",
			L"Error",
			MB_ICONERROR | MB_YESNO) == IDNO) {

			fclose(lpFile);
			MessageBox(GetMainWindowHandle(),
				L"Continuing without log file!",
				L"Invalid File Content",
				MB_ICONEXCLAMATION);
			return;
		}
		else {
			if (!SaveFile()) {
				fclose(lpFile);
				MessageBox(GetMainWindowHandle(),
					L"Failed to create a new file! Continuing without log file!",
					L"Invalid File Content",
					MB_ICONEXCLAMATION);
				return;
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
bool IceEncryptedFile::AddLog(wchar_t CarNumber[10], SYSTEMTIME EnterTime, SYSTEMTIME LeaveTime, int CarPos, int Fee) {
	if (!lpFile)																				//Continued without log file
		return false;

	LogInfo	info;																				//Set the content
	lstrcpyW(info.CarNumber, CarNumber);
	info.EnterTime = EnterTime;
	info.LeaveTime = LeaveTime;
	info.CarPos = CarPos;
	info.Fee = Fee;

	FileContent.LogData.push_back(info);														//Add log
	FileContent.ElementCount++;	
	return true;
}

bool IceEncryptedFile::SaveFile() {
	if (!lpFile)																				//Continued without log file
		return false;
	
	rewind(lpFile);
	//fwrite(&FileContent, sizeof(FileContent), 1, lpFile);
	fwrite(FileContent.Password, sizeof(wchar_t), 20, lpFile);									//Password
	fwrite(&(FileContent.ElementCount), sizeof(int), 4, lpFile);								//Element count
	fwrite(FileContent.LogData.data(), sizeof(LogInfo), FileContent.ElementCount, lpFile);		//All log data
	return true;
}