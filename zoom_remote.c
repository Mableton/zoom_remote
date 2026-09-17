/*
 * Zoom Remote – Fernbedienung für Zoom Workplace (Windows/macOS) per USB- oder
 * Bluetooth-HID. Einstiegspunkt, Navigation und Zusammenbau der Views.
 */
#include "zoom_remote.h"

#include <dolphin/dolphin.h>

#define TAG "ZoomRemote"

/* ---- Hilfsfunktionen ----------------------------------------------------- */

static void zoom_remote_switch_view(ZoomRemoteApp* app, ZoomViewId view_id) {
    app->current_view = view_id;
    view_dispatcher_switch_to_view(app->view_dispatcher, view_id);
}

/* Abfrageintervall für den Verbindungsstatus */
#define ZOOM_TICK_MS 250

/*
 * Verbindungsstatus zyklisch abfragen (wie die HID-App). Der USB-HID-Callback
 * der Firmware läuft im Interrupt und darf keine GUI-Events absetzen.
 */
static void zoom_remote_tick_callback(void* context) {
    ZoomRemoteApp* app = context;
    bool connected = app->transport && zoom_transport_is_connected(app->transport);
    if(connected == app->connected) return;

    app->connected = connected;
    zoom_remote_view_set_connected(app->remote_view, connected);
    zoom_share_view_set_connected(app->share_view, connected);
    notification_message(
        app->notifications, connected ? &sequence_blink_green_100 : &sequence_blink_red_100);
}

static void zoom_remote_transport_start(ZoomRemoteApp* app) {
    if(app->transport) return;
    app->transport = zoom_transport_alloc(app->settings.conn);
    app->connected = zoom_transport_is_connected(app->transport);
    zoom_remote_view_set_connected(app->remote_view, app->connected);
}

static void zoom_remote_transport_stop(ZoomRemoteApp* app) {
    if(!app->transport) return;
    zoom_transport_free(app->transport);
    app->transport = NULL;
    app->connected = false;
    zoom_remote_view_set_connected(app->remote_view, false);
}

/* Sendet einen Hotkey aus der Tabelle für das gewählte OS/Layout, mit Rückmeldung */
static bool zoom_remote_send_hotkey(ZoomRemoteApp* app, const ZoomHotkey* hotkey) {
    ZoomCombo combo = zoom_hotkey_combo(hotkey, app->settings.os);
    uint16_t hid_code = 0;

    if(!zoom_keymap_to_hid(combo, app->settings.os, app->settings.layout, &hid_code)) {
        FURI_LOG_W(TAG, "'%s' fuer dieses OS/Layout nicht sendbar", hotkey->name);
        notification_message(app->notifications, &sequence_error);
        return false;
    }
    if(!app->transport || !zoom_transport_tap(app->transport, hid_code)) {
        FURI_LOG_W(TAG, "'%s' nicht gesendet: keine Verbindung", hotkey->name);
        notification_message(app->notifications, &sequence_error);
        return false;
    }
    FURI_LOG_I(TAG, "Gesendet: %s (0x%04X)", hotkey->name, hid_code);
    notification_message(app->notifications, &sequence_blink_blue_10);
    return true;
}

static bool zoom_remote_send_id(ZoomRemoteApp* app, ZoomHotkeyId id) {
    return zoom_remote_send_hotkey(app, zoom_hotkey_get(id));
}

/* ---- Navigation ---------------------------------------------------------- */

/* Wird aufgerufen, wenn "Zurück" kurz gedrückt und von der View nicht verbraucht wurde */
static bool zoom_remote_navigation_callback(void* context) {
    ZoomRemoteApp* app = context;
    switch(app->current_view) {
    case ZoomViewMenuOs:
        return false; /* App beenden */
    case ZoomViewMenuConn:
        zoom_remote_switch_view(app, ZoomViewMenuOs);
        return true;
    case ZoomViewMenuLayout:
        zoom_remote_switch_view(app, ZoomViewMenuConn);
        return true;
    case ZoomViewRemote:
        /* zurück zum Startmenü: Transport beenden, damit USB/BT sauber zurückgesetzt wird */
        zoom_remote_transport_stop(app);
        zoom_remote_switch_view(app, ZoomViewMenuOs);
        return true;
    case ZoomViewCategories:
        zoom_remote_switch_view(app, ZoomViewRemote);
        return true;
    case ZoomViewHotkeys:
        zoom_remote_switch_view(app, ZoomViewCategories);
        return true;
    case ZoomViewDialog:
        zoom_remote_switch_view(
            app, (app->dialog_mode == ZoomDialogHotkeyInfo) ? ZoomViewHotkeys : ZoomViewRemote);
        app->dialog_mode = ZoomDialogNone;
        return true;
    default:
        zoom_remote_switch_view(app, ZoomViewRemote);
        return true;
    }
}

