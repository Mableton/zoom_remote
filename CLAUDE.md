# zoom_remote – Flipper Zero App (Zoom-Fernbedienung)

Native Flipper-Zero-App (.fap, C, ufbt) als Fernbedienung für Zoom Workplace
auf Windows und macOS, per USB-HID oder Bluetooth-HID.

## Feste Regeln

### Umgebung und Build
- Ziel: Flipper Zero mit Momentum Firmware **mntm-012**.
- ufbt baut gegen das Momentum-SDK:
  `ufbt update --index-url=https://up.momentum-fw.dev/firmware/directory.json`
- Nach jedem Arbeitsschritt mit `ufbt` bauen, Fehler selbst beheben.
- Deployen per `ufbt launch`; qFlipper ist dabei geschlossen.
- App-ID: `zoom_remote`. 10x10-Icon. `fap_category` sinnvoll (aktuell "Tools").

### Funktion
- Startmenü: OS (Windows / macOS) -> Verbindung (USB / Bluetooth) -> Layout
  (QWERTZ / QWERTY, nur bei Windows). Letzte Auswahl wird unter `/ext/apps_data/zoom_remote/`
  gespeichert.
- Remote-Screen: wichtigste Aktionen direkt auf den Tasten (Mikro, Kamera,
  Hand heben, Teilen). Mikro- und Kamerastatus werden nur lokal geschätzt und
  müssen als "geschätzt" gekennzeichnet sein.
- Untermenü mit allen weiteren Zoom-Hotkeys, nach Kategorien sortiert.
- Teilen-Modus: öffnet den Teilen-Dialog; danach Steuerkreuz = Pfeiltasten,
  OK = Enter, OK lang = Tab, Zurück = Escape und Modus verlassen. Status im
  Display deutlich anzeigen.
- Meeting verlassen/beenden nur mit Bestätigung.

### Hotkeys
- Quelle ist ausschließlich die offizielle Zoom-Supportseite
  "Using hot keys and keyboard shortcuts" (KB0067050), getrennt für Windows
  und macOS. Nichts aus dem Gedächtnis. Quelle und Stand im Code nennen.
- Markieren, welche Kürzel nur bei fokussiertem Zoom-Fenster wirken und welche
  in Zoom als globale Shortcuts aktivierbar sind.
- Hotkeys als Datentabelle in `zoom_hotkeys.c`, nicht in der Logik verstreut.

### Technik
- Vorlage für USB-HID und Bluetooth-HID ist die offizielle HID-App aus dem
  Momentum-Repo (`applications/system/hid_app/`, Tag mntm-012).
- Beim Beenden USB-Modus und Bluetooth-Profil sauber auf den Ursprungszustand
  zurücksetzen, genau wie die HID-App (USB-Konfig merken/wiederherstellen,
  BT-Keys-Pfad zurücksetzen, `bt_profile_restore_default`).
- Deutsche Tastaturen (QWERTZ): HID-Keycodes sind positionsbasiert.
  Unter **Windows** berücksichtigen Buchstaben-Hotkeys Y/Z-Tausch und
  Sonderzeichen (siehe `zoom_keymap.c`). Unter **macOS** wertet Zoom selbst
  Tastenpositionen (US-Belegung) aus: dort keine Umrechnung, das Layout-Menü
  wird übersprungen (geprüft 2026-09-17, Zoom 7.0.6, deutscher Mac).
- ViewDispatcher, Submenu, eigene Views für Remote-Screen und Teilen-Modus.
- Code-Kommentare auf Deutsch.

### Vorgehen
1. Plan mit Dateistruktur und Tastenbelegung zeigen, auf OK warten.
2. Schrittweise: Gerüst mit Menü -> USB HID Windows -> macOS -> Teilen-Modus
   -> Bluetooth.
3. Nach jedem Schritt bauen, Fehler beheben, dann sagen, was am Gerät zu
   testen ist.

## Tastenbelegung Remote-Screen

| Taste  | kurz                      | lang                          |
|--------|---------------------------|-------------------------------|
| OK     | Mikro an/aus              | geschätzten Status zurücksetzen |
| Hoch   | Kamera an/aus             | Kamera wechseln               |
| Runter | Hand heben/senken         | Chat-Panel                    |
| Links  | Teilen starten/stoppen    | Teilen-Modus öffnen           |
| Rechts | Teilnehmerliste           | Alle Hotkeys (Kategorien)     |
| Zurück | zurück zum Startmenü      | Meeting verlassen (Bestätigung) |

## Dateistruktur

```
application.fam          Manifest
zoom_remote.png          10x10 App-Icon
zoom_remote.h/.c         App-Struktur, ViewDispatcher, Navigation, Einstieg
zoom_hotkeys.h/.c        Hotkey-Datentabelle (Win/Mac, Kategorie, Fokus/Global)
zoom_keymap.h/.c         Kombi -> HID-Keycode, Layout QWERTZ/QWERTY
zoom_transport.h/.c      USB-/BLE-HID hinter einer API, Init/Teardown wie HID-App
zoom_settings.h/.c       Laden/Speichern der letzten Auswahl
views/zoom_remote_view.* Remote-Screen
views/zoom_share_view.*  Teilen-Modus
images/                  Icons für die Views
```
