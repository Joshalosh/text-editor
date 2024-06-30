#include <stdio.h>
#include <windows.h>
#include <conio.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_LINES 100
#define MAX_ROW_SIZE 80

unsigned char buffer[MAX_BUFFER_SIZE];
int row_line_sizes[MAX_LINES];
int buffer_index = 0;
int buffer_count = 0;

void ClearScreen() {
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

int GetXPosition(int cursor_index) {
    int result = cursor_index % MAX_ROW_SIZE;
    return result;
}

int GetYPosition(int cursor_index) {
    int result = cursor_index / MAX_ROW_SIZE;
    return result;
}
void SetCursorPosition(int cursor_index) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int x           = GetXPosition(cursor_index);
    int y           = GetYPosition(cursor_index);
    COORD coord     = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, coord);
}

void RefreshScreen(int cursor_index) {
    ClearScreen();
    printf("%s", buffer);
    SetCursorPosition(cursor_index);
}



int main() {
    int cursor_index = 0;
    RefreshScreen(cursor_index);
    int row_num = 0;
    unsigned char c = 0;
    while (c != 'q') {
        c = getch();
        switch (c) {
            case '\r': { // NOTE: Enter key
                buffer[buffer_index++] = '\n';
                buffer_count++;
                int next_line_start = (GetYPosition(cursor_index) + 1) * MAX_ROW_SIZE;
                if (next_line_start < MAX_BUFFER_SIZE) {
                    cursor_index = next_line_start;
                } else {
                    cursor_index = buffer_count;
                }
                RefreshScreen(cursor_index);
            } break;
            case 8: { // NOTE: Backspace key
                if (buffer_index > 0) {
                    for (int i = buffer_index - 1; i < buffer_count; i++) {
                        buffer[i] = buffer[i+1];
                    }

                    int cursor_y_position = GetYPosition(cursor_index);
                    row_line_sizes[cursor_y_position] -= 1;
                    buffer_count--;
                    buffer_index--;
                    cursor_index--;
                    RefreshScreen(cursor_index);
                }
            } break;
            case 0:
            case 224: {
                unsigned char arrow = _getch();
                switch (arrow) {
                    case 75: { // NOTE: Left arrow
                        if (buffer_index && cursor_index > 0) {
                            buffer_index--;
                            if (GetXPosition(cursor_index) == 0) {
                                int previous_row = GetYPosition(cursor_index) - 1;
                                int null_space = MAX_ROW_SIZE - row_line_sizes[previous_row];
                                cursor_index -= null_space;
                            } else {
                                cursor_index--;
                            }
                            SetCursorPosition(cursor_index);
                        }
                    } break;
                    case 77: { // NOTE: Right arrow
                        if (buffer_index < buffer_count) {
                            int cursor_x_position = GetXPosition(cursor_index);
                            int cursor_y_position = GetYPosition(cursor_index);
                            if (cursor_x_position == row_line_sizes[cursor_y_position]) {
                                int next_line_start = (cursor_y_position + 1) * MAX_ROW_SIZE;
                                if (next_line_start < MAX_BUFFER_SIZE) {
                                    cursor_index = next_line_start;
                                }
                            } else {
                                cursor_index++;
                            }
                            buffer_index++;
                            SetCursorPosition(cursor_index);
                        }
                    } break;
                }
            } break;
            default: { // NOTE: Other characters
                if (buffer_index < buffer_count) {
                    for (int i = buffer_count; i > buffer_index; i--) {
                        buffer[i] = buffer[i-1];
                    }
                }
                int cursor_y_position = GetYPosition(cursor_index);
                row_line_sizes[cursor_y_position] += 1;
                buffer[buffer_index++] = c;
                cursor_index++;
                buffer_count++;
                RefreshScreen(cursor_index);
            } break;
        }
    }
    ClearScreen();
}
