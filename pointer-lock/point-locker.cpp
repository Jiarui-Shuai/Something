#include <windows.h>
#include <iostream>
#pragma comment(lib, "gdi32.lib")

#define UNICODE
#define _UNICODE
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1 : 0)

// 全局变量用于跟踪锁定状态
bool locking = true;

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    switch(Message) {
        /* Upon destruction, tell the main thread to stop */
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        
        /* 处理窗口绘制消息 */
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // 设置背景为白色
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW+1));
            
            // 只在锁定状态时显示文字
            if(locking) {
                // 创建大号字体 - 使用宽字符版本
                HFONT hFont = CreateFontW(  // 使用 CreateFontW 显式指定宽字符版本
                    48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                    DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                    VARIABLE_PITCH, L"Arial");  // 宽字符字符串
                
                HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
                
                // 设置文本颜色为红色
                SetTextColor(hdc, RGB(255, 0, 0));
                SetBkMode(hdc, TRANSPARENT); // 透明背景
                
                // 在窗口中央绘制文本
                const wchar_t* text = L"鼠标已锁定";
                RECT textRect = rect;
                DrawTextW(hdc, text, -1, &textRect, 
                         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                
                // 恢复原始字体并删除新字体
                SelectObject(hdc, hOldFont);
                DeleteObject(hFont);
            }
            
            EndPaint(hwnd, &ps);
            break;
        }
        
        /* All other messages (a lot of them) are processed using default procedures */
        default:
            return DefWindowProc(hwnd, Message, wParam, lParam);
    }
    return 0;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEXW wc; // 使用宽字符版本
    HWND hwnd;
    MSG msg;

    /* zero out the struct and set the stuff we want to modify */
    memset(&wc, 0, sizeof(wc));
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName = L"WindowClass"; // 宽字符类名
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassExW(&wc)) { // 使用宽字符注册函数
        MessageBoxW(NULL, L"窗口注册失败!", L"错误!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"WindowClass", L"鼠标锁定", 
                          WS_VISIBLE | WS_OVERLAPPEDWINDOW,
                          0, 0, 640, 480, 
                          NULL, NULL, hInstance, NULL);

    if(hwnd == NULL) {
        MessageBoxW(NULL, L"窗口创建失败!", L"错误!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    RECT clipRect; // 用于存储鼠标限制区域

    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        RECT windowsize;
        GetWindowRect(hwnd, &windowsize);

        // 鼠标锁定功能
        if(locking) {
            // 设置鼠标限制区域为窗口矩形
            clipRect = windowsize;
            ClipCursor(&clipRect);
        } else {
            // 解除鼠标限制
            ClipCursor(NULL);
        }

        // 检测ESC键按下（添加防抖动处理）
        static bool lastEscState = false;
        bool currentEscState = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        
        if(currentEscState && !lastEscState) {
            locking = !locking;
            // 强制重绘窗口以更新文本显示
            InvalidateRect(hwnd, NULL, TRUE);
            // 更新窗口标题显示当前状态
            SetWindowTextW(hwnd, locking ? 
                L"鼠标锁定 (按ESC切换) - 已锁定" : 
                L"鼠标锁定 (按ESC切换) - 已解锁");
        }
        lastEscState = currentEscState;
        
        // 添加延迟防止过度占用CPU
        Sleep(10);
    }
    
    // 程序退出前确保解除鼠标限制
    ClipCursor(NULL);
    return (int)msg.wParam;
}
