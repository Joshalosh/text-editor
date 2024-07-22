#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_LINES 100
#define MAX_ROW_SIZE 80

#define Assert(expression) if(!(expression)) {*(int *)0 = 0;}

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

u8 buffer[MAX_BUFFER_SIZE];
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

struct Memory_Arena {
    u8 *start;
    u8 *current;
    u8 *end;
};

static Memory_Arena ArenaInit(size_t size) {
    Memory_Arena result;
    result.start   = (u8 *)malloc(size);
    result.current = result.start;
    result.end     = result.current + size;
    return result;
}

static void *ArenaAlloc(Memory_Arena *arena, size_t size) {
    void *result = NULL;
    Assert(arena->current + size <= arena->end);
    result          = arena->current;
    arena->current += size;
    return result;
}

#define ZeroStruct(instance) ZeroSize(&instance, sizeof(instance))
static void ZeroSize(void *ptr, size_t size) {
    u8 *byte = (u8 *)ptr;

    while (size--) {
        *byte++ = 0;
    }
}

static void ArenaFree(Memory_Arena *arena) {
    free(arena->start);
}

struct File_State {
    s32 cursor_index;
    s32 buffer_index;
    s32 buffer_count;
    s32 row_count;
    s32 row_line_sizes[MAX_LINES];
};

static File_State FileStateInit() {
    File_State result;
    result.cursor_index = 0;
    result.buffer_index = 0;
    result.buffer_count = 0;
    result.row_count    = 0;
    return result;
}


