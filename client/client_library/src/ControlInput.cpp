#define NOMINMAX
#include "ControlInput.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <vector>

struct Key {
    WORD vk = 0;
    bool extended = false;
};

static std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    return text;
}

static bool ParseKey(std::string const& rawName, Key& key)
{
    std::string const name = Lower(rawName);
    struct Named {
        char const* name;
        WORD vk;
        bool extended;
    };
    static constexpr Named Names[] = {
        {"shift", VK_SHIFT, false}, {"ctrl", VK_CONTROL, false}, {"control", VK_CONTROL, false},
        {"alt", VK_MENU, false}, {"enter", VK_RETURN, false}, {"return", VK_RETURN, false},
        {"tab", VK_TAB, false}, {"escape", VK_ESCAPE, false}, {"esc", VK_ESCAPE, false},
        {"space", VK_SPACE, false}, {"backspace", VK_BACK, false}, {"delete", VK_DELETE, true},
        {"insert", VK_INSERT, true}, {"home", VK_HOME, true}, {"end", VK_END, true},
        {"pageup", VK_PRIOR, true}, {"pagedown", VK_NEXT, true}, {"up", VK_UP, true},
        {"down", VK_DOWN, true}, {"left", VK_LEFT, true}, {"right", VK_RIGHT, true},
        {"plus", VK_OEM_PLUS, false}, {"minus", VK_OEM_MINUS, false}, {"comma", VK_OEM_COMMA, false},
        {"period", VK_OEM_PERIOD, false}, {"slash", VK_OEM_2, false}, {"grave", VK_OEM_3, false},
    };
    for (Named const& named : Names) {
        if (name == named.name) {
            key = {named.vk, named.extended};
            return true;
        }
    }
    if (name.size() == 1 && std::isalnum(static_cast<unsigned char>(name[0]))) {
        key = {static_cast<WORD>(std::toupper(static_cast<unsigned char>(name[0]))), false};
        return true;
    }
    if (name.size() >= 2 && name[0] == 'f') {
        int const number = std::atoi(name.c_str() + 1);
        if (number >= 1 && number <= 12) {
            key = {static_cast<WORD>(VK_F1 + number - 1), false};
            return true;
        }
    }
    return false;
}

static bool ParseCombo(std::string const& combo, std::vector<Key>& keys, std::string& error)
{
    std::size_t start = 0;
    while (start <= combo.size()) {
        std::size_t end = combo.find('+', start);
        if (end == std::string::npos)
            end = combo.size();
        std::string const part = combo.substr(start, end - start);
        Key key;
        if (!ParseKey(part, key)) {
            error = "unknown key '" + part + "'";
            return false;
        }
        keys.push_back(key);
        start = end + 1;
    }
    return !keys.empty();
}

static LPARAM KeyLParam(Key key, bool up)
{
    UINT const scan = MapVirtualKeyW(key.vk, MAPVK_VK_TO_VSC);
    LPARAM value = 1 | (static_cast<LPARAM>(scan) << 16);
    if (key.extended)
        value |= 1 << 24;
    if (up)
        value |= (1 << 30) | (static_cast<LPARAM>(1) << 31);
    return value;
}

static void BringToFront(HWND window)
{
    if (IsIconic(window))
        ShowWindow(window, SW_RESTORE);
    SetForegroundWindow(window);
    Sleep(50);
}

static void KeyEvent(HWND window, Key key, bool up, InputMethod method)
{
    if (method == InputMethod::SendInput) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = static_cast<WORD>(MapVirtualKeyW(key.vk, MAPVK_VK_TO_VSC));
        input.ki.dwFlags = KEYEVENTF_SCANCODE | (up ? KEYEVENTF_KEYUP : 0) | (key.extended ? KEYEVENTF_EXTENDEDKEY : 0);
        SendInput(1, &input, sizeof(input));
        return;
    }
    bool const alt = key.vk == VK_MENU;
    UINT const message = alt ? (up ? WM_SYSKEYUP : WM_SYSKEYDOWN) : (up ? WM_KEYUP : WM_KEYDOWN);
    PostMessageW(window, message, key.vk, KeyLParam(key, up));
}

bool InputKeys(HWND window, std::string const& combo, int repeat, int holdMs, InputMethod method, std::string& error)
{
    std::vector<Key> keys;
    if (!ParseCombo(combo, keys, error))
        return false;
    if (method == InputMethod::SendInput)
        BringToFront(window);

    for (int i = 0; i < std::max(1, repeat); ++i) {
        for (Key key : keys)
            KeyEvent(window, key, false, method);
        if (holdMs > 0)
            Sleep(holdMs);
        for (auto it = keys.rbegin(); it != keys.rend(); ++it)
            KeyEvent(window, *it, true, method);
        Sleep(20);
    }
    return true;
}

