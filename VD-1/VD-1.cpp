// VD-1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <shobjidl_core.h>
#include <shobjidl.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "VirtualDesktop.h"
//#include <ShObjIdl.h>

inline std::wstring GuidToString(const GUID& guid) noexcept
{
    OLECHAR* guidString;
    StringFromCLSID(guid, &guidString);

    std::wstring guidWString(guidString);
    ::CoTaskMemFree(guidString);

    return guidWString;
}
GUID StringToGuid(const std::wstring& str)
{
    GUID guid;
    if (CLSIDFromString(str.c_str(), &guid) == S_OK)
    {
        return guid;
    }
    std::cerr << "guid not found." << std::endl;
    return guid;
}

// Function to simulate key press
void keyPress(BYTE vkCode) {
    keybd_event(vkCode, 0, 0, 0);
}

// Function to simulate key release
void keyRelease(BYTE vkCode) {
    keybd_event(vkCode, 0, KEYEVENTF_KEYUP, 0);
}
void switchToNextDesktop() {
    keyPress(VK_LWIN);
    keyPress(VK_CONTROL);
    keyPress(VK_RIGHT);
    keyRelease(VK_RIGHT);
    keyRelease(VK_CONTROL);
    keyRelease(VK_LWIN);
}
int main() {
    CoInitialize(NULL);

    IVirtualDesktopManager* pDesktopManager = NULL;
    HRESULT hr = CoCreateInstance(CLSID_VirtualDesktopManager, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pDesktopManager));
    if (SUCCEEDED(hr)) {
        std::wcout << L"IVirtualDesktopManager instance created successfully." << std::endl;
        HWND hwnd = GetForegroundWindow();
        //HWND hwnd = FindWindow(nullptr, L"Calculator");
        if (!hwnd) {
            std::cerr << "Window not found." << std::endl;
            return -1;
        }
        GUID desktopId;
        hr = pDesktopManager->GetWindowDesktopId(hwnd, &desktopId);
        if (SUCCEEDED(hr)) {
            wchar_t guidString = { 0 };
            //StringFromGUID2(desktopId, &guidString, 40);
            std::wcout << L"Current virtual desktop ID: " << GuidToString(desktopId) << std::endl;
        }
        else {
            std::wcout << L"Failed to get the virtual desktop ID." << std::endl;
        }
        switchToNextDesktop();
        Sleep(2000);
        VirtualDesktop& vd = VirtualDesktop::instance();
        GUID currentDesktopId = vd.GetCurrentVirtualDesktopIdFromRegistry();
        std::wcout << L"Current virtual desktop ID: " << GuidToString(currentDesktopId) << std::endl;
        //std::wstring desktopIdStr = L"BE07745B-8826-4737-BAFD-84D1AF254BA6"; 
        //HWND hwnd0 = GetForegroundWindow();
        hr = pDesktopManager->MoveWindowToDesktop(hwnd, currentDesktopId);
        if (SUCCEEDED(hr)) {
            std::wcout << L"Window moved to the specified virtual desktop successfully." << std::endl;
            SetForegroundWindow(hwnd);
            ShowWindow(hwnd, SW_RESTORE);

        }
        else {
            std::wcout << L"Failed to move the window to the specified virtual desktop. HRESULT: " << hr << std::endl;
        }
        pDesktopManager->Release();
    }
    else {
        std::wcout << L"Failed to create IVirtualDesktopManager instance." << std::endl;
    }
    //switchToNextDesktop();
    Sleep(60000);
    CoUninitialize();



 
    
    //std::wstring desktopIdStr = L"{BE07745B-8826-4737-BAFD-84D1AF254BA6}"; 
    //MoveWindowToDesktop(hwnd, StringToGuid(desktopIdStr));
    
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
