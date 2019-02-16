/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "MessageHandler.h"

/*
Description:    To handle main window creation event
*/
void MainWindow_Create() {
	IceMenu mnuExit(GetMainWindowHandle(), ID_FILE_EXITSYSTEM, NULL);
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
		MainWindowProc, (LPARAM)((WindowCreateEvent)MainWindow_Create));
}
