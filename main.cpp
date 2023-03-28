#include <iostream>
#include <Windows.h>
#include "main.h"

int main() {
    HWND cwindow = GetConsoleWindow();
    HANDLE cconsole = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE, 0, NULL,
        CONSOLE_TEXTMODE_BUFFER, NULL
    );

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleActiveScreenBuffer(cconsole);
    GetConsoleScreenBufferInfo(cconsole, &csbi);
    SetConsoleTextAttribute(cconsole, FOREGROUND_GREEN);
    SetConsoleTitle(L"Amesu Software");
    
    int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    size_t char_string_size = (size_t)columns * rows;
    DWORD dw_chars;
    COORD pos;
    pos.X = 0;
    pos.Y = 0;

    field f(columns, rows);

    snake_spawner ss(&f);


    size_t logo_height = logo.size();
    size_t logo_width = logo[0].size();

    for (int r = logo_height - 1; r >= 0; r -= 1) {
        for (int c = 0; c < logo_width; c += 1) {
            int padding_x = (columns - logo_width) / 2;
            int padding_y = (rows - logo_height) / 2;

            // Gets pixel with padding from walls
            pixel* p = f.get_pixel(c + padding_x, r + padding_y);

            p->symbol = logo[logo_height - 1 - r][c];
            if(p->symbol != ' ') p->is_const = true;
        }
    }

    while (true) {
        const TCHAR* msg_buf = f.get_buffer();
        WriteConsoleOutputCharacter(cconsole, msg_buf, (DWORD)char_string_size, pos, &dw_chars);
        delete[] msg_buf;

        ss.step();
        Sleep((DWORD)200);
    }
}
