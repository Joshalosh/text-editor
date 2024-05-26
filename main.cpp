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

void SetCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, coord);
}

void RefreshScreen() {
    ClearScreen();
    printf("%s", buffer);
    int cursor_x = cursor_index % 80;
    int cursor_y = cursor_index / 80;
    SetCursorPosition(cursor_x, cursor_y);
}

int main() {

    printf("Let's see if this works: ");
    int buffer_index = 0;
    char c = 0;
    while (c != '0')
    {
        c = getch();
        if (c == '\r') {
            c = '\n';
            buffer[buffer_index] = c;
            buffer_index++;
            printf("\n");
        } else if (c == 8) {
            if (buffer_index > 0)
            {
                c = '\0';
                buffer_index--;
                buffer[buffer_index] = c;
                printf("\b \b");
            }
        } else {
            buffer[buffer_index] = c;
            buffer_index++;
            putch(c);
            printf("%d", c);
        }

#if 0
        ClearScreen();

        for (int index = 0; index < buffer_index; index++) {
            printf("%c", buffer[index]);
        }
#endif
    }
}
#endif
