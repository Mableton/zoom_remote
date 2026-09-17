#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "zoom_settings.h"
#include "zoom_i18n.h"

/*
 * Datentabelle aller Zoom-Hotkeys für Windows und macOS.
 *
 * Quelle: Zoom-Supportartikel "Using hot keys and keyboard shortcuts"
 *   https://support.zoom.com/hc/en/article?id=zm_kb&sysparm_article=KB0067050
 *   Abgerufen am 2026-09-15 (die Seite trägt keinen Stand-Datumsstempel).
 * Global-Markierung macOS: geprüft im Zoom-Workplace-Client 7.0.6 (84834),
 *   Einstellungen > Tastaturkürzel, Spalte "Allg. Tastaturkürzel" (2026-09-15).
 * Global-Markierung Windows: noch unbestätigt (kein Windows-Client verfügbar).
 */

/* Modifier-Bits einer Tastenkombination (OS-neutral) */
#define ZOOM_MOD_CTRL  (1 << 0)
#define ZOOM_MOD_SHIFT (1 << 1)
#define ZOOM_MOD_ALT   (1 << 2) /* Alt bzw. Option */
#define ZOOM_MOD_CMD   (1 << 3) /* Command (nur macOS) */

/* Tasten ohne ASCII-Zeichen (Werte >= 0x80, damit sie nicht mit ASCII kollidieren) */
enum {
    ZoomKeyNone = 0, /* nur Modifier senden */
    ZoomKeyF1 = 0x80,
    ZoomKeyF2,
    ZoomKeyF3,
    ZoomKeyF4,
    ZoomKeyF5,
    ZoomKeyF6,
    ZoomKeyF7,
    ZoomKeyF8,
    ZoomKeyF9,
    ZoomKeyF10,
    ZoomKeyF11,
    ZoomKeyF12,
    ZoomKeyPageUp,
    ZoomKeyPageDown,
    ZoomKeyUp,
    ZoomKeyDown,
    ZoomKeyLeft,
    ZoomKeyRight,
    ZoomKeyTab,
    ZoomKeySpace,
    ZoomKeyEscape,
    ZoomKeyEnter,
};

/* Eine Tastenkombination: Modifier + Taste (ASCII wie auf der Zoom-Seite, US-Layout) */
typedef struct {
    uint8_t mods;
    uint8_t key;
} ZoomCombo;

/* Kein Kürzel für dieses OS vorhanden */
#define ZOOM_COMBO_NONE ((ZoomCombo){0, 0})

/* Wirkungsbereich eines Kürzels */
typedef enum {
    ZoomScopeNone, /* für dieses OS nicht vorhanden */
    ZoomScopeFocus, /* wirkt nur bei fokussiertem Zoom-Fenster */
    ZoomScopeGlobal, /* in Zoom als globales Kürzel aktivierbar */
    ZoomScopeUnknown, /* nicht geprüft (Windows) */
} ZoomScope;

/* Kategorien für das Untermenü */
typedef enum {
    ZoomCatMeeting,
    ZoomCatShare,
    ZoomCatRecord,
    ZoomCatView,
    ZoomCatMeetingChat, /* Chatbereich im Meeting */
    ZoomCatTeamChat, /* Chat-Tab im Zoom-Hauptfenster */
    ZoomCatReactions,
    ZoomCatPhone,
    ZoomCatGeneral,
    ZoomCatCount,
} ZoomCategory;

/* IDs der Aktionen, die die Remote-View direkt auslöst */
typedef enum {
    ZoomHotkeyMuteAudio,
    ZoomHotkeyVideo,
    ZoomHotkeyRaiseHand,
    ZoomHotkeyShare,
    ZoomHotkeySwitchCamera,
    ZoomHotkeyParticipants,
    ZoomHotkeyChatPanel,
    ZoomHotkeyLeave,
    /* alle weiteren Einträge werden nur über die Tabelle adressiert */
} ZoomHotkeyId;

typedef struct {
    const char* name[ZoomLangCount]; /* Anzeigename (kurz, für Submenu): {Englisch, Deutsch} */
    ZoomCategory category;
    ZoomCombo win;
    ZoomCombo mac;
    ZoomScope scope_win;
    ZoomScope scope_mac;
    const char* note[ZoomLangCount]; /* Hinweis {Englisch, Deutsch}; NULL wenn keiner */
} ZoomHotkey;

/* Zugriff auf die Tabelle */
size_t zoom_hotkeys_count(void);
const ZoomHotkey* zoom_hotkey_get(size_t index);
const char* zoom_category_name(ZoomCategory category);

/* Name und Hinweis in der aktuellen Sprache (Hinweis kann NULL sein) */
const char* zoom_hotkey_name(const ZoomHotkey* hotkey);
const char* zoom_hotkey_note(const ZoomHotkey* hotkey);

/* Kombination und Wirkungsbereich für das gewählte OS */
ZoomCombo zoom_hotkey_combo(const ZoomHotkey* hotkey, ZoomOs os);
ZoomScope zoom_hotkey_scope(const ZoomHotkey* hotkey, ZoomOs os);
const char* zoom_scope_name(ZoomScope scope);

/* Kombination als Text, z.B. "Alt+Shift+Y" oder "Cmd+Shift+A" */
void zoom_combo_to_string(ZoomCombo combo, ZoomOs os, char* buffer, size_t size);
