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

/* Zweisprachige Texte: {Englisch, Deutsch} */
#define L(en, de) {(en), (de)}
#define NO_NOTE   {NULL, NULL}

/* Wiederkehrende Hinweise */
#define NOTE_MCHAT   L("Meeting chat must be open", "Chat im Meeting muss offen sein")
#define NOTE_TCHAT   L("Main window, Chat tab only", "Nur im Hauptfenster, Tab Chat")
#define NOTE_HOST    L("Host only", "nur Host")
#define NOTE_UNTEST  L("Mac: untested", "Mac: ungetestet")
#define NOTE_TOOLBAR L("Win: meeting toolbar needs focus", "Win: Meeting-Leiste braucht Fokus")

/*
 * Die ersten Einträge müssen in der Reihenfolge von ZoomHotkeyId stehen,
 * weil die Remote-View sie über den Index anspricht.
 */
static const ZoomHotkey zoom_hotkeys[] = {
    /* ---- Feste Aktionen der Remote-View (Reihenfolge = ZoomHotkeyId) ---- */
    {L("Mute/unmute mic", "Mikro an/aus"),
     ZoomCatMeeting,
     W(A, 'a'),
     M(G | S, 'a'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Start/stop video", "Kamera an/aus"),
     ZoomCatMeeting,
     W(A, 'v'),
     M(G | S, 'v'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Raise/lower hand", "Hand heben/senken"),
     ZoomCatMeeting,
     W(A, 'y'),
     M(A, 'y'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Start/stop share", "Teilen start/stopp"),
     ZoomCatShare,
     W(A, 's'),
     M(G | S, 's'),
     UNK,
     GLB,
     NOTE_TOOLBAR},
    {L("Switch camera", "Kamera wechseln"),
     ZoomCatMeeting,
     W(A, 'n'),
     M(G | S, 'n'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Participants panel", "Teilnehmer ein/aus"),
     ZoomCatMeeting,
     W(A, 'u'),
     M(G, 'u'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Meeting chat panel", "Chat ein/aus"),
     ZoomCatMeetingChat,
     W(A, 'h'),
     M(G | S, 'h'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Leave meeting", "Meeting verlassen"),
     ZoomCatMeeting,
     W(A, 'q'),
     M(G, 'w'),
     UNK,
     FOC,
     L("Zoom asks again to confirm", "Zoom fragt selbst nochmal nach")},

    /* ---- Meeting ---- */
    {L("Mute all (host)", "Alle stumm (Host)"),
     ZoomCatMeeting,
     W(A, 'm'),
     M(G | C, 'm'),
     UNK,
     GLB,
     NOTE_HOST},
    {L("Ask all to unmute", "Stummsch. aufheben (Host)"),
     ZoomCatMeeting,
     NONE,
     M(G | C, 'u'),
     NO_,
     GLB,
     NOTE_HOST},
    {L("Invite", "Einladen"), ZoomCatMeeting, W(A, 'i'), M(G, 'i'), UNK, GLB, NO_NOTE},
    {L("Copy invite link", "Einladungslink kopieren"),
     ZoomCatMeeting,
     W(A | S, 'i'),
     M(G | S, 'i'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Join meeting", "Meeting beitreten"), ZoomCatMeeting, NONE, M(G, 'j'), NO_, FOC, NO_NOTE},
    {L("Start meeting", "Meeting starten"), ZoomCatMeeting, NONE, M(G | C, 'v'), NO_, FOC, NO_NOTE},
    {L("Schedule meeting", "Meeting planen"), ZoomCatMeeting, NONE, M(G, 'd'), NO_, FOC, NO_NOTE},
    {L("Read active speaker", "Sprechername vorlesen"),
     ZoomCatMeeting,
     W(C, '2'),
     M(G | S, '2'),
     UNK,
     GLB,
     L("Mac: page says Cmd+2, client 7.0.6 shows Shift+Cmd+2",
       "Mac: Seite sagt Cmd+2, Client 7.0.6 zeigt Shift+Cmd+2")},
    {L("Begin remote control", "Fernsteuerung starten"),
     ZoomCatMeeting,
     W(A | S, 'r'),
     M(C | S, 'r'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Revoke remote control", "Fernsteuerung abgeben"),
     ZoomCatMeeting,
     W(A | S, 'g'),
     M(C | S, 'g'),
     UNK,
     GLB,
     NO_NOTE},
    {L("AI Companion panel", "AI-Companion-Panel"),
     ZoomCatMeeting,
     W(C | S, 'o'),
     M(G | S, 'o'),
     UNK,
     FOC,
     L("Needs AI Companion set up", "AI Companion muss eingerichtet sein")},

    /* ---- Teilen ---- */
    {L("Show shareable windows", "Freigabe-Fenster zeigen"),
     ZoomCatShare,
     W(A | S, 's'),
     NONE,
     UNK,
     NO_,
     NOTE_TOOLBAR},
    {L("Pause/resume share", "Teilen pausieren"),
     ZoomCatShare,
     W(A, 't'),
     M(G | S, 't'),
     UNK,
     GLB,
     NOTE_TOOLBAR},
    {L("Direct share", "Direktfreigabe"), ZoomCatShare, NONE, M(G | C, 's'), NO_, FOC, NO_NOTE},
    {L("Focus meeting controls", "Fokus Meeting-Leiste"),
     ZoomCatShare,
     W(C | A | S, ZoomKeyNone),
     NONE,
     UNK,
     NO_,
     L("Ctrl+Alt+Shift without a key", "Ctrl+Alt+Shift ohne Taste")},

    /* ---- Aufnahme ---- */
    {L("Local recording", "Aufnahme lokal"),
     ZoomCatRecord,
     W(A, 'r'),
     M(G | S, 'r'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Cloud recording", "Aufnahme Cloud"),
     ZoomCatRecord,
     W(A, 'c'),
     M(G | S, 'c'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Pause/resume recording", "Aufnahme pausieren"),
     ZoomCatRecord,
     W(A, 'p'),
     M(G | S, 'p'),
     UNK,
     GLB,
     NO_NOTE},

    /* ---- Ansicht ---- */
    {L("Speaker view", "Sprecheransicht"), ZoomCatView, W(A, ZoomKeyF1), NONE, UNK, NO_, NO_NOTE},
    {L("Gallery view", "Galerieansicht"), ZoomCatView, W(A, ZoomKeyF2), NONE, UNK, NO_, NO_NOTE},
    {L("Switch view", "Ansicht wechseln"), ZoomCatView, NONE, M(G | S, 'w'), NO_, GLB, NO_NOTE},
    {L("Gallery: previous page", "Galerie: Seite zurueck"),
     ZoomCatView,
     W(0, ZoomKeyPageUp),
     M(C, 'p'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Gallery: next page", "Galerie: Seite vor"),
     ZoomCatView,
     W(0, ZoomKeyPageDown),
     M(C, 'n'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Full screen", "Vollbild"), ZoomCatView, W(A, 'f'), M(G | S, 'f'), UNK, GLB, NO_NOTE},
    {L("Minimal window", "Minimalfenster"), ZoomCatView, NONE, M(G | S, 'm'), NO_, GLB, NO_NOTE},
    {L("Dual monitor mode", "Doppelmonitor"), ZoomCatView, NONE, M(G | S, 'd'), NO_, GLB, NO_NOTE},
    {L("Floating controls", "Schwebende Leiste"),
     ZoomCatView,
     W(C | A | S, 'h'),
     M(C | A | G, 'h'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Always show controls", "Leiste immer zeigen"),
     ZoomCatView,
     W(A, ZoomKeyNone),
     M(C, '\\'),
     UNK,
     GLB,
     NOTE_UNTEST},
    {L("Portrait/landscape", "Hoch-/Querformat"),
     ZoomCatView,
     W(A, 'l'),
     M(G, 'l'),
     UNK,
     FOC,
     NO_NOTE},
    {L("Close window", "Fenster schliessen"),
     ZoomCatView,
     W(A, ZoomKeyF4),
     M(G, 'w'),
     UNK,
     FOC,
     NO_NOTE},

    /*
     * ---- Meeting-Chat ----
     * Wirken nur, wenn der Chatbereich im Meeting offen ist und den Fokus hat
     * (Zoom-Seite: "only usable while in chat panel"). Am Mac geprüft 2026-09-17:
     * Cmd+N reagiert erst, nachdem der Chat mit Shift+Cmd+H geöffnet wurde.
     */
    {L("New chat", "Neuer Chat"), ZoomCatMeetingChat, W(C, 'n'), M(G, 'n'), UNK, FOC, NOTE_MCHAT},
    {L("Focus chat input", "Chat-Eingabefeld"),
     ZoomCatMeetingChat,
     W(A | S, 'e'),
     M(G | S, 'e'),
     UNK,
     FOC,
     NOTE_MCHAT},
    {L("Last message", "Letzte Nachricht"),
     ZoomCatMeetingChat,
     W(C | S, 'u'),
     M(G | S, 'u'),
     UNK,
     FOC,
     NOTE_MCHAT},
    {L("Focus chat list", "Chatliste fokussieren"),
     ZoomCatMeetingChat,
     W(C, 'l'),
     NONE,
     UNK,
     NO_,
     NOTE_MCHAT},
    {L("Jump to unread", "Zu ungelesenen"),
     ZoomCatMeetingChat,
     W(C | S, 'n'),
     NONE,
     UNK,
     NO_,
     NOTE_MCHAT},
    {L("Chat size up", "Chat groesser"),
     ZoomCatMeetingChat,
     W(C, '+'),
     M(G, '+'),
     UNK,
     FOC,
     NOTE_UNTEST},
    {L("Chat size down", "Chat kleiner"),
     ZoomCatMeetingChat,
     W(C, '-'),
     M(G, '-'),
     UNK,
     FOC,
     NOTE_UNTEST},
    {L("Reset chat size", "Chatgroesse zurueck"),
     ZoomCatMeetingChat,
     NONE,
     M(G, '0'),
     NO_,
     FOC,
     NO_NOTE},

    /*
     * ---- Team-Chat ----
     * Abschnitt "Chat" der Zoom-Seite: gilt für den Chat-Tab im Zoom-Hauptfenster,
     * nicht für das Meeting-Fenster. Das Hauptfenster muss den Fokus haben.
     */
    {L("Screenshot", "Screenshot"), ZoomCatTeamChat, W(A | S, 't'), M(G, 't'), UNK, GLB, NO_NOTE},
    {L("New message", "Neue Nachricht"),
     ZoomCatTeamChat,
     W(C, 'n'),
     M(G, 'n'),
     UNK,
     FOC,
     NOTE_TCHAT},
    {L("Search in chat", "Im Chat suchen"), ZoomCatTeamChat, NONE, M(G, 'f'), NO_, FOC, NOTE_TCHAT},
    {L("Hide chat", "Chat ausblenden"), ZoomCatTeamChat, W(C, 'w'), NONE, UNK, NO_, NOTE_TCHAT},
    {L("Previous chat", "Voriger Chat"),
     ZoomCatTeamChat,
     W(C, ZoomKeyUp),
     NONE,
     UNK,
     NO_,
     NOTE_TCHAT},
    {L("Next chat", "Naechster Chat"),
     ZoomCatTeamChat,
     W(C, ZoomKeyDown),
     NONE,
     UNK,
     NO_,
     NOTE_TCHAT},
    {L("Chat history back", "Chatverlauf zurueck"),
     ZoomCatTeamChat,
     W(A, ZoomKeyLeft),
     M(G, '['),
     UNK,
     FOC,
     NOTE_UNTEST},
    {L("Chat history forward", "Chatverlauf vor"),
     ZoomCatTeamChat,
     W(A, ZoomKeyRight),
     M(G, ']'),
     UNK,
     FOC,
     NOTE_UNTEST},
    {L("Focus chat list", "Chatliste fokussieren"),
     ZoomCatTeamChat,
     W(C, 'l'),
     M(G | S, 'l'),
     UNK,
     FOC,
     NOTE_TCHAT},
    {L("Latest message", "Neueste Nachricht"),
     ZoomCatTeamChat,
     W(C | S, 'u'),
     M(G | S, 'u'),
     UNK,
     FOC,
     NOTE_TCHAT},
    {L("Jump to new messages", "Zu neuen Nachrichten"),
     ZoomCatTeamChat,
     W(A, 'n'),
     M(G | C, 'n'),
     UNK,
     FOC,
     NOTE_TCHAT},
    {L("Focus chat input", "Chat-Eingabefeld"),
     ZoomCatTeamChat,
     NONE,
     M(G | S, 'e'),
     NO_,
     FOC,
     NOTE_TCHAT},
    {L("Add member", "Mitglied hinzufuegen"),
     ZoomCatTeamChat,
     W(C | A, 'i'),
     M(G | A, 'i'),
     UNK,
     FOC,
     NOTE_TCHAT},
    {L("Insert link", "Link einfuegen"), ZoomCatTeamChat, W(C, 'k'), NONE, UNK, NO_, NOTE_TCHAT},
    {L("Collapse all", "Alles einklappen"),
     ZoomCatTeamChat,
     NONE,
     M(A | G, '0'),
     NO_,
     FOC,
     NOTE_TCHAT},

    /* ---- Reaktionen ---- */
    {L("Reactions panel", "Reaktionen-Fenster"),
     ZoomCatReactions,
     W(C | S, 'y'),
     M(G | S, 'y'),
     UNK,
     FOC,
     NO_NOTE},
    {L("Clap", "Klatschen"), ZoomCatReactions, W(A | S, '4'), M(A | G, '4'), UNK, FOC, NO_NOTE},
    {L("Thumbs up", "Daumen hoch"),
     ZoomCatReactions,
     W(A | S, '5'),
     M(A | G, '5'),
     UNK,
     FOC,
     NO_NOTE},
    {L("Heart", "Herz"), ZoomCatReactions, W(A | S, '6'), M(A | G, '6'), UNK, FOC, NO_NOTE},
    {L("Joy", "Lachen"), ZoomCatReactions, W(A | S, '7'), M(A | G, '7'), UNK, FOC, NO_NOTE},
    {L("Open mouth", "Staunen"), ZoomCatReactions, W(A | S, '8'), M(A | G, '8'), UNK, FOC, NO_NOTE},
    {L("Tada", "Feiern"), ZoomCatReactions, W(A | S, '9'), M(A | G, '9'), UNK, FOC, NO_NOTE},

    /* ---- Telefon ---- */
    {L("Call number", "Nummer anrufen"), ZoomCatPhone, W(C, 'p'), M(C | S, 'c'), UNK, GLB, NO_NOTE},
    {L("Accept call", "Anruf annehmen"),
     ZoomCatPhone,
     W(C | S, 'a'),
     M(C | S, 'a'),
     UNK,
     GLB,
     NO_NOTE},
    {L("Decline call", "Anruf ablehnen"),
     ZoomCatPhone,
     W(C | S, 'd'),
     M(C | S, 'd'),
     UNK,
     GLB,
     NO_NOTE},
    {L("End call", "Anruf beenden"), ZoomCatPhone, W(C | S, 'e'), M(C | S, 'e'), UNK, GLB, NO_NOTE},
    {L("Mute call", "Anruf stumm"), ZoomCatPhone, W(C | S, 'm'), M(C | S, 'm'), UNK, GLB, NO_NOTE},
    {L("Hold call", "Anruf halten"), ZoomCatPhone, W(C | S, 'h'), M(C | S, 'h'), UNK, GLB, NO_NOTE},
    {L("Transfer call", "Anruf weiterleiten"),
     ZoomCatPhone,
     W(C | S, 't'),
     M(C | S, 't'),
     UNK,
     GLB,
     NO_NOTE},

    /* ---- Allgemein ---- */
    {L("Search", "Suche"), ZoomCatGeneral, W(C, 'f'), M(G, 'e'), UNK, FOC, NO_NOTE},
    {L("Calendar panel", "Kalender-Panel"),
     ZoomCatGeneral,
     W(C | S, 'j'),
     M(G | S, 'j'),
     UNK,
     FOC,
     NO_NOTE},
    {L("Next tab", "Naechster Tab"),
     ZoomCatGeneral,
     W(C, ZoomKeyTab),
     M(C, ZoomKeyTab),
     UNK,
     FOC,
     NO_NOTE},
    {L("Previous tab", "Voriger Tab"),
     ZoomCatGeneral,
     W(C | S, ZoomKeyTab),
     M(C | S, ZoomKeyTab),
     UNK,
     FOC,
     NO_NOTE},
    {L("Last tab", "Letzter Tab"), ZoomCatGeneral, W(C, '9'), M(G, '9'), UNK, FOC, NO_NOTE},
    {L("Cycle popup windows", "Popup-Fenster wechseln"),
     ZoomCatGeneral,
     W(0, ZoomKeyF6),
     NONE,
     UNK,
     NO_,
     NO_NOTE},
    {L("Network diagnostic", "Netzwerkdiagnose"),
     ZoomCatGeneral,
     W(C | A | S, 'd'),
     M(G | A | S, 'd'),
     UNK,
     FOC,
     L("Mac: client health check", "Mac: Healthcheck")},
    {L("Share log", "Log teilen"), ZoomCatGeneral, NONE, M(C | A | S, 'v'), NO_, FOC, NO_NOTE},
};

size_t zoom_hotkeys_count(void) {
    return COUNT_OF(zoom_hotkeys);
}

const ZoomHotkey* zoom_hotkey_get(size_t index) {
    furi_assert(index < COUNT_OF(zoom_hotkeys));
    return &zoom_hotkeys[index];
}

const char* zoom_category_name(ZoomCategory category) {
    static const char* names[ZoomCatCount][ZoomLangCount] = {
        {"Meeting", "Meeting"},
        {"Share", "Teilen"},
        {"Recording", "Aufnahme"},
        {"View", "Ansicht"},
        {"Meeting chat", "Meeting-Chat"},
        {"Team chat", "Team-Chat"},
        {"Reactions", "Reaktionen"},
        {"Phone", "Telefon"},
        {"General", "Allgemein"},
    };
    if(category >= ZoomCatCount) return "?";
    return names[category][zoom_lang_get()];
}

const char* zoom_hotkey_name(const ZoomHotkey* hotkey) {
    return hotkey->name[zoom_lang_get()];
}

const char* zoom_hotkey_note(const ZoomHotkey* hotkey) {
    return hotkey->note[zoom_lang_get()];
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
        return zoom_tr("Focused Zoom only", "Nur bei Zoom-Fokus");
    case ZoomScopeGlobal:
        return zoom_tr("Can be set global", "Global aktivierbar");
    case ZoomScopeUnknown:
        return zoom_tr("Global: unconfirmed", "Global: unbestaetigt");
    default:
        return zoom_tr("Not available", "Nicht vorhanden");
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
        return zoom_tr("Up", "Hoch");
    case ZoomKeyDown:
        return zoom_tr("Down", "Runter");
    case ZoomKeyLeft:
        return zoom_tr("Left", "Links");
    case ZoomKeyRight:
        return zoom_tr("Right", "Rechts");
    case ZoomKeyTab:
        return "Tab";
    case ZoomKeySpace:
        return zoom_tr("Space", "Leertaste");
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
