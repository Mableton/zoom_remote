#include "zoom_transport.h"

#include <furi.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>
#include <furi_hal_bt.h>
#include <bt/bt_service/bt.h>
#include <extra_profiles/hid_profile.h>
#include <storage/storage.h>

#define TAG "ZoomTransport"

/* Haltezeit eines Tastendrucks, damit der Rechner ihn sicher registriert */
#define ZOOM_TAP_HOLD_MS 30

struct ZoomTransport {
    ZoomConn conn;

    /* USB: vorherige USB-Konfiguration, wird beim Beenden wiederhergestellt */
    FuriHalUsbInterface* usb_mode_prev;

    /* Bluetooth: Dienst, aktives HID-Profil und zuletzt gemeldeter Status */
    Bt* bt;
    FuriHalBleProfileBase* ble_profile;
    volatile bool ble_connected;
};

/* Eigene Bonding-Keys, getrennt von der Flipper-App-Kopplung und vom BT-Remote */
#define ZOOM_BT_KEYS_PATH APP_DATA_PATH(".bt_hid.keys")

/* Eigener Gerätename ("Zoom <Flippername>") und eigene MAC, damit der Rechner die
 * Zoom-Fernbedienung nicht mit dem BT-Remote der Firmware verwechselt */
static BleProfileHidParams zoom_ble_params = {
    .device_name_prefix = "Zoom",
    .mac_xor = 0x0005,
};

/* ---- USB ----------------------------------------------------------------- */

static void zoom_transport_usb_start(ZoomTransport* transport) {
    /* Wie hid_usb_app(): alten Modus merken, entsperren, auf HID umschalten */
    transport->usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);
    FURI_LOG_I(TAG, "USB-HID gestartet");
}

static void zoom_transport_usb_stop(ZoomTransport* transport) {
    furi_hal_hid_kb_release_all();
    /* Ursprünglichen USB-Modus wiederherstellen (z.B. CDC für qFlipper/ufbt) */
    furi_hal_usb_set_config(transport->usb_mode_prev, NULL);
    FURI_LOG_I(TAG, "USB-HID beendet, vorheriger Modus wiederhergestellt");
}

/* ---- Bluetooth ------------------------------------------------------------ */

/* Kommt aus dem BT-Dienst-Thread: nur den Status merken, die App fragt ihn ab */
static void zoom_transport_ble_status_callback(BtStatus status, void* context) {
    ZoomTransport* transport = context;
    transport->ble_connected = (status == BtStatusConnected);
}

static void zoom_transport_ble_start(ZoomTransport* transport) {
    /* Ablauf wie hid_ble_app() der HID-App */
    transport->bt = furi_record_open(RECORD_BT);
    bt_disconnect(transport->bt);

    /* Warten, bis der zweite Kern den NVM-Speicher aktualisiert hat */
    furi_delay_ms(200);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(storage, EXT_PATH("apps_data"));
    storage_simply_mkdir(storage, EXT_PATH("apps_data/zoom_remote"));
    furi_record_close(RECORD_STORAGE);
    bt_keys_storage_set_storage_path(transport->bt, ZOOM_BT_KEYS_PATH);

    transport->ble_profile = bt_profile_start(transport->bt, ble_profile_hid, &zoom_ble_params);
    furi_check(transport->ble_profile);

    furi_hal_bt_start_advertising();
    bt_set_status_changed_callback(transport->bt, zoom_transport_ble_status_callback, transport);
    FURI_LOG_I(TAG, "Bluetooth-HID gestartet");
}

static void zoom_transport_ble_stop(ZoomTransport* transport) {
    ble_profile_hid_kb_release_all(transport->ble_profile);
    bt_set_status_changed_callback(transport->bt, NULL, NULL);
    bt_disconnect(transport->bt);

    /* Warten, bis der zweite Kern den NVM-Speicher aktualisiert hat */
    furi_delay_ms(200);

    /* Ursprungszustand: Standard-Keys und Standardprofil (Flipper-App) */
    bt_keys_storage_set_default_path(transport->bt);
    furi_check(bt_profile_restore_default(transport->bt));

    furi_record_close(RECORD_BT);
    transport->bt = NULL;
    transport->ble_profile = NULL;
    transport->ble_connected = false;
    FURI_LOG_I(TAG, "Bluetooth-HID beendet, Standardprofil wiederhergestellt");
}

void zoom_transport_forget_bt_pairing(void) {
    /* Nur die eigene Keys-Datei löschen; die Kopplung der Flipper-App bleibt erhalten */
    Storage* storage = furi_record_open(RECORD_STORAGE);
    storage_simply_remove(storage, ZOOM_BT_KEYS_PATH);
    furi_record_close(RECORD_STORAGE);
}

/* ---- Gemeinsame API ------------------------------------------------------ */

ZoomTransport* zoom_transport_alloc(ZoomConn conn) {
    ZoomTransport* transport = malloc(sizeof(ZoomTransport));
    transport->conn = conn;
    transport->usb_mode_prev = NULL;
    transport->bt = NULL;
    transport->ble_profile = NULL;
    transport->ble_connected = false;

    if(conn == ZoomConnUsb) {
        zoom_transport_usb_start(transport);
    } else {
        zoom_transport_ble_start(transport);
    }
    return transport;
}

void zoom_transport_free(ZoomTransport* transport) {
    furi_assert(transport);
    if(transport->conn == ZoomConnUsb) {
        zoom_transport_usb_stop(transport);
    } else {
        zoom_transport_ble_stop(transport);
    }
    free(transport);
}

bool zoom_transport_is_connected(ZoomTransport* transport) {
    furi_assert(transport);
    if(transport->conn == ZoomConnUsb) return furi_hal_hid_is_connected();
    return transport->ble_connected;
}

bool zoom_transport_press(ZoomTransport* transport, uint16_t hid_code) {
    furi_assert(transport);
    if(transport->conn == ZoomConnUsb) return furi_hal_hid_kb_press(hid_code);
    return ble_profile_hid_kb_press(transport->ble_profile, hid_code);
}

bool zoom_transport_release(ZoomTransport* transport, uint16_t hid_code) {
    furi_assert(transport);
    if(transport->conn == ZoomConnUsb) return furi_hal_hid_kb_release(hid_code);
    return ble_profile_hid_kb_release(transport->ble_profile, hid_code);
}

void zoom_transport_release_all(ZoomTransport* transport) {
    furi_assert(transport);
    if(transport->conn == ZoomConnUsb) {
        furi_hal_hid_kb_release_all();
    } else {
        ble_profile_hid_kb_release_all(transport->ble_profile);
    }
}

bool zoom_transport_tap(ZoomTransport* transport, uint16_t hid_code) {
    furi_assert(transport);
    if(!zoom_transport_is_connected(transport)) return false;
    bool ok = zoom_transport_press(transport, hid_code);
    furi_delay_ms(ZOOM_TAP_HOLD_MS);
    ok &= zoom_transport_release(transport, hid_code);
    return ok;
}
