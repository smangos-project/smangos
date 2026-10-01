# Control server

Optional HTTP server inside `custom.dll` for seeing and driving the running client: screenshots, keys, text, mouse.
Off by default; without the option no port is opened.

## Start

    custom_launcher.exe --control            # port 8790
    custom_launcher.exe --control=8791       # another port

Binds to `127.0.0.1` only, one request at a time, no auth. Each request is logged in `WowClassic.exe.custom.log`.
The server lives as long as the client; start and stop the client from the shell.

    curl -s localhost:8790/status                          # repeat until "window": true
    curl -s "localhost:8790/screenshot?save=C:/tmp/shot.jpg"   # then read the file
    curl -s -X POST localhost:8790/batch -d @login.json
    curl -s -X POST localhost:8790/quit

## Endpoints

Replies are JSON; errors are `{"error": "..."}` with 4xx/5xx (`503 no window yet` before the window exists).
Coordinates are client-area pixels of the WoW window.

| Endpoint | Body / query |
|---|---|
| `GET /status` | pid, window found, title, client size, foreground, minimized, uptime |
| `GET /screenshot` | `max_width` (1280), `format=png\|jpeg`, `region=x,y,w,h`, `raw=1` (image bytes, use `curl -o`), `save=<path>` (writes the file); default is base64 JSON. Replies with width, height, `scale` (encoded px per client px) |
| `POST /key` | `{"keys":"ctrl+a","repeat":1,"hold_ms":0}`; names: letters, digits, `f1`-`f12`, `enter`, `tab`, `esc`, `space`, `backspace`, `delete`, arrows, `home`, `end`, `pageup`, `pagedown`, `shift`, `ctrl`, `alt`, `plus`, `minus`, `comma`, `period`, `slash`, `grave` |
| `POST /type` | `{"text":"..."}` (UTF-8, sent as `WM_CHAR`) |
| `POST /click` | `{"x":..,"y":..,"button":"left","double":false}` |
| `POST /move` | `{"x":..,"y":..}` |
| `POST /drag` | `{"from":[x,y],"to":[x,y],"button":"right"}` |
| `POST /scroll` | `{"x":..,"y":..,"delta":-3}` (notches) |
| `POST /batch` | `{"actions":[{"key":"ctrl+a"},{"type":"name"},{"click":{"x":1,"y":2}},{"wait_ms":500}], "screenshot":{...}}` |
| `POST /dump` | writes `WowClassic.custom.dmp` (same as the `custom.dumpnow` trigger) |
| `POST /quit` | `WM_CLOSE` to the window; the process is terminated after 10 s if it is still there |

Every input endpoint also takes:

- `"screenshot": {...}`: same fields as `GET /screenshot` plus `settle_ms` (300); the reply then has a `screenshot` entry.
- `"space": "screenshot"`: coordinates are in the last screenshot's pixels (its scale and region are applied).
- `"method": "post" | "sendinput"` (default `post`).

## Input methods

`post` sends window messages to the client's window: no focus change, doesn't touch your mouse. `sendinput` brings the
window to the front and uses real input (moves the real cursor); use it for anything `post` doesn't get through.

Verified on the login screen with `post`: `/type`, `/key` (tab), `/click` (field focus, checkbox).
Not verified yet: character screen and in-world input (chat, camera drag, movement, modifiers), and the login batch.

## Login

No login endpoint; send a batch with your own credentials: click the account field, `ctrl+a`, type the name, `tab`,
type the password, `enter`. On the login screen at 1680x1050 the account field is at about (840, 549) and the
password field at about (840, 651) in client pixels.

## Screenshots

`PrintWindow` with full-content rendering is tried first (works when the window is covered); if that comes back black,
the screen is copied instead, which needs the window uncovered.
