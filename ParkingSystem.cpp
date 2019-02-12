/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "MessageHandler.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrebInstance, LPSTR lpCmdLine, int nShowCmd) {
    return DialogBox(hInstance, IDD_MAINWINDOW, NULL, MainWindowProc);
}