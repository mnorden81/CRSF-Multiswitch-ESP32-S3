# CRSF-Multiswitch-ESP32-S3

CRSF-Multiswitch mit 8 Schaltkanaelen auf einem Waveshare ESP32-S3-Zero.

Gleiche Funktion wie die C3-Variante, aber mit dem ESP32-S3-Zero und einem dritten UART. Das PCB wurde vom C3-Layout uebernommen, nur die Leitungen am ESP32 muessen neu geroutet werden.

Ausfuehrliche Projektbeschreibung: [docs/Projektbeschreibung.docx](docs/Projektbeschreibung.docx)

## Merkmale
- 8 Schaltkanaele ueber ULN2803A, Kontroll-LED je Kanal
- UART A (Eingang), UART B und UART C (Weiterleitung/Erweiterung)
- Versorgung des ESP32 aus +5 V des Empfaengers, Last getrennt mit 12-24 V
- Eigener THT-Footprint fuer den S3-Zero (nur 2x9 Header)

## Hardware (v0.4 (PCB vom C3 uebertragen))
Board: Waveshare ESP32-S3-Zero

- Lastspannung 12-24 V (VLOAD) an CN2 (JST PH 2), TVS-Diode P6KE33A als Schutz.
- Treiber ULN2803A, 8 Kanaele, max. 500 mA je Kanal (Verlustleistung des ULN2803A bei mehreren Kanaelen beachten).
- R9-R16 (THT 1/4 W, Standard 150 R) zwischen ULN-Ausgang und Stiftleiste J1, je nach Verbraucher waehlen.
- R1-R8 und LED1-LED8 (3 mm) als Kontroll-LED je Kanal, vom Steuersignal Sn gegen GND.
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
- CN2 Last 12-24 V (JST PH 2, max. ca. 2 A je Kontakt)
- J1 Ausgaenge S0..S7

### Hinweise
- UART A liegt auf UART0: beim Booten gibt das ROM ein Log auf IO43 aus.
- LED-Vorwiderstaende R1-R8 bei 3,3 V (ca. 5 mA): Rot/Gelb 270 R, Gruen 220 R, Blau/Weiss 56 R.
- Footprint ESP32-S3-Zero_THT: 2x9, 2,54 mm, Reihenabstand 15,24 mm. Vor dem Layout 1:1 am echten Board pruefen.

## Firmware (geplant)
- CRSF als einziges Protokoll, busfaehig ueber die UART-Anschluesse (Eingang plus Weiterleitung zum naechsten Modul).
- Gemeinsame CRSF-Bibliothek als Kopie im Projekt (firmware/lib), keine externe Abhaengigkeit.
- Einstellung der Adresse und Parameter ueber CRSF-Parameter (Lua am Sender) und Web-UI.
- Build mit PlatformIO, Bauweise wie bei den Sound-Mini-Repos.

## Ordner
- `hardware/`: KiCad-Projekt (Schaltplan, PCB, PDF, eigene Footprints, fp-lib-table). Entwurfsnotizen in [hardware/README-hardware.md](hardware/README-hardware.md).
- `firmware/`: Firmware, noch nicht begonnen.
- `docs/`: Projektbeschreibung als DOCX.

## Stand
- Hardware: Schaltplan fertig, PCB uebernommen. 20 Verbindungen an U1 sind noch nicht geroutet, J6 nur vorlaeufig platziert.
- Firmware: noch nicht begonnen.

## Offene Punkte
- Leiterbahnen an U1 routen, danach DRC
- Platzierung von J6 (UART C) festlegen
- Footprint am echten Board pruefen
- Firmware schreiben
