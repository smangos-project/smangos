#pragma once

#include <windows.h>

#include <string>

// Post: window messages to the client's own window, no focus change.
// SendInput: real input; brings the window to the front and moves the real cursor.
enum class InputMethod {
    Post,
    SendInput,
};

// Every function returns false and sets `error` on bad arguments. Coordinates are client pixels.
bool InputKeys(HWND window, std::string const& combo, int repeat, int holdMs, InputMethod method, std::string& error);
bool InputText(HWND window, std::string const& utf8, InputMethod method, std::string& error);
bool InputMove(HWND window, int x, int y, InputMethod method, std::string& error);
bool InputClick(HWND window, int x, int y, std::string const& button, bool doubleClick, InputMethod method,
                std::string& error);
bool InputDrag(HWND window, POINT from, POINT to, std::string const& button, InputMethod method, std::string& error);
bool InputScroll(HWND window, int x, int y, int notches, InputMethod method, std::string& error);
