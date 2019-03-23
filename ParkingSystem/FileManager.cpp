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
IceEncryptedFile::IceEncryptedFile(const wchar_t *FilePath) {
	//Open log file
	lstrcpyW(FileContent.Password, L"123");													//Set the default password
	FileContent.FeePerHour = 10;															//Set the default fee per hour
	FileContent.ElementCount = 0;															//Set the default element count
	fsFile.open(FilePath, ios::binary | ios::in | ios::out);								//Attempt to open the file with read/write privilege
	if (fsFile.fail()) {
		fsFile.open(FilePath, ios::binary | ios::out);											//Create log file if it doesn't exist, open it with write privilege only
		if (fsFile.fail()) {																		//Path not accessible
			fsFile.close();
			if (MessageBox(GetMainWindowHandle(),
				L"Failed to access log file! Continue without log file?",
				L"Error",
				MB_ICONERROR | MB_YESNO) == IDNO) {

				DestroyWindow(GetMainWindowHandle());												//Quit the system
				PostQuitMessage(0);
			}
			else																				//Continuing without log file
				WithoutFile = true;
			return;
		}
		if (!SaveFile()) {																	//Initalize the file content
			fsFile.close();
			if (MessageBox(GetMainWindowHandle(),
				L"Failed to initalize the log file content! Please make sure the file is writable! Continue without log file?",
				L"Error",
				MB_ICONERROR | MB_YESNO) == IDNO) {												//Show a prompt if failed to initalize file content

				DestroyWindow(GetMainWindowHandle());												//Quit the system
				PostQuitMessage(0);
			}
			else																				//Continuing without log file
				WithoutFile = true;
		}
		else {																				//Reopen the file with read/write privilege
			fsFile.close();
			fsFile.open(FilePath, ios::binary | ios::in | ios::out);
		}
	}

	//Check file size
	fsFile.seekg(0, ios::end);
	streamoff	szFile = fsFile.tellg();														//Get file size
	fsFile.clear();
	fsFile.seekg(0, ios::beg);
	if (szFile < sizeof(FileContent) - sizeof(wchar_t) * 20) {									//Invalid file size
		if (MessageBox(GetMainWindowHandle(),
			L"Invalid file content! Create a new file to replace current file?",
			L"Error",
			MB_ICONERROR | MB_YESNO) == IDNO) {

			fsFile.close();																			//Don't create a new file
			WithoutFile = true;																		//Continuing without log file
			MessageBox(GetMainWindowHandle(),
				L"Continuing without log file!",
				L"Invalid File Content",
				MB_ICONEXCLAMATION);
			return;
		}
		else {																					//Create a new file
			if (!SaveFile()) {																		//Failed to create a new file
				fsFile.close();
				WithoutFile = true;																		//Continuing without log file
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
	fsFile.close();
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
	if (fsFile.fail() || WithoutFile)															//No file opened
		return false;

	LogInfo	info;																				//Set the content
	lstrcpyW(info.CarNumber, CarNumber);
	info.EnterTime = EnterTime;
	info.LeaveTime = LeaveTime;
	info.CarPos = CarPos;
	info.Fee = Fee;

	FileContent.LogData.push_back(info);														//Add log
	FileContent.ElementCount++;	
	SaveFile();																					//Update the log file
	return true;
}

/*
Description:    Save the file content
Return:			true if succeed, false otherwise
*/
bool IceEncryptedFile::SaveFile() {
	if (fsFile.fail() || WithoutFile)															//No file opened
		return false;
	
	int		szFile = sizeof(wchar_t) * 20 + sizeof(UINT) + sizeof(int) + sizeof(LogInfo) * FileContent.ElementCount;
	unique_ptr<BYTE[]> Buffer(new BYTE[szFile]);												//Allocate binary content buffer

	memcpy(Buffer.get(), FileContent.Password, sizeof(wchar_t) * 20);							//Password
	memcpy(Buffer.get() + sizeof(wchar_t) * 20, &(FileContent.ElementCount), sizeof(UINT));		//Element count
	memcpy(Buffer.get() + sizeof(wchar_t) * 20 + sizeof(UINT),
		&(FileContent.FeePerHour), sizeof(int));												//Fee per hour
	memcpy(Buffer.get() + sizeof(wchar_t) * 20 + sizeof(UINT) + sizeof(int),
		FileContent.LogData.data(), sizeof(LogInfo) * FileContent.ElementCount);				//All log data
	int KeyLen = lstrlenW(FileContent.Password);
	if (KeyLen <= 0)																			//Check password length
		return false;
	for (int i = 0; i < szFile; Buffer.get()[i++] ^= (487 ^ FileContent.Password[i % KeyLen]));	//Encrypt binary data
	fsFile.seekg(0, ios::beg);
	fsFile.write((char*)Buffer.get(), szFile);													//Write to the file
	fsFile.flush();																				//Update the content of the file

	return true;
}

/*
Description:    Read the file and decrypt the content with the password provided
Args:			Password: The password to the file
Return:			true if succeed, false otherwise
*/
bool IceEncryptedFile::ReadFile(wchar_t *Password) {
	if (fsFile.fail() || WithoutFile)															//No file opened
		return false;

	int		KeyLen = lstrlenW(Password);														//Get password length
	if (KeyLen <= 0)																			//Password not provided
		return false;

	//Get file size
	fsFile.seekg(0, ios::end);
	streamoff	szFile = fsFile.tellg();														//Get file size
	if (szFile == -1) {																			//Failed to get file size
		return false;
	}
	fsFile.seekg(0, ios::beg);																	//Change file pointer to the beginning of file
	unique_ptr<BYTE[]> Buffer(new BYTE[(int)szFile]);											//Smart pointer to the buffer

	//Read file contents
	fsFile.read((char*)(Buffer.get()), szFile);													//Read whole file
	for (int i = 0; i < szFile; Buffer.get()[i++] ^= (487 ^ Password[i % KeyLen]));	 			//Decrypt binary data with the provided password
	if (!lstrcmpW((wchar_t*)Buffer.get(), Password)) {											//Check if the decrypted password matches with the provided password
		memcpy(FileContent.Password, Buffer.get(), sizeof(wchar_t) * 20);							//Password
		memcpy(&(FileContent.ElementCount), Buffer.get() + sizeof(wchar_t) * 20, sizeof(UINT));		//Element count
		memcpy(&(FileContent.FeePerHour),
			Buffer.get() + sizeof(wchar_t) * 20 + sizeof(UINT), sizeof(int));						//Fee per hour
		FileContent.LogData.resize(FileContent.ElementCount);										//Allocate LogData elements
		memcpy(FileContent.LogData.data(),
			Buffer.get() + sizeof(wchar_t) * 20 + sizeof(UINT) + sizeof(int),
			sizeof(LogInfo)* FileContent.ElementCount);												//All log data
		return true;
	}
	else {																						//Password unmatch
		return false;
	}
}