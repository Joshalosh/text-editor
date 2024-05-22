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
int main() {

    printf("Let's see if this works: ");
    char c = NULL;
    while (c != '0')
    {
        c = getch();
        if (c == '\r') {
            printf("\n");
        } else {
            putch(c);
        }
    }
}
#endif
