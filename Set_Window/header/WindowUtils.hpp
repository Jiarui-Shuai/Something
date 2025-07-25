// WindowUtils.hpp
#ifndef WINDOW_UTILS_HPP
#define WINDOW_UTILS_HPP

#include <Windows.h>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <tlhelp32.h>
#include <iostream>


#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

namespace WindowUtils {

// ������Ϣ�ṹ��
struct WindowInfo {
    HWND hWnd;
    DWORD processId;
    std::string processName;
    std::string windowTitle;
    
    // ��ӡ������Ϣ
    void print() const {
        std::cout << std::left
                  << std::dec
                  << std::setw(24) << reinterpret_cast<DWORD64>(hWnd)
                  << std::hex
                  << std::setw(24) << hWnd
                  << std::setw(16) << processId
                  << std::setw(32) << processName
                  << windowTitle
                  << std::endl;
    }
};

// ��ȡ������
static std::string GetProcessNameFromId(DWORD processId) {
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

// ö�ٴ��ڻص�����
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);
    if (IsWindowVisible(hwnd) && GetWindowTextLengthA(hwnd) > 0) {
        WindowInfo info;
        info.hWnd = hwnd;
        
        // ��ȡ����ID
        GetWindowThreadProcessId(hwnd, &info.processId);
        
        // ��ȡ������
        info.processName = GetProcessNameFromId(info.processId);
        
        // ��ȡ���ڱ���
        char title[256] = {0};
        GetWindowTextA(hwnd, title, sizeof(title));
        info.windowTitle = title;
        
        windows->push_back(info);
    }
    return TRUE;
}

// API: ö�����пɼ�����
inline std::vector<WindowInfo> EnumVisibleWindows() {
    std::vector<WindowInfo> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

// API: ��ӡ��ͷ
inline void PrintWindowListHead(){
    std::cout << std::left 
              << std::setw(24) << "Window Handle(DEC)"
              << std::setw(24) << "Window Handle(HEX)"
              << std::setw(16) << "Process ID"
              << std::setw(32) << "Process Name"
              << "Window Title"
              << std::endl;
    std::cout << std::setfill('-') << std::setw(100) << "" << std::setfill(' ') << std::endl;
}

// API: ��ӡ�����б�
inline void PrintWindowList() {
    // ��ӡ��ͷ
    PrintWindowListHead();

    // ��ӡ�ָ���
    std::cout << std::setfill('-') << std::setw(100) << "" << std::setfill(' ') << std::endl;
    
    // ö�ٲ���ӡ����
    auto windows = EnumVisibleWindows();
    for (const auto& window : windows) {
        window.print();
    }
}

// API: �������ھ��
inline HWND ParseWindowHandle(const std::string& input) {
    std::string cleanInput = input;
    
    // �Ƴ��ո�
    cleanInput.erase(std::remove_if(cleanInput.begin(), cleanInput.end(), 
        [](unsigned char c) { return std::isspace(c); }), 
        cleanInput.end());
    
    // ���ʮ�����Ƹ�ʽ
    bool isHex = false;
    if (cleanInput.size() > 2 && cleanInput.substr(0, 2) == "0x") {
        isHex = true;
        cleanInput = cleanInput.substr(2);
    } 
    else if (cleanInput.find_first_of("abcdefABCDEF") != std::string::npos) {
        isHex = true;
    }
    else if (!cleanInput.empty() && cleanInput.back() == 'h') {
        isHex = true;
        cleanInput.pop_back();
    }
    
    // �ַ���ת��ֵ
    std::stringstream ss;
    UINT_PTR value = 0;
    
    if (isHex) {
        ss << std::hex << cleanInput;
    } else {
        ss << std::dec << cleanInput;
    }
    
    if (!(ss >> value)) return nullptr;
    return reinterpret_cast<HWND>(static_cast<UINT_PTR>(value));
}

// API: ���ô��ڸ��ӹ�ϵ
inline bool SetWindowParent(HWND hwndChild, HWND hwndParent, HWND* prevParent = nullptr, DWORD* errorCode = nullptr) {
    if (!IsWindow(hwndChild)) {
        if (errorCode) *errorCode = ERROR_INVALID_WINDOW_HANDLE;
        return false;
    }
    
    if (hwndParent != nullptr && !IsWindow(hwndParent)) {
        if (errorCode) *errorCode = ERROR_INVALID_WINDOW_HANDLE;
        return false;
    }

    HWND prev = ::SetParent(hwndChild, hwndParent);
    DWORD error = GetLastError();
    
    if (prevParent) *prevParent = prev;
    if (errorCode) *errorCode = error;
    
    // �ɹ�������������֮ǰ�ĸ����ھ���������Ϊ�ɹ�
    return prev != nullptr || error == ERROR_SUCCESS;
}

// API: ����ʽ���ô��ڸ��ӹ�ϵ
inline bool InteractiveSetParent() {
    std::cout << "\nWindow Parent Setting Tool\n";
    std::cout << "=========================\n\n";
    std::cout << "Supports decimal and hexadecimal input\n";
    std::cout << "Hexadecimal format: 0x1234 or 1234h\n";
    std::cout << "Decimal format: 123456\n\n";

    // ��ȡ�����ھ��
    std::string fatherInput;
    std::cout << "Enter parent window handle: ";
    std::getline(std::cin, fatherInput);
    HWND hwndFather = ParseWindowHandle(fatherInput);
    
    if (hwndFather == nullptr) {
        std::cout << "\nFailed to parse parent window handle!\n";
        return false;
    }
    
    if (!IsWindow(hwndFather)) {
        std::cout << "\nInvalid parent window handle! (HWND: " 
                  << reinterpret_cast<UINT_PTR>(hwndFather) << ")\n";
        return false;
    }

    // ��ȡ�Ӵ��ھ��
    std::string childInput;
    std::cout << "Enter child window handle: ";
    std::getline(std::cin, childInput);
    HWND hwndChild = ParseWindowHandle(childInput);
    
    if (hwndChild == nullptr) {
        std::cout << "\nFailed to parse child window handle!\n";
        return false;
    }
    
    if (!IsWindow(hwndChild)) {
        std::cout << "\nInvalid child window handle! (HWND: " 
                  << reinterpret_cast<UINT_PTR>(hwndChild) << ")\n";
        return false;
    }

    // ���ô��ڸ��ӹ�ϵ
    HWND prevParent = nullptr;
    DWORD errorCode = 0;
    bool success = SetWindowParent(hwndChild, hwndFather, &prevParent, &errorCode);
    
    if (success) {
        std::cout << "\nParent set successfully!\n";
        std::cout << "Previous parent: " << std::dec 
                  << reinterpret_cast<UINT_PTR>(prevParent) 
                  << " (decimal)\n";
        std::cout << "Previous parent: 0x" << std::hex 
                  << reinterpret_cast<UINT_PTR>(prevParent) 
                  << " (hexadecimal)\n";
        return true;
    } else {
        std::cout << "\nFailed to set parent! Error code: " << errorCode << "\n";
        if (errorCode == ERROR_INVALID_WINDOW_HANDLE) {
            std::cout << "Reason: Invalid window handle\n";
        } else if (errorCode == ERROR_ACCESS_DENIED) {
            std::cout << "Reason: Access denied (may need admin rights)\n";
        } else if (errorCode == ERROR_INVALID_PARAMETER) {
            std::cout << "Reason: Invalid parameter\n";
        }
        return false;
    }
}

} // namespace WindowUtils

#endif // WINDOW_UTILS_HPP