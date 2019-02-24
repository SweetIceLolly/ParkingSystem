/*
Description:    Handle file creation, modification and
                other operations for the whole system
Author:         Hanson
File:           FileManager.h
*/

#include "MessageHandler.h"
#include <stdio.h>
#include <vector>

using namespace std;

/* Description:		Record file class */
class IceEncryptedFile {
private:
	/* Description:		Log record structure */
	struct LogInfo {
		wchar_t			CarNumber[10];					//Car number
		SYSTEMTIME		EnterTime;						//Enter time of the car
		SYSTEMTIME		LeaveTime;						//Leave time of the car
		int				CarPos;							//Parked position
		int				Fee;							//Fee paid
	};

	/* Description:		Encrypted file structure */
	struct RecordFile {
		wchar_t			Password[20];					//User password
		UINT			ElementCount;					//No. of elements of LogData
		vector<LogInfo>	LogData;						//File content
	};

public:
	FILE			*lpFile;							//File handle
	RecordFile		FileContent;						//Record file content

	IceEncryptedFile(wchar_t *FilePath);
	~IceEncryptedFile();
	bool AddLog(wchar_t *CarNumber, SYSTEMTIME EnterTime, SYSTEMTIME LeaveTime, int CarPos, int Fee);
	bool SaveFile();
	bool ReadFile(wchar_t *Password);
};