#pragma once

/*
 * Zweisprachige Oberfläche (Englisch als Standard, Deutsch umschaltbar).
 * Texte stehen direkt an der Verwendungsstelle: zoom_tr("English", "Deutsch").
 */
typedef enum {
    ZoomLangEn,
    ZoomLangDe,
    ZoomLangCount,
} ZoomLang;

void zoom_lang_set(ZoomLang lang);
ZoomLang zoom_lang_get(void);

/* Liefert den Text in der aktuell gewählten Sprache */
const char* zoom_tr(const char* en, const char* de);
