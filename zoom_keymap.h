#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "zoom_hotkeys.h"
#include "zoom_settings.h"

/*
 * Umsetzung einer Zoom-Tastenkombination in einen HID-Keycode inkl. Modifier-Bits
 * (Format wie in furi_hal_usb_hid.h: KEY_MOD_* | HID_KEYBOARD_*).
 *
 * HID-Keycodes sind positionsbasiert. Bei QWERTZ am Rechner liegt z.B. das "Y"
 * auf der Position des US-"Z"; deshalb wird für Windows je nach Layout umgerechnet.
 * Am Mac wertet Zoom selbst Tastenpositionen aus, dort wird nicht umgerechnet.
 */

/* Liefert false, wenn die Kombination mit dem Layout nicht sauber sendbar ist */
bool zoom_keymap_to_hid(ZoomCombo combo, ZoomOs os, ZoomLayout layout, uint16_t* hid_code);

/* Einzelne Sondertaste ohne Modifier (für den Teilen-Modus) */
uint16_t zoom_keymap_special(uint8_t zoom_key);
