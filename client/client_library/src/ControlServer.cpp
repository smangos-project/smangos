#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "ControlServer.hpp"
#include "Overlay.hpp"

#include "ControlCapture.hpp"
#include "ControlInput.hpp"
#include "DumpTrigger.hpp"

#include "custom/LaunchProtocol.hpp"

#include "hook_lib/Log.hpp"
#include "hook_lib/ProcessImage.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using Json = nlohmann::json;

constexpr int DefaultPort = 8790;
constexpr std::size_t MaxRequestBytes = 8 * 1024 * 1024;

struct Request {
    std::string method;
    std::string path;
    std::map<std::string, std::string> query;
    std::string body;
};

struct Response {
    int status = 200;
    std::string contentType = "application/json";
    std::string body;
};

static ULONGLONG g_startTick = 0;
static HWND g_window = nullptr;
// Where the last screenshot sits in client pixels, for "space": "screenshot" coordinates.
static double g_lastScale = 1.0;
static int g_lastOriginX = 0;
static int g_lastOriginY = 0;

static Response JsonResponse(int status, Json const& json)
{
    return {status, "application/json", json.dump()};
}

static Response ErrorResponse(int status, std::string const& message)
{
    return JsonResponse(status, Json{{"error", message}});
}

static std::string WideToUtf8(std::wstring const& text)
{
    if (text.empty())
        return {};
    int const size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr,
                                         nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size, nullptr, nullptr);
    return result;
}

static std::string Base64(std::vector<std::uint8_t> const& data)
{
    static constexpr char Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve((data.size() + 2) / 3 * 4);
    for (std::size_t i = 0; i < data.size(); i += 3) {
        std::uint32_t const chunk = (data[i] << 16) | (i + 1 < data.size() ? data[i + 1] << 8 : 0) |
                                    (i + 2 < data.size() ? data[i + 2] : 0);
        out += Alphabet[chunk >> 18 & 63];
        out += Alphabet[chunk >> 12 & 63];
        out += i + 1 < data.size() ? Alphabet[chunk >> 6 & 63] : '=';
        out += i + 2 < data.size() ? Alphabet[chunk & 63] : '=';
    }
    return out;
}

static std::string UrlDecode(std::string const& text)
{
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '+') {
            out += ' ';
        } else if (text[i] == '%' && i + 2 < text.size() && std::isxdigit(static_cast<unsigned char>(text[i + 1])) &&
                   std::isxdigit(static_cast<unsigned char>(text[i + 2]))) {
            out += static_cast<char>(std::strtol(text.substr(i + 1, 2).c_str(), nullptr, 16));
            i += 2;
        } else {
            out += text[i];
        }
    }
    return out;
}

static std::string GetString(Json const& params, char const* key, std::string const& fallback = {})
{
    auto it = params.find(key);
    if (it == params.end())
        return fallback;
    if (it->is_string())
        return it->get<std::string>();
    return it->dump();
}

static int GetInt(Json const& params, char const* key, int fallback)
{
    auto it = params.find(key);
    if (it == params.end())
        return fallback;
    if (it->is_number())
        return static_cast<int>(it->get<double>());
    if (it->is_string())
        return std::atoi(it->get<std::string>().c_str());
    return fallback;
}

static bool GetBool(Json const& params, char const* key, bool fallback)
{
    auto it = params.find(key);
    if (it == params.end())
        return fallback;
    if (it->is_boolean())
        return it->get<bool>();
    if (it->is_number())
        return it->get<double>() != 0;
    if (it->is_string()) {
        std::string const value = it->get<std::string>();
        return value == "1" || value == "true";
    }
    return fallback;
}

struct WindowSearch {
    DWORD pid;
    HWND best = nullptr;
    long long bestArea = 0;
};

static BOOL CALLBACK FindWindowProc(HWND window, LPARAM parameter)
{
    auto* search = reinterpret_cast<WindowSearch*>(parameter);
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    if (pid != search->pid || !IsWindowVisible(window) || GetWindow(window, GW_OWNER))
        return TRUE;
    RECT client = {};
    GetClientRect(window, &client);
    long long const area = static_cast<long long>(client.right) * client.bottom;
    if (area > search->bestArea) {
        search->best = window;
        search->bestArea = area;
    }
    return TRUE;
}

