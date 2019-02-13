/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "MessageHandler.h"

/*
Description:    The entry point of the program
Args:           All parameters are unused
Return:         Result of DialogBox
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrebInstance, LPSTR lpCmdLine, int nShowCmd) {
    ProgramInstance = hInstance;                                            //Record program instance
    return DialogBox(hInstance, IDD_MAINWINDOW, NULL, MainWindowProc);
}