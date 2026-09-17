#pragma once

#include <gui/view.h>
#include "../zoom_settings.h"

/* Eigene View für den Remote-Screen (Hauptbildschirm) */
typedef struct ZoomRemoteView ZoomRemoteView;

/* Ereignisse, die die View an die App meldet (Tastenbelegung siehe CLAUDE.md) */
typedef enum {
    ZoomRemoteEventMicToggle, /* OK kurz */
    ZoomRemoteEventResetState, /* OK lang */
    ZoomRemoteEventCamToggle, /* Hoch kurz */
    ZoomRemoteEventCamSwitch, /* Hoch lang */
    ZoomRemoteEventHandToggle, /* Runter kurz */
    ZoomRemoteEventChat, /* Runter lang */
    ZoomRemoteEventShareToggle, /* Links kurz */
    ZoomRemoteEventShareMode, /* Links lang */
    ZoomRemoteEventParticipants, /* Rechts kurz */
    ZoomRemoteEventHotkeys, /* Rechts lang */
    ZoomRemoteEventLeave, /* Zurück lang */
} ZoomRemoteEvent;

typedef void (*ZoomRemoteViewCallback)(ZoomRemoteEvent event, void* context);

ZoomRemoteView* zoom_remote_view_alloc(void);
void zoom_remote_view_free(ZoomRemoteView* view);
View* zoom_remote_view_get_view(ZoomRemoteView* view);

void zoom_remote_view_set_callback(
    ZoomRemoteView* view,
    ZoomRemoteViewCallback callback,
    void* context);

/* Kopfzeile: OS, Verbindung, Layout */
void zoom_remote_view_set_config(ZoomRemoteView* view, const ZoomSettings* settings);

/* Verbindungsstatus (USB angesteckt bzw. BT verbunden) */
void zoom_remote_view_set_connected(ZoomRemoteView* view, bool connected);

/* Geschätzter Mikro-/Kamerastatus */
void zoom_remote_view_set_state(ZoomRemoteView* view, bool mic_on, bool cam_on);