/* ---- Startmenü ----------------------------------------------------------- */

static void zoom_remote_menu_os_callback(void* context, uint32_t index) {
    ZoomRemoteApp* app = context;
    app->settings.os = (ZoomOs)index;
    submenu_set_selected_item(app->menu_conn, app->settings.conn);
    zoom_remote_switch_view(app, ZoomViewMenuConn);
}

/* Auswahl merken, Transport starten, Remote-Screen öffnen */
static void zoom_remote_enter_remote(ZoomRemoteApp* app) {
    zoom_settings_save(&app->settings);
    zoom_remote_view_set_config(app->remote_view, &app->settings);
    zoom_remote_view_set_state(app->remote_view, app->mic_on, app->cam_on);
    zoom_remote_transport_start(app);
    zoom_remote_switch_view(app, ZoomViewRemote);
}

/* Zusätzlicher Eintrag im Verbindungsmenü */
#define ZOOM_MENU_CONN_FORGET_BT 100

static void zoom_remote_menu_conn_callback(void* context, uint32_t index) {
    ZoomRemoteApp* app = context;

    if(index == ZOOM_MENU_CONN_FORGET_BT) {
        /* Transport läuft im Startmenü nie, die Keys-Datei ist also frei */
        zoom_transport_forget_bt_pairing();
        notification_message(app->notifications, &sequence_success);
        return;
    }
    app->settings.conn = (ZoomConn)index;

    /* Am Mac wertet Zoom Tastenpositionen aus -> Layout spielt keine Rolle */
    if(app->settings.os == ZoomOsMac) {
        zoom_remote_enter_remote(app);
        return;
    }
    submenu_set_selected_item(app->menu_layout, app->settings.layout);
    zoom_remote_switch_view(app, ZoomViewMenuLayout);
}

static void zoom_remote_menu_layout_callback(void* context, uint32_t index) {
    ZoomRemoteApp* app = context;
    app->settings.layout = (ZoomLayout)index;
    zoom_remote_enter_remote(app);
}

/* ---- Dialog (Bestätigung / Hotkey-Info) ---------------------------------- */

static void zoom_remote_dialog_callback(DialogExResult result, void* context) {
    ZoomRemoteApp* app = context;

    if(app->dialog_mode == ZoomDialogConfirmLeave) {
        if(result == DialogExResultRight) {
            zoom_remote_send_id(app, ZoomHotkeyLeave);
        }
        app->dialog_mode = ZoomDialogNone;
        zoom_remote_switch_view(app, ZoomViewRemote);
    } else if(app->dialog_mode == ZoomDialogHotkeyInfo) {
        app->dialog_mode = ZoomDialogNone;
        zoom_remote_switch_view(app, ZoomViewHotkeys);
    }
}

static void zoom_remote_show_confirm_leave(ZoomRemoteApp* app) {
    char combo_text[24];
    const ZoomHotkey* hotkey = zoom_hotkey_get(ZoomHotkeyLeave);
    zoom_combo_to_string(
        zoom_hotkey_combo(hotkey, app->settings.os),
        app->settings.os,
        combo_text,
        sizeof(combo_text));
    snprintf(
        app->dialog_text,
        sizeof(app->dialog_text),
        "Sendet %s an Zoom.\nZoom fragt danach\nnochmal nach.",
        combo_text);

    /* dialog_ex_reset löscht auch Callback und Kontext -> danach neu setzen */
    dialog_ex_reset(app->dialog);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_result_callback(app->dialog, zoom_remote_dialog_callback);
    dialog_ex_set_header(app->dialog, "Meeting verlassen?", 64, 2, AlignCenter, AlignTop);
    dialog_ex_set_text(app->dialog, app->dialog_text, 64, 16, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog, "Nein");
    dialog_ex_set_right_button_text(app->dialog, "Ja");
    app->dialog_mode = ZoomDialogConfirmLeave;
    zoom_remote_switch_view(app, ZoomViewDialog);
}

