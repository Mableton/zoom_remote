#pragma once

#include <gui/view.h>

/*
 * Eigene View für den Teilen-Modus: Steuerkreuz = Pfeiltasten, OK = Enter,
 * OK lang = Tab, Zurück = Escape und Modus verlassen.
 */
typedef struct ZoomShareView ZoomShareView;

typedef enum {
    ZoomShareEventPress, /* Taste halten (Pfeiltasten, für Tastenwiederholung am Rechner) */
    ZoomShareEventRelease, /* gehaltene Taste loslassen */
    ZoomShareEventTap, /* kurzer Tastendruck (Enter, Tab) */
    ZoomShareEventExit, /* Escape senden und Modus verlassen */
} ZoomShareEventType;

/* zoom_key ist ein ZoomKey*-Wert aus zoom_hotkeys.h */
typedef void (*ZoomShareViewCallback)(ZoomShareEventType type, uint8_t zoom_key, void* context);

ZoomShareView* zoom_share_view_alloc(void);
void zoom_share_view_free(ZoomShareView* view);
View* zoom_share_view_get_view(ZoomShareView* view);

void zoom_share_view_set_callback(
    ZoomShareView* view,
    ZoomShareViewCallback callback,
    void* context);

void zoom_share_view_set_connected(ZoomShareView* view, bool connected);
