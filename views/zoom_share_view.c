#include "zoom_share_view.h"
#include "../zoom_hotkeys.h"

#include <furi.h>
#include <gui/elements.h>
#include <assets_icons.h>

struct ZoomShareView {
    View* view;
    ZoomShareViewCallback callback;
    void* context;
};

typedef struct {
    bool connected;
    /* Gerade gehaltene Richtungen, für die Anzeige im Steuerkreuz */
    bool up;
    bool down;
    bool left;
    bool right;
    bool ok;
} ZoomShareViewModel;

static void zoom_share_view_draw(Canvas* canvas, void* model_raw) {
    ZoomShareViewModel* model = model_raw;
    canvas_clear(canvas);

    /* Deutlicher Modus-Hinweis: invertierter Balken über die volle Breite */
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 128, 13);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 64, 2, AlignCenter, AlignTop, zoom_tr("SHARE MODE", "TEILEN-MODUS"));
    canvas_set_color(canvas, ColorBlack);

    /* Steuerkreuz links; gedrückte Richtung wird als gefüllter Kreis markiert */
    const uint8_t cx = 24;
    const uint8_t cy = 38;
    canvas_draw_circle(canvas, cx, cy, 20);
    if(model->up) canvas_draw_disc(canvas, cx, cy - 13, 6);
    if(model->down) canvas_draw_disc(canvas, cx, cy + 13, 6);
    if(model->left) canvas_draw_disc(canvas, cx - 13, cy, 6);
    if(model->right) canvas_draw_disc(canvas, cx + 13, cy, 6);
    if(model->ok) canvas_draw_disc(canvas, cx, cy, 5);

    canvas_set_color(canvas, model->up ? ColorWhite : ColorBlack);
    canvas_draw_icon(canvas, cx - 3, cy - 15, &I_ButtonUp_7x4);
    canvas_set_color(canvas, model->down ? ColorWhite : ColorBlack);
    canvas_draw_icon(canvas, cx - 3, cy + 12, &I_ButtonDown_7x4);
    canvas_set_color(canvas, model->left ? ColorWhite : ColorBlack);
    canvas_draw_icon(canvas, cx - 15, cy - 3, &I_ButtonLeft_4x7);
    canvas_set_color(canvas, model->right ? ColorWhite : ColorBlack);
    canvas_draw_icon(canvas, cx + 12, cy - 3, &I_ButtonRight_4x7);
    canvas_set_color(canvas, model->ok ? ColorWhite : ColorBlack);
    canvas_draw_icon(canvas, cx - 3, cy - 3, &I_ButtonCenter_7x7);
    canvas_set_color(canvas, ColorBlack);

    /* Legende rechts */
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 50, 24, zoom_tr("Pad: arrow keys", "Kreuz: Pfeiltasten"));
    canvas_draw_str(canvas, 50, 34, "OK: Enter");
    canvas_draw_str(canvas, 50, 44, zoom_tr("Hold OK: Tab", "OK lang: Tab"));
    canvas_draw_str(canvas, 50, 54, zoom_tr("Back: Esc+exit", "Zurueck: Esc+Ende"));
    if(!model->connected) {
        canvas_draw_str(canvas, 50, 63, zoom_tr("Not connected!", "Keine Verbindung!"));
    }
}

/* Richtungstaste -> Zoom-Key; 0 wenn keine Richtungstaste */
static uint8_t zoom_share_view_arrow(InputKey key) {
    switch(key) {
    case InputKeyUp:
        return ZoomKeyUp;
    case InputKeyDown:
        return ZoomKeyDown;
    case InputKeyLeft:
        return ZoomKeyLeft;
    case InputKeyRight:
        return ZoomKeyRight;
    default:
        return 0;
    }
}

static void zoom_share_view_mark(ZoomShareView* view, InputKey key, bool pressed) {
    with_view_model(
        view->view,
        ZoomShareViewModel * model,
        {
            if(key == InputKeyUp) model->up = pressed;
            if(key == InputKeyDown) model->down = pressed;
            if(key == InputKeyLeft) model->left = pressed;
            if(key == InputKeyRight) model->right = pressed;
            if(key == InputKeyOk) model->ok = pressed;
        },
        true);
}

static bool zoom_share_view_input(InputEvent* event, void* context) {
    ZoomShareView* view = context;
    furi_assert(view);
    if(!view->callback) return true;

    /* Anzeige der gedrückten Taste */
    if(event->type == InputTypePress) zoom_share_view_mark(view, event->key, true);
    if(event->type == InputTypeRelease) zoom_share_view_mark(view, event->key, false);

    uint8_t arrow = zoom_share_view_arrow(event->key);
    if(arrow) {
        /* Pfeiltasten werden gehalten, damit der Rechner selbst wiederholt */
        if(event->type == InputTypePress) {
            view->callback(ZoomShareEventPress, arrow, view->context);
        } else if(event->type == InputTypeRelease) {
            view->callback(ZoomShareEventRelease, arrow, view->context);
        }
    } else if(event->key == InputKeyOk) {
        if(event->type == InputTypeShort) {
            view->callback(ZoomShareEventTap, ZoomKeyEnter, view->context);
        } else if(event->type == InputTypeLong) {
            view->callback(ZoomShareEventTap, ZoomKeyTab, view->context);
        }
    } else if(event->key == InputKeyBack) {
        if(event->type == InputTypeShort || event->type == InputTypeLong) {
            view->callback(ZoomShareEventExit, ZoomKeyEscape, view->context);
        }
    }
    /* Alles verbrauchen, damit "Zurück" hier nie die normale Navigation auslöst */
    return true;
}

/* Beim Betreten der View alle Markierungen zurücksetzen */
static void zoom_share_view_enter(void* context) {
    ZoomShareView* view = context;
    with_view_model(
        view->view,
        ZoomShareViewModel * model,
        {
            model->up = false;
            model->down = false;
            model->left = false;
            model->right = false;
            model->ok = false;
        },
        true);
}

ZoomShareView* zoom_share_view_alloc(void) {
    ZoomShareView* view = malloc(sizeof(ZoomShareView));
    view->view = view_alloc();
    view_set_context(view->view, view);
    view_allocate_model(view->view, ViewModelTypeLocking, sizeof(ZoomShareViewModel));
    view_set_draw_callback(view->view, zoom_share_view_draw);
    view_set_input_callback(view->view, zoom_share_view_input);
    view_set_enter_callback(view->view, zoom_share_view_enter);
    return view;
}

void zoom_share_view_free(ZoomShareView* view) {
    furi_assert(view);
    view_free(view->view);
    free(view);
}

View* zoom_share_view_get_view(ZoomShareView* view) {
    furi_assert(view);
    return view->view;
}

void zoom_share_view_set_callback(
    ZoomShareView* view,
    ZoomShareViewCallback callback,
    void* context) {
    furi_assert(view);
    view->callback = callback;
    view->context = context;
}

void zoom_share_view_set_connected(ZoomShareView* view, bool connected) {
    furi_assert(view);
    with_view_model(
        view->view, ZoomShareViewModel * model, { model->connected = connected; }, true);
}
