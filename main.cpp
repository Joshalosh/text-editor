#include <stdio.h>
#include <windows.h>
#include <conio.h>

#if 0 
HANDLE hStdin;
DWORD fdwSaveOldMode;

void DisableRawMode() {
    SetConsoleMode(hStdin, fdwSaveOldMode);
}

void EnableRawMode() {
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwSaveOldMode);

    DWORD fdwMode = ENABLE_VIRTUAL_TERMINAL_INPUT;
    SetConsoleMode(hStdin, fdwMode);
}

int main() {
    EnableRawMode();

    while(1) {
        if (_kbhit()) {
            char c  = _getch();
            if (iscntrl(c)) {
                printf("%c\r\n", c);
            } else {
                printf("%c", c);
            }
            if (c == '0') break;
        }
    }

    DisableRawMode();
}

#else

#define MAX_BUFFER_SIZE 1024

char buffer[MAX_BUFFER_SIZE];

void ClearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0,0};
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten)) return;
    if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) return;
    SetConsoleCursorPosition(hConsole, coordScreen);
}

int main() {

    printf("Let's see if this works: ");
    int buffer_index = 0;
    char c = 0;
    while (c != '0')
    {
        c = getch();
        if (c == '\r') {
            //printf("\n");
            c = '\n';
            buffer[buffer_index] = c;
            buffer_index++;
        } else if (c == 8) {
            //printf("\b \b");
            c = '\0';
            buffer_index--;
            buffer[buffer_index] = c;
        } else {
            buffer[buffer_index] = c;
            buffer_index++;
            //putch(c);
        }

        ClearScreen();

        for (int index = 0; index < buffer_index; index++) {
            printf("%c", buffer[index]);
        }
    }
}
#endif
