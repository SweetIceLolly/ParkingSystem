/*
Description:    Handle file creation, modification and
                other operations for the whole system
Author:         Hanson
File:           FileManager.cpp
*/

#include "FileManager.h"
#include <string.h>

/*
Description:    Constructor of encrypted file class
Args:			FilePath: Log file path
*/
IceEncryptedFile::IceEncryptedFile(wchar_t *FilePath) {
	//Open log file
	_wfopen_s(&lpFile, FilePath, L"r+");
	if (!lpFile) {
		_wfopen_s(&lpFile, FilePath, L"w+");													//Create log file if it doesn't exist
		if (!lpFile) {																			//Path not accessible
			if (MessageBox(GetMainWindowHandle(),
				L"Failed to access log file! Continue without log file?",
				L"Error",
				MB_ICONERROR | MB_YESNO) == IDNO) {

				DestroyWindow(GetMainWindowHandle());												//Quit the system
				PostQuitMessage(0);
				return;
			}
			
			if (!SaveFile()) {																	//Initalize the file content
				if (MessageBox(GetMainWindowHandle(),
					L"Failed to initalize the log file content! Please make sure the file is writable! Continue without log file?",
					L"Error",
					MB_ICONERROR | MB_YESNO) == IDNO) {

					DestroyWindow(GetMainWindowHandle());												//Quit the system
					PostQuitMessage(0);
				}
				return;
			}
		}
	}

	//Check file size
	fseek(lpFile, 0, SEEK_END);
	int		szFile = ftell(lpFile);																//Get file size
	rewind(lpFile);
	if (szFile < sizeof(FileContent)) {															//Invalid file size
		if (MessageBox(GetMainWindowHandle(),
			L"Invalid file content! Create a new file to replace current file?",
			L"Error",
			MB_ICONERROR | MB_YESNO) == IDNO) {

			fclose(lpFile);																			//Don't create a new file
			MessageBox(GetMainWindowHandle(),
				L"Continuing without log file!",
				L"Invalid File Content",
				MB_ICONEXCLAMATION);
			return;
		}
		else {																					//Create a new file
			if (!SaveFile()) {																		//Failed to create a new file
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
Description:    Destructor of encrypted file class
*/
IceEncryptedFile::~IceEncryptedFile() {
	//Close log file
	fclose(lpFile);
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

/*
Description:    Save the file content
Return:			true if succeed, false otherwise
*/
bool IceEncryptedFile::SaveFile() {
	if (!lpFile)																				//Continued without log file
		return false;
	
	int		szFile = sizeof(wchar_t) * 20 + sizeof(UINT) + sizeof(LogInfo) * FileContent.ElementCount;
	BYTE	*Buffer = new BYTE[szFile];															//Allocate binary content buffer

	memcpy(Buffer, FileContent.Password, sizeof(wchar_t) * 20);									//Password
	memcpy(Buffer + sizeof(wchar_t) * 20, &(FileContent.ElementCount), sizeof(UINT));			//Element count
	memcpy(Buffer + sizeof(wchar_t) * 20 + 4, FileContent.LogData.data(),
		sizeof(LogInfo) * FileContent.ElementCount);											//All log data
	int KeyLen = lstrlenW(FileContent.Password);
	for (int i = 0; i < szFile; Buffer[i++] ^= (487 ^ FileContent.Password[i % KeyLen]));		//Encrypt binary data
	rewind(lpFile);
	fwrite(Buffer, szFile, 1, lpFile);															//Write to the file
	fflush(lpFile);

	delete[] Buffer;																			//Deallocate binary content buffer
	return true;
}

/*
Description:    Read the file and decrypt the content with the password provided
Args:			Password: The password to the file
Return:			true if succeed, false otherwise
*/
bool IceEncryptedFile::ReadFile(wchar_t *Password) {
	if (!lpFile)																				//Continued without log file
		return false;

	int		KeyLen = lstrlenW(Password);														//Get password length
	if (KeyLen <= 0)																			//Password not provided
		return false;

	//Get file size
	fseek(lpFile, 0, SEEK_END);
	int		szFile = ftell(lpFile);
	rewind(lpFile);
	BYTE	*Buffer = new BYTE[szFile];															//Allocate file reading buffer
	
	//Read file contents
	fread_s(Buffer, szFile, szFile, 1, lpFile);													//Read whole file
	for (int i = 0; i < szFile; Buffer[i++] ^= (487 ^ Password[i % KeyLen]));	 				//Decrypt binary data with the provided password
	if (!lstrcmpW((LPWSTR)Buffer, Password)) {													//Check if the decrypted password matches with the provided password
		memcpy(FileContent.Password, Buffer, sizeof(wchar_t) * 20);									//Password
		memcpy(&(FileContent.ElementCount), Buffer + sizeof(wchar_t) * 20, sizeof(UINT));			//Element count
		FileContent.LogData.resize(FileContent.ElementCount);										//Allocate LogData elements
		memcpy(FileContent.LogData.data(), Buffer + sizeof(wchar_t) * 20 + 4,
			sizeof(LogInfo)* FileContent.ElementCount);												//All log data
		return true;
	}
	else																						//Password unmatch
		return false;

	delete[] Buffer;																			//Deallocate buffer
	return true;
}