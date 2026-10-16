# Firmware CRSF-Multiswitch v4.00

Arduino-Sketch in `CRSF_Multiswitch/`, gleicher Quellcode fuer C3 und S3. Basis ist MultiSwitch_ESP32_V3 v3.12. Plan und offene Punkte: `docs/Firmware-Spezifikation.md`.

## Bauen mit der Arduino IDE
- Board-Paket: esp32 von Espressif, Version 3.x (getestet im CI mit 3.0.7)
- Bibliothek: Bolder Flight Systems SBUS 8.1.4
- Board: **ESP32C3 Dev Module** (C3 SuperMini) oder **ESP32S3 Dev Module** (S3-Zero). Das Board wird in `board_config.h` automatisch erkannt.
- **USB CDC On Boot: Enabled** (sonst bricht der Build mit einer Meldung ab, UART0 wird fuer CRSF/SBUS gebraucht)
- Partition Scheme: eines mit OTA, z.B. "Default 4MB with spiffs"

## Stand
- Schritt 1 (dieser Stand): Pins und UART A fuer C3/S3, CRSF oder SBUS auf UART A, C3-Ausgang 7/8 digital.
- LED-Muster (Spezifikation Kapitel 6, nur S3): aus = kein Signal, 500 ms blinken = OK, schnell 100 ms = Failsafe/kein RC-Link, Doppelblitz = Konfigurationsfehler (ab Schritt 3).
- Schritt 2: BOOT-Taster fuer den WLAN-AP.
- Schritt 3: CRSF-Router fuer Modulketten (UART B/C).
- Schritt 4: Abnahmetest.
