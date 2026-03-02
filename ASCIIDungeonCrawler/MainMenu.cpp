#include "MainMenu.h"
#include "Types.h"
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <conio.h>

namespace DungeonGame {

    static void writeStrAt(HANDLE console, int col, int row, const std::string& text) {
        COORD pos = { (SHORT)col, (SHORT)row };
        DWORD written;
        WriteConsoleOutputCharacterA(console, text.c_str(), (DWORD)text.size(), pos, &written);
    }

    bool showMainMenu() {
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

        // hide cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(console, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(console, &cursorInfo);

        // set console size to match game
        SMALL_RECT minWindow = { 0, 0, 1, 1 };
        SetConsoleWindowInfo(console, TRUE, &minWindow);
        COORD bufferSize = { (SHORT)CONSOLE_WIDTH, (SHORT)MAP_HEIGHT };
        SetConsoleScreenBufferSize(console, bufferSize);
        SMALL_RECT windowSize = { 0, 0, (SHORT)(CONSOLE_WIDTH - 1), (SHORT)(MAP_HEIGHT - 1) };
        SetConsoleWindowInfo(console, TRUE, &windowSize);

        // clear screen
        DWORD written;
        COORD origin = { 0, 0 };
        FillConsoleOutputCharacterA(console, ' ',
            CONSOLE_WIDTH * MAP_HEIGHT, origin, &written);

        // layout
        const int centerCol = CONSOLE_WIDTH / 2;
        const int titleRow = MAP_HEIGHT / 2 - 4;
        const int startRow = MAP_HEIGHT / 2;
        const int quitRow = MAP_HEIGHT / 2 + 2;

        // title
        std::string title = "Project DungeonCrawl";
        writeStrAt(console, centerCol - (int)title.size() / 2, titleRow, title);

        // instructions
/*        std::string instruction = "Press Enter to Make Your Selection...";
        writeStrAt(console, centerCol - (int)title.size() / 2, titleRow + 2, instruction);*/

        bool selected = true; 

        while (true) {

            std::string startLine = selected
                ? "-> Start Game <-"
                : "   Start Game   ";
            std::string quitLine = !selected
                ? "-> Quit Game <-"
                : "   Quit Game   ";

            writeStrAt(console, centerCol - (int)startLine.size() / 2, startRow, startLine);
            writeStrAt(console, centerCol - (int)quitLine.size() / 2, quitRow, quitLine);

  
            int ch = _getch();
            if (ch == 224) {
                int arrow = _getch();
                if (arrow == 72 || arrow == 80) // up or down
                    selected = !selected;
            }
            else if (ch == 13) { // Enter confirms selection
                return selected;
            }
            else if (ch == 27) { // ESC = quit
                return false;
            }
        }
    }

}