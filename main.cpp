#include <stdio.h>
#include <windows.h>
#include <conio.h>

#define MAX_BUFFER_SIZE 1024

unsigned char buffer[MAX_BUFFER_SIZE];
int buffer_index = 0;

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
    int cursor_x = buffer_index % 80;
    int cursor_y = buffer_index / 80;
    SetCursorPosition(cursor_x, cursor_y);
}

int main() {
    RefreshScreen();
    unsigned char appended_buffer[MAX_BUFFER_SIZE];
    int appended_buffer_count = 0;
    unsigned char c = 0;
    while (c != 'q')
    {
        c = getch();
        switch (c) {
            case '\r': {
                c = '\n';
                buffer[buffer_index] = c;
                buffer_index++;
                printf("\n");
            } break;
            case 8: {
                if (buffer_index > 0)
                {
                    c = '\0';
                    buffer_index--;
                    buffer[buffer_index] = c;
                    printf("\b \b");
                }
            } break;
            case 0:
            case 224: {
                unsigned char arrow = _getch();
                switch (arrow) {
                    case 75: { // NOTE: Left arrow
                        if (buffer_index > 0) {
                            buffer_index--;
                            appended_buffer[appended_buffer_count] = buffer[buffer_index];
                            appended_buffer_count++;
                            SetCursorPosition(buffer_index % 80, buffer_index / 80);
                        }
                    } break;
                    case 77: { // NOTE: Right arrow
                        if ((buffer_index < MAX_BUFFER_SIZE - 1) && (buffer[buffer_index] != '\0')) {
                            buffer_index++;
                            appended_buffer_count--;
                            SetCursorPosition(buffer_index % 80, buffer_index / 80);
                        }
                    } break;
                    default:
                    {
                        printf("Unexpected arrow key value: %d (%'c')\n", arrow, arrow);
                    } break;
                }
            } break;
            default: {
                buffer[buffer_index] = c;
                buffer_index++;
                putch(c);
            } break;
        }

        if (appended_buffer_count > 0)
        {
            appended_buffer_count--;
            for (int index = 0; index <= appended_buffer_count; index++)
            {
                buffer_index++;
                int appended_reverse_index = appended_buffer_count - index;
                buffer[buffer_index] = appended_buffer[appended_reverse_index];
                printf("%c", buffer[buffer_index]);
            }

            appended_buffer_count++;
            buffer_index -= appended_buffer_count;
            SetCursorPosition(buffer_index % 80, buffer_index / 80);
        }
    }
}
