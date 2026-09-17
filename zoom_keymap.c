#include "zoom_keymap.h"

#include <furi.h>
#include <furi_hal_usb_hid.h>

uint16_t zoom_keymap_special(uint8_t zoom_key) {
    switch(zoom_key) {
    case ZoomKeyF1:
        return HID_KEYBOARD_F1;
    case ZoomKeyF2:
        return HID_KEYBOARD_F2;
    case ZoomKeyF3:
        return HID_KEYBOARD_F3;
    case ZoomKeyF4:
        return HID_KEYBOARD_F4;
    case ZoomKeyF5:
        return HID_KEYBOARD_F5;
    case ZoomKeyF6:
        return HID_KEYBOARD_F6;
    case ZoomKeyF7:
        return HID_KEYBOARD_F7;
    case ZoomKeyF8:
        return HID_KEYBOARD_F8;
    case ZoomKeyF9:
        return HID_KEYBOARD_F9;
    case ZoomKeyF10:
        return HID_KEYBOARD_F10;
    case ZoomKeyF11:
        return HID_KEYBOARD_F11;
    case ZoomKeyF12:
        return HID_KEYBOARD_F12;
    case ZoomKeyPageUp:
        return HID_KEYBOARD_PAGE_UP;
    case ZoomKeyPageDown:
        return HID_KEYBOARD_PAGE_DOWN;
    case ZoomKeyUp:
        return HID_KEYBOARD_UP_ARROW;
    case ZoomKeyDown:
        return HID_KEYBOARD_DOWN_ARROW;
    case ZoomKeyLeft:
        return HID_KEYBOARD_LEFT_ARROW;
    case ZoomKeyRight:
        return HID_KEYBOARD_RIGHT_ARROW;
    case ZoomKeyTab:
        return HID_KEYBOARD_TAB;
    case ZoomKeySpace:
        return HID_KEYBOARD_SPACEBAR;
    case ZoomKeyEscape:
        return HID_KEYBOARD_ESCAPE;
    case ZoomKeyEnter:
        return HID_KEYBOARD_RETURN;
    default:
        return HID_KEYBOARD_NONE;
    }
}

/* Buchstabe/Ziffer/Zeichen -> HID-Code (ggf. mit Shift) für das Layout */
static bool zoom_keymap_char(char c, ZoomLayout layout, uint16_t* code) {
    bool qwertz = (layout == ZoomLayoutQwertz);

    if(c >= 'a' && c <= 'z') {
        /* QWERTZ: Y und Z sind physisch vertauscht */
        if(qwertz && c == 'y')
            c = 'z';
        else if(qwertz && c == 'z')
            c = 'y';
        *code = HID_KEYBOARD_A + (c - 'a');
        return true;
    }
    if(c >= '1' && c <= '9') {
        *code = HID_KEYBOARD_1 + (c - '1');
        return true;
    }
    if(c == '0') {
        *code = HID_KEYBOARD_0;
        return true;
    }

    switch(c) {
    case '+':
        /* DE: eigene Taste rechts von Ü (US-Position "]"); US: Taste "=" (ohne Shift) */
        *code = qwertz ? HID_KEYBOARD_CLOSE_BRACKET : HID_KEYBOARD_EQUAL_SIGN;
        return true;
    case '-':
        /* DE: rechts vom Punkt (US-Position "/"); US: eigene Taste */
        *code = qwertz ? HID_KEYBOARD_SLASH : HID_KEYBOARD_MINUS;
        return true;
    case '\\':
        /* DE: nur über AltGr+ß erreichbar -> als Kürzel nicht sauber sendbar */
        if(qwertz) return false;
        *code = HID_KEYBOARD_BACKSLASH;
        return true;
    case '[':
        if(qwertz) return false; /* DE: Option+5 (Mac) bzw. AltGr+8 (Win) */
        *code = HID_KEYBOARD_OPEN_BRACKET;
        return true;
    case ']':
        if(qwertz) return false; /* DE: Option+6 (Mac) bzw. AltGr+9 (Win) */
        *code = HID_KEYBOARD_CLOSE_BRACKET;
        return true;
    default:
        return false;
    }
}

bool zoom_keymap_to_hid(ZoomCombo combo, ZoomOs os, ZoomLayout layout, uint16_t* hid_code) {
    furi_assert(hid_code);

    /*
     * macOS: Zoom wertet Kürzel nach Tastenposition (virtueller Keycode, US-Belegung)
     * aus, nicht nach dem erzeugten Zeichen. Geprüft am 2026-09-17 mit Zoom 7.0.6 auf
     * einem Mac mit deutschem Layout: Option+Y (Hand heben) und Shift+Cmd+Y
     * (Reaktionen) reagieren nur auf die Taste an der US-Y-Position, also die
     * deutsche Z-Taste. Deshalb am Mac keine Layout-Umrechnung.
     * Windows: Hotkeys laufen über layoutabhängige VK-Codes, dort wird umgerechnet
     * (mangels Windows-Gerät ungetestet).
     */
    if(os == ZoomOsMac) layout = ZoomLayoutQwerty;

    uint16_t code = 0;

    if(combo.mods & ZOOM_MOD_CTRL) code |= KEY_MOD_LEFT_CTRL;
    if(combo.mods & ZOOM_MOD_SHIFT) code |= KEY_MOD_LEFT_SHIFT;
    if(combo.mods & ZOOM_MOD_ALT) code |= KEY_MOD_LEFT_ALT;
    if(combo.mods & ZOOM_MOD_CMD) code |= KEY_MOD_LEFT_GUI;

    if(combo.key == ZoomKeyNone) {
        if(combo.mods == 0) return false; /* leere Kombination */
        *hid_code = code;
        return true;
    }

    if(combo.key >= 0x80) {
        uint16_t special = zoom_keymap_special(combo.key);
        if(special == HID_KEYBOARD_NONE) return false;
        *hid_code = code | special;
        return true;
    }

    uint16_t key_code = 0;
    if(!zoom_keymap_char((char)combo.key, layout, &key_code)) return false;
    *hid_code = code | key_code;
    return true;
}
