#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Ausgewähltes Betriebssystem des Zoom-Rechners */
typedef enum {
    ZoomOsWindows,
    ZoomOsMac,
    ZoomOsCount,
} ZoomOs;

/* Verbindungsart zum Rechner */
typedef enum {
    ZoomConnUsb,
    ZoomConnBt,
    ZoomConnCount,
} ZoomConn;

/* Tastaturlayout, das am Rechner aktiv ist (HID-Codes sind positionsbasiert) */
typedef enum {
    ZoomLayoutQwertz,
    ZoomLayoutQwerty,
    ZoomLayoutCount,
} ZoomLayout;

/* Gespeicherte Auswahl aus dem Startmenü */
typedef struct {
    ZoomOs os;
    ZoomConn conn;
    ZoomLayout layout;
} ZoomSettings;

/* Lädt die letzte Auswahl; bei Fehler bleiben die Standardwerte stehen */
void zoom_settings_load(ZoomSettings* settings);

/* Speichert die Auswahl unter /ext/apps_data/zoom_remote/settings.txt */
bool zoom_settings_save(const ZoomSettings* settings);

/* Kurze Anzeigenamen für die Views */
const char* zoom_os_name(ZoomOs os);
const char* zoom_conn_name(ZoomConn conn);
const char* zoom_layout_name(ZoomLayout layout);
