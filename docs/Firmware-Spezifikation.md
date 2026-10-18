# RC-Multiswitch – Firmware-Spezifikation

ESP32-C3 SuperMini und ESP32-S3-Zero · Basis V3 v3.10 · Stand 16.10.2026


## 1. Zweck und Vorgehen

Diese Spezifikation beschreibt die Firmware für die beiden Multiswitch-Platinen **ESP32-C3 SuperMini** und **ESP32-S3-Zero** (Hardware siehe Hardware-Beschreibung). Sie ist als Arbeitsgrundlage für die Entwicklung im bestehenden Firmware-Projekt gedacht (Basis: Multiswitch ESP32 V3, v3.10).

- **Funktionsumfang der V3 bleibt erhalten** (Konfig1/2/3 je Ausgang, MKan1/MKan2, Web-UI, Lua, Failsafe, NVS, OTA, JSON-Export/Import).
- **Neu:** hardwareabhängige Pin-/Board-Konfiguration (C3/S3), CRSF-Weiterleitung (Router) für Modulketten, jedes Modul in der Kette erreichbar, WLAN per BOOT-Taster, Status-LED nach CRSF-Linkzustand.
- **Arbeitsregeln:** minimal-invasive Änderungen, bestehendes Verhalten nicht verändern, Versionsstring und README konsistent halten, vor größeren Änderungen Sicherung (backups/).


## 2. Zielhardware und Pinbelegung

Die Pins werden zentral in einer Board-Datei (`board_config.h`) per `#if defined(BOARD_C3)` / `BOARD_S3` gekapselt. Die bisherigen Pins der V3 (`OutPin[]`, `WifiPin=13`, `LedPin=2`, UART2 an 16/17) entfallen auf den neuen Platinen.

**ESP32-C3 SuperMini**

| Signal | GPIO | Funktion |
|---|---|---|
| S0 | 0 | Ausgang 1 (PWM/digital) |
| S1 | 1 | Ausgang 2 (PWM/digital) |
| S2 | 2 | Ausgang 3 (PWM/digital) |
| S3 | 3 | Ausgang 4 (PWM/digital) |
| S4 | 4 | Ausgang 5 (PWM/digital) |
| S5 | 5 | Ausgang 6 (PWM/digital) |
| S6 | 6 | Ausgang 7 (PWM/digital) |
| S7 | 7 | Ausgang 8 (PWM/digital) |
| UA_RX | 20 | CRSF-UART A RX |
| UA_TX | 21 | CRSF-UART A TX |
| UB_TX | 10 | CRSF-UART B TX |
| UB_RX | 8 | CRSF-UART B RX |

**ESP32-S3-Zero**

| Signal | GPIO | Funktion |
|---|---|---|
| S0 | 11 | Ausgang 1 (PWM/digital) |
| S1 | 4 | Ausgang 2 (PWM/digital) |
| S2 | 5 | Ausgang 3 (PWM/digital) |
| S3 | 6 | Ausgang 4 (PWM/digital) |
| S4 | 10 | Ausgang 5 (PWM/digital) |
| S5 | 9 | Ausgang 6 (PWM/digital) |
| S6 | 8 | Ausgang 7 (PWM/digital) |
| S7 | 7 | Ausgang 8 (PWM/digital) |
| UA_RX | 44 | CRSF-UART A RX |
| UA_TX | 43 | CRSF-UART A TX |
| UB_TX | 13 | CRSF-UART B TX |
| UB_RX | 12 | CRSF-UART B RX |
| UC_TX | 2 | CRSF-UART C TX |
| UC_RX | 1 | CRSF-UART C RX |

**Boardspezifika**

| Punkt | C3 | S3 |
|---|---|---|
| Arduino-Board | ESP32C3 Dev Module | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled (Debug über USB) | Enabled (Debug über USB) |
| UART A | Serial0-Pins IO20 RX / IO21 TX (UART0) | IO44 RX / IO43 TX (UART0) |
| UART B | UART1, Pins IO8 RX / IO10 TX per GPIO-Matrix | UART1, IO12 RX / IO13 TX |
| UART C | nicht vorhanden | UART2, IO1 RX / IO2 TX |
| LEDC / PWM | 6 Kanäle: Ausgang 1–6 PWM, 7–8 nur digital | 8 Kanäle, alle PWM |
| Kerne | 1 (Router, Web, Lua im selben Task-Umfeld) | 2 (Router/CRSF auf Core 1, WLAN auf Core 0 möglich) |
| BOOT-Taster | IO9 | IO0 |
| Status-LED | keine (IO8 = UB_RX) → deaktiviert | WS2812 an IO21 (`neopixelWrite`) |
| Partition | Schema mit OTA (zwei App-Slots) | Schema mit OTA; Flash-Größe laut Modul prüfen |

