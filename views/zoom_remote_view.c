#include "zoom_remote_view.h"

#include <furi.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include "zoom_remote_icons.h"

struct ZoomRemoteView {
    View* view;
    ZoomRemoteViewCallback callback;
    void* context;
};

/* Modell der View: alles, was zum Zeichnen gebraucht wird */
typedef struct {
    ZoomSettings settings;
    bool connected;
    bool mic_on;
    bool cam_on;
} ZoomRemoteViewModel;

/* Eine Statuskachel: Rahmen (AN) oder gefüllt (AUS), mit Icon, Name und Status */
static void zoom_remote_view_draw_tile(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    const Icon* icon,
    uint8_t icon_w,
    uint8_t icon_h,
    const char* name,
    bool on) {
    const uint8_t w = 60;
    const uint8_t h = 21;

    canvas_set_color(canvas, ColorBlack);
    if(on) {
        elements_slightly_rounded_frame(canvas, x, y, w, h);
    } else {
        elements_slightly_rounded_box(canvas, x, y, w, h);
        canvas_set_color(canvas, ColorWhite);
    }

    /* Icon vertikal mittig links */
    canvas_draw_icon(canvas, x + 5, y + (h - icon_h) / 2, icon);
    UNUSED(icon_w);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, x + 26, y + 10, name);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, x + 26, y + 19, on ? "AN" : "AUS");
    canvas_set_color(canvas, ColorBlack);
}

static void zoom_remote_view_draw(Canvas* canvas, void* model_raw) {
    ZoomRemoteViewModel* model = model_raw;
    canvas_clear(canvas);

    /* Kopfzeile: invertierter Balken mit App-Name, Auswahl und Verbindungs-Icon */
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 12);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Zoom");
    canvas_set_font(canvas, FontSecondary);
    const char* parts[] = {
        zoom_os_name(model->settings.os),
        zoom_conn_name(model->settings.conn),
        /* Layout ist nur unter Windows relevant */
        (model->settings.os == ZoomOsWindows) ? zoom_layout_name(model->settings.layout) : "",
    };
    uint8_t x = 2 + canvas_string_width(canvas, "Zoom") + 7;
    for(size_t i = 0; i < COUNT_OF(parts); i++) {
        canvas_draw_str(canvas, x, 9, parts[i]);
        x += canvas_string_width(canvas, parts[i]) + 5;
    }
    /* Verbindungs-Icon rechts; ohne Verbindung durchgestrichen */
    if(model->settings.conn == ZoomConnUsb) {
        canvas_draw_icon(canvas, 118, 1, &I_usb_7x11);
    } else {
        canvas_draw_icon(canvas, 119, 2, &I_bt_5x9);
    }
    if(!model->connected) {
        canvas_draw_line(canvas, 115, 11, 126, 0);
        canvas_draw_line(canvas, 116, 11, 127, 0);
    }
    canvas_set_color(canvas, ColorBlack);

    /* Statuskacheln Mikro und Kamera (Status nur geschätzt) */
    zoom_remote_view_draw_tile(canvas, 2, 14, &I_mic_9x13, 9, 13, "Mic", model->mic_on);
    zoom_remote_view_draw_tile(canvas, 66, 14, &I_cam_15x10, 15, 10, "Cam", model->cam_on);

    /* Hinweiszeile 1: Hoch/Runter kurz und Schätz-Hinweis */
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_icon(canvas, 3, 38, &I_ButtonUp_7x4);
    canvas_draw_str(canvas, 12, 43, "Cam");
    canvas_draw_icon(canvas, 36, 38, &I_ButtonDown_7x4);
    canvas_draw_str(canvas, 45, 43, "Hand");
    canvas_draw_str_aligned(canvas, 126, 43, AlignRight, AlignBottom, "(geschaetzt)");

    /* Hinweiszeile 2: Belegung bei langem Druck */
    uint8_t lx = 2;
    canvas_draw_str(canvas, lx, 51, "lang:");
    lx += canvas_string_width(canvas, "lang:") + 4;
    canvas_draw_icon(canvas, lx, 44, &I_ButtonLeft_4x7);
    canvas_draw_str(canvas, lx + 6, 51, "Modus");
    lx += 6 + canvas_string_width(canvas, "Modus") + 4;
    canvas_draw_icon(canvas, lx, 44, &I_ButtonRight_4x7);
    canvas_draw_str(canvas, lx + 6, 51, "Menu");
    lx += 6 + canvas_string_width(canvas, "Menu") + 4;
    canvas_draw_icon(canvas, lx, 46, &I_ButtonDown_7x4);
    canvas_draw_str(canvas, lx + 9, 51, "Chat");

    /* Tastenleiste unten im Flipper-Stil */
    elements_button_left(canvas, "Teilen");
    elements_button_center(canvas, "Mic");
    elements_button_right(canvas, "Teiln.");
}

