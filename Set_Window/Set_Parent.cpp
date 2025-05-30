#include <Windows.h>
#include <iostream>
#include <string>
#include <cctype>
#include <sstream>
#include <algorithm>
HWND ParseHandle(const std::string& input) {
    std::string cleanInput = input;
    
    cleanInput.erase(std::remove_if(cleanInput.begin(), cleanInput.end(), 
        [](unsigned char c) { return std::isspace(c); }), 
        cleanInput.end());
    
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
    
    std::stringstream ss;
    UINT_PTR value = 0;
    
    if (isHex) {
        ss << std::hex << cleanInput;
    } else {
        ss << std::dec << cleanInput;
    }
    
    if (!(ss >> value)) {
        return 0;
    }
    
    return reinterpret_cast<HWND>(static_cast<UINT_PTR>(value));
}

int main() {
    std::cout << "Window Parent Setting Tool\n";
    std::cout << "=========================\n\n";
    std::cout << "Supports decimal and hexadecimal input\n";
    std::cout << "Hexadecimal format: 0x1234 or 1234h\n";
    std::cout << "Decimal format: 123456\n\n";

    std::string fatherInput;
    std::cout << "Enter parent window handle: ";
    std::getline(std::cin, fatherInput);
    HWND hwndFather = ParseHandle(fatherInput);
    
    if (hwndFather == 0) {
        std::cout << "\nFailed to parse parent window handle!\n";
        return 1;
    }
    
    if (!IsWindow(hwndFather)) {
        std::cout << "\nInvalid parent window handle! (HWND: " 
                  << reinterpret_cast<UINT_PTR>(hwndFather) << ")\n";
        return 1;
    }

    std::string childInput;
    std::cout << "Enter child window handle: ";
    std::getline(std::cin, childInput);
    HWND hwndChild = ParseHandle(childInput);
    
    if (hwndChild == 0) {
        std::cout << "\nFailed to parse child window handle!\n";
        return 1;
    }
    
    if (!IsWindow(hwndChild)) {
        std::cout << "\nInvalid child window handle! (HWND: " 
                  << reinterpret_cast<UINT_PTR>(hwndChild) << ")\n";
        return 1;
    }

    HWND prevParent = SetParent(hwndChild, hwndFather);
    
    if (prevParent) {
        std::cout << "\nParent set successfully!\n";
        std::cout << "Previous parent: " << reinterpret_cast<UINT_PTR>(prevParent) << " (decimal)\n";
        std::cout << "Previous parent: 0x" << std::hex << reinterpret_cast<UINT_PTR>(prevParent) 
                  << " (hexadecimal)\n";
    } else {
        DWORD error = GetLastError();
        std::cout << "\nFailed to set parent! Error code: " << error << "\n";
        if (error == ERROR_INVALID_WINDOW_HANDLE) {
            std::cout << "Reason: Invalid window handle\n";
        } else if (error == ERROR_ACCESS_DENIED) {
            std::cout << "Reason: Access denied (may need admin rights)\n";
        } else if (error == 1400) { // ERROR_INVALID_WINDOW_HANDLE
            std::cout << "Reason: The window handle is invalid\n";
        }
        return 1;
    }
    
    return 0;
}
