# Firmware CRSF-Multiswitch v4.01

Arduino-Sketch in `CRSF_Multiswitch/`, gleicher Quellcode fuer C3 und S3. Basis ist MultiSwitch_ESP32_V3 v3.12. Spezifikation: `docs/Firmware-Spezifikation.md`.

## Bauen mit der Arduino IDE
- Board-Paket: esp32 von Espressif, Version 3.x (getestet im CI mit 3.0.7)
- Bibliothek: Bolder Flight Systems SBUS 8.1.4
- Board: **ESP32C3 Dev Module** (C3 SuperMini) oder **ESP32S3 Dev Module** (S3-Zero). Das Board wird in `board_config.h` automatisch erkannt.
- **USB CDC On Boot: Enabled** (sonst bricht der Build mit einer Meldung ab, UART0 wird fuer CRSF/SBUS gebraucht)
- Partition Scheme: eines mit OTA, z.B. "Default 4MB with spiffs"
- Fertige .bin-Dateien fuer C3 und S3 erzeugt GitHub Actions bei jedem Push (Artefakte im Workflow "firmware").

## Funktionen
- Alle Funktionen der V3 (Konfig 1/2/3 je Ausgang, MKan1/MKan2, Web-UI, Lua, Failsafe, NVS, OTA, JSON-Export/-Import).
- UART A (CN1): CRSF (RC-System 4) oder SBUS (RC-System 0-3) vom Empfaenger.
- C3: Ausgang 7/8 nur digital (an ab 50 %), Ausgang 1-6 PWM. S3: alle 8 PWM.
- **Modulkette (nur CRSF):** UART B (C3 und S3) und UART C (nur S3) gehen zum naechsten Modul. Jedes Modul braucht eine eigene Modul-Adresse (0-15). Alle Module erscheinen im Lua-Menue als `<Name>@<Adresse>` und lassen sich einzeln einstellen.
- **BOOT-Taster:** 3 s gedrueckt halten schaltet den WLAN-AP ein oder aus (bis zum Neustart, die Einstellung "Dauerhaft an" bleibt unveraendert).
- **Status-LED (nur S3, gruen):** aus = kein Signal, 500 ms blinken = OK, schnell 100 ms = Failsafe bzw. kein RC-Link, Doppelblitz = doppelte Modul-Adresse in der Kette.
- Debug-Tab der Web-UI: CRSF-Zaehler plus Router-Zaehler (weitergeleitet, verworfen) und Warnung bei doppelter Adresse.

## Router-Regeln
- A -> B/C: alle gueltigen Frames, ausser Extended-Frames an die eigene Adresse.
- B/C -> A: alle gueltigen Frames.
- B <-> C: nie.
- Frames gehen ganz und unveraendert weiter (CRC geprueft). Je Port eine Warteschlange mit 8 Frames; bei Ueberlauf werden zuerst alte RC-Kanal-Frames verworfen.

## Stand
- v4.01 kompiliert fuer C3 und S3 (CI). Noch nicht auf Hardware getestet.
- Offen: Abnahmetest nach Kapitel 8 der Spezifikation (1, 2 und 3 Module), C3-Strapping-Pin IO2 (Kanal 3) beim ersten Boot pruefen.
