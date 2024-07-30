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
}

