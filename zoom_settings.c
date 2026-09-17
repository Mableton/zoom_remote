#include "zoom_settings.h"

#include <furi.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

#define TAG "ZoomSettings"

/* Ablageort: /ext/apps_data/zoom_remote/ (APP_DATA_PATH löst auf die App-ID auf) */
#define ZOOM_SETTINGS_DIR     EXT_PATH("apps_data/zoom_remote")
#define ZOOM_SETTINGS_FILE    APP_DATA_PATH("settings.txt")
#define ZOOM_SETTINGS_HEADER  "Zoom Remote Settings"
#define ZOOM_SETTINGS_VERSION 1

/* Schlüssel in der Datei */
#define KEY_OS     "OS"
#define KEY_CONN   "Verbindung"
#define KEY_LAYOUT "Layout"

void zoom_settings_load(ZoomSettings* settings) {
    furi_assert(settings);

    /* Standardwerte, falls nichts gespeichert ist */
    settings->os = ZoomOsWindows;
    settings->conn = ZoomConnUsb;
    settings->layout = ZoomLayoutQwertz;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);
    FuriString* header = furi_string_alloc();

    do {
        if(!flipper_format_file_open_existing(ff, ZOOM_SETTINGS_FILE)) break;

        uint32_t version = 0;
        if(!flipper_format_read_header(ff, header, &version)) break;
        if(furi_string_cmp_str(header, ZOOM_SETTINGS_HEADER) != 0) break;
        if(version != ZOOM_SETTINGS_VERSION) break;

        uint32_t value = 0;
        if(flipper_format_read_uint32(ff, KEY_OS, &value, 1) && value < ZoomOsCount) {
            settings->os = (ZoomOs)value;
        }
        if(flipper_format_read_uint32(ff, KEY_CONN, &value, 1) && value < ZoomConnCount) {
            settings->conn = (ZoomConn)value;
        }
        if(flipper_format_read_uint32(ff, KEY_LAYOUT, &value, 1) && value < ZoomLayoutCount) {
            settings->layout = (ZoomLayout)value;
        }
        FURI_LOG_I(TAG, "Einstellungen geladen");
    } while(false);

    furi_string_free(header);
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
}

bool zoom_settings_save(const ZoomSettings* settings) {
    furi_assert(settings);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    /* Ordner anlegen, falls die App zum ersten Mal läuft */
    storage_simply_mkdir(storage, EXT_PATH("apps_data"));
    storage_simply_mkdir(storage, ZOOM_SETTINGS_DIR);

    FlipperFormat* ff = flipper_format_file_alloc(storage);
    bool ok = false;

    do {
        if(!flipper_format_file_open_always(ff, ZOOM_SETTINGS_FILE)) break;
        if(!flipper_format_write_header_cstr(ff, ZOOM_SETTINGS_HEADER, ZOOM_SETTINGS_VERSION))
            break;

        uint32_t value = settings->os;
        if(!flipper_format_write_uint32(ff, KEY_OS, &value, 1)) break;
        value = settings->conn;
        if(!flipper_format_write_uint32(ff, KEY_CONN, &value, 1)) break;
        value = settings->layout;
        if(!flipper_format_write_uint32(ff, KEY_LAYOUT, &value, 1)) break;
        ok = true;
    } while(false);

    if(!ok) FURI_LOG_E(TAG, "Einstellungen konnten nicht gespeichert werden");

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

const char* zoom_os_name(ZoomOs os) {
    switch(os) {
    case ZoomOsWindows:
        return "Win";
    case ZoomOsMac:
        return "Mac";
    default:
        return "?";
    }
}

const char* zoom_conn_name(ZoomConn conn) {
    switch(conn) {
    case ZoomConnUsb:
        return "USB";
    case ZoomConnBt:
        return "BT";
    default:
        return "?";
    }
}

const char* zoom_layout_name(ZoomLayout layout) {
    switch(layout) {
    case ZoomLayoutQwertz:
        return "QWERTZ";
    case ZoomLayoutQwerty:
        return "QWERTY";
    default:
        return "?";
    }
}
