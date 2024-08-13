#include <windows.h>
#include <stdint.h>
#include <stdio.h>

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

#define Assert(expression) if(!(expression)) {*(int *)0 = 0;}

u8 buffer[MAX_BUFFER_SIZE];

struct File_State {
    s32 cursor_index;
    s32 buffer_index;
    s32 buffer_count;
    s32 row_count;
    s32 row_line_sizes[MAX_LINES];
};

static File_State FileStateInit() {
    File_State result = {};
    result.cursor_index = 0;
    result.buffer_index = 0;
    result.buffer_count = 0;
    result.row_count    = 0;

    return result;
}

File_State file_state;

s32 CursorXPosition(s32 cursor_index) {
    return cursor_index % MAX_ROW_SIZE;
}

s32 CursorYPosition(s32 cursor_index) {
    return cursor_index / MAX_ROW_SIZE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void DrawText(HWND hwnd);
void HandleCharInput(HWND hwnd, char c);
void HandleBackspace(HWND hwnd); 
void HandleArrowKeys(HWND hwnd, int key);

int WINAPI WinMain(HINSTANCE hInstance, HISNTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "TextEditorClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Text Editor", 
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 
                               800, 600, NULL, NULL, hInstance, NULL);

    if (hwnd != NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    file_state = FileStateInit();

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT:
            DrawText(hwnd);
            return 0;

        case WM_CHAR:
            HandleCharInput(hwnd, (char)wParam);
            return 0;

        case WM_KEYDOWN:
            switch (wParam) {
                case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                    HandleArrowKeys(hwnd, wParam);
                    break;
                case VK_BACK:
                    HandleBackspace(hwnd);
                    break;
            }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void DrawText(HWND hwnd) {
    PAINSTRUCT ps;
    HDC hdc = BeginPaint(hwnd &ps);

    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                             OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, 
                             VARIABLE_PITCH, TEXT("Courier New"));
    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    RECT rect;
    GetClientRect(hwnd, &rect);

    // NOTE: Draw the buffer content
    DrawText(hdc, (LPCSTR)buffer, file_state.buffer_count, &rect, DT_LEFT | DT_TOP);

    // NOTE: Draw the cursor
    TEXTMETRIC tm;
    GetTextMetric(hdc, &tm);
    int char_width = tm.tmAveCharWidth;
    int char_height = tm.tmHeight;

    int cursor_x = CursorXPosition(file_state.cursor_index) * char_width;
    int cursor_y = CursorYPosition(file_state.cursor_index) * char_height;

    RECT cursor_rect = {cursor_x, cursor_y, cursor_x + 2, cursor_y + char_height};
    FillRect(hdc, &cursor_rect, (HBRUSH)GetStockObject(BlackBrush));

    DeleteObject(hFont);
    EndPaint(hwnd, &ps);
}

void HandleCharInput(HWND hwnd, char c) {
    if (file_state.buffer_index < file_state.buffer_count) {
        for (s32 i = file_state.buffer_count; i > file_state.buffer_index; i--) {
            buffer[i] = buffer[i-1];
        }
    }

    if (c == '\r') {
        c = '\n';
        s32 chars_for_new_line = file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)] - 
                                 CursorXPosition(file_state.cursor_index);
        s32 next_line_start = (CursorYPosition(file_state.cursor_index) + 1) * MAX_ROW_SIZE;
        if (next_line_start < MAX_ROW_SIZE) {
            file_state.cursor_index = next_line_start;
            file_state.row_count++;
            if (CursorYPosition(file_state.cursor_index) < file_state.row_count) {
                for (s32 i = file_state.row_count; i > CursorYPosition(file_state.cursor_index); i--) {
                    file_state.row_line_sizes[i] = file_state.row_line_sizes[i-1];
                }
            }
            file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)] = 0;
            file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)] += chars_for_new_line;
            file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)-1] -= chars_for_new_line;
        } else {
            file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)] += 1;
            file_state.cursor_index++;
        }
        buffer[file_state.buffer_index++] = c;
        file_state.buffer_count++;
        InvalidateRect(hwnd, NULL, true);
    }
}

void HandleBackspace(HWND hwnd) {
    if (file_state.buffer_index > 0) {
        for (s32 i = file_state.buffer_index - 1; i < file_state.buffer_count; i++) {
            buffer[i] = buffer[i+1];
        }

        if (CursorXPosition(file_state.cursor_index) == 0) {
            s32 previous_row_size = file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)] + 1;
            s32 previous_line = CursorYPosition(file_state.cursor_index) - 1;
            s32 null_space = MAX_ROW_SIZE file_state.row_line_sizes[previous_line];
            file_state.cursor_index -= null_space;
            file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)] += previous_row_size;

            if (CursorYPosition(file_state.cursor_index) < file_state.row_count) {
                for (s32 i = CursorYPosition(file_state.cursor_index) + 1; i < file_state.row_count; i++) {
                    file_state.row_line_sizes[i] = file_state.row_line_sizes[i+1];
                }
                file_state.row_count--;
            }
        } else {
            file_state.cursor_index--;
        }

        file_state.row_line_sizes[CursorYPosition(file_state.cursor_index)] -= 1;
        file_state.buffer_count--;
        file_state.buffer_index--;
        InvalidateRect(hwnd, NULL, true);
    }
}

void HandleArrowKeys(HWND hwnd, int key) {
}

