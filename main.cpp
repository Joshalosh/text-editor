#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_LINES 100
#define MAX_ROW_SIZE 80

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

u8 buffer[MAX_BUFFER_SIZE];
s32 row_line_sizes[MAX_LINES];
s32 buffer_index = 0;
s32 buffer_count = 0;

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

s32 CursorXPosition(s32 cursor_index) {
    s32 result = cursor_index % MAX_ROW_SIZE;
    return result;
}

s32 CursorYPosition(s32 cursor_index) {
    s32 result = cursor_index / MAX_ROW_SIZE;
    return result;
}
void SetCursorPosition(s32 cursor_index) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    s32 x           = CursorXPosition(cursor_index);
    s32 y           = CursorYPosition(cursor_index);
    COORD coord     = {(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, coord);
}

void RefreshScreen(s32 cursor_index) {
    ClearScreen();
    printf("%s", buffer);
    SetCursorPosition(cursor_index);
}

struct File_State {
    s32 cursor_index;
    s32 buffer_index;
    s32 buffer_count;
    s32 row_line_sizes[MAX_LINES];
    s32 row_count;
};



int main() {
    s32 cursor_index = 0;
    RefreshScreen(cursor_index);
    s32 row_count = 0;
    u8 c = 0;
    while (c != 'q') {
        c = getch();
        switch (c) {
            case 8: { // NOTE: Backspace key
                if (buffer_index > 0) {
                    for (s32 i = buffer_index - 1; i < buffer_count; i++) {
                        buffer[i] = buffer[i+1];
                    }

                    row_line_sizes[CursorYPosition(cursor_index)] -= 1;
                    buffer_count--;
                    buffer_index--;
                    cursor_index--;
                    RefreshScreen(cursor_index);
                }
            } break;
            case 0:
            case 224: {
                u8 arrow = _getch();
                switch (arrow) {
                    case 72: { // NOTE: Up arrow
                        if (CursorYPosition(cursor_index) > 0) {
                            s32 old_cursor_x = CursorXPosition(cursor_index);
                            if (CursorXPosition(cursor_index) > row_line_sizes[CursorYPosition(cursor_index)-1]) {
                                cursor_index -= MAX_ROW_SIZE;
                                cursor_index -= CursorXPosition(cursor_index) - row_line_sizes[CursorYPosition(cursor_index)];
                            } else {
                                cursor_index -= MAX_ROW_SIZE;
                            }
                            s32 new_cursor_x_to_line_end = row_line_sizes[CursorYPosition(cursor_index)] - CursorXPosition(cursor_index);
                            buffer_index -= old_cursor_x + new_cursor_x_to_line_end + 1;
                            SetCursorPosition(cursor_index);
                        }
                    } break;
                    case 75: { // NOTE: Left arrow
                        if (buffer_index && cursor_index > 0) {
                            buffer_index--;
                            if (CursorXPosition(cursor_index) == 0) {
                                s32 previous_line = CursorYPosition(cursor_index) - 1;
                                s32 null_space = MAX_ROW_SIZE - row_line_sizes[previous_line];
                                cursor_index -= null_space;
                            } else {
                                cursor_index--;
                            }
                            SetCursorPosition(cursor_index);
                        }
                    } break;
                    case 77: { // NOTE: Right arrow
                        if (buffer_index < buffer_count) {
                            if (CursorXPosition(cursor_index) == row_line_sizes[CursorYPosition(cursor_index)]) {
                                s32 next_line_start = (CursorYPosition(cursor_index) + 1) * MAX_ROW_SIZE;
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
                    case 80: { // NOTE: Down arrow
                        if (buffer_index < buffer_count) {
                            s32 cursor_x = CursorXPosition(cursor_index);
                            s32 projected_cursor_index = cursor_index;
                            if (CursorXPosition(cursor_index) > row_line_sizes[CursorYPosition(cursor_index)+1]) {
                                projected_cursor_index += MAX_ROW_SIZE;
                                projected_cursor_index -= CursorXPosition(projected_cursor_index) - row_line_sizes[CursorYPosition(projected_cursor_index)];
                            } else {
                                projected_cursor_index += MAX_ROW_SIZE;
                            }
                            s32 cursor_x_to_line_end = row_line_sizes[CursorYPosition(cursor_index)] - cursor_x;
                            s32 projected_buffer_index = buffer_index;
                            projected_buffer_index += cursor_x_to_line_end + CursorXPosition(projected_cursor_index) + 1;

                            if (projected_buffer_index <= buffer_count) {
                                cursor_index = projected_cursor_index;
                                buffer_index = projected_buffer_index;
                                SetCursorPosition(cursor_index);
                            }
                        }
                    } break;
                }
            } break;
            default: { // NOTE: Other characters
                if (buffer_index < buffer_count) {
                    for (s32 i = buffer_count; i > buffer_index; i--) {
                        buffer[i] = buffer[i-1];
                    }
                }
                if (c == '\r') { // NOTE: Enter key
                    c = '\n';
                    // TODO: I'm going to need to have a max row count and shift the row sizes in the buffer when new lines are created and deleted I think
                    s32 chars_for_new_line = row_line_sizes[CursorYPosition(cursor_index)] - CursorXPosition(cursor_index);
                    s32 next_line_start = (CursorYPosition(cursor_index) + 1) * MAX_ROW_SIZE;
                    if (next_line_start < MAX_BUFFER_SIZE) {
                        cursor_index = next_line_start;
                        row_count++;
                        if (CursorYPosition(cursor_index) < row_count) {
                            for (s32 i = row_count; i > CursorYPosition(cursor_index); i--) {
                                row_line_sizes[i] = row_line_sizes[i-1];
                            }
                        }
                        row_line_sizes[CursorYPosition(cursor_index)] = 0;
                        row_line_sizes[CursorYPosition(cursor_index)] += chars_for_new_line;
                        row_line_sizes[CursorYPosition(cursor_index)-1] -= chars_for_new_line;
                    } else {
                        cursor_index = buffer_count;
                    }
                } else {
                    row_line_sizes[CursorYPosition(cursor_index)] += 1;
                    cursor_index++;
                }
                buffer[buffer_index++] = c;
                buffer_count++;
                RefreshScreen(cursor_index);
            } break;
        }
    }
    ClearScreen();
}
