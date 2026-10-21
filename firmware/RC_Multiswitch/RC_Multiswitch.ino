/*
    RC-Multiswitch  v4.05  (ESP32-C3 SuperMini / Waveshare ESP32-S3-Zero)
   v4.00 abgezweigt von ESP32-MultiSwitch v3.12 (Repo ESP32-Multiswitch-
   CRSF-bidirektional, MultiSwitch_ESP32_V3). Gleicher Quellcode fuer beide
   Platinen, Board-Auswahl in board_config.h.
   v3.00 erstellt von: PiperPilot - eigenstaendiges, unabhaengig baubares
   Projekt, abgezweigt von ESP32-MultiSwitch v2.02 (siehe ESP32_Multiswitch_2/
   ESP32-Multiswitch-CRSF-bidirektional/). v2.xx bleibt unveraendert bestehen.
   v2.00 erstellt von: PiperPilot
   Basiert auf: ESP32-SBus-Switch 0.6   (Ziege-One / Der RC-Modellbauer)
   CRSF-Integration: ESP32-RC-Sound 0.43 (Ziege-One / Der RC-Modellbauer)

 /////Projektbeschreibung////
 RC-gesteuerter 8-Kanal-Schalter/PWM-Ausgabemodul für ESP32. Empfängt
 Schaltbefehle über CRSF (ExpressLRS/TBS CrossFire, bidirektional) oder
 über SBUS-basierte RC-Systeme (FrSky/FlySky/ELRS normiert/HoTT) und
 steuert darüber acht digitale bzw. PWM-fähige Ausgänge (Licht, Blinker,
 Servos, Relais o.ä.). Die gesamte Konfiguration erfolgt über eine im
 Modul integrierte Weboberfläche (WLAN-Access-Point) - es wird kein
 SD-Karten-Slot benötigt oder verwendet. Ausführliche Beschreibung siehe
 ESP32-MultiSwitch-V2-Projektbeschreibung-v2.00.docx / README.md.

 /////Pin Belegung////
 Siehe board_config.h (C3 und S3) bzw. docs/Hardware-Beschreibung_C3_S3.docx.
 CRSF und SBUS liegen beide auf UART A (CN1), je nach RC-System.

 RC-System Werte (unveraendert zu V3):
   0 = FrSky          (SBUS)
   1 = FlySky         (SBUS)
   2 = ELRS normiert  (SBUS)
   3 = HoTT           (SBUS)
   4 = CRSF           (ExpressLRS / TBS CrossFire, bidirektional)
 Lua-Konfiguration und (ab Schritt 3) Modulkette gibt es nur bei CRSF.

 ÄNDERUNGEN v4.05 (gemeinsame Basis RC_Common 1.1.0):
   - Gemeinsamer Code aller ESP32-Module liegt jetzt in src/RC_Common/
     (identische Kopie in jedem Projekt, siehe src/RC_Common/README.md):
     CRSF-Bibliothek 1.1.0 (mit Router), UART-Rollen, S.Port, GPS,
     Lua-Menue als Tabelle, NVS-Schluessel, WLAN-Failsafe, Status-LED,
     OTA-Upload. Die eigenen Kopien crsf_esp32.*, sport_lipo.*,
     gps_speed.* entfallen.
   - FIX: Command-CRC (0xBA) der eigenen CRSF-Kopie war falsch, MWprop
     ohne Command-CRC. Mit der Bibliothek jetzt laut Spezifikation.
   - UART B/C: neue Rolle "Raw" (UART offen, keine Auswertung) und eine
     Baudrate je UART (Lua-Felder 84/85, Web-UI). Rollennummern bleiben
     (0 Aus, 1 CRSF, 2 S.Port, 3 GPS, 4 Raw). Feld 84 war "GPS Baud".
   - NEU: Baudrate "Auto" (RC_Common 1.1.0), Vorgabe fuer GPS: der UART
     sucht die Baudrate des GPS selbst (9600/115200/38400/57600/19200),
     stellt es auf 115200 Baud und 10 Hz um (ATGM336H und u-blox) und
     sucht neu, wenn 3 s keine Daten kommen. Bisher eingestellte GPS-
     Baudraten werden einmalig auf "Auto" gesetzt. Bei den anderen Rollen
     heisst "Auto" die Protokoll-Baudrate (CRSF 420000, S.Port 57600,
     Raw 115200).
   - Lua-Menue als Tabelle (buildLuaMenu()), gleiche Feld-IDs 0-83.
   - NVS: gemeinsame Schluessel wip, madr, wfauto, wftmo, wfalways,
     u1role, u1baud, u2role, u2baud (RC_Common settings_common.h). Beim
     ersten Start werden die alten Werte (ip, mod_adr, wauto, watout,
     walways, pbrole, pcrole, gpsbaud) einmalig uebernommen; die alten
     Schluessel bleiben fuer ein Zurueckspielen von v4.04 stehen.
   - Status-LED, WLAN-Auto-Failsafe und OTA-Upload aus RC_Common, das
     Verhalten ist unveraendert.

 ÄNDERUNGEN v4.04:
   - NEU: Rolle von UART B (C3/S3) und UART C (nur S3) waehlbar, wie beim
     Soundmodul: Aus, CRSF (Modulkette), S.Port (FrSky FLVSS/MLVSS
     LiPo-Sensoren) oder GPS (NMEA). Einstellbar im Lua-Menue (Ordner
     "UART B/C", Felder 81-84) und in der Web-UI (RC-Tab). Wirkt nach einem
     Neustart. Default CRSF auf beiden Ports = Verhalten wie bis v4.03.
   - S.Port und GPS laufen nur auf einem Port: bei doppelter Wahl bleibt
     UART C aus (Hinweis in der Web-UI).
   - Bei CRSF gehen Zellspannungen (Frame 0x0E) und GPS (Frame 0x02) als
     Telemetrie an den Empfaenger, alle 100 ms. Bei SBUS werden S.Port und
     GPS nur gelesen (Anzeige im Debug-Tab), CRSF auf B/C ist dort aus.
   - sport_lipo.cpp/.h und gps_speed.cpp/.h aus dem Soundmodul Mini
     uebernommen. Neue Bibliothek: TinyGPSPlus.
   - Neue NVS-Schluessel "pbrole", "pcrole", "gpsbaud" (fehlen sie, gilt
     CRSF/CRSF/9600).

 ÄNDERUNGEN v4.03:
   - FIX: SBUS-Zeitlimit. Kommen 10 s lang gar keine SBUS-Frames mehr
     (Stecker ab, Empfaenger ohne Strom), gilt das Signal als weg: BUS_OK
     wird false, die Ausgaenge gehen aus und das WLAN-Auto-Failsafe kann
     greifen. Vorher wurde BUS_OK nur ueber das Failsafe-Bit des
     Empfaengers geloescht, ohne Frames blieb der letzte Zustand stehen.
     Siehe checkSbusTimeout().

 ÄNDERUNGEN v4.02:
   - FIX: CRSF-Link-Statistik (Frame 0x14) wurde um ein Byte verschoben
     gelesen - das Feld RSSI Antenne 2 fehlte. uplinkLQ war damit in
     Wahrheit RSSI Antenne 2, die Status-LED zeigte bei Empfaengern mit
     einer Antenne faelschlich "schnell" (kein RC-Link). Jetzt laut
     CRSF-Spezifikation (crsf_esp32.cpp updateLink_Statistics()).

 ÄNDERUNGEN v4.01 (Schritt 2 und 3 der Firmware-Spezifikation):
   - NEU: BOOT-Taster (C3 IO9, S3 IO0) lang druecken (>= 3 s) schaltet den
     WLAN-AP um (an <-> aus), ohne Neustart und ohne NVS-Aenderung. Wird
     erst ausgewertet, nachdem der Taster nach dem Booten einmal losgelassen
     war (Strapping-Pin, siehe bootButtonCheck()).
   - NEU: CRSF-Router fuer Modulketten (nur bei RC_System 4 = CRSF):
     UART B (C3/S3) und UART C (nur S3) fuehren zum naechsten Modul.
     A -> B/C: alle gueltigen Frames ausser Extended-Frames an die eigene
     Adresse; B/C -> A: alle gueltigen Frames; nie B <-> C. Frames werden
     ganz und unveraendert weitergegeben (CRC geprueft), je Port eine
     Warteschlange mit 8 Frames, bei Ueberlauf werden zuerst alte
     RC-Kanal-Frames verworfen. Jedes Modul antwortet wie bisher in seinem
     Zeitschlitz und ist damit ueber Lua einzeln erreichbar (Geraetename
     "<Name>@<Adresse>").
   - NEU: Doppelte Modul-Adresse wird erkannt (Device-Info mit eigener
     Adresse kommt von B/C zurueck): Doppelblitz der Status-LED (S3) und
     Warnung im Debug-Tab der Web-UI. Gilt bis zum Neustart.
   - FIX: CRSF-Parser auf UART A behaelt seinen Zustand zwischen zwei
     Loop-Durchlaeufen; vorher ging ein Frame verloren, der ueber zwei
     Durchlaeufe verteilt ankam.
   - UART-Puffer fuer CRSF auf 1024 Byte Empfang / 512 Byte Senden
     vergroessert.
   - Debug-Tab: Zaehler fuer weitergeleitete und verworfene Frames.
   - NICHT veraendert: Ausgangslogik, Lua-Parameterbaum, NVS-Schluessel,
     SBUS-Betrieb (dort kein Router).

 ÄNDERUNGEN v4.00 (neue Platinen C3/S3, Schritt 1 der Firmware-Spezifikation):
   - NEU: board_config.h mit Pins, UARTs, BOOT-Taster und Status-LED fuer
     ESP32-C3 SuperMini (BOARD_C3) und ESP32-S3-Zero (BOARD_S3), automatisch
     aus dem gewaehlten Chip erkannt.
   - UART A (CN1) = UART0: C3 RX=IO20/TX=IO21, S3 RX=IO44/TX=IO43. CRSF und
     SBUS laufen beide dort, je nach RC_System wird beim Booten eines davon
     gestartet. SBUS bleibt erhalten (Entscheidung 16.10.2026, Annahme A2
     der Spezifikation gestrichen); die Invertierung macht die UART-Hardware.
     Debug-Ausgaben ueber USB-CDC ("USB CDC On Boot: Enabled" noetig).
   - WifiPin (GPIO13) entfaellt; AP wie bisher ueber "Dauerhaft an" und
     Auto-Failsafe. Der BOOT-Taster folgt in Schritt 2.
   - C3: nur 6 LEDC-Kanaele, Ausgang 7/8 schalten digital (an ab 50 % Duty,
     Blinken weiter per Software). Neue Hilfsfunktion outWrite(), nur aus
     Output() aufgerufen.
   - Status-LED (Spezifikation Kapitel 6): S3 WS2812 (IO21) gruen, C3 ohne
     Status-LED (Code laeuft, Ausgabe entfaellt). Muster:
       aus                   = kein gueltiger Frame (> 2 s)
       500 ms an / 500 ms aus = Verbindung OK
       schnell (100 ms)      = Frames da, aber Failsafe bzw. kein RC-Link
                               (CRSF: > 1 s kein Kanal-Frame oder Uplink-LQ 0,
                               SBUS: Failsafe-Flag)
       Doppelblitz           = Konfigurationsfehler (g_ledConfigError,
                               ab v4.01 bei doppelter Adresse gesetzt)
     Nur Anzeige: BUS_OK, Failsafe der Ausgaenge und WLAN-Auto-Failsafe
     verhalten sich wie in V3.
   - NICHT verändert: Ausgangslogik (Konfig 1/2/3, MKan, Blink, PWM, MWprop),
     CRSF- und SBUS-Auswertung, Lua-Parameterbaum, NVS-Schluessel,
     Web-UI-Funktionen.

 ÄNDERUNGEN v3.12 (CRSF-Fix, übernommen aus v2.03):
   - CRSF "Parameter Version Number" im DEVICE_INFO-Frame war fest 0x00;
     jetzt wird die Firmware-Version mitgegeben (siehe send_device_info()).
     Damit lesen Konfiguratoren wie TBS Agent eine geaenderte
     Parameterstruktur neu ein.

 ÄNDERUNGEN v3.10 (Mehrfachkonfiguration pro Ausgang):
   - NEU: bis zu 3 unabhängige (Kanal-Quelle -> Blink/PWM)-Konfigurationen
     je Ausgang, Priorität Konfig 1 > Konfig 2 > Konfig 3 - ist Konfig 1
     aktiv, gilt deren Blink/PWM, unabhängig davon ob 2/3 ebenfalls aktiv
     wären; erst wenn 1 inaktiv ist, wird 2 geprüft, usw.
   - Konfig 1 = weiterhin Ausgang_Kanal[]/mode[]/pwm_wert[] (unverändert,
     bleibt auch im CRSF-LUA-Menü erreichbar). Konfig 2/3 = NEU
     (Ausgang_Kanal2/3[], mode2/3[], pwm_wert2/3[]), aktuell NUR über die
     Web-UI konfigurierbar, nicht über das LUA-Menü (hält dessen Feldzahl
     bei ~80 statt ~200).
   - Neue Helfer evalSlotActive() (prüft eine (Kanal,PWM)-Konfiguration auf
     aktiv/inaktiv - vorher inline in Output()) und getPwmDuty(int pv)
     (Signatur geändert: nimmt jetzt den PWM-Wert direkt statt intern
     pwm_wert[x] zu lesen, da je nach aktiver Konfiguration ein anderes
     Array gilt).
   - NVS: 6 neue Preferences-Keys pro Ausgang (ak2_x/pw2_x/mo2_x/ak3_x/
     pw3_x/mo3_x), Default bei fehlendem Key = Konfiguration ungenutzt
     (255/255/0) - Update von v3.00 lädt bestehende Konfig 1 unverändert,
     Konfig 2/3 automatisch als "aus".
   - NICHT verändert: Konfig-1-Verhalten/-Timing, MKan-Gruppenlogik (V3.00),
     Hardware-Pinbelegung, manueller Web-UI-Override (nutzt weiterhin
     immer mode[x]/pwm_wert[x], unabhängig von Konfig 2/3).

 ÄNDERUNGEN v3.00 (MKan-Mehrfachadress-Erweiterung, Konzept + Vorbild aus dem
 Soundmodul-Projekt ESP32-RC-Sound firmware-v7, dort config.h/
 ESP32-RC-Sound.ino "MKan" Quelltyp 80-95):
   - NEU: 2 zusätzliche, unabhängige 8-Bit-Kanalquellen ("MKan1"/"MKan2")
     zusätzlich zur bestehenden Einzelkanal-Adresse (modul_adress/
     einkanal_mode - unverändert). Bei CRSF über 2 frei konfigurierbare
     Gruppenadressen (EK_Gruppen_Adresse[], 255=aus), bei SBUS über 2 fest
     zugewiesene SBUS-Kanäle (SBUS_Gruppen_Channel[]/SBUS_Gruppen_Mode[]).
     Ergebnis in einkanal_Flat16 (16 Bit), ausgewertet in Output() über
     Ausgang_Kanal 80-95 (80-87 = MKan1 Bit 0-7, 88-95 = MKan2 Bit 0-7).
   - Neue Funktionen: updateGroupFromAddress() (CRSF-Gruppenadress-Abgleich,
     aufgerufen aus einkanalFunctionCRSF()/MWset4/MWset4m) und
     einkanalFunctionSBUSGroup() (SBUS-Pendant, eigenständige Funktion,
     bestehende einkanalFunctionSBUS() bewusst unverändert gelassen).
   - Web-UI: neue Karte "MKan-Gruppen (V3)" im RC-Tab (Adresse bzw.
     Kanal+Modus je Gruppe) + 2 neue Einträge in der Kanalquellen-Auswahl
     pro Ausgang ("MKan1"/"MKan2"). CRSF-LUA-Menü: "Ausgang-Quelle" pro
     Ausgang um MKan1/MKan2 erweitert; die beiden Gruppenadressen/-kanäle
     selbst sind aktuell NUR über die Web-UI konfigurierbar, nicht über
     das LUA-Menü (bewusst kleiner gehalten, siehe README).
   - NVS: 6 neue Preferences-Keys (gadr0/1, gch0/1, gmod0/1), Defaults
     255/999/0 (= alle Gruppen aus) - Firmware-Wechsel auf einem bereits
     mit v2.xx betriebenen Board bootet damit unverändert ohne Migration.
   - Versionsstring in der CRSF-INFO-Antwort ("v1.42 ESP32", war seit v2.xx
     nicht mehr mit der numerischen Version synchron) auf "v3.00 ESP32"
     korrigiert.
   - NICHT verändert: Output-Timing/Blink-Logik, Einzelkanal-Auswertung
     (modul_adress/einkanal_mode), Hardware-Pinbelegung, PWM/MWprop.

 ÄNDERUNGEN v2.00 (übertragen aus dem Soundmodul-Projekt, ESP32-RC-Sound v7):
   - CRSF-Diagnose-Zähler (rohe Bytes, gültige Frames, CRC-Fehler, PING/
     PARAM-READ/WRITE) in crsf_esp32.h/.cpp, sichtbar im Debug-Tab der
     Weboberfläche - macht "geht nicht" ohne Seriell-Monitor sichtbar.
   - BUS_OK bei CRSF jetzt an den Zähler für gültige Frames gekoppelt statt
     an "channel_output[0] > 0" (ein Kanalwert von 0 ist legitim und kein
     zuverlässiger Nachweis für einen frischen Frame).
   - CRC-Fehler werden gezählt statt bei jedem einzelnen Fehler sofort per
     Serial.println() ausgegeben - verhindert, dass anhaltendes Funkrauschen
     den Loop ausbremst; Sammel-Meldung höchstens alle 2s.
   - updateDevice_Info() gegen zu kurze/kaputte DEVICE_INFO-Frames
     abgesichert (signed-Rechnung + Mindestlängen-Check).
   - CRSF_FRAMETYPE_COMMAND prüft jetzt die Zieladresse wie PING/PARAMETER_
     READ/WRITE (mit Broadcast-Ausnahme fürs WM-Protokoll).
   - Konfiguration exportieren/importieren als JSON-Datei über den Browser
     (kein SD-Karten-Slot vorhanden - rein Web/WLAN-basiert), siehe webui.h.
   - WLAN-Steuerung: optionales Auto-Failsafe schaltet den Access Point
     automatisch ein, wenn längere Zeit kein gültiges RC-Signal anliegt
     (z.B. Sender aus/außer Reichweite) - Default AN seit v2.01, siehe
     wifiFailsafeCheck() unten. Ergänzend manueller "WLAN jetzt ein/aus"-
     Schalter (Web-UI), der ohne Neustart sofort wirkt, siehe webui.h.
   - Firmware-Update per WLAN (OTA): eine mit der Arduino-IDE gebaute
     .bin-Datei wird direkt im Browser hochgeladen und landet im gerade
     inaktiven OTA-Flash-Segment - Ausbauen/USB-Flashen ist für spätere
     Updates damit nicht mehr nötig. Schlägt der Upload fehl oder wird er
     abgebrochen, bootet das Modul unverändert mit der bisherigen
     Firmware weiter (kein Bricking-Risiko), siehe
     handleOtaUpdateData()/handleOtaUpdate() in webui.h. Voraussetzung:
     einmalig ein OTA-fähiges Partitionsschema per USB flashen, siehe
     README. WICHTIG (Erfahrung aus dem Soundmodul-Projekt übernommen):
     Update möglichst bei über ESC/Akku bestromtem Modul durchführen,
     nicht nur über USB - manche USB-Anschlüsse liefern beim
     Flash-Schreiben nicht genug Strom, ein dadurch ausgelöster Reset
     bricht den Upload mittendrin ab.
   - eeprom_esp32.h/hal_esp32.h/output_ctrl.h sind weiterhin NICHT
     eingebunden (totes Code-Fragment eines nicht gemergten Refactorings,
     jetzt oben in den Dateien selbst als solches gekennzeichnet).
   - Fix (nach erstem Test): webui_init() rief webServer.begin() bisher
     unabhängig vom AP-Status auf, ohne den WiFi-Treiber vorher zu
     initialisieren. War der AP beim Booten nicht aktiv, fehlten dessen
     interne FreeRTOS-Queues und es kam zum sofortigen Absturz
     (assert failed: xQueueSemaphoreTake) mit Bootloop. Behoben durch
     WiFi.mode(WIFI_AP) am Anfang von webui_init() (siehe webui.h).

 ÄNDERUNGEN v0.14 (Verbesserungen):
   - MWprop-Unterstützung aus ESP32-RC-Sound übernommen:
     Lautstärke/PWM-Steuerung einzelner Ausgänge über CRSF-Kanal möglich.
   - wm_prop_value[8] Speicher für MWprop-Kanalwerte (duty 0–255).
   - nvsSave() jetzt nur noch bei tatsächlicher Änderung (nvsDirty-Flag
     war schon vorhanden, wird nun auch in webui korrekt gesetzt).
   - CRSF-Einkanal-Timeout erhöht auf 2000 ms (war 1000 ms) –
     robuster bei kurzen Signalunterbrechungen.
   - Output(): pwm_wert >= 300 Mapping auf benannte Konstante ausgelagert.
   - Konsistente Verwendung von static constexpr überall.
   - README.md aktualisiert.
*/