static void zoom_remote_show_hotkey_info(ZoomRemoteApp* app, const ZoomHotkey* hotkey) {
    char combo_text[24];
    zoom_combo_to_string(
        zoom_hotkey_combo(hotkey, app->settings.os),
        app->settings.os,
        combo_text,
        sizeof(combo_text));
    snprintf(
        app->dialog_text,
        sizeof(app->dialog_text),
        "%s: %s\n%s\n%s",
        zoom_os_name(app->settings.os),
        combo_text,
        zoom_scope_name(zoom_hotkey_scope(hotkey, app->settings.os)),
        hotkey->note ? hotkey->note : "");

    dialog_ex_reset(app->dialog);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_result_callback(app->dialog, zoom_remote_dialog_callback);
    dialog_ex_set_header(app->dialog, hotkey->name, 64, 2, AlignCenter, AlignTop);
    dialog_ex_set_text(app->dialog, app->dialog_text, 2, 16, AlignLeft, AlignTop);
    dialog_ex_set_center_button_text(app->dialog, "OK");
    app->dialog_mode = ZoomDialogHotkeyInfo;
    zoom_remote_switch_view(app, ZoomViewDialog);
}

/* ---- Hotkey-Untermenü ---------------------------------------------------- */

static void
    zoom_remote_menu_hotkeys_callback(void* context, InputType input_type, uint32_t index) {
    ZoomRemoteApp* app = context;
    const ZoomHotkey* hotkey = zoom_hotkey_get(index);
    if(input_type == InputTypeShort) {
        zoom_remote_send_hotkey(app, hotkey);
    } else if(input_type == InputTypeLong) {
        zoom_remote_show_hotkey_info(app, hotkey);
    }
}

/* Baut die Liste einer Kategorie für das aktuelle OS/Layout neu auf */
static void zoom_remote_fill_hotkeys(ZoomRemoteApp* app, ZoomCategory category) {
    submenu_reset(app->menu_hotkeys);
    submenu_set_header(app->menu_hotkeys, zoom_category_name(category));

    for(size_t i = 0; i < zoom_hotkeys_count(); i++) {
        const ZoomHotkey* hotkey = zoom_hotkey_get(i);
        if(hotkey->category != category) continue;
        if(zoom_hotkey_scope(hotkey, app->settings.os) == ZoomScopeNone) continue;

        /* Nur Kürzel anzeigen, die mit dem Layout auch sendbar sind */
        uint16_t hid_code = 0;
        ZoomCombo combo = zoom_hotkey_combo(hotkey, app->settings.os);
        if(!zoom_keymap_to_hid(combo, app->settings.os, app->settings.layout, &hid_code)) continue;

        submenu_add_item_ex(
            app->menu_hotkeys, hotkey->name, i, zoom_remote_menu_hotkeys_callback, app);
    }
    submenu_set_selected_item(app->menu_hotkeys, 0);
}

static void zoom_remote_menu_categories_callback(void* context, uint32_t index) {
    ZoomRemoteApp* app = context;
    app->current_category = (ZoomCategory)index;
    zoom_remote_fill_hotkeys(app, app->current_category);
    zoom_remote_switch_view(app, ZoomViewHotkeys);
}

/* ---- Teilen-Modus ------------------------------------------------------- */

static void zoom_remote_share_callback(ZoomShareEventType type, uint8_t zoom_key, void* context) {
    ZoomRemoteApp* app = context;
    if(!app->transport) return;
    uint16_t hid_code = zoom_keymap_special(zoom_key);

    switch(type) {
    case ZoomShareEventPress:
        zoom_transport_press(app->transport, hid_code);
        break;
    case ZoomShareEventRelease:
        zoom_transport_release(app->transport, hid_code);
        break;
    case ZoomShareEventTap:
        zoom_transport_tap(app->transport, hid_code);
        break;
    case ZoomShareEventExit:
        /* Escape senden, alles loslassen, zurück zum Remote-Screen */
        zoom_transport_release_all(app->transport);
        zoom_transport_tap(app->transport, hid_code);
        notification_message(app->notifications, &sequence_reset_green);
        zoom_remote_switch_view(app, ZoomViewRemote);
        break;
    }
}