Die V3-Regel bleibt: **nur Output() ruft ledcWrite() auf.** Auf dem C3 wird für Ausgang 7/8 in `Output()` auf `digitalWrite()` (Schwelle ≥ 50 %) bzw. Software-Blinken zurückgefallen; PWM-Parameter werden für diese Ausgänge im Web-UI/Lua ausgeblendet oder ignoriert.


## 3. CRSF-Grundverhalten

- 420 000 Baud, 8N1, nicht invertiert, Full-Duplex (separate TX/RX-Leitung) – Annahme: Empfänger-UART arbeitet nicht halbduplex (A1).
- Frameformat: `[Sync/Adresse][Länge][Typ][Nutzdaten …][CRC8]`, CRC8 mit Polynom 0xD5 über Typ + Nutzdaten. Max. Framelänge 64 Byte.
- Modul-Adresse wie V3: `0xC0 + modul_adress` (0…15, also 0xC0…0xCF). Sie muss in einer Kette **eindeutig** sein.
- RC-Channels-Frame (Typ 0x16) liefert die Kanäle; MKan/Konfig-Logik wie V3. Link-Statistics (0x14) wird ausgewertet (falls vorhanden) und unverändert weitergeleitet.
- Verbindungsstatus (BUS_OK) wie V3: gültige Frames in einem Zeitfenster; Timeout 2000 ms → Failsafe-Verhalten wie V3.
- SBUS-Eingang der V3 entfällt für die neuen Platinen (Annahme A2).


## 4. Modulkette – CRSF-Router

Mehrere Module werden hintereinander geschaltet: Empfänger → Modul 1 (UART A) → Modul 2 (an UART B/C von Modul 1) → … Jedes Modul arbeitet als **Store-and-Forward-Router**.


### 4.1 Regeln

1. Jeder Frame wird vollständig empfangen und per CRC geprüft; fehlerhafte Frames werden verworfen (nicht weitergeleitet).
2. **Von A (Richtung Ketten-Ende):** Frame für die eigene Adresse → intern bearbeiten, nicht weiterleiten. Frame für fremde Adresse → unverändert an B und C weiterleiten. Broadcast (Adresse 0x00 / 0xEE / 0xC8, z. B. RC-Channels, Ping) → intern bearbeiten **und** weiterleiten.
3. **Von B oder C (Richtung Empfänger):** Frames (Antworten auf Ping/Parameter, Geräteinfos) → unverändert an A weiterleiten.
4. **Nie** zwischen B und C weiterleiten (kein Ringschluss).
5. Frames werden **atomar** ausgegeben: kein Einfügen eigener Frames mitten in einen weitergeleiteten Frame; pro Ausgangsport eine Sendewarteschlange (≥ 8 Frames).
6. Warteschlange voll: veraltete RC-Channels-Frames zuerst verwerfen (Aktualität vor Vollständigkeit); Parameter-/Geräte-Frames nicht verwerfen.
7. Optional: Routing-Tabelle lernen (Absenderadresse → Port B oder C) und gerichtete Frames nur an den passenden Port senden; unbekannte Ziele an beide.
8. Weiterleitungslatenz je Modul klein halten (Ziel ≤ 2 ms; Framezeit bei 420 kBd ca. 1,5 ms je 64 Byte).


### 4.2 Jedes Modul erreichbar

- Auf einen Ping (Typ 0x28) antwortet jedes Modul mit Device-Info (Typ 0x29) im Zeitschlitz `adresse × 2 × 5 ms` (wie V3) – dadurch kollidieren die Antworten nicht. Der Ping wird zusätzlich weitergeleitet, damit alle nachfolgenden Module ihn erhalten.
- Antworten nachfolgender Module laufen durch den Router nach oben (A). Eigene Antwort und weitergeleitete Frames verwenden dieselbe Sendewarteschlange.
- Geräte-Name im Lua-Geräte-Menü eindeutig: z. B. `MultiSwitch A<adr>` (Adresse im Namen).
- Parameter-Lese-/Schreib-Frames (Typ 0x2C/0x2D, extended header mit Ziel- und Ursprungsadresse) werden anhand der Zieladresse an das richtige Modul geroutet; Antworten (0x2B) laufen mit unveränderter Ursprungsadresse zurück. **Damit kann jedes Modul der Kette über Lua konfiguriert werden.**
- Der Parameterbaum (V3: ca. 80 Parameter) ist je Modul identisch aufgebaut; die Parameter gelten jeweils für das adressierte Modul.


### 4.3 Adressvergabe

- Adresse einstellbar über Web-UI und Lua (Parameter „Modul-Adresse“, 0…15), in NVS gespeichert; Standardwert 0.
- Änderung der Adresse wirkt erst nach Neustart des Moduls (Router und Zeitschlitz bleiben bis dahin konsistent).
- Doppelte Adressen in einer Kette sind ein Konfigurationsfehler: Erkennung optional (zwei Device-Info-Antworten mit gleicher Adresse) und Warnung in Web-UI/Debug-Log.


