#include <windows.h>

void virus(){
    int result = MessageBox(
        NULL,
        "正在删除系统文件...",
        "Virus",
        MB_ABORTRETRYIGNORE | MB_ICONERROR
    );
    switch(result){
        case IDABORT: MessageBox(NULL, "错误：未能终止病毒进程", "System", MB_OK | MB_ICONERROR); break;
        case IDRETRY: MessageBox(NULL, "错误：无效的操作", "System", MB_OK | MB_ICONERROR); break;
    }

    system("dir C:\\Windows\\System32"); // 假装删除系统文件

    MessageBox(NULL, "病毒已删除系统文件", "System", MB_OK | MB_ICONERROR);
    MessageBox(NULL, "你上当了吗？ 其实这是个假病毒 ——程序猿", "程序猿的恶作剧", MB_OK | MB_ICONINFORMATION);
}

void yes_yes(){
    int result = MessageBox(
        NULL,
        "正在扫描电脑...",
        "Windows Defender",
        MB_ABORTRETRYIGNORE | MB_ICONINFORMATION
    );
    MessageBox(NULL, "发现病毒，正在删除...", "Windows Defender", MB_OK | MB_ICONINFORMATION);
    MessageBox(NULL, "病毒删除失败", "Windows Defender", MB_OK | MB_ICONERROR);
    virus();

}

void main_idyes(){
    int result = MessageBox(
        NULL,
        "错误：无法解决",
        "此电脑",
        MB_OK | MB_ICONERROR
    );
    
    result = MessageBox(
        NULL,
        "是否扫描此电脑？",
        "Windows Defender",
        MB_YESNO | MB_ICONINFORMATION
    );

    switch (result) {
        case IDYES: yes_yes(); break;
        case IDNO: virus(); break;
    }

}


void main_idno(){
    virus();
}

int main() {
    unsigned int result = 0;
    result = MessageBox(
        NULL,
        "出现了一个未知问题，是否解决？",
        "此电脑",
        MB_YESNO | MB_ICONWARNING
    );
    switch (result) {
        case IDYES: main_idyes(); break;
        case IDNO: main_idno(); break;

    }
}
