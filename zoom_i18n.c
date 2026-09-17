#include "zoom_i18n.h"

/* Aktuelle Sprache; die App läuft nur einmal, daher genügt eine Modulvariable */
static ZoomLang zoom_lang_current = ZoomLangEn;

void zoom_lang_set(ZoomLang lang) {
    if(lang < ZoomLangCount) zoom_lang_current = lang;
}

ZoomLang zoom_lang_get(void) {
    return zoom_lang_current;
}

const char* zoom_tr(const char* en, const char* de) {
    return (zoom_lang_current == ZoomLangDe) ? de : en;
}
