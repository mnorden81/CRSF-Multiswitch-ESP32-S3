# RC-Multiswitch-ESP32-S3

RC-Multiswitch mit 8 Schaltkanaelen auf einem Waveshare ESP32-S3-Zero.

Frueher `CRSF-Multiswitch-ESP32-S3`. Der neue Name ist neutral, weil der Multiswitch neben CRSF auch SBUS (UART A) als Eingang versteht.

Gleiche Funktion wie die C3-Variante, aber mit dem ESP32-S3-Zero und einem dritten UART. Das PCB wurde vom C3-Layout uebernommen, die Leitungen am ESP32 sind neu verlegt (DRC steht noch aus).

Dokumente:
- [docs/Projektbeschreibung.docx](docs/Projektbeschreibung.docx): Projektbeschreibung
- [docs/Hardware-Beschreibung_C3_S3.docx](docs/Hardware-Beschreibung_C3_S3.docx): Hardware-Beschreibung C3 und S3 (Stecker, Kanalzuordnung, Stueckliste, Dimensionierung)
- [docs/Firmware-Spezifikation.md](docs/Firmware-Spezifikation.md) (auch als DOCX): Firmware-Spezifikation (Basis V3, Router fuer Modulketten, BOOT-Taster, Status-LED)

## Merkmale
- 8 Schaltkanaele ueber ULN2803A, Kontroll-LED je Kanal
- UART A (Eingang), UART B und UART C mit waehlbarer Rolle: Modulkette (CRSF), S.Port-LiPo-Sensoren, GPS oder Raw, je mit eigener Baudrate
- Versorgung des ESP32 aus +5 V des Empfaengers, Last getrennt mit 5-12 V
- Eigener THT-Footprint fuer den S3-Zero (nur 2x9 Header)

## Hardware (v0.4 (PCB vom C3 uebertragen))
Board: Waveshare ESP32-S3-Zero

- Lastspannung 5-12 V (VLOAD) an CN2 (JST PH 2), TVS-Diode P6KE18A als Schutz.
- Treiber ULN2803A, 8 Kanaele, max. 500 mA je Kanal (Verlustleistung des ULN2803A bei mehreren Kanaelen beachten).
- R9-R16 (THT 1/4 W) zwischen ULN-Ausgang und Stiftleiste J1: kein Wert vorgegeben, der Anwender legt sie passend zu Verbraucher und Lastspannung aus.
- R1-R8 und LED1-LED8 (3 mm) als Kontroll-LED je Kanal, vom Steuersignal Sn gegen GND. Werte legt der Anwender fest, eine Empfehlung ist auf der Platine aufgedruckt.
- Keine Pulldowns und keine Widerstaende in den UART-Leitungen. Der ESP32 wird aus +5 V von CN1 (BEC/Empfaenger) versorgt.

### Belegung
| Funktion | GPIO |
|---|---|
| S0..S7 | IO11, IO4, IO5, IO6, IO10, IO9, IO8, IO7 |
| UART A (CN1, Eingang) | TX IO43, RX IO44 (UART0) |
| UART B (J5) | TX IO13, RX IO12 |
| UART C (J6) | TX IO2, RX IO1 |
| Frei | IO3 (Strapping), IO14-IO16 (Loetpads) |

### Anschluesse
- CN1, J5, J6: UART A/B/C als JST-GH 4 (Footprint BM04B-GHS-TBT)
- CN2 Last 5-12 V (JST PH 2, max. ca. 2 A je Kontakt)
- J1 Ausgaenge S0..S7

### Hinweise
- UART A liegt auf UART0: beim Booten gibt das ROM ein Log auf IO43 aus.
- Empfehlung LED-Vorwiderstaende R1-R8 bei 3,3 V (ca. 5 mA, auch auf der Platine aufgedruckt): Rot/Gelb 270 R, Gruen 220 R, Blau/Weiss 56 R.
- Footprint ESP32-S3-Zero_THT: 2x9, 2,54 mm, Reihenabstand 15,24 mm. Vor dem Layout 1:1 am echten Board pruefen.

## Firmware (v4.05)
- Arduino-Sketch `firmware/RC_Multiswitch`, gleicher Quellcode fuer C3 und S3 (Board wird automatisch erkannt). Basis: MultiSwitch_ESP32_V3 v3.12.
- CRSF auf UART A, wahlweise SBUS. UART B und UART C per Lua oder Web-UI auf Aus, CRSF (Modulkette), S.Port, GPS oder Raw einstellbar, mit eigener Baudrate je UART. Beim GPS sucht die Baudrate „Auto“ (Vorgabe) die Rate selbst und stellt das GPS auf 115200 Baud und 10 Hz um, wie bei Betaflight. In der Modulkette ist jedes Modul mit eigener Adresse ueber Lua einzeln einstellbar.
- Gemeinsame Basis aller ESP32-Module seit v4.05: `firmware/RC_Multiswitch/src/RC_Common/` (siehe deren README).
- BOOT-Taster 3 s: WLAN-AP ein/aus. Status-LED nur beim S3.
- Details und Bauanleitung: [firmware/README.md](firmware/README.md).

## Ordner
- `hardware/`: KiCad-Projekt (Schaltplan, PCB, PDF, eigene Footprints, fp-lib-table). Entwurfsnotizen in [hardware/README-hardware.md](hardware/README-hardware.md).
- `firmware/`: Firmware v4.05 (Arduino-Sketch).
- `docs/`: Projektbeschreibung, Hardware-Beschreibung und Firmware-Spezifikation.

## Stand
- Hardware: Schaltplan fertig, PCB vom C3 uebernommen und an U1 neu verlegt. DRC in KiCad steht noch aus, J6 nur vorlaeufig platziert.
- Firmware: v4.05 kompiliert fuer C3 und S3, noch nicht auf Hardware getestet.

## Offene Punkte
- DRC in KiCad ausfuehren (an U1 sind laut PCB-Datei alle Anschluesse verlegt)
- Platzierung von J6 (UART C) festlegen
- Footprint am echten Board pruefen
- Firmware auf Hardware testen (Einzelmodul und Kette mit 2-3 Modulen)
