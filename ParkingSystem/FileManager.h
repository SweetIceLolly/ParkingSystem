/*
Description:    Handle file creation, modification and
                other operations for the whole system
Author:         Hanson
File:           FileManager.h
*/

#include "MessageHandler.h"

using namespace std;

/* Description:		Log record structure */
struct LogInfo {
	wchar_t			CarNumber[15];					//Car number
	SYSTEMTIME		EnterTime;						//Enter time of the car
	SYSTEMTIME		LeaveTime;						//Leave time of the car. If the car is not left, LeaveTime.wYear = 0
	int				CarPos;							//Parked position
	float			Fee;							//Fee paid
};

/* Description:		Encrypted file structure */
struct RecordFile {
	wchar_t			Password[20];					//User password
	UINT			ElementCount;					//No. of elements of LogData
	float			FeePerHour;						//Fee per hour
	vector<LogInfo>	LogData;						//File content
};

/* Description:		Record file class */
class IceEncryptedFile {
public:
	fstream			fsFile;							//File input/output stream
	RecordFile		FileContent;					//Record file content
	bool			WithoutFile = false;			//If the user selected continue without log file
	bool			CreatedNewFile = false;			//If the program created a new file (If so, the user should modify the default password)

	IceEncryptedFile(const wchar_t *FilePath);
	~IceEncryptedFile();
	bool AddLog(wchar_t *CarNumber, SYSTEMTIME EnterTime, SYSTEMTIME LeaveTime, int CarPos, float Fee);
	bool SaveFile();
	bool ReadFile(wchar_t *Password);
};