// The client recreates its window on some display changes, so a cached handle is re-checked on every use.
static HWND ClientWindow()
{
    if (g_window && IsWindow(g_window))
        return g_window;
    WindowSearch search{GetCurrentProcessId()};
    EnumWindows(FindWindowProc, reinterpret_cast<LPARAM>(&search));
    g_window = search.best;
    return g_window;
}

static Response NoWindow()
{
    return ErrorResponse(503, "no window yet");
}

static std::string WindowTitle(HWND window)
{
    wchar_t title[256] = {};
    GetWindowTextW(window, title, 256);
    return WideToUtf8(title);
}

static Response HandleStatus()
{
    Json json = {{"pid", GetCurrentProcessId()}, {"uptime_ms", GetTickCount64() - g_startTick}};
    HWND window = ClientWindow();
    json["window"] = window != nullptr;
    if (window) {
        RECT client = {};
        GetClientRect(window, &client);
        json["title"] = WindowTitle(window);
        json["width"] = client.right;
        json["height"] = client.bottom;
        json["foreground"] = GetForegroundWindow() == window;
        json["minimized"] = IsIconic(window) != FALSE;
    }
    return JsonResponse(200, json);
}

// Runs a capture from `params` (query or JSON). The image goes to `capture`; the returned JSON describes it.
static bool Screenshot(HWND window, Json const& params, CaptureResult& capture, Json& info, std::string& error)
{
    CaptureOptions options;
    options.maxWidth = GetInt(params, "max_width", 1280);
    options.png = GetString(params, "format", "jpeg") == "png";
    std::string const region = GetString(params, "region");
    if (!region.empty()) {
        int x, y, w, h;
        if (std::sscanf(region.c_str(), "%d,%d,%d,%d", &x, &y, &w, &h) != 4) {
            error = "region must be x,y,w,h";
            return false;
        }
        options.hasRegion = true;
        options.region = {x, y, x + w, y + h};
    }

    capture = CaptureWindow(window, options);
    if (!capture.ok) {
        error = capture.error;
        return false;
    }
    g_lastScale = capture.scale;
    g_lastOriginX = capture.originX;
    g_lastOriginY = capture.originY;

    info = {{"width", capture.width}, {"height", capture.height}, {"scale", capture.scale},
            {"format", options.png ? "png" : "jpeg"}, {"region_x", capture.originX}, {"region_y", capture.originY}};

    std::string const save = GetString(params, "save");
    if (!save.empty()) {
        FILE* file = std::fopen(save.c_str(), "wb");
        if (!file || std::fwrite(capture.bytes.data(), 1, capture.bytes.size(), file) != capture.bytes.size()) {
            if (file)
                std::fclose(file);
            error = "could not write '" + save + "'";
            return false;
        }
        std::fclose(file);
        info["path"] = save;
    } else if (!GetBool(params, "raw", false)) {
        info["mime"] = capture.mime;
        info["data"] = Base64(capture.bytes);
    }
    return true;
}

static Response HandleScreenshot(Json const& params)
{
    HWND window = ClientWindow();
    if (!window)
        return NoWindow();
    CaptureResult capture;
    Json info;
    std::string error;
    if (!Screenshot(window, params, capture, info, error))
        return ErrorResponse(500, error);
    if (GetBool(params, "raw", false) && GetString(params, "save").empty())
        return {200, capture.mime, std::string(capture.bytes.begin(), capture.bytes.end())};
    return JsonResponse(200, info);
}

static POINT ReadPoint(Json const& params, char const* xKey, char const* yKey)
{
    int x = GetInt(params, xKey, 0);
    int y = GetInt(params, yKey, 0);
    if (GetString(params, "space") == "screenshot") {
        x = g_lastOriginX + static_cast<int>(x / g_lastScale + 0.5);
        y = g_lastOriginY + static_cast<int>(y / g_lastScale + 0.5);
    }
    return {x, y};
}

