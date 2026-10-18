# RC-Multiswitch-ESP32-S3 (v0.4, PCB vom C3 uebertragen)
Ein Multiswitch nach CRSF Multiswitch THT v1.1 mit Waveshare ESP32-S3-Zero, ULN2803A, 8 Kanaele.

- Last: 5-12V (VLOAD, JST PH 2 an CN2, max. ca. 2A je Kontakt; TVS P6KE18A), max. 500mA je Kanal.
- R9-R16 (THT 1/4W) zwischen ULN-Ausgang und Stiftleiste J1: kein Wert vorgegeben, der Anwender legt sie je Verbraucher aus, Leistung beachten.
- R1-R8 (Wert vom Anwender, Empfehlung auf der Platine) + LED1-8 (3mm): Kontroll-LED je Kanal, vom Steuersignal Sn gegen GND.
- Keine Pulldowns, keine Widerstaende in den UART-Leitungen. ESP32 wird aus +5V von CN1 versorgt. Adresse per Firmware/Web-UI/Lua.
- UART-Buchsen CN1, J5, J6 als JST-GH (SMD-only, Footprint BM04B-GHS-TBT).

## Belegung (nur Header-Pins des S3-Zero, laut Waveshare-Pinout)
Zuordnung so gewaehlt, dass die vorhandenen Leiterbahnen des C3-Layouts moeglichst passen:
- S0..S7 = IO11, IO4, IO5, IO6, IO10, IO9, IO8, IO7
- UART A (Eingang, CN1): TX=IO43, RX=IO44 (UART0; beim Booten kommt das ROM-Log auf IO43)
- UART B (J5): TX=IO13, RX=IO12
- UART C (J6): TX=IO2, RX=IO1
- Frei: IO3 (Strapping), IO14-IO16 (Loetpads), Rueckseitenpads

## PCB (aus dem C3-Layout uebertragen)
- Platine, Bauteilpositionen, Beschriftungen und alle Leiterbahnen ausser am ESP32 unveraendert uebernommen.
- U1 (ESP32-S3-Zero, 2x9) steht an derselben Stelle wie der C3 (Mittelpunkt), Pads neu belegt. Die Leiterbahnen an U1 sind in der PCB-Datei (Stand KiCad-ZIP 2026-10-15) neu verlegt: jeder belegte Pin hat eine Bahn mit passendem Netz. Grobe Pruefung per Skript ohne KiCad (2026-10-18): an U1 fehlt keine Verbindung, keine Kurzschluesse gefunden; offen erscheinen nur 3 Stellen an VLOAD/GND, vermutlich T-Abzweige mitten auf einer Bahn. Massgeblich ist der DRC in KiCad.
- J6 (UART C) ist neu, Platzierung unten rechts nur ein Vorschlag.
- Nach dem Oeffnen: Pruefen -> DRC ausfuehren, Footprint U1 am echten Board gegenpruefen. Im Entstehungs-Sandkasten war kein KiCad 9 verfuegbar, daher kein ERC/DRC.

## Symbol/Footprint
- Pad-Nummern: linke Reihe 1..9 von oben (5V, GND, 3V3, IO1..IO6), rechte Reihe 10..18 von oben (TX43, RX44, IO13..IO7), USB oben.
- Footprint `footprints.pretty/ESP32-S3-Zero_THT`: nur 2x9 Header, 2,54 mm Raster, Reihenabstand 15,24 mm (Waveshare-Wiki). Loetpads 14-16 und Rueckseitenpads sind nicht enthalten. Bindung per `fp-lib-table`.
- Vor dem Layout 1:1 am echten Board pruefen. Kein ERC, keine Firmware getestet.

## LED-Vorwiderstaende R1-R8 bei 3,3V (ca. 5mA, mittlere Helligkeit)
| Farbe | Uf typ. | R |
|---|---|---|
| Rot | 1,9V | 270R |
| Gelb | 2,0V | 270R |
| Gruen | 2,1V | 220R |
| Blau | 3,0V | 56R |
| Weiss | 3,0V | 56R |
Blau/Weiss: Uf streut stark, bei Bedarf am echten LED nachmessen. Dieselbe Tabelle steht als Hinweis im Schaltplan und ist als Aufdruck auf der Platine.
