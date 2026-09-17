#include "zoom_hotkeys.h"

#include <furi.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Kurzschreibweisen für die Tabelle */
#define W(m, k) ((ZoomCombo){(m), (k)})
#define M(m, k) ((ZoomCombo){(m), (k)})
#define NONE    ZOOM_COMBO_NONE

#define C ZOOM_MOD_CTRL
#define S ZOOM_MOD_SHIFT
#define A ZOOM_MOD_ALT
#define G ZOOM_MOD_CMD

/* Wirkungsbereich */
#define FOC ZoomScopeFocus
#define GLB ZoomScopeGlobal
#define UNK ZoomScopeUnknown
#define NO_ ZoomScopeNone

/* Wiederkehrende Hinweise */
#define NOTE_MCHAT "Chat im Meeting muss offen sein"
#define NOTE_TCHAT "Nur im Hauptfenster, Tab Chat"

/*
 * Die ersten Einträge müssen in der Reihenfolge von ZoomHotkeyId stehen,
 * weil die Remote-View sie über den Index anspricht.
 */
static const ZoomHotkey zoom_hotkeys[] = {
    /* ---- Feste Aktionen der Remote-View (Reihenfolge = ZoomHotkeyId) ---- */
    {"Mikro an/aus", ZoomCatMeeting, W(A, 'a'), M(G | S, 'a'), UNK, GLB, NULL},
    {"Kamera an/aus", ZoomCatMeeting, W(A, 'v'), M(G | S, 'v'), UNK, GLB, NULL},
    {"Hand heben/senken", ZoomCatMeeting, W(A, 'y'), M(A, 'y'), UNK, GLB, NULL},
    {"Teilen start/stopp",
     ZoomCatShare,
     W(A, 's'),
     M(G | S, 's'),
     UNK,
     GLB,
     "Win: nur wenn Meeting-Leiste Fokus hat"},
    {"Kamera wechseln", ZoomCatMeeting, W(A, 'n'), M(G | S, 'n'), UNK, GLB, NULL},
    {"Teilnehmer ein/aus", ZoomCatMeeting, W(A, 'u'), M(G, 'u'), UNK, GLB, NULL},
    {"Chat ein/aus", ZoomCatMeetingChat, W(A, 'h'), M(G | S, 'h'), UNK, GLB, NULL},
    {"Meeting verlassen",
     ZoomCatMeeting,
     W(A, 'q'),
     M(G, 'w'),
     UNK,
     FOC,
     "Zoom fragt selbst nochmal nach"},

    /* ---- Meeting ---- */
    {"Alle stumm (Host)", ZoomCatMeeting, W(A, 'm'), M(G | C, 'm'), UNK, GLB, "nur Host"},
    {"Stummsch. aufheben (Host)", ZoomCatMeeting, NONE, M(G | C, 'u'), NO_, GLB, "nur Host"},
    {"Einladen", ZoomCatMeeting, W(A, 'i'), M(G, 'i'), UNK, GLB, NULL},
    {"Einladungslink kopieren", ZoomCatMeeting, W(A | S, 'i'), M(G | S, 'i'), UNK, GLB, NULL},
    {"Meeting beitreten", ZoomCatMeeting, NONE, M(G, 'j'), NO_, FOC, NULL},
    {"Meeting starten", ZoomCatMeeting, NONE, M(G | C, 'v'), NO_, FOC, NULL},
    {"Meeting planen", ZoomCatMeeting, NONE, M(G, 'd'), NO_, FOC, NULL},
    {"Sprechername vorlesen",
     ZoomCatMeeting,
     W(C, '2'),
     M(G | S, '2'),
     UNK,
     GLB,
     "Mac: Seite sagt Cmd+2, Client 7.0.6 zeigt Shift+Cmd+2"},
    {"Fernsteuerung starten", ZoomCatMeeting, W(A | S, 'r'), M(C | S, 'r'), UNK, GLB, NULL},
    {"Fernsteuerung abgeben", ZoomCatMeeting, W(A | S, 'g'), M(C | S, 'g'), UNK, GLB, NULL},
    {"AI-Companion-Panel", ZoomCatMeeting, W(C | S, 'o'), M(G | S, 'o'), UNK, FOC, NULL},

    /* ---- Teilen ---- */
    {"Freigabe-Fenster zeigen",
     ZoomCatShare,
     W(A | S, 's'),
     NONE,
     UNK,
     NO_,
     "nur wenn Meeting-Leiste Fokus hat"},
    {"Teilen pausieren",
     ZoomCatShare,
     W(A, 't'),
     M(G | S, 't'),
     UNK,
     GLB,
     "Win: nur wenn Meeting-Leiste Fokus hat"},
    {"Direktfreigabe", ZoomCatShare, NONE, M(G | C, 's'), NO_, FOC, NULL},
    {"Fokus Meeting-Leiste",
     ZoomCatShare,
     W(C | A | S, ZoomKeyNone),
     NONE,
     UNK,
     NO_,
     "Ctrl+Alt+Shift ohne Taste"},

    /* ---- Aufnahme ---- */
    {"Aufnahme lokal", ZoomCatRecord, W(A, 'r'), M(G | S, 'r'), UNK, GLB, NULL},
    {"Aufnahme Cloud", ZoomCatRecord, W(A, 'c'), M(G | S, 'c'), UNK, GLB, NULL},
    {"Aufnahme pausieren", ZoomCatRecord, W(A, 'p'), M(G | S, 'p'), UNK, GLB, NULL},

    /* ---- Ansicht ---- */
    {"Sprecheransicht", ZoomCatView, W(A, ZoomKeyF1), NONE, UNK, NO_, NULL},
    {"Galerieansicht", ZoomCatView, W(A, ZoomKeyF2), NONE, UNK, NO_, NULL},
    {"Ansicht wechseln", ZoomCatView, NONE, M(G | S, 'w'), NO_, GLB, NULL},
    {"Galerie: Seite zurueck", ZoomCatView, W(0, ZoomKeyPageUp), M(C, 'p'), UNK, GLB, NULL},
    {"Galerie: Seite vor", ZoomCatView, W(0, ZoomKeyPageDown), M(C, 'n'), UNK, GLB, NULL},
    {"Vollbild", ZoomCatView, W(A, 'f'), M(G | S, 'f'), UNK, GLB, NULL},
    {"Minimalfenster", ZoomCatView, NONE, M(G | S, 'm'), NO_, GLB, NULL},
    {"Doppelmonitor", ZoomCatView, NONE, M(G | S, 'd'), NO_, GLB, NULL},
    {"Schwebende Leiste", ZoomCatView, W(C | A | S, 'h'), M(C | A | G, 'h'), UNK, GLB, NULL},
    {"Leiste immer zeigen", ZoomCatView, W(A, ZoomKeyNone), M(C, '\\'), UNK, GLB, "Mac: ungetestet"},
    {"Hoch-/Querformat", ZoomCatView, W(A, 'l'), M(G, 'l'), UNK, FOC, NULL},
    {"Fenster schliessen", ZoomCatView, W(A, ZoomKeyF4), M(G, 'w'), UNK, FOC, NULL},

    /*
     * ---- Meeting-Chat ----
     * Wirken nur, wenn der Chatbereich im Meeting offen ist und den Fokus hat
     * (Zoom-Seite: "only usable while in chat panel"). Am Mac geprüft 2026-09-17:
     * Cmd+N reagiert erst, nachdem der Chat mit Shift+Cmd+H geöffnet wurde.
     */
    {"Neuer Chat", ZoomCatMeetingChat, W(C, 'n'), M(G, 'n'), UNK, FOC, NOTE_MCHAT},
    {"Chat-Eingabefeld", ZoomCatMeetingChat, W(A | S, 'e'), M(G | S, 'e'), UNK, FOC, NOTE_MCHAT},
    {"Letzte Nachricht", ZoomCatMeetingChat, W(C | S, 'u'), M(G | S, 'u'), UNK, FOC, NOTE_MCHAT},
    {"Chatliste fokussieren", ZoomCatMeetingChat, W(C, 'l'), NONE, UNK, NO_, NOTE_MCHAT},
    {"Zu ungelesenen", ZoomCatMeetingChat, W(C | S, 'n'), NONE, UNK, NO_, NOTE_MCHAT},
    {"Chat groesser", ZoomCatMeetingChat, W(C, '+'), M(G, '+'), UNK, FOC, "Mac: ungetestet"},
    {"Chat kleiner", ZoomCatMeetingChat, W(C, '-'), M(G, '-'), UNK, FOC, "Mac: ungetestet"},
    {"Chatgroesse zurueck", ZoomCatMeetingChat, NONE, M(G, '0'), NO_, FOC, NULL},

    /*
     * ---- Team-Chat ----
     * Abschnitt "Chat" der Zoom-Seite: gilt für den Chat-Tab im Zoom-Hauptfenster,
     * nicht für das Meeting-Fenster. Das Hauptfenster muss den Fokus haben.
     */
    {"Screenshot", ZoomCatTeamChat, W(A | S, 't'), M(G, 't'), UNK, GLB, NULL},
    {"Neue Nachricht", ZoomCatTeamChat, W(C, 'n'), M(G, 'n'), UNK, FOC, NOTE_TCHAT},
    {"Im Chat suchen", ZoomCatTeamChat, NONE, M(G, 'f'), NO_, FOC, NOTE_TCHAT},
    {"Chat ausblenden", ZoomCatTeamChat, W(C, 'w'), NONE, UNK, NO_, NOTE_TCHAT},
    {"Voriger Chat", ZoomCatTeamChat, W(C, ZoomKeyUp), NONE, UNK, NO_, NOTE_TCHAT},
    {"Naechster Chat", ZoomCatTeamChat, W(C, ZoomKeyDown), NONE, UNK, NO_, NOTE_TCHAT},
    {"Chatverlauf zurueck",
     ZoomCatTeamChat,
     W(A, ZoomKeyLeft),
     M(G, '['),
     UNK,
     FOC,
     "Mac: ungetestet"},
    {"Chatverlauf vor", ZoomCatTeamChat, W(A, ZoomKeyRight), M(G, ']'), UNK, FOC, "Mac: ungetestet"},
    {"Chatliste fokussieren", ZoomCatTeamChat, W(C, 'l'), M(G | S, 'l'), UNK, FOC, NOTE_TCHAT},
    {"Neueste Nachricht", ZoomCatTeamChat, W(C | S, 'u'), M(G | S, 'u'), UNK, FOC, NOTE_TCHAT},
    {"Zu neuen Nachrichten", ZoomCatTeamChat, W(A, 'n'), M(G | C, 'n'), UNK, FOC, NOTE_TCHAT},
    {"Chat-Eingabefeld", ZoomCatTeamChat, NONE, M(G | S, 'e'), NO_, FOC, NOTE_TCHAT},
    {"Mitglied hinzufuegen", ZoomCatTeamChat, W(C | A, 'i'), M(G | A, 'i'), UNK, FOC, NOTE_TCHAT},
    {"Link einfuegen", ZoomCatTeamChat, W(C, 'k'), NONE, UNK, NO_, NOTE_TCHAT},
    {"Alles einklappen", ZoomCatTeamChat, NONE, M(A | G, '0'), NO_, FOC, NOTE_TCHAT},

    /* ---- Reaktionen ---- */
    {"Reaktionen-Fenster", ZoomCatReactions, W(C | S, 'y'), M(G | S, 'y'), UNK, FOC, NULL},
    {"Klatschen", ZoomCatReactions, W(A | S, '4'), M(A | G, '4'), UNK, FOC, NULL},
    {"Daumen hoch", ZoomCatReactions, W(A | S, '5'), M(A | G, '5'), UNK, FOC, NULL},
    {"Herz", ZoomCatReactions, W(A | S, '6'), M(A | G, '6'), UNK, FOC, NULL},
    {"Lachen", ZoomCatReactions, W(A | S, '7'), M(A | G, '7'), UNK, FOC, NULL},
    {"Staunen", ZoomCatReactions, W(A | S, '8'), M(A | G, '8'), UNK, FOC, NULL},
    {"Feiern", ZoomCatReactions, W(A | S, '9'), M(A | G, '9'), UNK, FOC, NULL},

    /* ---- Telefon ---- */
    {"Nummer anrufen", ZoomCatPhone, W(C, 'p'), M(C | S, 'c'), UNK, GLB, NULL},
    {"Anruf annehmen", ZoomCatPhone, W(C | S, 'a'), M(C | S, 'a'), UNK, GLB, NULL},
    {"Anruf ablehnen", ZoomCatPhone, W(C | S, 'd'), M(C | S, 'd'), UNK, GLB, NULL},
    {"Anruf beenden", ZoomCatPhone, W(C | S, 'e'), M(C | S, 'e'), UNK, GLB, NULL},
    {"Anruf stumm", ZoomCatPhone, W(C | S, 'm'), M(C | S, 'm'), UNK, GLB, NULL},
    {"Anruf halten", ZoomCatPhone, W(C | S, 'h'), M(C | S, 'h'), UNK, GLB, NULL},
    {"Anruf weiterleiten", ZoomCatPhone, W(C | S, 't'), M(C | S, 't'), UNK, GLB, NULL},

    /* ---- Allgemein ---- */
    {"Suche", ZoomCatGeneral, W(C, 'f'), M(G, 'e'), UNK, FOC, NULL},
    {"Kalender-Panel", ZoomCatGeneral, W(C | S, 'j'), M(G | S, 'j'), UNK, FOC, NULL},
    {"Naechster Tab", ZoomCatGeneral, W(C, ZoomKeyTab), M(C, ZoomKeyTab), UNK, FOC, NULL},
    {"Voriger Tab", ZoomCatGeneral, W(C | S, ZoomKeyTab), M(C | S, ZoomKeyTab), UNK, FOC, NULL},
    {"Letzter Tab", ZoomCatGeneral, W(C, '9'), M(G, '9'), UNK, FOC, NULL},
    {"Popup-Fenster wechseln", ZoomCatGeneral, W(0, ZoomKeyF6), NONE, UNK, NO_, NULL},
    {"Netzwerkdiagnose",
     ZoomCatGeneral,
     W(C | A | S, 'd'),
     M(G | A | S, 'd'),
     UNK,
     FOC,
     "Mac: Healthcheck"},
    {"Log teilen", ZoomCatGeneral, NONE, M(C | A | S, 'v'), NO_, FOC, NULL},
};