static bool ReadPair(Json const& params, char const* key, POINT& point, std::string& error)
{
    auto it = params.find(key);
    if (it == params.end() || !it->is_array() || it->size() != 2) {
        error = std::string(key) + " must be [x, y]";
        return false;
    }
    Json const pair = {{"x", (*it)[0]}, {"y", (*it)[1]}, {"space", GetString(params, "space")}};
    point = ReadPoint(pair, "x", "y");
    return true;
}

static InputMethod ReadMethod(Json const& params)
{
    return GetString(params, "method", "post") == "sendinput" ? InputMethod::SendInput : InputMethod::Post;
}

// One input action. `name` is the endpoint name or batch key.
static bool RunAction(HWND window, std::string const& name, Json const& params, std::string& error)
{
    InputMethod const method = ReadMethod(params);
    if (name == "key")
        return InputKeys(window, GetString(params, "keys"), GetInt(params, "repeat", 1), GetInt(params, "hold_ms", 0),
                         method, error);
    if (name == "type")
        return InputText(window, GetString(params, "text"), method, error);
    if (name == "move") {
        POINT const point = ReadPoint(params, "x", "y");
        return InputMove(window, point.x, point.y, method, error);
    }
    if (name == "click") {
        POINT const point = ReadPoint(params, "x", "y");
        return InputClick(window, point.x, point.y, GetString(params, "button", "left"),
                          GetBool(params, "double", false), method, error);
    }
    if (name == "drag") {
        POINT from, to;
        if (!ReadPair(params, "from", from, error) || !ReadPair(params, "to", to, error))
            return false;
        return InputDrag(window, from, to, GetString(params, "button", "left"), method, error);
    }
    if (name == "scroll") {
        POINT const point = ReadPoint(params, "x", "y");
        return InputScroll(window, point.x, point.y, GetInt(params, "delta", -1), method, error);
    }
    if (name == "wait") {
        Sleep(std::clamp(GetInt(params, "ms", 0), 0, 60000));
        return true;
    }
    error = "unknown action '" + name + "'";
    return false;
}

// Batch items are {"key": "ctrl+a"}, {"type": "text"}, {"click": {...}}, {"wait_ms": 500}, ...
static bool RunBatchItem(HWND window, Json const& item, Json const& defaults, std::string& error)
{
    if (!item.is_object() || item.size() != 1) {
        error = "each action must be an object with one key";
        return false;
    }
    std::string const name = item.begin().key();
    Json value = item.begin().value();

    std::string action = name;
    Json params = Json::object();
    if (name == "wait_ms") {
        action = "wait";
        params["ms"] = value;
    } else if (name == "key" && value.is_string()) {
        params["keys"] = value;
    } else if (name == "type" && value.is_string()) {
        params["text"] = value;
    } else if (value.is_object()) {
        params = value;
    } else {
        error = "bad parameters for '" + name + "'";
        return false;
    }
    for (auto const& entry : defaults.items())
        if (!params.contains(entry.key()))
            params[entry.key()] = entry.value();
    return RunAction(window, action, params, error);
}

static Response HandleInput(Request const& request, std::string const& action)
{
    HWND window = ClientWindow();
    if (!window)
        return NoWindow();

    Json params = request.body.empty() ? Json::object() : Json::parse(request.body, nullptr, false);
    if (!params.is_object())
        return ErrorResponse(400, "body must be a JSON object");

    std::string error;
    if (action == "batch") {
        auto actions = params.find("actions");
        if (actions == params.end() || !actions->is_array())
            return ErrorResponse(400, "actions must be an array");
        Json defaults = Json::object();
        for (char const* key : {"method", "space"})
            if (params.contains(key))
                defaults[key] = params[key];
        int index = 0;
        for (Json const& item : *actions) {
            if (!RunBatchItem(window, item, defaults, error))
                return ErrorResponse(400, "action " + std::to_string(index) + ": " + error);
            ++index;
        }
    } else if (!RunAction(window, action, params, error)) {
        return ErrorResponse(400, error);
    }

    Json result = {{"ok", true}};
    auto shot = params.find("screenshot");
    if (shot != params.end() && shot->is_object()) {
        Sleep(std::clamp(GetInt(*shot, "settle_ms", 300), 0, 10000));
        CaptureResult capture;
        Json info;
        Json shotParams = *shot;
        shotParams["raw"] = false;
        if (!Screenshot(window, shotParams, capture, info, error))
            return ErrorResponse(500, "action done, screenshot failed: " + error);
        result["screenshot"] = info;
    }
    return JsonResponse(200, result);
}