// ======== Bibliotheken ==========================================
/*
  Bolder Flight Systems SBUS  8.1.4   (für RC_System 0-3)
  TinyGPSPlus 1.0.3                   (v4.04: Rolle GPS auf UART B/C)
  RC_Common 1.1.0 im Sketch-Ordner src/RC_Common/ (v4.05), enthaelt
    CRSF_ESP32 1.1.0, Basis https://github.com/Ziege-One/CRSF_ESP32
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "board_config.h"
#include "sbus.h"
#include "blink_presets.h"
// v4.05: gemeinsame Basis aller ESP32-Module
#include "src/RC_Common/rc_common.h"
#include "src/RC_Common/CRSF_ESP32/crsf_esp32.h"
#include "src/RC_Common/uart_roles.h"
#include "src/RC_Common/uart_ports.h"
#include "src/RC_Common/sport_lipo.h"
#include "src/RC_Common/gps_speed.h"
#include "src/RC_Common/lua_menu.h"
#include "src/RC_Common/settings_common.h"
#include "src/RC_Common/status_led.h"
#include "src/RC_Common/wifi_failsafe.h"
#include "src/RC_Common/web_common.h"

constexpr uint16_t Version = 405; // 4.05 - CRSF Parameter Version Number

// ======== SBUS-Schwellen (benannte Konstanten) ==================
static constexpr uint16_t SBUS_LOW_THRESHOLD  =  800;
static constexpr uint16_t SBUS_HIGH_THRESHOLD = 1200;
static constexpr uint16_t SBUS_PWM_MIN        =  200;
static constexpr uint16_t SBUS_PWM_MAX        = 1850;

// ======== PWM-Wert-Quellengrenze ================================
// pwm_wert[x] >= PWM_CHANNEL_OFFSET → Kanalgesteuert (Kanal = pwm_wert - Offset)
static constexpr int PWM_CHANNEL_OFFSET = 300;

// ======== MultiSwitch-Protokoll =================================
static constexpr uint8_t MWset       = 0x01;
static constexpr uint8_t MWprop      = 0x02;  // NEU v0.14: Proportional-Kanal
static constexpr uint8_t MWset4      = 0x07;
static constexpr uint8_t MWset4m     = 0x09;
static constexpr uint8_t Multiswitch = 0xA1;

// ======== PWM-Konfiguration =====================================
static constexpr int     PWM_FREQ       = 5000;
static constexpr int     PWM_RESOLUTION = 8;
static constexpr uint8_t PWM_MAX        = 255;

// ======== GPIO ==================================================
// v4.00: OutPin[]/OutPwm[], UARTs und Status-LED siehe board_config.h

// ======== Ausgangszustand =======================================
int     pwm_wert[8]      = {255, 255, 255, 255, 255, 255, 255, 255};
int     mode[8]          = {0, 0, 0, 0, 0, 0, 0, 0};
int     Ausgang_Kanal[8] = {0, 1, 2, 3, 4, 5, 6, 7};

// ======== Mehrfachkonfiguration pro Ausgang (NEU V3.10) ==========
// Bis zu 3 unabhaengige (Kanal-Quelle -> Blink/PWM)-Konfigurationen je
// Ausgang, Prioritaet Konfig 1 > Konfig 2 > Konfig 3 (siehe
// evalSlotActive()/Output()). Konfig 1 = weiterhin Ausgang_Kanal[]/mode[]/
// pwm_wert[] oben (unveraendert, bleibt auch im CRSF-LUA-Menue erreichbar).
// Konfig 2/3 sind NEU und aktuell nur ueber die Web-UI konfigurierbar;
// kanal=255 = Konfiguration ungenutzt (255 faellt in Output()/
// evalSlotActive() aus jedem gueltigen Kanal-Bereich heraus -> immer aus,
// ausser der zugehoerige PWM-Wert steht auf MWprop 200-207).
int     Ausgang_Kanal2[8] = {255, 255, 255, 255, 255, 255, 255, 255};
int     mode2[8]          = {0, 0, 0, 0, 0, 0, 0, 0};
int     pwm_wert2[8]      = {255, 255, 255, 255, 255, 255, 255, 255};
int     Ausgang_Kanal3[8] = {255, 255, 255, 255, 255, 255, 255, 255};
int     mode3[8]          = {0, 0, 0, 0, 0, 0, 0, 0};
int     pwm_wert3[8]      = {255, 255, 255, 255, 255, 255, 255, 255};
char    Ausgang_Name[8][17] = {
    "Ausgang 1","Ausgang 2","Ausgang 3","Ausgang 4",
    "Ausgang 5","Ausgang 6","Ausgang 7","Ausgang 8"
};
bool    Ausgang[8]       = {};

unsigned long previousTimeLED[8] = {};
bool          blinkOn[8]          = {};

// ======== RC-Bus ================================================
int  RC_System_boot = 0;

bfs::SbusRx   sbus_rx(&UART_A_SERIAL, UART_A_RX, UART_A_TX, true); // v4.00: SBUS auf UART A (wie CRSF)
bfs::SbusData sbus_data;

CRSF crsf;

uint16_t channel_output[16] = {};
bool     BUS_OK              = false;

// ======== UART B/C: waehlbare Rolle (v4.04, ab v4.05 RC_Common) ==
// UART A bleibt fest der RC-Eingang (CRSF oder SBUS). UART B und C (nur S3)
// bekommen eine Rolle aus RC_Common/uart_roles.h (0 Aus, 1 CRSF =
// Modulkette, 2 S.Port, 3 GPS, 4 Raw) und je eine Baudrate. Gestartet
// werden sie in setup() ueber uartPortsBegin(), Aenderungen wirken nach
// einem Neustart.
uint8_t  g_port_role[2] = {UROLE_CRSF, UROLE_CRSF};   // NVS u1role/u2role
uint32_t g_port_baud[2] = {420000, 420000};           // NVS u1baud/u2baud
#if defined(UART_C_SERIAL)
static constexpr uint8_t UART_PORT_COUNT = 2;
#else
static constexpr uint8_t UART_PORT_COUNT = 1;
#endif
// Stand beim Booten (activeRole, duplicate, idle), gesetzt in setup()
UartPort g_ports[2] = {};

// Neue Rolle: Baudrate auf den Protokollwert der Rolle setzen (Raw und
// Aus behalten die bisherige)
static void setPortRole(uint8_t i, uint8_t role) {
    role = uartRoleClamp(role);
    if (i > 1 || role == g_port_role[i]) return;
    g_port_role[i] = role;
    uint32_t b = uartRoleDefaultBaud(role);
    if (b && role != UROLE_RAW) g_port_baud[i] = b;
}

// ======== CRSF-Timeout ==========================================
// v0.14: auf 2000 ms erhöht – robuster bei kurzen Unterbrechungen
static constexpr unsigned long CRSF_TIMEOUT_MS = 2000;
unsigned long lastCrsfPacket = 0;

// ======== MWprop-Speicher (NEU v0.14) ===========================
// Speichert duty-Werte (0–255) für bis zu 8 proportionale CRSF-Kanäle.
// Verwendung: pwm_wert[x] = 200..207 → wm_prop_value[pwm_wert[x]-200]
uint8_t wm_prop_value[8] = {255, 255, 255, 255, 255, 255, 255, 255};

// ======== Konfiguration (aus NVS) ===============================
int RC_System      = 0;
int einkanal_mode  = 0;
int CRSF_Channel   = 4;
int modul_adress   = 0;

uint16_t einkanal_Data       = 0;
uint16_t einkanal_SpeicherWM = 0;

// ======== MKan: Mehrfachadress-Erweiterung (NEU V3.00) ===========
// Analog zur "MKan"-Erweiterung im ESP32-RC-Sound-Projekt (firmware-v7,
// config.h/ESP32-RC-Sound.ino): zusaetzlich zur bestehenden Einzelkanal-
// Adresse oben (modul_adress/einkanal_mode - unveraendert) gibt es 2
// weitere, unabhaengige 8-Bit-Kanalquellen ("Gruppen"). Bei CRSF ueber
// frei waehlbare Gruppenadressen (EK_Gruppen_Adresse[], 255=aus), bei
// SBUS ueber fest zugewiesene SBUS-Kanaele (SBUS_Gruppen_Channel[],
// 999=aus) + eigenem Modus (SBUS_Gruppen_Mode[], gleiche Konvention wie
// einkanal_mode: 0=Normal, 10-13=WM Adr 0-3). Beide Quellen fuellen
// gemeinsam einkanal_Flat16 (16 Bit = 2x8 Bit), ausgewertet in Output()
// ueber Ausgang_Kanal 80-95.
static constexpr int MULTISW_NUM_GROUPS = 2;
uint8_t  einkanal_Gruppe[MULTISW_NUM_GROUPS]         = {0, 0};
uint16_t einkanal_Flat16                             = 0;
uint8_t  einkanal_SpeicherWM_Grp[MULTISW_NUM_GROUPS] = {0, 0};
int      EK_Gruppen_Adresse[MULTISW_NUM_GROUPS]      = {255, 255};
int      SBUS_Gruppen_Channel[MULTISW_NUM_GROUPS]    = {999, 999};
int      SBUS_Gruppen_Mode[MULTISW_NUM_GROUPS]       = {0, 0};
uint16_t Data                = 0;

// ======== WiFi ==================================================
char        g_wifi_ssid[33]     = "MultiSwitch";
char        g_wifi_pass[64]     = "123456789";
char        g_wifi_ip[16]       = "192.168.1.1";   // konfigurierbare AP-IP
const char* AP_IP_STR           = g_wifi_ip;        // Alias fuer Abwaertskompatibilitaet
char        g_device_name[24]   = "MultiSwitch";    // NEU V1.4: Geraetename im TBS Agent / LUA

// ======== WLAN-Auto-Failsafe (NEU v2.00, Default AN seit v2.01) ========
// Schaltet den Access Point automatisch ein, wenn ueber die eingestellte
// Zeit kein gueltiges RC-Signal anliegt (nutzt BUS_OK) - z.B. wenn kein
// Sender gebunden ist oder der Empfaenger fehlt, damit man trotzdem per
// WLAN ins Web-Interface kommt, auch wenn der Boot-Pin (GPIO13) gerade
// nicht erreichbar ist. Schaltet NUR ein, nie automatisch wieder aus
// (siehe wifiFailsafeCheck() weiter unten).
// Seit v2.01 Default AN - analog zum Soundmodul-Projekt (dortiges Feature
// "WLAN dauerhaft an"), ebenfalls fuer bestehende Geraete beim Update
// (siehe Migrations-Default in nvsLoad() unten: p.getBool("wauto", true)).
// WICHTIG: dieser Schalter schaltet WLAN NICHT sofort beim Booten ein,
// sondern erst nach g_wifi_auto_timeout Sekunden OHNE gueltiges RC-Signal -
// bei normal gebundenem Sender bleibt WLAN also weiterhin aus, anders als
// beim Soundmodul, das WLAN dort unabhaengig vom RC-Signal sofort startet.
bool     g_wifi_auto         = true;
uint16_t g_wifi_auto_timeout = 60;   // Sekunden, gueltiger Bereich 5-240

// WLAN dauerhaft aktiv (NEU v2.02, uebernommen aus dem Soundmodul-Projekt
// ESP32-RC-Sound v7.16, dortiges Feature "WifiAlwaysOn"): der AP wird in
// webui_init() fest eingeschaltet und danach von der Firmware nie mehr
// ausgeschaltet - der GPIO13-Bootpin (siehe webui_init() in webui.h) UND
// der Auto-Failsafe oben (g_wifi_auto/g_wifi_auto_timeout) werden dabei
// uebersprungen, siehe wifiFailsafeCheck() weiter unten. Anders als beim
// Soundmodul gibt es hier keinen Sofort-Schalter aus dem CRSF-Menue heraus -
// Feld 80 unten wirkt wie 78/79 rein persistierend, kein Aufruf von
// webui_enableAP() aus luaSetWifiAlways() heraus (siehe Kommentar dort).
// Default AN (true), auch fuer Bestandsgeraete beim ersten Boot nach einem
// Firmware-Update (siehe Migrations-Default in nvsLoad() unten:
// p.getBool("walways", true)).
bool     g_wifi_always_on    = true;

// ======== NVS ===================================================
static bool nvsDirty = false;
static bool nvsSaveScheduled = false;
static unsigned long nvsSaveDueMs = 0;
static constexpr unsigned long NVS_SAVE_DEBOUNCE_MS = 500;

// v4.05: WLAN, Moduladresse, WLAN-Failsafe und UART B/C liegen unter den
// gemeinsamen RC_Common-Schluesseln, siehe src/RC_Common/settings_common.h
static void copyStr(char* dst, const char* src, size_t len) {
    strncpy(dst, src, len - 1);
    dst[len - 1] = '\0';
}

static void commonToSettings(RcCommonSettings& s) {
    memset(&s, 0, sizeof(s));
    copyStr(s.ssid,    g_wifi_ssid,   sizeof(s.ssid));
    copyStr(s.pass,    g_wifi_pass,   sizeof(s.pass));
    copyStr(s.ip,      g_wifi_ip,     sizeof(s.ip));
    copyStr(s.devName, g_device_name, sizeof(s.devName));
    s.moduleAddr  = modul_adress;
    s.wifiAuto    = g_wifi_auto;
    s.wifiTimeout = g_wifi_auto_timeout;
    s.wifiAlways  = g_wifi_always_on;
    for (int i = 0; i < 2; i++) { s.role[i] = g_port_role[i]; s.baud[i] = g_port_baud[i]; }
}

static void settingsToCommon(const RcCommonSettings& s) {
    copyStr(g_wifi_ssid,   s.ssid,    sizeof(g_wifi_ssid));
    copyStr(g_wifi_pass,   s.pass,    sizeof(g_wifi_pass));
    copyStr(g_wifi_ip,     s.ip,      sizeof(g_wifi_ip));
    copyStr(g_device_name, s.devName, sizeof(g_device_name));
    modul_adress        = s.moduleAddr;
    g_wifi_auto         = s.wifiAuto;
    g_wifi_auto_timeout = s.wifiTimeout;
    g_wifi_always_on    = s.wifiAlways;
    for (int i = 0; i < 2; i++) { g_port_role[i] = s.role[i]; g_port_baud[i] = s.baud[i]; }
}

static void nvsWriteNow() {
    Preferences p;
    p.begin("msw", false);
    p.putInt("rc_sys",  RC_System);
    p.putInt("crsf_ch", CRSF_Channel);
    p.putInt("ek_mode", einkanal_mode);
    for (int g = 0; g < MULTISW_NUM_GROUPS; g++) {
        char key[8];
        snprintf(key, sizeof(key), "gadr%d", g); p.putInt(key, EK_Gruppen_Adresse[g]);
        snprintf(key, sizeof(key), "gch%d",  g); p.putInt(key, SBUS_Gruppen_Channel[g]);
        snprintf(key, sizeof(key), "gmod%d", g); p.putInt(key, SBUS_Gruppen_Mode[g]);
    }
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "ak%d", i); p.putInt(key, Ausgang_Kanal[i]);
        snprintf(key, sizeof(key), "pw%d", i); p.putInt(key, pwm_wert[i]);
        snprintf(key, sizeof(key), "mo%d", i); p.putInt(key, mode[i]);
        snprintf(key, sizeof(key), "nm%d", i); p.putString(key, Ausgang_Name[i]);
        // NEU V3.10: Konfig 2/3
        snprintf(key, sizeof(key), "ak2_%d", i); p.putInt(key, Ausgang_Kanal2[i]);
        snprintf(key, sizeof(key), "pw2_%d", i); p.putInt(key, pwm_wert2[i]);
        snprintf(key, sizeof(key), "mo2_%d", i); p.putInt(key, mode2[i]);
        snprintf(key, sizeof(key), "ak3_%d", i); p.putInt(key, Ausgang_Kanal3[i]);
        snprintf(key, sizeof(key), "pw3_%d", i); p.putInt(key, pwm_wert3[i]);
        snprintf(key, sizeof(key), "mo3_%d", i); p.putInt(key, mode3[i]);
    }
    // v4.05: gemeinsame Schluessel aus RC_Common (settings_common.h)
    RcCommonSettings s;
    commonToSettings(s);
    rcSettingsSave(p, s);
    p.end();
    nvsDirty = false;
    nvsSaveScheduled = false;
    Serial.println("NVS gespeichert.");
}

static void nvsSave() {
    nvsDirty = true;
    nvsSaveScheduled = true;
    nvsSaveDueMs = millis() + NVS_SAVE_DEBOUNCE_MS;
}

static void nvsFlushPending() {
    if (nvsDirty || nvsSaveScheduled) {
        nvsWriteNow();
    }
}

static void nvsProcessPending() {
    if (!nvsSaveScheduled) return;
    if ((long)(millis() - nvsSaveDueMs) >= 0) {
        nvsWriteNow();
    }
}

static void nvsLoad() {
    Preferences p;
    p.begin("msw", true);
    if (!p.isKey("rc_sys")) { p.end(); return; }
    RC_System     = p.getInt("rc_sys",  0);
    CRSF_Channel  = p.getInt("crsf_ch", 4);
    einkanal_mode = p.getInt("ek_mode", 0);
    for (int g = 0; g < MULTISW_NUM_GROUPS; g++) {
        char key[8];
        snprintf(key, sizeof(key), "gadr%d", g); EK_Gruppen_Adresse[g]   = p.getInt(key, 255);
        snprintf(key, sizeof(key), "gch%d",  g); SBUS_Gruppen_Channel[g] = p.getInt(key, 999);
        snprintf(key, sizeof(key), "gmod%d", g); SBUS_Gruppen_Mode[g]    = p.getInt(key, 0);
    }
    for (int i = 0; i < 8; i++) {
        char key[8];
        snprintf(key, sizeof(key), "ak%d", i); Ausgang_Kanal[i] = p.getInt(key, i);
        snprintf(key, sizeof(key), "pw%d", i); pwm_wert[i]      = p.getInt(key, 255);
        snprintf(key, sizeof(key), "mo%d", i); mode[i]          = p.getInt(key, 0);
        snprintf(key, sizeof(key), "nm%d", i);
        String nm = p.getString(key, Ausgang_Name[i]);
        strncpy(Ausgang_Name[i], nm.c_str(), 16); Ausgang_Name[i][16] = '\0';
        // NEU V3.10: Konfig 2/3 - fehlen die Keys (Update von vor v3.10),
        // liefert getInt() den Default 255/255/0 = Konfiguration ungenutzt.
        snprintf(key, sizeof(key), "ak2_%d", i); Ausgang_Kanal2[i] = p.getInt(key, 255);
        snprintf(key, sizeof(key), "pw2_%d", i); pwm_wert2[i]      = p.getInt(key, 255);
        snprintf(key, sizeof(key), "mo2_%d", i); mode2[i]          = p.getInt(key, 0);
        snprintf(key, sizeof(key), "ak3_%d", i); Ausgang_Kanal3[i] = p.getInt(key, 255);
        snprintf(key, sizeof(key), "pw3_%d", i); pwm_wert3[i]      = p.getInt(key, 255);
        snprintf(key, sizeof(key), "mo3_%d", i); mode3[i]          = p.getInt(key, 0);
    }
    // v4.05: gemeinsame Schluessel (RC_Common). Fehlen sie (erster Start
    // nach dem Update), werden die alten Schluessel einmalig uebernommen.
    // Defaults wie bisher: WLAN-Auto-Failsafe und "dauerhaft an" AN (siehe
    // Kommentare bei g_wifi_auto/g_wifi_always_on), UART B/C CRSF.
    RcCommonSettings s;
    commonToSettings(s);
    bool migrated = rcSettingsLoad(p, s, nullptr);
    settingsToCommon(s);
    p.end();
    nvsDirty = false;
    nvsSaveScheduled = false;
    Serial.println("NVS geladen.");
    if (migrated) {
        nvsWriteNow();
        Serial.println("NVS: Einstellungen auf die RC_Common-Schluessel uebernommen.");
    }
}

static void nvsReset() {
    RC_System = 0; CRSF_Channel = 4; einkanal_mode = 0; modul_adress = 0;
    for (int g = 0; g < MULTISW_NUM_GROUPS; g++) {
        EK_Gruppen_Adresse[g] = 255; SBUS_Gruppen_Channel[g] = 999; SBUS_Gruppen_Mode[g] = 0;
    }
    for (int i = 0; i < 8; i++) {
        Ausgang_Kanal[i] = i; pwm_wert[i] = 255; mode[i] = 0;
        snprintf(Ausgang_Name[i], 17, "Ausgang %d", i + 1);
        // NEU V3.10: Konfig 2/3 zuruecksetzen (ungenutzt)
        Ausgang_Kanal2[i] = 255; pwm_wert2[i] = 255; mode2[i] = 0;
        Ausgang_Kanal3[i] = 255; pwm_wert3[i] = 255; mode3[i] = 0;
    }
    strncpy(g_wifi_ssid,   "MultiSwitch", sizeof(g_wifi_ssid)-1);
    strncpy(g_wifi_pass,   "123456789",   sizeof(g_wifi_pass)-1);
    strncpy(g_wifi_ip,     "192.168.1.1", sizeof(g_wifi_ip)-1);
    strncpy(g_device_name, "MultiSwitch", sizeof(g_device_name)-1);  // NEU V1.4
    g_wifi_auto = true; g_wifi_auto_timeout = 60;    // NEU v2.00, Default AN seit v2.01
    g_wifi_always_on = true;                         // NEU v2.02
    for (int i = 0; i < 2; i++) { g_port_role[i] = UROLE_CRSF; g_port_baud[i] = uartRoleDefaultBaud(UROLE_CRSF); }  // v4.05
    nvsSave();
    nvsFlushPending();
}

void storageSave() { nvsFlushPending(); }

// ======== Status-LED ============================================
// v4.00: Muster nach Firmware-Spezifikation Kapitel 6, ab v4.05 aus
// RC_Common (status_led.h): aus = kein Signal, langsam = OK, schnell =
// Frames ohne RC-Link bzw. Failsafe, Doppelblitz = Konfigurationsfehler.
// Reine Anzeige - aendert weder BUS_OK noch das Ausgangsverhalten.

// Konfigurationsfehler (Doppelblitz). v4.01: gesetzt, wenn der Router eine
// doppelte Moduladresse erkennt (siehe loop()).
bool g_ledConfigError = false;
static unsigned long lastSbusFrameMs = 0;
static StatusLed statusLed;   // S3 WS2812 gruen, C3 ohne LED (Pin -1)

static void updateLed() {
    unsigned long now = millis();
    const LinkStatistics& ls = crsf.getLinkStatistics();
    StatusLedInput in = {};
    in.configError     = g_ledConfigError;
    in.crsf            = (RC_System_boot == 4);
    in.busOk           = BUS_OK;
    in.lastRcFrameMs   = crsf.getLastRcFrameMs();
    in.linkStatsMs     = ls.lastUpdate;
    in.uplinkLQ        = ls.uplinkLQ;
    in.lastSbusFrameMs = lastSbusFrameMs;
    in.sbusFailsafe    = sbus_data.failsafe;
    statusLed.update(statusLedMode(in, now), now);
}

// ======== CRSF-Failsafe =========================================
// Nur BUS_OK setzen – kein ledcWrite() hier!
// Output() schaltet Ausgänge kontrolliert ab.
static void checkCrsfTimeout() {
    if (RC_System_boot != 4) return;
    if (millis() - lastCrsfPacket > CRSF_TIMEOUT_MS) {
        BUS_OK = false;
    }
}

// ======== SBUS-Zeitlimit (NEU v4.03) ============================
// Ohne SBUS-Frames (nicht nur Failsafe-Bit) gilt das Signal nach 10 s als
// weg. lastSbusFrameMs wird beim Empfang jedes Frames gesetzt (loop()).
static constexpr unsigned long SBUS_TIMEOUT_MS = 10000;
static void checkSbusTimeout() {
    if (RC_System_boot == 4) return;
    if (lastSbusFrameMs == 0 || millis() - lastSbusFrameMs > SBUS_TIMEOUT_MS) {
        BUS_OK = false;
    }
}

// ======== MultiSwitch-Decoder ===================================
static uint8_t compressSwitches(uint16_t state) {
    uint8_t result = 0;
    for (uint8_t i = 0; i < 8; i++) result |= ((state >> (i * 2)) & 0x1) << i;
    return result;
}

// MKan (NEU V3.00): prueft eine eingehende Multiswitch-Adresse zusaetzlich
// gegen die 2 frei konfigurierbaren Gruppenadressen (unabhaengig von
// modul_adress/einkanal_Data oben). Trifft eine davon zu, landet der
// 8-Bit-Wert in einkanal_Gruppe[g] und wird zu einkanal_Flat16 kombiniert -
// Grundlage fuer Ausgang_Kanal 80-95 in Output(). 1:1 uebernommen aus
// updateGroupFromAddress() im ESP32-RC-Sound-Projekt (firmware-v7).
static void updateGroupFromAddress(uint8_t addr, uint8_t bits) {
    for (uint8_t g = 0; g < MULTISW_NUM_GROUPS; g++) {
        if (EK_Gruppen_Adresse[g] != 255 && addr == (uint8_t)EK_Gruppen_Adresse[g]) {
            einkanal_Gruppe[g] = bits;
        }
    }
    // Bewusst per Hand aufgezaehlt (2 Gruppen) statt generisch geloopt -
    // siehe Kommentar bei MULTISW_NUM_GROUPS.
    einkanal_Flat16 = (uint16_t)einkanal_Gruppe[0]
                     | ((uint16_t)einkanal_Gruppe[1] << 8);
}

static void einkanalFunctionCRSF() {
    uint8_t WMcode  = crsf.get_cmd_buffer(5);
    uint8_t command = crsf.get_cmd_buffer(6);
    if (WMcode != Multiswitch) return;
    switch (command) {
        case MWset4: {
            uint8_t address = crsf.get_cmd_buffer(7);
            uint16_t state = ((uint16_t)crsf.get_cmd_buffer(8) << 8)
                           |             crsf.get_cmd_buffer(9);
            uint8_t bits = compressSwitches(state);
            if (address == (uint8_t)modul_adress) einkanal_Data = bits;
            updateGroupFromAddress(address, bits);  // NEU V3.00: MKan
            break;
        }
        case MWset4m: {
            uint8_t count = min((uint8_t)crsf.get_cmd_buffer(7), (uint8_t)7);
            for (uint8_t i = 0; i < count; i++) {
                uint8_t address = crsf.get_cmd_buffer(8 + (3 * i));
                uint16_t state = ((uint16_t)crsf.get_cmd_buffer(9  + (3*i)) << 8)
                               |             crsf.get_cmd_buffer(10 + (3*i));
                uint8_t bits = compressSwitches(state);
                if (address == (uint8_t)modul_adress) einkanal_Data = bits;
                updateGroupFromAddress(address, bits);  // NEU V3.00: MKan
            }
            break;
        }
        case MWset: {
            uint8_t address = crsf.get_cmd_buffer(7);
            if (address == (uint8_t)modul_adress)
                einkanal_Data = crsf.get_cmd_buffer(8);
            break;
        }
        // NEU v0.14: MWprop – setzt duty-Wert für proportionale PWM-Ausgänge
        case MWprop: {
            uint8_t address = crsf.get_cmd_buffer(7);
            if (address == (uint8_t)modul_adress) {
                uint8_t channel = crsf.get_cmd_buffer(8);
                uint8_t duty    = crsf.get_cmd_buffer(9); // 0-100%
                if (channel < 8) {
                    // MWprop sendet Prozent (0-100), PWM braucht 0-255
                    wm_prop_value[channel] = (uint8_t)((uint16_t)duty * 255 / 100);
                }
            }
            break;
        }
    }
}

// ======== SBUS-Einkanal-Decoder =================================
static void einkanalFunctionSBUS(uint16_t channel) {
    einkanal_Data = channel;
    if (einkanal_mode == 0) {
        switch (RC_System) {
            case 0: einkanal_Data /= 8; break;
            case 1:
                einkanal_Data = constrain(einkanal_Data, 206, 1837);
                einkanal_Data = ((einkanal_Data - 206) * 10 + 20) / 64;
                break;
            case 2: {
                float v = ((float)einkanal_Data - 172.0f + 1.5f) * 0.155677655677655f;
                einkanal_Data = (uint16_t)v;
                break;
            }
        }
    } else if (einkanal_mode >= 10) {
        uint16_t n = 0;
        switch (RC_System) {
            case 0: n = (channel >= 172) ? (channel - 172 + 1) : 0; break;
            case 1: n = (channel >= 220) ? (channel - 220)     : 0; n += (n >> 6); break;
            case 2: n = (channel >= 172) ? (channel - 172)     : 0; break;
            case 3: n = (channel >= 205) ? (channel - 205)     : 0; break;
        }
        uint8_t v       = (uint8_t)(n >> 4);
        uint8_t address = (v >> 4) & 0b11;
        uint8_t sw      = (v >> 1) & 0b111;
        uint8_t state   = v & 0b1;
        if (address == (uint8_t)(einkanal_mode - 10))
            bitWrite(einkanal_SpeicherWM, sw, state);
        einkanal_Data = einkanal_SpeicherWM;
    }
}

// ======== SBUS-MKan-Gruppen-Decoder (NEU V3.00) ==================
// SBUS-Pendant zu updateGroupFromAddress(): SBUS kennt keine adressierten
// Pakete, daher braucht jede MKan-Gruppe hier einen eigenen, fest
// zugewiesenen SBUS-Kanal (SBUS_Gruppen_Channel[g]) statt einer frei
// waehlbaren Busadresse. SBUS_Gruppen_Mode[g] uebernimmt dieselbe
// Werte-/Bedeutungskonvention wie einkanal_mode oben:
//   0     = "Normal"  - kompletter 8-Bit-Kanalwert (ch/8) direkt uebernehmen
//   10-13 = "WM Adr 0..3" - amplituden-codierter WM-Modus, bitweise
//           akkumuliert (wie einkanal_mode>=10 oben) - bewusst als
//           eigenstaendige Funktion gehalten statt mit einkanalFunctionSBUS()
//           zusammengelegt, um die bestehende, funktionierende Einzelkanal-
//           Auswertung nicht anzufassen.
// Ergebnis landet in einkanal_Gruppe[g] und wird wie bei
// updateGroupFromAddress() zu einkanal_Flat16 kombiniert.
static void einkanalFunctionSBUSGroup(uint8_t g, uint16_t channel) {
    if (SBUS_Gruppen_Mode[g] == 0) {
        einkanal_Gruppe[g] = (uint8_t)(channel / 8);
    } else {
        uint16_t n = 0;
        switch (RC_System) {
            case 0: n = (channel >= 172) ? (channel - 172 + 1) : 0; break;
            case 1: n = (channel >= 220) ? (channel - 220)     : 0; n += (n >> 6); break;
            case 2: n = (channel >= 172) ? (channel - 172)     : 0; break;
            case 3: n = (channel >= 205) ? (channel - 205)     : 0; break;
        }
        uint8_t v       = (uint8_t)(n >> 4);
        uint8_t address = (v >> 4) & 0b11;
        uint8_t sw      = (v >> 1) & 0b111;
        uint8_t state   = v & 0b1;
        if (address == (uint8_t)(SBUS_Gruppen_Mode[g] - 10))
            bitWrite(einkanal_SpeicherWM_Grp[g], sw, state);
        einkanal_Gruppe[g] = einkanal_SpeicherWM_Grp[g];
    }
    einkanal_Flat16 = (uint16_t)einkanal_Gruppe[0]
                     | ((uint16_t)einkanal_Gruppe[1] << 8);
}

// ======== CRSF LUA Parameter-System =====================================
// Parameterstruktur (nach rcmultiswitchG030 Referenz):
//   0:  Root Folder, 1: Version Info, 2: Folder "Global"
//   3: Switch Addr, 4: CRSF Kanal
//   5+x*5+0..4: Folder + 4 Parameter pro Ausgang (x=0..7)
// ======== CRSF LUA / TBS Agent Parameter-System ========================
//
// Pro Ausgang (9 Eintraege inkl. Folder):
//   fi+0: Folder
//   fi+1: Sel "Ausgang-Quelle"  Einzelkanal;Kanal_L;Kanal_H
//   fi+2: U8  "Kanal Nr"        1-8 (Einzelkanal) oder 1-16 (Kanal_L/H)
//   fi+3: Sel "Blink"           Dauerlicht;Blinken
//   fi+4: U8  "Blink AN *10ms"  1-250
//   fi+5: U8  "Blink AUS *10ms" 1-250
//   fi+6: Sel "PWM Modus"       Festwert;MWprop (nur CRSF)
//   fi+7: U8  "PWM Festwert"    0-255
//   fi+8: Sel "Test"            Aus;Ein
// Global: 0=Root,1=Version,2=Folder,3=Switch Addr,4=CRSF Kanal
// WLAN (NEU v2.02, uebernommen aus dem Soundmodul-Projekt, dortiges
// CRSF/Lua-Menue fuer WLAN-Einstellungen, Felder 174-181): eigener
// Ordner 77 mit Kindern 78-80, an die Root angehaengt statt bestehende
// IDs 0-76 umzunummerieren, damit bestehende Radio-Profile/Lua-Skripte
// mit festen Feld-IDs weiterlaufen (gleiches Vorgehen wie beim
// Soundmodul beim Anhaengen von Feld 181 an Ordner 160).
//   77: Folder "WLAN"         (Kinder 78,79,80)
//   78: Sel  "Auto-Failsafe"  Aus;Ein  -> g_wifi_auto
//   79: U8   "Auto Timeout"   5-240 s  -> g_wifi_auto_timeout
//   80: Sel  "Dauerhaft an"   Aus;Ein  -> g_wifi_always_on
// Total: 5+8*9+4=81 Parameter (0..80)
// UART B/C (NEU v4.04), ebenfalls angehaengt:
//   81: Folder "UART B/C"     (Kinder 82,83,84)
//   82: Sel  "UART B"         Aus;CRSF;S.Port;GPS;Raw -> g_port_role[0]
//   83: Sel  "UART C"         wie 82 (S3), Info beim C3 -> g_port_role[1]
//   84: Sel  "UART B Baud"    Auto;9600..420000 (v4.05, vorher "GPS Baud")
//   85: Sel  "UART C Baud"    wie 84 (S3), Info beim C3
// Total: 86 Parameter (0..85), Tabelle siehe buildLuaMenu()

// ── CRSF-Geraeteadresse + Ping-Slot aus der WM-Adresse (uebernommen aus Soundmodul v1.24) ──
// Adresse = 0xC0 + modul_adress; Slot = (Adresse-0xC0)*2; Antwort erst im eigenen
// Zeit-Slot, damit sich die Geraeteantworten mehrerer Module nicht ueberlappen.
#define CRSF_SLOT_MS 5
static inline uint8_t  crsfAddrFromWM()  { return 0xC0 + (uint8_t)constrain(modul_adress, 0, 15); }
static inline uint16_t crsfSlotDelayMs() { return (uint16_t)((uint8_t)constrain(modul_adress, 0, 15) * 2) * CRSF_SLOT_MS; }

// Ausgang_Kanal[x] -> Quelle (0=Einzelkanal,1=Kanal_L,2=Kanal_H,3=MKan1,4=MKan2)
// + Kanalnummer (1-basiert). MKan1/MKan2 (NEU V3.00) nutzen Ausgang_Kanal
// 80-87/88-95 (siehe Output()); die Gruppenadressen/-kanaele dafuer werden
// aktuell NUR ueber die Web-UI konfiguriert (EK_Gruppen_Adresse[]/
// SBUS_Gruppen_Channel[]/SBUS_Gruppen_Mode[]), nicht ueber dieses Lua-Menue.
static uint8_t getKanalQuelle(int x) {
    int k = Ausgang_Kanal[x];
    if (k < 20) return 0;
    if (k < 40) return 1;
    if (k < 60) return 2;
    if (k < 88) return 3;
    return 4;
}
static uint8_t getKanalNr(int x) {
    int k = Ausgang_Kanal[x];
    if (k < 20) return k + 1;       // Einzelkanal: Bit 1-8
    if (k < 40) return k - 20 + 1; // Kanal_L: Kanal 1-16
    if (k < 60) return k - 40 + 1; // Kanal_H: Kanal 1-16
    if (k < 88) return k - 80 + 1; // MKan1: Bit 1-8
    return k - 88 + 1;              // MKan2: Bit 1-8
}
static void setKanalQuelleNr(int x, uint8_t quelle, uint8_t nr) {
    uint8_t idx = (nr > 0) ? nr - 1 : 0; // 0-basiert
    if (quelle == 0) Ausgang_Kanal[x] = min(idx, (uint8_t)7);  // Einzelkanal: 0-7
    else if (quelle == 1) Ausgang_Kanal[x] = 20 + min(idx, (uint8_t)15); // Kanal_L
    else if (quelle == 2) Ausgang_Kanal[x] = 40 + min(idx, (uint8_t)15); // Kanal_H
    else if (quelle == 3) Ausgang_Kanal[x] = 80 + min(idx, (uint8_t)7);  // MKan1: 80-87
    else Ausgang_Kanal[x] = 88 + min(idx, (uint8_t)7);                  // MKan2: 88-95
}

// PWM-Modus: 0=Festwert, 1=MWprop
static uint8_t getPwmModus(int x) {
    return (pwm_wert[x] >= 200 && pwm_wert[x] <= 207) ? 1 : 0;
}

// ── Lua-Menue als Tabelle (v4.05, RC_Common lua_menu.h) ──────────────
// Jeder Eintrag steht einmal in buildLuaMenu(), Senden und Schreiben
// laufen generisch ueber luaMenu. Lese-/Schreibfunktionen bekommen als
// Argument die Ausgang- bzw. UART-Nummer. Schreiben ist auf min/max der
// gerade gesendeten Ansicht begrenzt.
static LuaItem luaItems[96];
static LuaMenu luaMenu(luaItems, 96);
static char    luaOutName[8][12];   // "Ausgang 1".."Ausgang 8"

static uint8_t luaGetAddr(uint8_t)              { return (uint8_t)modul_adress; }
static void    luaSetAddr(uint8_t, uint8_t v)   { modul_adress = v; crsf.setDeviceAddress(crsfAddrFromWM()); nvsSave(); }
static uint8_t luaGetCrsfCh(uint8_t)            { return (uint8_t)CRSF_Channel; }
static void    luaSetCrsfCh(uint8_t, uint8_t v) { CRSF_Channel = v; nvsSave(); }

static uint8_t luaGetQuelle(uint8_t x)            { return getKanalQuelle(x); }
static void    luaSetQuelle(uint8_t x, uint8_t v) { setKanalQuelleNr(x, v, getKanalNr(x)); nvsSave(); }  // Nr bleibt
static uint8_t luaGetNr(uint8_t x)                { return getKanalNr(x); }
static void    luaSetNr(uint8_t x, uint8_t v)     { setKanalQuelleNr(x, getKanalQuelle(x), v); nvsSave(); }
static void    luaViewNr(uint8_t x, LuaView& v, char*) {
    uint8_t q = getKanalQuelle(x);
    bool rc = (q == 1 || q == 2);   // Kanal_L/Kanal_H: RC-Kanal 1-16, sonst Bit 1-8
    v.name = rc ? "RC-Kanal Nr" : "Bit Nr";
    v.max  = rc ? 16 : 8;
}

static uint8_t luaGetBlink(uint8_t x) { return ((mode[x] >> 8) & 0xFF) ? 1 : 0; }
static void    luaSetBlink(uint8_t x, uint8_t v) {
    if (v == 0) {
        mode[x] = 0;
    } else {
        uint8_t mH = (mode[x] >> 8) & 0xFF;
        uint8_t mL =  mode[x]        & 0xFF;
        if (!mH) mH = 10;
        if (!mL) mL = 10;
        mode[x] = ((uint16_t)mH << 8) | mL;
    }
    nvsSave();
}
static uint8_t luaGetBlinkOn(uint8_t x)  { uint8_t mH = (mode[x] >> 8) & 0xFF; return mH ? mH : 10; }
static void    luaSetBlinkOn(uint8_t x, uint8_t v)  { mode[x] = ((uint16_t)v << 8) | (mode[x] & 0xFF); nvsSave(); }
static uint8_t luaGetBlinkOff(uint8_t x) { uint8_t mL = mode[x] & 0xFF; return mL ? mL : 10; }
static void    luaSetBlinkOff(uint8_t x, uint8_t v) { mode[x] = (mode[x] & 0xFF00) | v; nvsSave(); }

static uint8_t luaGetPwmModus(uint8_t x) { return getPwmModus(x); }
static void    luaSetPwmModus(uint8_t x, uint8_t v) {
    if (v == 1) {
        pwm_wert[x] = 200 + x;        // MWprop
        einkanal_Data |= (1 << x);    // Ausgang einschalten
    } else if (getPwmModus(x) == 1) {
        pwm_wert[x] = 255;            // zurueck auf Festwert
    }
    nvsSave();
}
static void luaViewPwmModus(uint8_t, LuaView& v, char*) {
    if (RC_System_boot != 4) { v.type = LUA_INFO; v.text = "Festwert (SBUS)"; }   // MWprop nur bei CRSF
}
static uint8_t luaGetPwm(uint8_t x)            { return (uint8_t)pwm_wert[x]; }
static void    luaSetPwm(uint8_t x, uint8_t v) { pwm_wert[x] = v; nvsSave(); }
static void    luaViewPwm(uint8_t x, LuaView& v, char* buf) {
    if (getPwmModus(x) != 1) return;
    snprintf(buf, LUA_VIEW_BUF, "MWprop Kanal %d (auto)", x + 1);
    v.type = LUA_INFO; v.name = "PWM Wert"; v.text = buf;
}
static uint8_t luaGetTest(uint8_t)             { return 0; }
static void    luaSetTest(uint8_t x, uint8_t v) {
    if (v) einkanal_Data |=  (1 << x);
    else   einkanal_Data &= ~(1 << x);
}

// WLAN 78-80: rein persistierend, wirkt beim naechsten Timeout bzw. Booten
// (kein WLAN-Start aus dem CRSF-Handler, Lehre aus dem Soundmodul v7.15)
static uint8_t luaGetWifiAuto(uint8_t)              { return g_wifi_auto ? 1 : 0; }
static void    luaSetWifiAuto(uint8_t, uint8_t v)   { g_wifi_auto = (v != 0); nvsSave(); }
static uint8_t luaGetWifiTo(uint8_t)                { return (uint8_t)constrain(g_wifi_auto_timeout, 5, 240); }
static void    luaSetWifiTo(uint8_t, uint8_t v)     { g_wifi_auto_timeout = v; nvsSave(); }
static uint8_t luaGetWifiAlways(uint8_t)            { return g_wifi_always_on ? 1 : 0; }
static void    luaSetWifiAlways(uint8_t, uint8_t v) { g_wifi_always_on = (v != 0); nvsSave(); }

// UART B/C 82-85: wirken nach einem Neustart (UARTs starten in setup())
static uint8_t luaGetRole(uint8_t i)            { return g_port_role[i]; }
static void    luaSetRole(uint8_t i, uint8_t v) { setPortRole(i, v); nvsSave(); }
static uint8_t luaGetBaud(uint8_t i)            { return uartBaudToIndex(g_port_baud[i]); }
static void    luaSetBaud(uint8_t i, uint8_t v) { g_port_baud[i] = uartBaudFromIndex(v); nvsSave(); }

static void buildLuaMenu() {
    LuaMenu& m = luaMenu;
    m.clear();
    m.folder(0, 0, "");
    m.info(1, 0, "Version", "v4.05 " BOARD_NAME);
    m.folder(2, 0, "Global");
    m.u8(3, 2, "Switch Addr", 0, 20, "", luaGetAddr, luaSetAddr);
    m.u8(4, 2, "CRSF Kanal", 0, 15, "", luaGetCrsfCh, luaSetCrsfCh);
    for (uint8_t x = 0; x < 8; x++) {
        uint8_t fi = 5 + x * 9;
        snprintf(luaOutName[x], sizeof(luaOutName[x]), "Ausgang %d", x + 1);
        m.folder(fi, 0, luaOutName[x]);
        m.select(fi + 1, fi, "Ausgang-Quelle", "Einzelkanal;Kanal_L;Kanal_H;MKan1;MKan2",
                 luaGetQuelle, luaSetQuelle, x);
        m.u8(fi + 2, fi, "Bit Nr", 1, 8, "", luaGetNr, luaSetNr, x)->view = luaViewNr;
        m.select(fi + 3, fi, "Blink", "Dauerlicht;Blinken", luaGetBlink, luaSetBlink, x);
        m.u8(fi + 4, fi, "Blink AN *10ms", 1, 250, "", luaGetBlinkOn, luaSetBlinkOn, x);
        m.u8(fi + 5, fi, "Blink AUS *10ms", 1, 250, "", luaGetBlinkOff, luaSetBlinkOff, x);
        m.select(fi + 6, fi, "PWM Modus", "Festwert;MWprop (Kanal=Ausgang)",
                 luaGetPwmModus, luaSetPwmModus, x)->view = luaViewPwmModus;
        m.u8(fi + 7, fi, "PWM Festwert", 0, 255, "", luaGetPwm, luaSetPwm, x)->view = luaViewPwm;
        m.select(fi + 8, fi, "Test", "Aus;Ein", luaGetTest, luaSetTest, x);
    }
    m.folder(77, 0, "WLAN");
    m.select(78, 77, "Auto-Failsafe", "Aus;Ein", luaGetWifiAuto, luaSetWifiAuto);
    m.u8(79, 77, "Auto Timeout", 5, 240, "s", luaGetWifiTo, luaSetWifiTo);
    m.select(80, 77, "Dauerhaft an", "Aus;Ein", luaGetWifiAlways, luaSetWifiAlways);
    m.folder(81, 0, "UART B/C");
    m.select(82, 81, "UART B (Neustart)", uartRoleOptions(), luaGetRole, luaSetRole, 0);
#if defined(UART_C_SERIAL)
    m.select(83, 81, "UART C (Neustart)", uartRoleOptions(), luaGetRole, luaSetRole, 1);
#else
    m.info(83, 81, "UART C", "nicht vorhanden (C3)");
#endif
    m.select(84, 81, "UART B Baud", uartBaudOptions(), luaGetBaud, luaSetBaud, 0);
#if defined(UART_C_SERIAL)
    m.select(85, 81, "UART C Baud", uartBaudOptions(), luaGetBaud, luaSetBaud, 1);
#else
    m.info(85, 81, "UART C Baud", "nicht vorhanden (C3)");
#endif
}

// ======== Webinterface ==========================================





#include "webui.h"

// ======== WLAN-Auto-Failsafe (NEU v2.00) ========================
// Schaltet NUR ein, nie automatisch wieder aus: einmal aktiviertes WLAN
// bleibt an bis zum manuellen Ausschalten (Web) oder Neustart - ein
// automatisches Wieder-Abschalten wuerde eine gerade laufende Konfiguration
// mitten drin abwuergen, sobald zufaellig kurz ein gueltiges RC-Signal
// hereinkaeme. g_wifi_auto/g_wifi_auto_timeout siehe weiter oben, webui_
// isApActive()/webui_enableAP() in webui.h.
static WifiFailsafe wifiFailsafe;   // RC_Common wifi_failsafe.h (v4.05)
static void wifiFailsafeCheck() {
    // "Dauerhaft an": AP laeuft seit dem Booten (webui_init()), der Timer
    // ist dann wirkungslos. Laeuft der AP schon, ist nichts zu tun.
    if (wifiFailsafe.check(millis(), g_wifi_always_on, g_wifi_auto, g_wifi_auto_timeout,
                           BUS_OK, webui_isApActive())) {
        Serial.printf("Auto-WLAN-Failsafe: %u s ohne gueltiges RC-Signal - schalte WLAN ein.\n",
                      (unsigned)g_wifi_auto_timeout);
        webui_enableAP();
    }
}

// ======== BOOT-Taster (NEU v4.01) ===============================
// Langer Druck (>= 3 s) schaltet den WLAN-AP um, wirkt sofort und ohne
// nvsSave() - wie der Sofort-Schalter in der Web-UI. Der Taster ist ein
// Strapping-Pin (beim Reset gedrueckt = Download-Modus). Deshalb wird er
// erst ausgewertet, nachdem er nach dem Booten einmal losgelassen war.
static constexpr unsigned long BOOT_LONG_PRESS_MS = 3000;
static void bootButtonCheck() {
    static bool          armed   = false;
    static bool          pressed = false;
    static bool          fired   = false;
    static unsigned long downMs  = 0;

    bool down = (digitalRead(BOOT_PIN) == LOW);
    if (!armed) { if (!down) armed = true; return; }
    if (!down) { pressed = false; return; }
    unsigned long now = millis();
    if (!pressed) { pressed = true; fired = false; downMs = now; return; }
    if (fired || now - downMs < BOOT_LONG_PRESS_MS) return;
    fired = true;   // nur einmal je Druck
    if (webui_isApActive()) {
        Serial.println("BOOT-Taster: WLAN-AP aus");
        webui_disableAP();
    } else {
        Serial.println("BOOT-Taster: WLAN-AP ein");
        webui_enableAP();
    }
}

// ======== PWM-Duty aus einem PWM-Wert ermitteln (NEU v0.14) ======
// Unterstützt drei Quellen:
//   pv < 200                → Festwert (direkt als uint8_t)
//   pv 200..207             → wm_prop_value[pv-200] (MWprop)
//   pv >= PWM_CHANNEL_OFFSET → Kanalgesteuert (SBUS/CRSF-Kanal)
// NEU V3.10: nimmt den PWM-Wert direkt entgegen statt intern pwm_wert[x] zu
// lesen - noetig, weil je nach aktiver Konfiguration (1/2/3, siehe Output())
// pwm_wert[x]/pwm_wert2[x]/pwm_wert3[x] gelten kann.
static uint8_t getPwmDuty(int pv) {
    if (pv >= PWM_CHANNEL_OFFSET) {
        // Kanal-gemappter PWM-Wert
        return (uint8_t)map(channel_output[pv - PWM_CHANNEL_OFFSET],
                            SBUS_PWM_MIN, SBUS_PWM_MAX, 0, PWM_MAX);
    } else if (pv >= 200 && pv <= 207) {
        // MWprop-Kanal (NEU v0.14)
        return wm_prop_value[pv - 200];
    } else {
        return (uint8_t)pv;
    }
}

// ======== Kanal-Quelle auswerten (NEU V3.10) =====================
// Prueft, ob eine gegebene (Kanal, PWM-Wert)-Konfiguration aktiv ist -
// dieselbe Logik, die vorher direkt inline in Output() stand, jetzt als
// Helfer, damit sie fuer bis zu 3 Konfigurationen pro Ausgang wiederverwendet
// werden kann. k=255 (Konfiguration ungenutzt) faellt aus jedem gueltigen
// Bereich heraus -> liefert false, ausser MWprop (pv 200-207) ist gesetzt.
static bool evalSlotActive(int k, int pv, uint16_t d) {
    if (pv >= 200 && pv <= 207) return true;  // MWprop: Ausgang immer an
    if (k < 20)                 return bitRead(d, k);
    if (k >= 20 && k < 40)      return (channel_output[k-20] < SBUS_LOW_THRESHOLD);
    if (k >= 40 && k < 60)      return (channel_output[k-40] > SBUS_HIGH_THRESHOLD);
    if (k >= 80 && k <= 95)     return bitRead(einkanal_Flat16, k - 80);  // MKan (V3.00)
    return false;
}

// ======== Ausgang schreiben (NEU v4.00) ==========================
// Einziger Ort mit ledcWrite() auf OutPin[], nur aus Output() aufgerufen.
// Ausgaenge ohne LEDC-Kanal (C3: Ausgang 7/8, OutPwm[x]==false) schalten
// digital: an ab 50 % Duty.
static void outWrite(int x, uint8_t duty) {
    if (OutPwm[x]) ledcWrite(OutPin[x], duty);
    else           digitalWrite(OutPin[x], duty >= 128 ? HIGH : LOW);
}

// ======== Output ================================================
// Zentrale Ausgabefunktion – EINZIGER Aufrufer von outWrite().
static void Output(uint16_t d) {
    unsigned long now = millis();
    for (int x = 0; x < 8; x++) {
        // NEU V3.10: bis zu 3 Konfigurationen je Ausgang, Prioritaet 1>2>3.
        // activeSlot merkt sich, welche Konfiguration gerade Ausgang[x]
        // bestimmt hat - massgeblich fuer Blink/PWM weiter unten. Default 0
        // (Konfig 1) fuer den manuellen Override, der wie bisher immer
        // mode[x]/pwm_wert[x] nutzt.
        int activeSlot = 0;

        if (g_manual_override[x]) {
            bool newState = g_manual_state[x];
            if (newState && !Ausgang[x]) {
                previousTimeLED[x] = now - 9999;
                blinkOn[x] = false;
            }
            Ausgang[x] = newState;

        } else if (!BUS_OK) {
            Ausgang[x] = false;

        } else if (evalSlotActive(Ausgang_Kanal[x], pwm_wert[x], d)) {
            Ausgang[x] = true;  activeSlot = 0;  // Konfig 1
        } else if (evalSlotActive(Ausgang_Kanal2[x], pwm_wert2[x], d)) {
            Ausgang[x] = true;  activeSlot = 1;  // Konfig 2 (NEU V3.10)
        } else if (evalSlotActive(Ausgang_Kanal3[x], pwm_wert3[x], d)) {
            Ausgang[x] = true;  activeSlot = 2;  // Konfig 3 (NEU V3.10)
        } else {
            Ausgang[x] = false;
        }

        if (Ausgang[x]) {
            int curMode, curPwm;
            switch (activeSlot) {
                case 1:  curMode = mode2[x]; curPwm = pwm_wert2[x]; break;
                case 2:  curMode = mode3[x]; curPwm = pwm_wert3[x]; break;
                default: curMode = mode[x];  curPwm = pwm_wert[x];  break;
            }
            int modeH = (curMode >> 8) & 0xFF;
            int modeL =  curMode        & 0xFF;
            if (modeH > 0 && modeL == 0) modeL = modeH;

            uint8_t duty = getPwmDuty(curPwm);

            if (modeH > 0) {
                if (!blinkOn[x]) {
                    if (now - previousTimeLED[x] >= (unsigned long)(10 * modeL)) {
                        previousTimeLED[x] = now;
                        blinkOn[x] = true;
                        outWrite(x, duty);
                    }
                } else {
                    if (now - previousTimeLED[x] >= (unsigned long)(10 * modeH)) {
                        previousTimeLED[x] = now;
                        blinkOn[x] = false;
                        outWrite(x, 0);
                    }
                }
            } else {
                blinkOn[x] = true;
                outWrite(x, duty);
            }
        } else {
            blinkOn[x] = false;
            outWrite(x, 0);
        }
    }
}

// ======== Setup =================================================
void setup() {
    Serial.begin(115200);
    nvsLoad();

    RC_System_boot = RC_System;

    if (RC_System_boot == 4) {
        // v4.00: CRSF auf UART A (UART0), Pins siehe board_config.h
        crsf.init_crsf(&UART_A_SERIAL, UART_A_RX, UART_A_TX);
        crsf.setDeviceAddress(crsfAddrFromWM());   // eindeutige CRSF-Adresse aus WM-Adresse
        Serial.printf("CRSF gestartet (Board %s, RX=%d, TX=%d, 420000 Bd)  Geraeteadresse 0x%02X  Slot %d\n",
                      BOARD_NAME, UART_A_RX, UART_A_TX,
                      crsfAddrFromWM(), (uint8_t)constrain(modul_adress,0,15)*2);
    } else {
        sbus_rx.Begin();   // v4.00: ebenfalls UART A
        Serial.printf("SBUS gestartet (Board %s, RX=%d, RC-System=%d)\n",
                      BOARD_NAME, UART_A_RX, RC_System_boot);
    }

    // v4.04/v4.05: UART B/C nach Rolle starten (RC_Common uart_ports.h):
    // CRSF als Router fuer die Modulkette (nur bei RC_System 4), S.Port,
    // GPS oder Raw. S.Port, GPS und Raw laufen nur auf einem Port, doppelt
    // gewaehlt bleibt UART C aus.
    g_ports[0] = {"UART B", &UART_B_SERIAL, UART_B_RX, UART_B_TX, g_port_role[0], g_port_baud[0]};
#if defined(UART_C_SERIAL)
    g_ports[1] = {"UART C", &UART_C_SERIAL, UART_C_RX, UART_C_TX, g_port_role[1], g_port_baud[1]};
#endif
    uartPortsBegin(g_ports, UART_PORT_COUNT, crsf, RC_System_boot == 4);
    buildLuaMenu();

    for (int i = 0; i < 8; i++) {
        if (OutPwm[i]) {
            ledcAttach(OutPin[i], PWM_FREQ, PWM_RESOLUTION);
            ledcWrite(OutPin[i], 0);
        } else {
            pinMode(OutPin[i], OUTPUT);   // C3: Ausgang 7/8 ohne LEDC-Kanal
            digitalWrite(OutPin[i], LOW);
        }
    }

#if STATUS_LED_WS2812
    statusLed.begin(STATUS_LED_PIN, true, STATUS_LED_BRIGHTNESS);
#else
    statusLed.begin(STATUS_LED_PIN, false);
#endif
    pinMode(BOOT_PIN, INPUT_PULLUP);   // v4.01: BOOT-Taster fuer den WLAN-AP

    webui_init();

    Serial.printf("RC-Multiswitch %s v%d.%02d\n", BOARD_NAME, Version / 100, Version % 100);
    Serial.printf("SSID: %s\n", g_wifi_ssid);
    Serial.printf("IP:   %s\n", AP_IP_STR);
}

// ======== Loop ==================================================
void loop() {

    nvsProcessPending();
    wifiFailsafeCheck(); // NEU v2.00 - unbedingt am Loop-Anfang, damit auch der SBUS-Failsafe-Fruehausstieg unten ihn nicht ueberspringt
    bootButtonCheck();   // NEU v4.01 - aus demselben Grund am Loop-Anfang
    uartPortsUpdate();   // NEU v4.04 - S.Port/GPS, tut nichts ohne diese Rolle

    if (RC_System_boot == 4) {
        crsf.read_packets(0);
        crsf.route_packets();                         // v4.01: Modulkette B/C
        if (crsf.isDuplicateAddress()) g_ledConfigError = true;
        for (int i = 0; i < 16; i++)
            channel_output[i] = crsf.get_crfs_channels(i);

        // BUS_OK (v2.00 FIX): jetzt an "wurde gerade ein gueltiger CRSF-Frame
        // geparst" gekoppelt (crsf.getValidFrames() hat sich erhoeht), nicht
        // mehr an "channel_output[0] > 0" - ein Kanalwert von 0 ist legitim
        // und daher kein zuverlaessiger Nachweis fuer einen frischen Frame.
        static uint32_t lastValidFrameCount = 0;
        uint32_t validFrames = crsf.getValidFrames();
        if (validFrames != lastValidFrameCount) {
            lastValidFrameCount = validFrames;
            BUS_OK = true;
            lastCrsfPacket = millis();
        }
        checkCrsfTimeout();
        uartPortsTelemetry(crsf);   // v4.04: S.Port/GPS an den Empfaenger

        // Kein Telemetrie-Dummy: DEVICE_PING / LUA-Pakete werden durch
        // device_info und param-Antworten sichergestellt (keine Dummy-Daten noetig)

        // Bei CRSF: Einkanal-Steuerung NUR ueber WM-Protokoll (einkanalFunctionCRSF).
        // einkanalFunctionSBUS NICHT aufrufen - CRSF-Kanalwerte sind zu gross
        // fuer SBUS-WM-Dekodierung und wuerden einkanal_Data verfaelschen.
        // (Fehler in V1.1: ELSE-Zweig lief auch bei CRSF -> falsches Schalten)
        // ELRS LUA Script: Device Info Antwort auf DEVICE_PING
        if (crsf.getDeviceInfoReplyPending() && (millis() - crsf.getPingTime() >= crsfSlotDelayMs())) {
            crsf.setDeviceInfoReplyPending(false);
            char devName[24];
            snprintf(devName, sizeof(devName), "%s@%d", g_device_name, modul_adress);  // NEU V1.4
            // Firmware-Version als CRSF "Parameter Version Number"
            // mitgeben (war zuvor fest 0x00, siehe Kommentar in
            // crsf_esp32.cpp/send_device_info(); Fix aus v2.03
            // uebernommen) - damit erkennen Konfiguratoren wie TBS Agent
            // eine geaenderte Parameterstruktur zuverlaessig und lesen
            // sie neu ein.
            crsf.send_device_info(devName, luaMenu.maxId(), (uint8_t)(Version & 0xFF));
        }
        // ELRS LUA Script: Parameter lesen
        if (crsf.getDeviceReadReplyPending()) {
            crsf.setDeviceReadReplyPending(false);
            luaMenu.send(crsf, crsf.getParamReadIndex());
        }
        // ELRS LUA Script: Parameter schreiben + Read-Back (CRSF Standard)
        if (crsf.getDeviceWriteReplyPending()) {
            uint8_t widx = crsf.getParamWriteIndex();
            uint8_t wval = crsf.getParamWriteValue();
            crsf.setDeviceWriteReplyPending(false);
            luaMenu.write(widx, wval);
            // Read-Back: geschriebenen Parameter direkt zuruecksenden
            luaMenu.send(crsf, widx);
        }
        // WM-Protokoll Befehle
        if (crsf.getDeviceCommandReplyPending()) {
            crsf.setDeviceCommandReplyPending(false);
            einkanalFunctionCRSF();
        }
        Data = einkanal_Data;

    } else {
        if (sbus_rx.Read()) {
            sbus_data = sbus_rx.data();
            lastSbusFrameMs = millis();   // v4.00: fuer die Status-LED

            if (sbus_data.failsafe) {
                BUS_OK = false;
                updateLed();
                webui_handle();
                return;
            }

            BUS_OK = true;
            for (int i = 0; i < 16; i++)
                channel_output[i] = sbus_data.ch[i];

            if (einkanal_mode != 999 && CRSF_Channel < 16)
                einkanalFunctionSBUS(sbus_data.ch[CRSF_Channel]);
            // NEU V3.00: MKan-Gruppen ueber SBUS (unabhaengig von obiger
            // Einzelkanal-Auswertung - siehe einkanalFunctionSBUSGroup()).
            for (uint8_t g = 0; g < MULTISW_NUM_GROUPS; g++) {
                int c = SBUS_Gruppen_Channel[g];
                if (c >= 0 && c < 16) einkanalFunctionSBUSGroup(g, sbus_data.ch[c]);
            }
            Data = einkanal_Data;
        }
        checkSbusTimeout();   // v4.03
    }

    Output(Data);
    updateLed();
    webui_handle();
}