/* ---- Remote-Screen ------------------------------------------------------- */

static void zoom_remote_view_callback(ZoomRemoteEvent event, void* context) {
    ZoomRemoteApp* app = context;
    switch(event) {
    case ZoomRemoteEventMicToggle:
        /* Status nur umschalten, wenn der Hotkey auch rausging */
        if(zoom_remote_send_id(app, ZoomHotkeyMuteAudio)) app->mic_on = !app->mic_on;
        break;
    case ZoomRemoteEventCamToggle:
        if(zoom_remote_send_id(app, ZoomHotkeyVideo)) app->cam_on = !app->cam_on;
        break;
    case ZoomRemoteEventResetState:
        app->mic_on = true;
        app->cam_on = true;
        notification_message(app->notifications, &sequence_single_vibro);
        break;
    case ZoomRemoteEventCamSwitch:
        zoom_remote_send_id(app, ZoomHotkeySwitchCamera);
        break;
    case ZoomRemoteEventHandToggle:
        zoom_remote_send_id(app, ZoomHotkeyRaiseHand);
        break;
    case ZoomRemoteEventParticipants:
        zoom_remote_send_id(app, ZoomHotkeyParticipants);
        break;
    case ZoomRemoteEventShareToggle:
        zoom_remote_send_id(app, ZoomHotkeyShare);
        break;
    case ZoomRemoteEventShareMode:
        /* Teilen-Dialog öffnen; nur bei Erfolg in den Teilen-Modus wechseln */
        if(zoom_remote_send_id(app, ZoomHotkeyShare)) {
            zoom_share_view_set_connected(app->share_view, app->connected);
            notification_message(app->notifications, &sequence_set_only_green_255);
            zoom_remote_switch_view(app, ZoomViewShare);
        }
        break;
    case ZoomRemoteEventHotkeys:
        submenu_set_selected_item(app->menu_categories, app->current_category);
        zoom_remote_switch_view(app, ZoomViewCategories);
        break;
    case ZoomRemoteEventChat:
        zoom_remote_send_id(app, ZoomHotkeyChatPanel);
        break;
    case ZoomRemoteEventLeave:
        zoom_remote_show_confirm_leave(app);
        break;
    }
    zoom_remote_view_set_state(app->remote_view, app->mic_on, app->cam_on);
}

/* ---- Auf- und Abbau ------------------------------------------------------ */