static Response HandleDump()
{
    WriteDumpNow();
    return JsonResponse(200, Json{{"ok", true}});
}

static Response HandleQuit()
{
    HWND window = ClientWindow();
    if (window)
        PostMessageW(window, WM_CLOSE, 0, 0);
    std::thread([] {
        Sleep(10000);
        TerminateProcess(GetCurrentProcess(), 0);
    }).detach();
    return JsonResponse(200, Json{{"ok", true}});
}

// Debug read of client memory. `addr` is a Ghidra address (image base 0x140000000); `follow` is a comma list applied in order:
// `*` dereferences the pointer at the current address, a hex number is added to it.
static Response HandleMem(Json const& params)
{
    auto const base = reinterpret_cast<std::uintptr_t>(GetProcessImage().BaseAddress);
    std::uintptr_t address = base + (std::strtoull(GetString(params, "addr").c_str(), nullptr, 16) - 0x140000000ull);
    int const length = std::clamp(GetInt(params, "len", 64), 1, 4096);

    std::stringstream offsets(GetString(params, "follow"));
    std::string item;
    while (std::getline(offsets, item, ',')) {
        if (item != "*") {
            address += std::strtoull(item.c_str(), nullptr, 16);
            continue;
        }
        std::uintptr_t pointer = 0;
        if (!ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void const*>(address), &pointer, sizeof(pointer), nullptr))
            return ErrorResponse(400, "unreadable pointer");
        address = pointer;
    }

    std::string bytes(length, '\0');
    SIZE_T read = 0;
    ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void const*>(address), bytes.data(), length, &read);
    char const* digits = "0123456789abcdef";
    std::string hex;
    for (SIZE_T i = 0; i < read; ++i) {
        auto const byte = static_cast<unsigned char>(bytes[i]);
        hex += digits[byte >> 4];
        hex += digits[byte & 15];
    }
    return JsonResponse(200, Json{{"address", address}, {"base", base}, {"hex", hex}});
}

static Response Route(Request const& request)
{
    Json query = Json::object();
    for (auto const& [key, value] : request.query)
        query[key] = value;

    bool const get = request.method == "GET";
    bool const post = request.method == "POST";
    if (get && request.path == "/status")
        return HandleStatus();
    if (get && request.path == "/screenshot")
        return HandleScreenshot(query);
    if (post && request.path == "/overlay") {
        for (auto const& [key, value] : request.query)
            SetOverlayParameter(key, std::strtof(value.c_str(), nullptr));
        return JsonResponse(200, Json{{"ok", true}});
    }
    if (get && request.path == "/mem")
        return HandleMem(query);
    if (post) {
        for (char const* action : {"key", "type", "click", "move", "drag", "scroll", "batch"})
            if (request.path == std::string("/") + action)
                return HandleInput(request, action);
        if (request.path == "/dump")
            return HandleDump();
        if (request.path == "/quit")
            return HandleQuit();
    }
    return ErrorResponse(404, "unknown endpoint " + request.method + " " + request.path);
}

