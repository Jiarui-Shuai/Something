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

// 窗口信息结构体
struct WindowInfo {
    HWND hWnd;
    DWORD processId;
    std::string processName;
    std::string windowTitle;
    
    // 打印窗口信息
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

// 获取进程名
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

// 枚举窗口回调函数
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);
    if (IsWindowVisible(hwnd) && GetWindowTextLengthA(hwnd) > 0) {
        WindowInfo info;
        info.hWnd = hwnd;
        
        // 获取进程ID
        GetWindowThreadProcessId(hwnd, &info.processId);
        
        // 获取进程名
        info.processName = GetProcessNameFromId(info.processId);
        
        // 获取窗口标题
        char title[256] = {0};
        GetWindowTextA(hwnd, title, sizeof(title));
        info.windowTitle = title;
        
        windows->push_back(info);
    }
    return TRUE;
}

// API: 枚举所有可见窗口
inline std::vector<WindowInfo> EnumVisibleWindows() {
    std::vector<WindowInfo> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

// API: 打印表头
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

// API: 打印窗口列表
inline void PrintWindowList() {
    // 打印表头
    PrintWindowListHead();

    // 打印分割线
    std::cout << std::setfill('-') << std::setw(100) << "" << std::setfill(' ') << std::endl;
    
    // 枚举并打印窗口
    auto windows = EnumVisibleWindows();
    for (const auto& window : windows) {
        window.print();
    }
}

// API: 解析窗口句柄
inline HWND ParseWindowHandle(const std::string& input) {
    std::string cleanInput = input;
    
    // 移除空格
    cleanInput.erase(std::remove_if(cleanInput.begin(), cleanInput.end(), 
        [](unsigned char c) { return std::isspace(c); }), 
        cleanInput.end());
    
    // 检查十六进制格式
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
    
    // 字符串转数值
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

// API: 设置窗口父子关系
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
    
    // 成功条件：返回了之前的父窗口句柄或错误码为成功
    return prev != nullptr || error == ERROR_SUCCESS;
}

// API: 交互式设置窗口父子关系
inline bool InteractiveSetParent() {
    std::cout << "\nWindow Parent Setting Tool\n";
    std::cout << "=========================\n\n";
    std::cout << "Supports decimal and hexadecimal input\n";
    std::cout << "Hexadecimal format: 0x1234 or 1234h\n";
    std::cout << "Decimal format: 123456\n\n";

    // 获取父窗口句柄
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

    // 获取子窗口句柄
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

    // 设置窗口父子关系
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