bool InputText(HWND window, std::string const& utf8, InputMethod method, std::string& error)
{
    int const length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), static_cast<int>(utf8.size()),
                                           nullptr, 0);
    if (length <= 0 && !utf8.empty()) {
        error = "text is not valid UTF-8";
        return false;
    }
    std::wstring text(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), text.data(), length);

    if (method == InputMethod::SendInput)
        BringToFront(window);
    for (wchar_t c : text) {
        if (method == InputMethod::Post) {
            PostMessageW(window, WM_CHAR, c, 1);
        } else {
            INPUT inputs[2] = {};
            for (int i = 0; i < 2; ++i) {
                inputs[i].type = INPUT_KEYBOARD;
                inputs[i].ki.wScan = c;
                inputs[i].ki.dwFlags = KEYEVENTF_UNICODE | (i ? KEYEVENTF_KEYUP : 0);
            }
            SendInput(2, inputs, sizeof(INPUT));
        }
        Sleep(5);
    }
    return true;
}

static LPARAM PointLParam(int x, int y)
{
    return MAKELPARAM(static_cast<WORD>(static_cast<short>(x)), static_cast<WORD>(static_cast<short>(y)));
}

struct Button {
    UINT down;
    UINT up;
    UINT doubleClick;
    WPARAM flag;
    DWORD sendDown;
    DWORD sendUp;
};

static bool ParseButton(std::string const& name, Button& button)
{
    std::string const lower = Lower(name);
    if (lower.empty() || lower == "left")
        button = {WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK, MK_LBUTTON, MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP};
    else if (lower == "right")
        button = {WM_RBUTTONDOWN, WM_RBUTTONUP, WM_RBUTTONDBLCLK, MK_RBUTTON, MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP};
    else if (lower == "middle")
        button = {WM_MBUTTONDOWN, WM_MBUTTONUP, WM_MBUTTONDBLCLK, MK_MBUTTON, MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP};
    else
        return false;
    return true;
}

static void MoveEvent(HWND window, int x, int y, WPARAM buttons, InputMethod method)
{
    if (method == InputMethod::Post) {
        PostMessageW(window, WM_MOUSEMOVE, buttons, PointLParam(x, y));
        return;
    }
    POINT point = {x, y};
    ClientToScreen(window, &point);
    SetCursorPos(point.x, point.y);
}

static void ButtonEvent(HWND window, Button button, int x, int y, bool up, bool doubleClick, InputMethod method)
{
    if (method == InputMethod::Post) {
        UINT const message = doubleClick ? button.doubleClick : (up ? button.up : button.down);
        PostMessageW(window, message, up ? 0 : button.flag, PointLParam(x, y));
        return;
    }
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = up ? button.sendUp : button.sendDown;
    SendInput(1, &input, sizeof(input));
}

bool InputMove(HWND window, int x, int y, InputMethod method, std::string&)
{
    if (method == InputMethod::SendInput)
        BringToFront(window);
    MoveEvent(window, x, y, 0, method);
    return true;
}

bool InputClick(HWND window, int x, int y, std::string const& buttonName, bool doubleClick, InputMethod method,
                std::string& error)
{
    Button button;
    if (!ParseButton(buttonName, button)) {
        error = "unknown button '" + buttonName + "'";
        return false;
    }
    if (method == InputMethod::SendInput)
        BringToFront(window);
    MoveEvent(window, x, y, 0, method);
    Sleep(30);
    ButtonEvent(window, button, x, y, false, false, method);
    Sleep(30);
    ButtonEvent(window, button, x, y, true, false, method);
    if (doubleClick) {
        Sleep(30);
        ButtonEvent(window, button, x, y, false, true, method);
        Sleep(30);
        ButtonEvent(window, button, x, y, true, false, method);
    }
    return true;
}

bool InputDrag(HWND window, POINT from, POINT to, std::string const& buttonName, InputMethod method, std::string& error)
{
    Button button;
    if (!ParseButton(buttonName, button)) {
        error = "unknown button '" + buttonName + "'";
        return false;
    }
    if (method == InputMethod::SendInput)
        BringToFront(window);
    MoveEvent(window, from.x, from.y, 0, method);
    Sleep(30);
    ButtonEvent(window, button, from.x, from.y, false, false, method);
    int const steps = 20;
    for (int i = 1; i <= steps; ++i) {
        Sleep(15);
        MoveEvent(window, from.x + (to.x - from.x) * i / steps, from.y + (to.y - from.y) * i / steps, button.flag,
                  method);
    }
    Sleep(30);
    ButtonEvent(window, button, to.x, to.y, true, false, method);
    return true;
}

bool InputScroll(HWND window, int x, int y, int notches, InputMethod method, std::string&)
{
    if (method == InputMethod::SendInput)
        BringToFront(window);
    MoveEvent(window, x, y, 0, method);
    Sleep(30);
    if (method == InputMethod::Post) {
        // WM_MOUSEWHEEL carries screen coordinates.
        POINT point = {x, y};
        ClientToScreen(window, &point);
        PostMessageW(window, WM_MOUSEWHEEL, MAKEWPARAM(0, static_cast<WORD>(static_cast<short>(notches * WHEEL_DELTA))),
                     PointLParam(point.x, point.y));
    } else {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        input.mi.mouseData = static_cast<DWORD>(notches * WHEEL_DELTA);
        SendInput(1, &input, sizeof(input));
    }
    return true;
}
