#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "zoom_settings.h"

/*
 * Transport-Schicht: USB-HID oder Bluetooth-HID hinter einer gemeinsamen API.
 * Vorlage: applications/system/hid_app (Momentum, mntm-012), dort transport_usb.c
 * und transport_ble.c. Hier zur Laufzeit wählbar statt zur Compile-Zeit.
 */
typedef struct ZoomTransport ZoomTransport;

/*
 * Startet den Transport (USB-Modus umschalten bzw. BLE-Profil starten).
 * Kein Status-Callback: Der USB-HID-Callback der Firmware läuft im Interrupt
 * (usbd_poll im USB_LP_IRQHandler), dort darf kein ViewDispatcher-Event
 * abgesetzt werden. Der Status wird deshalb wie in der HID-App per
 * zoom_transport_is_connected() zyklisch abgefragt.
 */
ZoomTransport* zoom_transport_alloc(ZoomConn conn);

/* Stoppt den Transport und stellt den Ursprungszustand wieder her */
void zoom_transport_free(ZoomTransport* transport);

bool zoom_transport_is_connected(ZoomTransport* transport);

/* Taste (inkl. Modifier-Bits) drücken / loslassen / alles loslassen */
bool zoom_transport_press(ZoomTransport* transport, uint16_t hid_code);
bool zoom_transport_release(ZoomTransport* transport, uint16_t hid_code);
void zoom_transport_release_all(ZoomTransport* transport);

/* Löscht die gespeicherte Bluetooth-Kopplung der App (nur bei gestopptem Transport) */
void zoom_transport_forget_bt_pairing(void);

/* Kurzer Tastendruck: drücken, kurz warten, loslassen */
bool zoom_transport_tap(ZoomTransport* transport, uint16_t hid_code);