## 5. WLAN und Web-UI

- Web-UI, JSON-Export/-Import, OTA und AP-Verhalten wie V3 (Einstellung „AP dauerhaft an“ mit Standard AN, Auto-Failsafe-AP, manuell an/aus).
- **BOOT-Taster langer Druck (≥ 3 s)** schaltet den WLAN-Access-Point zur Laufzeit um (an ↔ aus). Auswertung **nur nach dem Booten** (Taster ist Strapping-Pin: low beim Reset = Download-Modus).
- `WifiPin=13` der V3 entfällt komplett – GPIO13 ist auf dem S3 UART-B-TX.
- Der WLAN-Zustand wird im Status-LED-Muster nicht abgebildet (siehe 6), Rückmeldung ggf. über Web-UI/Debug-Log.


## 6. Status-LED

Die Status-LED zeigt den CRSF-Linkzustand. Ausgangsbasis ist das V3-Verhalten; die ergänzenden Muster sind Vorschläge und zu bestätigen (O1).

| Zustand | Muster |
|---|---|
| Kein gültiger CRSF-Frame (BUS nicht OK, Timeout > 2 s) | aus (V3) |
| CRSF-Frames gültig, Link aktiv (BUS_OK) | blinkt 500 ms an / 500 ms aus (V3) |
| Optional (O1): Frames da, aber Failsafe / kein RC-Link | schnelles Blinken (z. B. 100 ms) |
| Optional (O1): Konfigurationsfehler (doppelte Adresse) | Doppelblitz |

- C3: keine steuerbare Status-LED (Onboard-LED IO8 ist UB_RX) → `STATUS_LED_PIN = -1`, Funktion kompiliert aus. Der LED-Pin darf nicht als UART-B-Pin umkonfiguriert werden.
- S3: WS2812 an IO21 per `neopixelWrite(21, r, g, b)`, Farbe z. B. grün für Link aktiv; Helligkeit niedrig (≤ 20 %).
- Aktualisierung nicht blockierend (`millis()`-basiert), unabhängig vom Router.


## 7. Persistenz und Konfiguration

- NVS (Preferences) wie V3; Schlüsselnamen unverändert, damit bestehende Konfigurationen/JSON-Exporte kompatibel bleiben.
- Neue Schlüssel nur ergänzen (z. B. `board`-unabhängige Einstellungen wie Routing an/aus); keine Umbenennung.
- Ausgangskonfiguration: pro Ausgang Konfig1/2/3 (Priorität 1 > 2 > 3, in Lua nur Konfig1), MKan1/MKan2 mit `EK_Gruppen_Adresse[]` und `Ausgang_Kanal` 80–95 – unverändert.


## 8. Abnahme-Checkliste

1. Kompiliert für C3 und S3 (je ein Build-Ziel) ohne Warnungen in neuen Teilen.
2. Einzelmodul am Empfänger: Kanäle schalten, LEDs folgen, Failsafe nach 2 s.
3. Zwei und drei Module in Kette: Alle erscheinen im Lua-Geräte-Menü; Konfiguration eines Moduls hinten in der Kette ändert nur dieses.
4. RC-Channels erreichen alle Module; Latenz-Messung über die Kette (≤ wenige ms je Modul).
5. Keine Frames B ↔ C; keine Endlosschleife; keine Verfälschung der Frames (CRC beim Nachbarn gültig).
6. BOOT-Langdruck schaltet AP; Kurzdruck/Reset löst keine falsche Aktion aus; Boot in Download-Modus nur bei gedrücktem Taster beim Reset.
7. Status-LED zeigt Muster nach Tabelle (S3); C3 ohne LED-Funktion, UART B unbeeinträchtigt.
8. OTA, JSON-Export/-Import, NVS-Kompatibilität mit V3-Konfig.


## 9. Annahmen und offene Punkte

- **A1:** Empfänger-UART ist Full-Duplex (separate RX/TX-Leitungen). Bei halbduplex-Empfängern ist eine Anpassung nötig.
- **A2:** SBUS-Eingang entfällt auf den neuen Platinen. Falls gewünscht, bitte angeben.
- **O1:** CRSF selbst definiert keine verbindliche LED-Blinksequenz; es gilt das V3-Verhalten (aus = kein Signal, 500 ms = verbunden). Ergänzende Muster bitte bestätigen oder streichen.
- **O2:** C3: Ausgang 7/8 nur digital – ist das akzeptabel, oder soll Software-PWM ergänzt werden?
- **O3:** Strapping-Pin IO2 (C3, Kanal 3) und IO8 (UB_RX/Onboard-LED) beim ersten Boot prüfen.
- Die Hardware wurde nicht in KiCad-ERC/DRC geprüft und die Firmware ist hier nicht kompiliert oder getestet worden.

