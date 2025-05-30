#include <Windows.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <tlhelp32.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

std::string GetProcessNameFromId(DWORD processId) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return "Snapshot Failed";
    }
    
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return "Enumeration Failed";
    }
    
    do {
        if (pe32.th32ProcessID == processId) {
            CloseHandle(hSnapshot);
            return pe32.szExeFile;
        }
    } while (Process32Next(hSnapshot, &pe32));
    
    CloseHandle(hSnapshot);
    return "Unknown";
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    std::vector<HWND>* handles = reinterpret_cast<std::vector<HWND>*>(lParam);
    if (IsWindowVisible(hwnd) && GetWindowTextLengthA(hwnd) > 0) {
        handles->push_back(hwnd);
    }
    return TRUE;
}

int main() {
    SetConsoleOutputCP(CP_ACP);

    std::cout << std::left 
              << std::setw(16) << "Window Handle"
              << std::setw(16) << "Process ID"
              << std::setw(32) << "Process Name"
              << "Window Title"
              << std::endl;
    std::cout << std::setfill('-') << std::setw(100) << "" << std::setfill(' ') << std::endl;

    std::vector<HWND> windowHandles;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windowHandles));

    for (size_t i = 0; i < windowHandles.size(); i++) {
        HWND hwnd = windowHandles[i];
        
        char title[256] = {0};
        GetWindowTextA(hwnd, title, 256);
        
        DWORD processId = 0;
        GetWindowThreadProcessId(hwnd, &processId);

        std::string processName = "Unknown";
        if (processId != 0) {
            processName = GetProcessNameFromId(processId);
        }

        std::cout << std::left 
                  << std::setw(16) << reinterpret_cast<UINT_PTR>(hwnd)
                  << std::setw(16) << processId
                  << std::setw(32) << processName
                  << title
                  << std::endl;
    }

    return 0;
}
