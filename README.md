# Zoom Remote für Flipper Zero

Native Flipper-Zero-App (`.fap`, C, ufbt): Fernbedienung für Zoom Workplace auf
macOS und Windows, per USB-HID oder Bluetooth-HID.

- Ziel-Firmware: Momentum `mntm-012`
- Getestet: macOS mit Zoom Workplace 7.0.6, USB und Bluetooth
- Ungetestet: Windows (Kürzel stammen von der Zoom-Supportseite)

## Funktionen

- Startmenü: Betriebssystem, Verbindung (USB/Bluetooth), unter Windows zusätzlich
  Tastaturlayout (QWERTZ/QWERTY). Die letzte Auswahl wird unter
  `/ext/apps_data/zoom_remote/` gespeichert.
- Remote-Screen mit Mikro- und Kamerastatus (lokal geschätzt).
- Teilen-Modus: Steuerkreuz = Pfeiltasten, OK = Enter, OK lang = Tab,
  Zurück = Escape und Modus verlassen.
- Untermenü mit allen Zoom-Hotkeys nach Kategorien. OK sendet, OK lang zeigt
  Kombination, Wirkungsbereich und Hinweise.
- Meeting verlassen nur mit Bestätigung.
- Beim Beenden werden USB-Modus und Bluetooth-Profil zurückgesetzt.

## Tastenbelegung Remote-Screen

| Taste  | kurz                   | lang                            |
|--------|------------------------|---------------------------------|
| OK     | Mikro an/aus           | geschätzten Status zurücksetzen |
| Hoch   | Kamera an/aus          | Kamera wechseln                 |
| Runter | Hand heben/senken      | Chat-Panel                      |
| Links  | Teilen starten/stoppen | Teilen-Modus                    |
| Rechts | Teilnehmerliste        | Alle Hotkeys                    |
| Zurück | zum Startmenü          | Meeting verlassen (Bestätigung) |

## Bauen und installieren

```bash
ufbt update --index-url=https://up.momentum-fw.dev/firmware/directory.json
ufbt
ufbt launch
```

`ufbt launch` funktioniert nur, solange die App nicht im Remote-Screen ist
(dort meldet sich der Flipper als Tastatur statt als serieller Port).

## Hinweise

- Hotkey-Quelle: Zoom-Supportartikel „Using hot keys and keyboard shortcuts“
  (KB0067050), Stand 2026-09-15. Die Tabelle steht in `zoom_hotkeys.c`.
- Zoom wertet Kürzel unter macOS nach Tastenposition aus. Deshalb sendet die App
  am Mac US-Positionen ohne Layout-Umrechnung. Unter Windows wird für QWERTZ
  umgerechnet (Y/Z-Tausch, `+`, `-`).
- USB- und Bluetooth-Code folgen der HID-App der Momentum-Firmware
  (`applications/system/hid_app`).
