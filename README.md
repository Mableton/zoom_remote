# Zoom Remote for Flipper Zero

A native Flipper Zero app (`.fap`) that turns your Flipper into a remote control
for **Zoom Workplace** on macOS and Windows, over **USB HID** or **Bluetooth HID**.

> Not affiliated with or endorsed by Zoom Video Communications, Inc.

- Built and tested on Momentum firmware `mntm-012` (API 87.1). It only uses
  APIs that also exist in the official firmware SDK.
- **Tested:** macOS with Zoom Workplace 7.0.6, USB and Bluetooth.
- **Untested:** Windows. The Windows shortcuts come straight from Zoom's support
  page, but nobody has verified them on a real Windows machine yet. Feedback welcome.

## Features

- Start menu: host OS, connection (USB / Bluetooth) and, on Windows, the host
  keyboard layout (QWERTZ / QWERTY). Your last choice is remembered.
- Remote screen with the most important actions on the buttons, plus a locally
  tracked mic and camera state. The state is an estimate, Zoom gives no feedback.
- Share mode: opens the share dialog, then the D-pad sends arrow keys,
  OK sends Enter, hold OK sends Tab, Back sends Escape and leaves the mode.
- Menu with all Zoom hotkeys, grouped by category. OK sends the hotkey, hold OK
  shows the key combination, whether it can be made global in Zoom, and notes.
- Leaving a meeting always asks for confirmation first.
- English and German UI, switchable in the first menu.
- On exit the USB mode and the Bluetooth profile are restored, like the stock
  HID app does.

## Remote screen buttons

| Button | Short press            | Long press                     |
|--------|------------------------|--------------------------------|
| OK     | Mute / unmute mic      | Reset the estimated state      |
| Up     | Start / stop video     | Switch camera                  |
| Down   | Raise / lower hand     | Meeting chat panel             |
| Left   | Start / stop share     | Share mode                     |
| Right  | Participants panel     | All hotkeys                    |
| Back   | Back to the start menu | Leave meeting (asks to confirm)|

## Good to know

- Most Zoom shortcuts only work while the Zoom meeting window has focus. Many
  can be enabled as **global shortcuts** in Zoom under Settings > Keyboard
  shortcuts. The hotkey info screen tells you which ones.
- Meeting chat shortcuts need the meeting chat panel to be open. Team chat
  shortcuts only work in Zoom's main window on the Chat tab.
- On macOS, Zoom matches shortcuts by physical key position, so the app sends
  US key positions and ignores the host layout. On Windows, shortcuts follow the
  active layout, so the app converts for QWERTZ (Y/Z swap, `+`, `-`).
- Bluetooth: the Flipper shows up as "Zoom <your Flipper name>" with its own
  address and its own pairing keys. If pairing gets stuck, remove the device on
  the computer and use "Forget BT pairing" in the connection menu.

## Install

Download the build for your firmware from the
[latest release](https://github.com/Mableton/zoom_remote/releases/latest):

- `zoom_remote_official-1.4.3.fap` for official Flipper firmware 1.4.3
- `zoom_remote_momentum-mntm-012.fap` for Momentum firmware mntm-012

Copy it to `SD Card/apps/Tools/` with qFlipper or the mobile app. On other
firmware versions the Flipper may warn about an API mismatch. In that case build
from source. The app is also submitted to the Flipper Apps Catalog (in review).

## Build from source

```bash
# Momentum SDK (use plain `ufbt update` for official firmware)
ufbt update --index-url=https://up.momentum-fw.dev/firmware/directory.json
ufbt            # build
ufbt launch     # build, install and start on a connected Flipper
```

`ufbt launch` only works while the app is not on the remote screen, because the
Flipper then presents itself as a keyboard instead of a serial port.

## Hotkey source

All shortcuts come from Zoom's support article
"Using hot keys and keyboard shortcuts" (KB0067050), retrieved 2026-09-15.
The "can be set global" flags for macOS were checked in the Zoom Workplace
7.0.6 client. The table lives in `zoom_hotkeys.c`.

## License

GPL-3.0, see [LICENSE](LICENSE). The USB and Bluetooth handling follows the HID
app of the Flipper Zero / Momentum firmware.