static ZoomRemoteApp* zoom_remote_app_alloc(void) {
    ZoomRemoteApp* app = malloc(sizeof(ZoomRemoteApp));

    zoom_settings_load(&app->settings);
    app->mic_on = true;
    app->cam_on = true;
    app->transport = NULL;
    app->connected = false;
    app->current_category = ZoomCatMeeting;
    app->dialog_mode = ZoomDialogNone;

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, zoom_remote_navigation_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, zoom_remote_tick_callback, ZOOM_TICK_MS);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    /* Menü 1: Betriebssystem */
    app->menu_os = submenu_alloc();
    submenu_set_header(app->menu_os, "Zoom auf welchem OS?");
    submenu_add_item(app->menu_os, "Windows", ZoomOsWindows, zoom_remote_menu_os_callback, app);
    submenu_add_item(app->menu_os, "macOS", ZoomOsMac, zoom_remote_menu_os_callback, app);
    submenu_set_selected_item(app->menu_os, app->settings.os);
    view_dispatcher_add_view(app->view_dispatcher, ZoomViewMenuOs, submenu_get_view(app->menu_os));

    /* Menü 2: Verbindung */
    app->menu_conn = submenu_alloc();
    submenu_set_header(app->menu_conn, "Verbindung");
    submenu_add_item(app->menu_conn, "USB", ZoomConnUsb, zoom_remote_menu_conn_callback, app);
    submenu_add_item(app->menu_conn, "Bluetooth", ZoomConnBt, zoom_remote_menu_conn_callback, app);
    submenu_add_item(
        app->menu_conn,
        "BT-Kopplung loeschen",
        ZOOM_MENU_CONN_FORGET_BT,
        zoom_remote_menu_conn_callback,
        app);
    submenu_set_selected_item(app->menu_conn, app->settings.conn);
    view_dispatcher_add_view(
        app->view_dispatcher, ZoomViewMenuConn, submenu_get_view(app->menu_conn));

    /* Menü 3: Tastaturlayout am Rechner */
    app->menu_layout = submenu_alloc();
    submenu_set_header(app->menu_layout, "Tastatur am Rechner");
    submenu_add_item(
        app->menu_layout, "QWERTZ (DE)", ZoomLayoutQwertz, zoom_remote_menu_layout_callback, app);
    submenu_add_item(
        app->menu_layout, "QWERTY (US)", ZoomLayoutQwerty, zoom_remote_menu_layout_callback, app);
    submenu_set_selected_item(app->menu_layout, app->settings.layout);
    view_dispatcher_add_view(
        app->view_dispatcher, ZoomViewMenuLayout, submenu_get_view(app->menu_layout));

    /* Remote-Screen */
    app->remote_view = zoom_remote_view_alloc();
    zoom_remote_view_set_callback(app->remote_view, zoom_remote_view_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher, ZoomViewRemote, zoom_remote_view_get_view(app->remote_view));

    /* Teilen-Modus */
    app->share_view = zoom_share_view_alloc();
    zoom_share_view_set_callback(app->share_view, zoom_remote_share_callback, app);
    view_dispatcher_add_view(
        app->view_dispatcher, ZoomViewShare, zoom_share_view_get_view(app->share_view));

    /* Hotkey-Kategorien */
    app->menu_categories = submenu_alloc();
    submenu_set_header(app->menu_categories, "Alle Hotkeys");
    for(uint32_t c = 0; c < ZoomCatCount; c++) {
        submenu_add_item(
            app->menu_categories,
            zoom_category_name((ZoomCategory)c),
            c,
            zoom_remote_menu_categories_callback,
            app);
    }
    view_dispatcher_add_view(
        app->view_dispatcher, ZoomViewCategories, submenu_get_view(app->menu_categories));

    /* Hotkeys einer Kategorie (wird bei Auswahl gefüllt) */
    app->menu_hotkeys = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, ZoomViewHotkeys, submenu_get_view(app->menu_hotkeys));

    /* Dialog für Bestätigung und Hotkey-Info */
    app->dialog = dialog_ex_alloc();
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_result_callback(app->dialog, zoom_remote_dialog_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, ZoomViewDialog, dialog_ex_get_view(app->dialog));

    return app;
}

static void zoom_remote_app_free(ZoomRemoteApp* app) {
    furi_assert(app);

    /* Falls die App aus dem Remote-Screen heraus beendet wurde */
    zoom_remote_transport_stop(app);
    notification_message(app->notifications, &sequence_reset_rgb);

    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewMenuOs);
    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewMenuConn);
    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewMenuLayout);
    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewRemote);
    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewShare);
    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewCategories);
    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewHotkeys);
    view_dispatcher_remove_view(app->view_dispatcher, ZoomViewDialog);

    submenu_free(app->menu_os);
    submenu_free(app->menu_conn);
    submenu_free(app->menu_layout);
    submenu_free(app->menu_categories);
    submenu_free(app->menu_hotkeys);
    dialog_ex_free(app->dialog);
    zoom_remote_view_free(app->remote_view);
    zoom_share_view_free(app->share_view);

    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    free(app);
}

/* ---- Einstiegspunkt ------------------------------------------------------ */

int32_t zoom_remote_app(void* p) {
    UNUSED(p);
    ZoomRemoteApp* app = zoom_remote_app_alloc();

    dolphin_deed(DolphinDeedPluginStart);
    zoom_remote_switch_view(app, ZoomViewMenuOs);
    view_dispatcher_run(app->view_dispatcher);

    zoom_remote_app_free(app);
    return 0;
}