size_t zoom_hotkeys_count(void) {
    return COUNT_OF(zoom_hotkeys);
}

const ZoomHotkey* zoom_hotkey_get(size_t index) {
    furi_assert(index < COUNT_OF(zoom_hotkeys));
    return &zoom_hotkeys[index];
}

const char* zoom_category_name(ZoomCategory category) {
    static const char* names[ZoomCatCount] = {
        "Meeting",
        "Teilen",
        "Aufnahme",
        "Ansicht",
        "Meeting-Chat",
        "Team-Chat",
        "Reaktionen",
        "Telefon",
        "Allgemein",
    };
    if(category >= ZoomCatCount) return "?";
    return names[category];
}

ZoomCombo zoom_hotkey_combo(const ZoomHotkey* hotkey, ZoomOs os) {
    return (os == ZoomOsMac) ? hotkey->mac : hotkey->win;
}

ZoomScope zoom_hotkey_scope(const ZoomHotkey* hotkey, ZoomOs os) {
    return (os == ZoomOsMac) ? hotkey->scope_mac : hotkey->scope_win;
}

const char* zoom_scope_name(ZoomScope scope) {
    switch(scope) {
    case ZoomScopeFocus:
        return "Nur bei Zoom-Fokus";
    case ZoomScopeGlobal:
        return "Global aktivierbar";
    case ZoomScopeUnknown:
        return "Global: unbestaetigt";
    default:
        return "Nicht vorhanden";
    }
}

