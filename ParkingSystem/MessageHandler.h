/*
Description:    Handle messages for windows and controls, and
                invoke control events
Author:         Hanson
File:           MessageHandler.h
*/

#include <Windows.h>
#include <windef.h>
#include <WinUser.h>
#include <WinBase.h>

void RecordProgramInstance(HINSTANCE hInstance);										//This copys hInstance to ProgramInstance
INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    //Main window procedure
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);        //Button control procedure

/* Control event types */
typedef void(*ButtonClickEvent)();              //Button_Click

/* Description:    Button control class */
//class IceButton {
//private:
//    char                *Caption;
//    ButtonClickEvent    EventFunction;
//    HWND                hWnd;
//
//public:
//    IceButton(wchar_t *Caption, int X, int Y, int Width, int Height, HWND ParentHwnd, ButtonClickEvent Event) {
//        EventFunction = Event;
//        hWnd = CreateWindowEx(NULL, L"IceButton", Caption, WS_CHILD | WS_VISIBLE, X, Y,
//            Width, Height, ParentHwnd, (HMENU)0, ProgramInstance, NULL);
//        SetProp(hWnd, L"Event", (HANDLE)EventFunction);
//    }
//
//	~IceButton() {
//
//	}
//
//    /*
//    Description:    Set the caption of the button control
//    Args:           Caption: New caption
//    */
//    void SetCaption(wchar_t *Caption) {
//        SetWindowText(hWnd, Caption);
//    }
//
//    /*
//    Description:    Set the visiblility of the button control
//    Args:           Visible: New visibility status
//    */
//    void SetVisible(bool Visible) {
//        if (Visible)
//            ShowWindow(hWnd, SW_SHOW);
//        else
//            ShowWindow(hWnd, SW_HIDE);
//    }
//
//    /*
//    Description:    Set the enabled status of the button control
//    Args:           Visible: New enabled status
//    */
//    void SetEnable(bool Enabled) {
//        EnableWindow(hWnd, Enabled);
//    }
//}