static bool ReadRequest(SOCKET client, Request& request)
{
    std::string data;
    std::size_t headerEnd = std::string::npos;
    std::size_t contentLength = 0;
    char buffer[8192];
    while (data.size() < MaxRequestBytes) {
        if (headerEnd != std::string::npos && data.size() >= headerEnd + 4 + contentLength)
            break;
        int const received = recv(client, buffer, sizeof(buffer), 0);
        if (received <= 0)
            return false;
        data.append(buffer, received);
        if (headerEnd == std::string::npos) {
            headerEnd = data.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                std::string headers = data.substr(0, headerEnd);
                for (char& c : headers)
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                std::size_t const at = headers.find("content-length:");
                if (at != std::string::npos)
                    contentLength = std::strtoull(headers.c_str() + at + 15, nullptr, 10);
                if (contentLength > MaxRequestBytes)
                    return false;
            }
        }
    }
    if (headerEnd == std::string::npos)
        return false;

    std::string const line = data.substr(0, data.find("\r\n"));
    std::size_t const firstSpace = line.find(' ');
    std::size_t const secondSpace = line.find(' ', firstSpace + 1);
    if (firstSpace == std::string::npos || secondSpace == std::string::npos)
        return false;
    request.method = line.substr(0, firstSpace);
    std::string target = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);

    std::size_t const question = target.find('?');
    if (question != std::string::npos) {
        std::string query = target.substr(question + 1);
        target.resize(question);
        std::size_t start = 0;
        while (start <= query.size()) {
            std::size_t end = query.find('&', start);
            if (end == std::string::npos)
                end = query.size();
            std::string const pair = query.substr(start, end - start);
            std::size_t const equals = pair.find('=');
            if (!pair.empty())
                request.query[UrlDecode(pair.substr(0, equals))] =
                    equals == std::string::npos ? "1" : UrlDecode(pair.substr(equals + 1));
            start = end + 1;
        }
    }
    request.path = target;
    request.body = data.substr(headerEnd + 4, contentLength);
    return true;
}

static void SendAll(SOCKET client, char const* data, std::size_t size)
{
    while (size > 0) {
        int const sent = send(client, data, static_cast<int>(std::min<std::size_t>(size, 1 << 20)), 0);
        if (sent <= 0)
            return;
        data += sent;
        size -= sent;
    }
}

static void WriteResponse(SOCKET client, Response const& response)
{
    char const* reason = response.status == 200 ? "OK" : "Error";
    char header[256];
    int const length = std::snprintf(header, sizeof(header),
                                     "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                                     response.status, reason, response.contentType.c_str(), response.body.size());
    SendAll(client, header, length);
    SendAll(client, response.body.data(), response.body.size());
}

static void ServeClient(SOCKET client)
{
    DWORD timeout = 5000;
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char const*>(&timeout), sizeof(timeout));

    Request request;
    if (!ReadRequest(client, request)) {
        WriteResponse(client, ErrorResponse(400, "bad request"));
        return;
    }
    HOOK_LOG_INFO("control: %s %s", request.method.c_str(), request.path.c_str());
    FlushLog();

    Response response;
    try {
        response = Route(request);
    } catch (std::exception const& e) {
        response = ErrorResponse(500, e.what());
    }
    WriteResponse(client, response);
}

static void ServerThread(int port)
{
    // Coordinates must be physical client pixels whatever the process default is.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        HOOK_LOG_ERROR("control: WSAStartup failed");
        return;
    }

    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<u_short>(port));
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    if (listener == INVALID_SOCKET || bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
        listen(listener, 4) != 0) {
        HOOK_LOG_ERROR("control: cannot listen on 127.0.0.1:%d (%d)", port, WSAGetLastError());
        FlushLog();
        return;
    }
    HOOK_LOG_INFO("control: listening on 127.0.0.1:%d", port);
    FlushLog();

    for (;;) {
        SOCKET client = accept(listener, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            continue;
        ServeClient(client);
        closesocket(client);
    }
}

void StartControlServerIfRequested()
{
    for (std::wstring const& argument : DllArguments()) {
        int port = 0;
        if (_wcsicmp(argument.c_str(), L"--control") == 0)
            port = DefaultPort;
        else if (_wcsnicmp(argument.c_str(), L"--control=", 10) == 0)
            port = _wtoi(argument.c_str() + 10);
        else
            continue;

        if (port <= 0 || port > 65535) {
            HOOK_LOG_ERROR("control: invalid port in '%ls'", argument.c_str());
            return;
        }
        g_startTick = GetTickCount64();
        std::thread(ServerThread, port).detach();
        return;
    }
}