/* Name einer Sondertaste für die Anzeige */
static const char* zoom_key_name(uint8_t key) {
    switch(key) {
    case ZoomKeyF1:
        return "F1";
    case ZoomKeyF2:
        return "F2";
    case ZoomKeyF3:
        return "F3";
    case ZoomKeyF4:
        return "F4";
    case ZoomKeyF5:
        return "F5";
    case ZoomKeyF6:
        return "F6";
    case ZoomKeyF7:
        return "F7";
    case ZoomKeyF8:
        return "F8";
    case ZoomKeyF9:
        return "F9";
    case ZoomKeyF10:
        return "F10";
    case ZoomKeyF11:
        return "F11";
    case ZoomKeyF12:
        return "F12";
    case ZoomKeyPageUp:
        return "PageUp";
    case ZoomKeyPageDown:
        return "PageDown";
    case ZoomKeyUp:
        return "Hoch";
    case ZoomKeyDown:
        return "Runter";
    case ZoomKeyLeft:
        return "Links";
    case ZoomKeyRight:
        return "Rechts";
    case ZoomKeyTab:
        return "Tab";
    case ZoomKeySpace:
        return "Leertaste";
    case ZoomKeyEscape:
        return "Esc";
    case ZoomKeyEnter:
        return "Enter";
    default:
        return NULL;
    }
}

