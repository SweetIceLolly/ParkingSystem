/*
Description:    Handle file creation, modification and
                other operations for the whole system
Author:         Hanson
File:           FileManager.h
*/

#include "MessageHandler.h"

/* Description:		Log record structure */
struct LogInfo {
	char		CarNumber[10];					//Car number
	SYSTEMTIME	EnterTime;						//Enter time of the car
	SYSTEMTIME	LeaveTime;						//Leave time of the car
	int			CarPos;							//Parked position
	int			Fee;							//Fee paid
};

/* Description:		Record file class */
//class 