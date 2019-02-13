/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "MessageHandler.h"
#include "resource.h"

/*
Description:    The entry point of the program
Args:           All parameters are unused
Return:         Result of DialogBox
*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	RecordProgramInstance(hInstance);														//Record program instance
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINWINDOW), NULL, MainWindowProc);
}