static bool zoom_remote_view_input(InputEvent* event, void* context) {
    ZoomRemoteView* view = context;
    furi_assert(view);

    /* Zurück kurz wird nicht verbraucht -> Navigation im ViewDispatcher */
    if(event->key == InputKeyBack && event->type == InputTypeShort) return false;

    if(!view->callback) return true;

    bool is_short = (event->type == InputTypeShort);
    bool is_long = (event->type == InputTypeLong);
    if(!is_short && !is_long) return true;

    switch(event->key) {
    case InputKeyOk:
        view->callback(
            is_short ? ZoomRemoteEventMicToggle : ZoomRemoteEventResetState, view->context);
        break;
    case InputKeyUp:
        view->callback(
            is_short ? ZoomRemoteEventCamToggle : ZoomRemoteEventCamSwitch, view->context);
        break;
    case InputKeyDown:
        view->callback(is_short ? ZoomRemoteEventHandToggle : ZoomRemoteEventChat, view->context);
        break;
    case InputKeyLeft:
        view->callback(
            is_short ? ZoomRemoteEventShareToggle : ZoomRemoteEventShareMode, view->context);
        break;
    case InputKeyRight:
        view->callback(
            is_short ? ZoomRemoteEventParticipants : ZoomRemoteEventHotkeys, view->context);
        break;
    case InputKeyBack:
        if(is_long) view->callback(ZoomRemoteEventLeave, view->context);
        break;
    default:
        break;
    }
    return true;
}

ZoomRemoteView* zoom_remote_view_alloc(void) {
    ZoomRemoteView* view = malloc(sizeof(ZoomRemoteView));
    view->view = view_alloc();
    view_set_context(view->view, view);
    view_allocate_model(view->view, ViewModelTypeLocking, sizeof(ZoomRemoteViewModel));
    view_set_draw_callback(view->view, zoom_remote_view_draw);
    view_set_input_callback(view->view, zoom_remote_view_input);

    with_view_model(
        view->view,
        ZoomRemoteViewModel * model,
        {
            model->connected = false;
            model->mic_on = true;
            model->cam_on = true;
        },
        true);
    return view;
}

void zoom_remote_view_free(ZoomRemoteView* view) {
    furi_assert(view);
    view_free(view->view);
    free(view);
}

View* zoom_remote_view_get_view(ZoomRemoteView* view) {
    furi_assert(view);
    return view->view;
}

void zoom_remote_view_set_callback(
    ZoomRemoteView* view,
    ZoomRemoteViewCallback callback,
    void* context) {
    furi_assert(view);
    view->callback = callback;
    view->context = context;
}

void zoom_remote_view_set_config(ZoomRemoteView* view, const ZoomSettings* settings) {
    furi_assert(view);
    with_view_model(
        view->view, ZoomRemoteViewModel * model, { model->settings = *settings; }, true);
}

void zoom_remote_view_set_connected(ZoomRemoteView* view, bool connected) {
    furi_assert(view);
    with_view_model(
        view->view, ZoomRemoteViewModel * model, { model->connected = connected; }, true);
}

void zoom_remote_view_set_state(ZoomRemoteView* view, bool mic_on, bool cam_on) {
    furi_assert(view);
    with_view_model(
        view->view,
        ZoomRemoteViewModel * model,
        {
            model->mic_on = mic_on;
            model->cam_on = cam_on;
        },
        true);
}
