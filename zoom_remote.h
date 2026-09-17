#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "zoom_settings.h"
#include "zoom_hotkeys.h"
#include "zoom_keymap.h"
#include "zoom_transport.h"
#include "views/zoom_remote_view.h"
#include "views/zoom_share_view.h"

/* IDs der Views im ViewDispatcher */
typedef enum {
    ZoomViewMenuOs,
    ZoomViewMenuConn,
    ZoomViewMenuLayout,
    ZoomViewRemote,
    ZoomViewCategories,
    ZoomViewHotkeys,
    ZoomViewShare,
    ZoomViewDialog,
} ZoomViewId;

/* Wofür der Dialog gerade benutzt wird */
typedef enum {
    ZoomDialogNone,
    ZoomDialogConfirmLeave, /* "Meeting verlassen?" */
    ZoomDialogHotkeyInfo, /* Details zu einem Hotkey */
} ZoomDialogMode;

/* Zentrale App-Struktur */
typedef struct {
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;

    Submenu* menu_os;
    Submenu* menu_conn;
    Submenu* menu_layout;
    Submenu* menu_categories;
    Submenu* menu_hotkeys;
    DialogEx* dialog;
    ZoomRemoteView* remote_view;
    ZoomShareView* share_view;

    ZoomSettings settings;
    ZoomViewId current_view;
    ZoomTransport* transport; /* NULL, solange kein Remote-Screen aktiv ist */
    bool connected; /* zuletzt angezeigter Verbindungsstatus */

    ZoomCategory current_category;
    ZoomDialogMode dialog_mode;
    char dialog_text[128];

    /* Lokal geschätzter Zustand im Meeting */
    bool mic_on;
    bool cam_on;
} ZoomRemoteApp;