void zoom_combo_to_string(ZoomCombo combo, ZoomOs os, char* buffer, size_t size) {
    furi_assert(buffer && size > 0);
    buffer[0] = '\0';

    if(combo.mods == 0 && combo.key == ZoomKeyNone) {
        snprintf(buffer, size, "-");
        return;
    }

    /* Reihenfolge wie auf der Zoom-Seite: Ctrl, Alt/Option, Shift, Cmd */
    if(combo.mods & ZOOM_MOD_CTRL) strlcat(buffer, "Ctrl+", size);
    if(combo.mods & ZOOM_MOD_ALT) strlcat(buffer, (os == ZoomOsMac) ? "Opt+" : "Alt+", size);
    if(combo.mods & ZOOM_MOD_SHIFT) strlcat(buffer, "Shift+", size);
    if(combo.mods & ZOOM_MOD_CMD) strlcat(buffer, "Cmd+", size);

    if(combo.key == ZoomKeyNone) {
        /* nur Modifier: abschließendes "+" entfernen */
        size_t len = strlen(buffer);
        if(len > 0) buffer[len - 1] = '\0';
        return;
    }

    const char* special = zoom_key_name(combo.key);
    if(special) {
        strlcat(buffer, special, size);
    } else {
        char letter[2] = {(char)toupper(combo.key), '\0'};
        strlcat(buffer, letter, size);
    }
}
