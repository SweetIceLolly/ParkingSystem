/*
Description:    Create the main window and handle
                events for it
Author:         Hanson
File:           ParkingSystem.cpp
*/

#include "FileManager.h"

/*
Description:    To handle main window creation event
*/
void MainWindow_Create() {
	//Set the icon of the window
	HICON hIcon = (HICON)LoadImage(GetProgramInstance(),
		MAKEINTRESOURCE(IDI_MAINICON),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON), 0);
	if (hIcon)
		SendMessage(GetMainWindowHandle(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	//Load data file
	IceEncryptedFile encFile(L"Log.dat");
	encFile.ReadFile(L"I Like Emily");
}

/*
Description:	To handle Exit menu event
*/
void mnuExit_Click() {
	//if (MessageBox(GetMainWindowHandle(), L"Exit Parking System?", L"Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES) {
		DestroyWindow(GetMainWindowHandle());								//Close the window and exit the program
		PostQuitMessage(0);
	//}
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
