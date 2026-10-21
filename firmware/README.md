# Firmware RC-Multiswitch v4.04

Arduino-Sketch in `RC_Multiswitch/`, gleicher Quellcode fuer C3 und S3. Basis ist MultiSwitch_ESP32_V3 v3.12. Spezifikation: `docs/Firmware-Spezifikation.md`.

## Bauen mit der Arduino IDE
- Board-Paket: esp32 von Espressif, Version 3.x (getestet im CI mit 3.0.7)
- Bibliotheken: Bolder Flight Systems SBUS 8.1.4, TinyGPSPlus 1.0.3 (ab v4.04)
- Board: **ESP32C3 Dev Module** (C3 SuperMini) oder **ESP32S3 Dev Module** (S3-Zero). Das Board wird in `board_config.h` automatisch erkannt.
- **USB CDC On Boot: Enabled** (sonst bricht der Build mit einer Meldung ab, UART0 wird fuer CRSF/SBUS gebraucht)
- Partition Scheme: eines mit OTA, z.B. "Default 4MB with spiffs"
- Fertige .bin-Dateien fuer C3 und S3 erzeugt GitHub Actions bei jedem Push (Artefakte im Workflow "firmware").

## Funktionen
- Alle Funktionen der V3 (Konfig 1/2/3 je Ausgang, MKan1/MKan2, Web-UI, Lua, Failsafe, NVS, OTA, JSON-Export/-Import).
- UART A (CN1): CRSF (RC-System 4) oder SBUS (RC-System 0-3) vom Empfaenger.
- C3: Ausgang 7/8 nur digital (an ab 50 %), Ausgang 1-6 PWM. S3: alle 8 PWM.
- **UART B/C mit waehlbarer Rolle (ab v4.04):** UART B (C3 und S3) und UART C (nur S3) koennen im Lua-Menue (Ordner "UART B/C") oder in der Web-UI (RC-Tab) auf Aus, CRSF, S.Port oder GPS gestellt werden, wie beim Soundmodul. Wirkt nach einem Neustart. Werkseinstellung: CRSF auf beiden.
  - CRSF: Modulkette zum naechsten Modul (siehe unten), nur bei RC-System CRSF.
  - S.Port: bis zu 2 FrSky FLVSS/MLVSS LiPo-Sensoren (Poll-ID 0xA1 und 0x22), Anschluss TX ueber Diode 1N4148 und RX zusammen an die Signalleitung (siehe `sport_lipo.h`).
  - GPS: NMEA-Empfaenger (z. B. ATGM336H), Baudrate 9600-115200 waehlbar.
  - Bei CRSF gehen Zellspannungen und GPS alle 100 ms als Telemetrie an den Sender. Bei SBUS werden die Werte nur im Debug-Tab angezeigt.
  - S.Port und GPS gibt es nur einmal: steht UART C auf derselben Rolle wie UART B, bleibt UART C aus.
- **Modulkette (nur CRSF):** UART B (C3 und S3) und UART C (nur S3) mit Rolle CRSF gehen zum naechsten Modul. Jedes Modul braucht eine eigene Modul-Adresse (0-15). Alle Module erscheinen im Lua-Menue als `<Name>@<Adresse>` und lassen sich einzeln einstellen.
- **BOOT-Taster:** 3 s gedrueckt halten schaltet den WLAN-AP ein oder aus (bis zum Neustart, die Einstellung "Dauerhaft an" bleibt unveraendert).
- **Status-LED (nur S3, gruen):** aus = kein Signal, 500 ms blinken = OK, schnell 100 ms = Failsafe bzw. kein RC-Link, Doppelblitz = doppelte Modul-Adresse in der Kette.
- Debug-Tab der Web-UI: CRSF-Zaehler plus Router-Zaehler (weitergeleitet, verworfen) und Warnung bei doppelter Adresse.

## Router-Regeln
- A -> B/C: alle gueltigen Frames, ausser Extended-Frames an die eigene Adresse.
- B/C -> A: alle gueltigen Frames.
- B <-> C: nie.
- Frames gehen ganz und unveraendert weiter (CRC geprueft). Je Port eine Warteschlange mit 8 Frames; bei Ueberlauf werden zuerst alte RC-Kanal-Frames verworfen.

## Stand
- v4.04 kompiliert fuer C3 und S3 (CI). v4.02: CRSF-Link-Statistik (LQ) korrigiert. v4.03: SBUS-Zeitlimit 10 s (ohne Frames gehen die Ausgaenge aus). v4.04: Rolle von UART B/C waehlbar (CRSF, S.Port, GPS). Noch nicht auf Hardware getestet.
- Offen: Abnahmetest nach Kapitel 8 der Spezifikation (1, 2 und 3 Module).

## Bekanntes Risiko C3: IO2 (Ausgang 3) ist ein Strapping-Pin
IO2 muss beim Einschalten HIGH sein, damit der ESP32-C3 normal startet. Auf der Platine haengen an IO2 (Ausgang 3) die Kontroll-LED LED3 mit R3 nach GND und der Eingang des ULN2803A. Beides kann IO2 beim Reset nach LOW ziehen, der C3 startet dann evtl. nicht oder nur unzuverlaessig. Die Platine bleibt bewusst so (bereits bestellt, Stand 21.10.2026).
- Beim ersten Aufbau pruefen: mehrmals aus- und einschalten, mit und ohne Last an Ausgang 3.
- Startet der C3 nicht sicher: R3 hochohmiger waehlen (z. B. 10 kOhm) oder LED3/R3 weglassen; als letzte Moeglichkeit einen 10-kOhm-Pull-up von IO2 nach 3V3 nachruesten.
- Der S3 ist nicht betroffen.
