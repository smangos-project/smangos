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
| `POST /key` | `{"keys":"ctrl+a","repeat":1,"hold_ms":0}`; names: letters, digits, `f1`-`f12`, `enter`, `tab`, `esc`, `space`, `backspace`, `delete`, arrows, `home`, `end`, `pageup`, `pagedown`, `shift`, `ctrl`, `alt`, `plus`, `minus`, `comma`, `period`, `slash`, `grave`, `printscreen` |
| `POST /type` | `{"text":"..."}` (UTF-8, sent as `WM_CHAR`) |
| `POST /click` | `{"x":..,"y":..,"button":"left","double":false}` |
| `POST /move` | `{"x":..,"y":..}` |
| `POST /drag` | `{"from":[x,y],"to":[x,y],"button":"right"}` |
| `POST /scroll` | `{"x":..,"y":..,"delta":-3}` (notches) |
| `POST /batch` | `{"actions":[{"key":"ctrl+a"},{"type":"name"},{"click":{"x":1,"y":2}},{"wait_ms":500}], "screenshot":{...}}` |
| `GET /mem` | debug: `addr` (Ghidra address, hex), `len` (64, max 4096), `follow` (comma list: `*` dereferences the pointer at the current address, a hex number is added); replies with `hex` bytes |
| `POST /dump` | writes `WowClassic.custom.dmp` (same as the `custom.dumpnow` trigger) |
| `POST /quit` | `WM_CLOSE` to the window; the process is terminated after 10 s if it is still there |

Every input endpoint also takes:

- `"screenshot": {...}`: same fields as `GET /screenshot` plus `settle_ms` (300); the reply then has a `screenshot` entry.
- `"space": "screenshot"`: coordinates are in the last screenshot's pixels (its scale and region are applied).
- `"method": "post" | "sendinput"` (default `post`).

## Input methods

`post` sends window messages to the client's window: no focus change, doesn't touch your mouse. `sendinput` brings the
window to the front and uses real input (moves the real cursor); use it for anything `post` doesn't get through.

Measured with `post` (login screen, realm list, character screen): `/type`, `/key` (enter, tab, backspace), `/click`
and screenshots work, including a full login through to the character screen and the "Enter World" click.

Known gaps with `post`:
- `ctrl+a` does not select all: the client reads modifier state from the real keyboard, so use `backspace` with
  `repeat` to clear a field. Other modifier combos likely behave the same; retry with `"method":"sendinput"`.
- A posted double click only selects a list row; click the confirm button instead.
- In-world input (chat, camera drag, movement) is not measured yet.

## Login

No login endpoint; send a batch with your own credentials. At 1680x1050 the account field is at about (840, 549) and
the password field at (840, 651) in client pixels. A dialog on screen (e.g. a disconnect notice) must be dismissed first.

    {"actions":[{"click":{"x":840,"y":549}},{"key":{"keys":"backspace","repeat":20}},{"type":"ACCOUNT"},
                {"click":{"x":840,"y":651}},{"key":{"keys":"backspace","repeat":20}},{"type":"PASSWORD"},
                {"key":"enter"}]}

Then pick the realm in the list and click Okay, and click "Enter World" on the character screen
(positions vary with the window size; take a screenshot and use `"space":"screenshot"`).

## Screenshots

The next presented D3D12 frame is copied from the swap chain first (works under wine and when the window is covered;
hooks Present on the first request). If that fails, `PrintWindow` with full-content rendering is tried, then a copy of
the screen, which needs the window uncovered. Under wine, `save` paths go through the `Z:` drive (`Z:/tmp/shot.jpg`).