int main() {
    Memory_Arena arena = ArenaInit(1024ULL*1024);
    File_State *file_state = (File_State *)ArenaAlloc(&arena, sizeof(File_State));
    ZeroStruct(*file_state);
    RefreshScreen(file_state->cursor_index);
    u8 c = 0;
    while (c != 'q') {
        c = getch();
        switch (c) {
            case 8: { // NOTE: Backspace key
                if (file_state->buffer_index > 0) {
                    for (s32 i = file_state->buffer_index - 1; i < file_state->buffer_count; i++) {
                        buffer[i] = buffer[i+1];
                    }

                    if (CursorXPosition(file_state->cursor_index) == 0) {
                        s32 previous_row_size = file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)] + 1;
                        s32 previous_line = CursorYPosition(file_state->cursor_index) - 1;
                        s32 null_space = MAX_ROW_SIZE - file_state->row_line_sizes[previous_line];
                        file_state->cursor_index -= null_space;
                        file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)] += previous_row_size;

                        if (CursorYPosition(file_state->cursor_index) < file_state->row_count) {
                            file_state->row_count--;
                            for (s32 i = CursorYPosition(file_state->cursor_index) + 1; i < file_state->row_count; i++) {
                                file_state->row_line_sizes[i] = file_state->row_line_sizes[i+1];
                            }
                        }
                    } else {
                        file_state->cursor_index--;
                    }

                    file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)] -= 1;
                    file_state->buffer_count--;
                    file_state->buffer_index--;
                    RefreshScreen(file_state->cursor_index);
                }
            } break;
            case 0:
            case 224: {
                u8 arrow = _getch();
                switch (arrow) {
                    case 72: { // NOTE: Up arrow
                        if (CursorYPosition(file_state->cursor_index) > 0) {
                            s32 old_cursor_x = CursorXPosition(file_state->cursor_index);
                            if (CursorXPosition(file_state->cursor_index) > file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)-1]) {
                                file_state->cursor_index -= MAX_ROW_SIZE;
                                file_state->cursor_index -= CursorXPosition(file_state->cursor_index) - file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)];
                            } else {
                                file_state->cursor_index -= MAX_ROW_SIZE;
                            }
                            s32 new_cursor_x_to_line_end = file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)] - CursorXPosition(file_state->cursor_index);
                            file_state->buffer_index -= old_cursor_x + new_cursor_x_to_line_end + 1;
                            SetCursorPosition(file_state->cursor_index);
                        }
                    } break;
                    case 75: { // NOTE: Left arrow
                        if (file_state->buffer_index && file_state->cursor_index > 0) {
                            file_state->buffer_index--;
                            if (CursorXPosition(file_state->cursor_index) == 0) {
                                s32 previous_line = CursorYPosition(file_state->cursor_index) - 1;
                                s32 null_space = MAX_ROW_SIZE - file_state->row_line_sizes[previous_line];
                                file_state->cursor_index -= null_space;
                            } else {
                                file_state->cursor_index--;
                            }
                            SetCursorPosition(file_state->cursor_index);
                        }
                    } break;
                    case 77: { // NOTE: Right arrow
                        if (file_state->buffer_index < file_state->buffer_count) {
                            if (CursorXPosition(file_state->cursor_index) == file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)]) {
                                s32 next_line_start = (CursorYPosition(file_state->cursor_index) + 1) * MAX_ROW_SIZE;
                                if (next_line_start < MAX_BUFFER_SIZE) {
                                    file_state->cursor_index = next_line_start;
                                }
                            } else {
                                file_state->cursor_index++;
                            }
                            file_state->buffer_index++;
                            SetCursorPosition(file_state->cursor_index);
                        }
                    } break;
                    case 80: { // NOTE: Down arrow
                        if (file_state->buffer_index < file_state->buffer_count) {
                            s32 cursor_x = CursorXPosition(file_state->cursor_index);
                            s32 projected_cursor_index = file_state->cursor_index;
                            if (CursorXPosition(file_state->cursor_index) > file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)+1]) {
                                projected_cursor_index += MAX_ROW_SIZE;
                                projected_cursor_index -= CursorXPosition(projected_cursor_index) - file_state->row_line_sizes[CursorYPosition(projected_cursor_index)];
                            } else {
                                projected_cursor_index += MAX_ROW_SIZE;
                            }
                            s32 cursor_x_to_line_end = file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)] - cursor_x;
                            s32 projected_buffer_index = file_state->buffer_index;
                            projected_buffer_index += cursor_x_to_line_end + CursorXPosition(projected_cursor_index) + 1;

                            if (projected_buffer_index <= file_state->buffer_count) {
                                file_state->cursor_index = projected_cursor_index;
                                file_state->buffer_index = projected_buffer_index;
                                SetCursorPosition(file_state->cursor_index);
                            }
                        }
                    } break;
                }
            } break;
            default: { // NOTE: Other characters
                if (file_state->buffer_index < file_state->buffer_count) {
                    for (s32 i = file_state->buffer_count; i > file_state->buffer_index; i--) {
                        buffer[i] = buffer[i-1];
                    }
                }
                if (c == '\r') { // NOTE: Enter key
                    c = '\n';
                    // TODO: I'm going to need to have a max row count and shift the row sizes in the buffer when new lines are created and deleted I think
                    s32 chars_for_new_line = file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)] - CursorXPosition(file_state->cursor_index);
                    s32 next_line_start = (CursorYPosition(file_state->cursor_index) + 1) * MAX_ROW_SIZE;
                    if (next_line_start < MAX_BUFFER_SIZE) {
                        file_state->cursor_index = next_line_start;
                        file_state->row_count++;
                        if (CursorYPosition(file_state->cursor_index) < file_state->row_count) {
                            for (s32 i = file_state->row_count; i > CursorYPosition(file_state->cursor_index); i--) {
                                file_state->row_line_sizes[i] = file_state->row_line_sizes[i-1];
                            }
                        }
                        file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)]    = 0;
                        file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)]   += chars_for_new_line;
                        file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)-1] -= chars_for_new_line;
                    } else {
                        file_state->cursor_index = file_state->buffer_count;
                    }
                } else {
                    file_state->row_line_sizes[CursorYPosition(file_state->cursor_index)] += 1;
                    file_state->cursor_index++;
                }
                buffer[file_state->buffer_index++] = c;
                file_state->buffer_count++;
                RefreshScreen(file_state->cursor_index);
            } break;
        }
    }
    ClearScreen();
}
