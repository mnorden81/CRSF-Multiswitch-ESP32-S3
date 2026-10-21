/*
    RC-Multiswitch  v4.04  (ESP32-C3 SuperMini / Waveshare ESP32-S3-Zero)
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
  CRSF_ESP32  Ziege-One               (für RC_System 4)
  TinyGPSPlus 1.1.0                   (v4.04: Rolle GPS auf UART B/C)
    https://github.com/Ziege-One/CRSF_ESP32
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "board_config.h"
#include "sbus.h"
#include "crsf_esp32.h"
#include "blink_presets.h"
#include "sport_lipo.h"   // v4.04: Rolle S.Port
#include "gps_speed.h"    // v4.04: Rolle GPS

constexpr uint16_t Version = 404; // 4.04 - CRSF Parameter Version Number

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

// ======== UART B/C: waehlbare Rolle (NEU v4.04) =================
// Wie beim Soundmodul: jeder Zusatz-UART bekommt eine Rolle. UART A bleibt
// fest der RC-Eingang (CRSF oder SBUS). Werte als int (kein eigenes enum
// als Rueckgabetyp, Arduino-Prototyp-Generator).
static constexpr int UROLE_OFF   = 0;
static constexpr int UROLE_CRSF  = 1;   // Modulkette (CRSF-Router), nur bei RC_System 4
static constexpr int UROLE_SPORT = 2;   // FrSky FLVSS/MLVSS, siehe sport_lipo.h
static constexpr int UROLE_GPS   = 3;   // NMEA-GPS, siehe gps_speed.h
static constexpr int UROLE_ROLE_COUNT = 4;
static const char* const UROLE_ROLE_OPTIONS = "Aus;CRSF;S.Port;GPS";
static const uint32_t GPS_BAUD_LIST[] = {9600, 19200, 38400, 57600, 115200};
static constexpr uint8_t GPS_BAUD_COUNT = 5;
static const char* const GPS_BAUD_OPTIONS = "9600;19200;38400;57600;115200";

int      g_portb_role = UROLE_CRSF;      // NVS "pbrole"
int      g_portc_role = UROLE_CRSF;      // NVS "pcrole", nur S3 wirksam
uint32_t g_gps_baud   = 9600;           // NVS "gpsbaud"
uint8_t  g_sport_poll_id[2] = {0xA1, 0x22};   // Werkseinstellung FLVSS 1/2
// Stand beim Booten (Aenderungen wirken erst nach Neustart)
int      g_portb_role_boot = UROLE_OFF;
int      g_portc_role_boot = UROLE_OFF;
bool     g_portc_duplicate = false;     // UART C wegen doppelter Rolle aus

static const char* portRoleName(int r) {
    switch (r) {
        case UROLE_CRSF:  return "CRSF";
        case UROLE_SPORT: return "S.Port";
        case UROLE_GPS:   return "GPS";
        default:         return "Aus";
    }
}
static uint8_t gpsBaudToIndex(uint32_t baud) {
    for (uint8_t i = 0; i < GPS_BAUD_COUNT; i++) if (GPS_BAUD_LIST[i] == baud) return i;
    return 0;
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
// webui_enableAP() aus crsfWriteParam() heraus (siehe Kommentar dort).
// Default AN (true), auch fuer Bestandsgeraete beim ersten Boot nach einem
// Firmware-Update (siehe Migrations-Default in nvsLoad() unten:
// p.getBool("walways", true)).
bool     g_wifi_always_on    = true;

// ======== NVS ===================================================
static bool nvsDirty = false;
static bool nvsSaveScheduled = false;
static unsigned long nvsSaveDueMs = 0;
static constexpr unsigned long NVS_SAVE_DEBOUNCE_MS = 500;

static void nvsWriteNow() {
    Preferences p;
    p.begin("msw", false);
    p.putInt("rc_sys",  RC_System);
    p.putInt("crsf_ch", CRSF_Channel);
    p.putInt("ek_mode", einkanal_mode);
    p.putInt("mod_adr", modul_adress);
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
    p.putString("ssid", g_wifi_ssid);
    p.putString("pass", g_wifi_pass);
    p.putString("ip",   g_wifi_ip);
    p.putString("dnam", g_device_name);   // NEU V1.4
    p.putBool("wauto",  g_wifi_auto);         // NEU v2.00
    p.putUInt("watout", g_wifi_auto_timeout); // NEU v2.00
    p.putBool("walways", g_wifi_always_on);   // NEU v2.02
    p.putInt("pbrole",  g_portb_role);        // NEU v4.04
    p.putInt("pcrole",  g_portc_role);        // NEU v4.04
    p.putUInt("gpsbaud", g_gps_baud);         // NEU v4.04
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
    modul_adress  = p.getInt("mod_adr", 0);
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
    String s    = p.getString("ssid", g_wifi_ssid);
    String pw   = p.getString("pass", g_wifi_pass);
    String ip   = p.getString("ip",   g_wifi_ip);
    String dnam = p.getString("dnam", g_device_name);  // NEU V1.4
    strncpy(g_wifi_ssid,    s.c_str(),    sizeof(g_wifi_ssid)-1);
    strncpy(g_wifi_pass,    pw.c_str(),   sizeof(g_wifi_pass)-1);
    strncpy(g_wifi_ip,      ip.c_str(),   sizeof(g_wifi_ip)-1);
    strncpy(g_device_name,  dnam.c_str(), sizeof(g_device_name)-1);
    // Migrations-Default seit v2.01 bewusst true (nicht false) - siehe
    // Kommentar bei g_wifi_auto weiter oben: ein Geraet, dessen NVS den
    // Schluessel "wauto" noch nicht kennt (Update von vor v2.01), soll den
    // neuen Default ("an") automatisch bekommen.
    g_wifi_auto         = p.getBool("wauto",  true);  // NEU v2.00, Default AN seit v2.01
    g_wifi_auto_timeout = p.getUInt("watout", 60);    // NEU v2.00
    if (g_wifi_auto_timeout < 5 || g_wifi_auto_timeout > 240) g_wifi_auto_timeout = 60;
    // Migrations-Default seit v2.02 bewusst true - siehe Kommentar bei
    // g_wifi_always_on weiter oben.
    g_wifi_always_on = p.getBool("walways", true);    // NEU v2.02
    // NEU v4.04: Rollen UART B/C, Default CRSF = Verhalten bis v4.03
    g_portb_role = p.getInt("pbrole", UROLE_CRSF);
    g_portc_role = p.getInt("pcrole", UROLE_CRSF);
    if (g_portb_role < 0 || g_portb_role >= UROLE_ROLE_COUNT) g_portb_role = UROLE_CRSF;
    if (g_portc_role < 0 || g_portc_role >= UROLE_ROLE_COUNT) g_portc_role = UROLE_CRSF;
    g_gps_baud = GPS_BAUD_LIST[gpsBaudToIndex(p.getUInt("gpsbaud", 9600))];
    p.end();
    nvsDirty = false;
    nvsSaveScheduled = false;
    Serial.println("NVS geladen.");
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
    g_portb_role = UROLE_CRSF; g_portc_role = UROLE_CRSF; g_gps_baud = 9600;  // NEU v4.04
    nvsSave();
    nvsFlushPending();
}

void storageSave() { nvsFlushPending(); }

// ======== Status-LED ============================================
// v4.00: Muster nach Firmware-Spezifikation Kapitel 6, nicht blockierend.
// Reine Anzeige - aendert weder BUS_OK noch das Ausgangsverhalten.
// Rueckgabe als uint8_t: der Arduino-Prototyp-Generator setzt Funktions-
// prototypen vor eigene Typen, ein enum als Rueckgabetyp baut dann nicht.
enum LedMode : uint8_t { LED_OFF, LED_SLOW, LED_FAST, LED_DOUBLE };

// Konfigurationsfehler (Doppelblitz). v4.01: gesetzt, wenn der Router eine
// doppelte Moduladresse erkennt (siehe loop()).
bool g_ledConfigError = false;

// CRSF: so lange ohne RC-Kanal-Frame gilt der RC-Link als weg, obwohl noch
// andere Frames (Ping, Parameter, Modulkette) ankommen.
static constexpr unsigned long RC_LINK_TIMEOUT_MS = 1000;
// SBUS: ohne Frame so lange gilt "kein Signal" (wie CRSF_TIMEOUT_MS).
static constexpr unsigned long SBUS_LED_TIMEOUT_MS = 2000;
static unsigned long lastSbusFrameMs = 0;

extern LinkStatistics linkStats;   // crsf_esp32.cpp

static uint8_t       ledMode      = LED_OFF;
static unsigned long ledModeStart = 0;
static bool          ledState     = false;

// v4.00: Ausgabe ueber statusLedWrite() - S3 WS2812 gruen, C3 ohne LED.
static void statusLedWrite(bool on) {
#if STATUS_LED_WS2812
    neopixelWrite(STATUS_LED_PIN, 0, on ? STATUS_LED_BRIGHTNESS : 0, 0);
#else
    (void)on;
#endif
}

static uint8_t currentLedMode(unsigned long now) {
    if (g_ledConfigError) return LED_DOUBLE;
    if (RC_System_boot == 4) {
        if (!BUS_OK) return LED_OFF;
        unsigned long rc = crsf.getLastRcFrameMs();
        bool noRc   = (rc == 0) || (now - rc > RC_LINK_TIMEOUT_MS);
        bool lqNull = linkStats.lastUpdate != 0 &&
                      now - linkStats.lastUpdate <= CRSF_TIMEOUT_MS &&
                      linkStats.uplinkLQ == 0;
        return (noRc || lqNull) ? LED_FAST : LED_SLOW;
    }
    // SBUS: BUS_OK ist im Failsafe false, deshalb eigene Frame-Zeit
    if (lastSbusFrameMs == 0 || now - lastSbusFrameMs > SBUS_LED_TIMEOUT_MS)
        return LED_OFF;
    return sbus_data.failsafe ? LED_FAST : LED_SLOW;
}

static void updateLed() {
    unsigned long now = millis();
    uint8_t m = currentLedMode(now);
    if (m != ledMode) { ledMode = m; ledModeStart = now; }
    unsigned long t = now - ledModeStart;
    bool on;
    switch (ledMode) {
        case LED_SLOW:   on = (t % 1000) < 500; break;           // 500/500 ms
        case LED_FAST:   on = (t % 200)  < 100; break;           // 100/100 ms
        case LED_DOUBLE: { unsigned long p = t % 1000;           // 2x 100 ms, Pause
                           on = p < 100 || (p >= 250 && p < 350); } break;
        default:         on = false; break;
    }
    if (on != ledState) { ledState = on; statusLedWrite(on); }
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
//   82: Sel  "UART B"         Aus;CRSF;S.Port;GPS -> g_portb_role
//   83: Sel  "UART C"         wie 82 (S3), Info beim C3 -> g_portc_role
//   84: Sel  "GPS Baud"       9600..115200        -> g_gps_baud
// Total: 85 Parameter (0..84)

static constexpr uint8_t CRSF_PARAM_COUNT = 84;

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

static void crsfSendParam(uint8_t idx) {
    char buf[32];
    const bool isCrsf = (RC_System_boot == 4);

    if (idx == 0) {
        crsf.send_param_response_CRSF_FOLDER(0, 0, "",
            {1,2,5,14,23,32,41,50,59,68,77,81});
    } else if (idx == 1) {
        crsf.send_param_response_CRSF_INFO(1, 0, "Version", "v4.04 " BOARD_NAME);
    } else if (idx == 2) {
        crsf.send_param_response_CRSF_FOLDER(2, 0, "Global", {3,4});
    } else if (idx == 3) {
        crsf.send_param_response_CRSF_UINT8(3, 2, "Switch Addr",
            (uint8_t)modul_adress, 0, 20, "");
    } else if (idx == 4) {
        crsf.send_param_response_CRSF_UINT8(4, 2, "CRSF Kanal",
            (uint8_t)CRSF_Channel, 0, 15, "");
    } else if (idx >= 5 && idx <= 76) {
        uint8_t x   = (idx - 5) / 9;
        uint8_t sub = (idx - 5) % 9;
        uint8_t fi  = 5 + x * 9;
        uint8_t mH  = (mode[x] >> 8) & 0xFF;
        uint8_t mL  =  mode[x]        & 0xFF;
        snprintf(buf, sizeof(buf), "Ausgang %d", x + 1);

        switch (sub) {
        case 0: // Folder
            crsf.send_param_response_CRSF_FOLDER(fi, 0, buf,
                {(uint8_t)(fi+1),(uint8_t)(fi+2),(uint8_t)(fi+3),
                 (uint8_t)(fi+4),(uint8_t)(fi+5),(uint8_t)(fi+6),
                 (uint8_t)(fi+7),(uint8_t)(fi+8)});
            break;
        case 1: // Ausgang-Quelle
            crsf.send_param_response_CRSF_TEXT_SELECTION(fi+1, fi,
                "Ausgang-Quelle", "Einzelkanal;Kanal_L;Kanal_H;MKan1;MKan2",
                getKanalQuelle(x), 0, 4);
            break;
        case 2: // Kanal Nr
        {
            uint8_t q = getKanalQuelle(x);
            uint8_t maxNr = (q == 1 || q == 2) ? 16 : 8;
            snprintf(buf, sizeof(buf), (q == 1 || q == 2) ? "RC-Kanal Nr" : "Bit Nr");
            crsf.send_param_response_CRSF_UINT8(fi+2, fi,
                buf, getKanalNr(x), 1, maxNr, "");
            break;
        }
        case 3: // Blink
            crsf.send_param_response_CRSF_TEXT_SELECTION(fi+3, fi,
                "Blink", "Dauerlicht;Blinken",
                (mH > 0) ? 1 : 0, 0, 1);
            break;
        case 4: // Blink AN
            crsf.send_param_response_CRSF_UINT8(fi+4, fi,
                "Blink AN *10ms", mH ? mH : 10, 1, 250, "");
            break;
        case 5: // Blink AUS
            crsf.send_param_response_CRSF_UINT8(fi+5, fi,
                "Blink AUS *10ms", mL ? mL : 10, 1, 250, "");
            break;
        case 6: // PWM Modus
            if (isCrsf) {
                crsf.send_param_response_CRSF_TEXT_SELECTION(fi+6, fi,
                    "PWM Modus", "Festwert;MWprop (Kanal=Ausgang)",
                    getPwmModus(x), 0, 1);
            } else {
                crsf.send_param_response_CRSF_INFO(fi+6, fi,
                    "PWM Modus", "Festwert (SBUS)");
            }
            break;
        case 7: // PWM Festwert
            if (getPwmModus(x) == 1) {
                snprintf(buf, sizeof(buf), "MWprop Kanal %d (auto)", x+1);
                crsf.send_param_response_CRSF_INFO(fi+7, fi, "PWM Wert", buf);
            } else {
                crsf.send_param_response_CRSF_UINT8(fi+7, fi,
                    "PWM Festwert", (uint8_t)pwm_wert[x], 0, 255, "");
            }
            break;
        case 8: // Test
            crsf.send_param_response_CRSF_TEXT_SELECTION(fi+8, fi,
                "Test", "Aus;Ein", 0, 0, 1);
            break;
        }
    } else if (idx == 77) {
        // NEU v2.02, uebernommen aus dem Soundmodul-Projekt (dort Ordner 160)
        crsf.send_param_response_CRSF_FOLDER(77, 0, "WLAN", {78,79,80});
    } else if (idx == 78) {
        // NEU v2.02, uebernommen aus dem Soundmodul-Projekt (dort Feld 175)
        crsf.send_param_response_CRSF_TEXT_SELECTION(78, 77,
            "Auto-Failsafe", "Aus;Ein",
            g_wifi_auto ? 1 : 0, 0, 1);
    } else if (idx == 79) {
        // NEU v2.02, uebernommen aus dem Soundmodul-Projekt (dort Feld 176)
        crsf.send_param_response_CRSF_UINT8(79, 77,
            "Auto Timeout", (uint8_t)constrain(g_wifi_auto_timeout,5,240), 5, 240, "s");
    } else if (idx == 80) {
        // NEU v2.02, uebernommen aus dem Soundmodul-Projekt (dort Feld 181,
        // "WLAN dauerhaft an") - Default AN, ueberstimmt 78/79 vollstaendig
        // (siehe wifiFailsafeCheck()/webui_init()).
        crsf.send_param_response_CRSF_TEXT_SELECTION(80, 77,
            "Dauerhaft an", "Aus;Ein",
            g_wifi_always_on ? 1 : 0, 0, 1);
    } else if (idx == 81) {
        // NEU v4.04: Rollen der Zusatz-UARTs, an die Root angehaengt
        crsf.send_param_response_CRSF_FOLDER(81, 0, "UART B/C", {82,83,84});
    } else if (idx == 82) {
        crsf.send_param_response_CRSF_TEXT_SELECTION(82, 81,
            "UART B (Neustart)", UROLE_ROLE_OPTIONS,
            (uint8_t)g_portb_role, 0, UROLE_ROLE_COUNT - 1);
    } else if (idx == 83) {
#if defined(UART_C_SERIAL)
        crsf.send_param_response_CRSF_TEXT_SELECTION(83, 81,
            "UART C (Neustart)", UROLE_ROLE_OPTIONS,
            (uint8_t)g_portc_role, 0, UROLE_ROLE_COUNT - 1);
#else
        crsf.send_param_response_CRSF_INFO(83, 81, "UART C", "nicht vorhanden (C3)");
#endif
    } else if (idx == 84) {
        crsf.send_param_response_CRSF_TEXT_SELECTION(84, 81,
            "GPS Baud (Neustart)", GPS_BAUD_OPTIONS,
            gpsBaudToIndex(g_gps_baud), 0, GPS_BAUD_COUNT - 1);
    }
}

static void crsfWriteParam(uint8_t idx, uint8_t val) {
    const bool isCrsf = (RC_System_boot == 4);
    if (idx == 3) {
        modul_adress = val; crsf.setDeviceAddress(crsfAddrFromWM()); nvsSave();
    } else if (idx == 4) {
        CRSF_Channel = val; nvsSave();
    } else if (idx >= 5 && idx <= 76) {
        uint8_t x   = (idx - 5) / 9;
        uint8_t sub = (idx - 5) % 9;
        switch (sub) {
        case 1: // Ausgang-Quelle: Quelle wechseln, Kanalnummer beibehalten
            setKanalQuelleNr(x, val, getKanalNr(x));
            nvsSave(); break;
        case 2: // Kanal Nr
            setKanalQuelleNr(x, getKanalQuelle(x), val);
            nvsSave(); break;
        case 3: // Blink
            if (val == 0) {
                mode[x] = 0;
            } else {
                uint8_t mH = (mode[x] >> 8) & 0xFF;
                uint8_t mL =  mode[x]        & 0xFF;
                if (!mH) mH = 10;
                if (!mL) mL = 10;
                mode[x] = ((uint16_t)mH << 8) | mL;
            }
            nvsSave(); break;
        case 4: // Blink AN
            mode[x] = ((uint16_t)val << 8) | (mode[x] & 0xFF);
            nvsSave(); break;
        case 5: // Blink AUS
            mode[x] = (mode[x] & 0xFF00) | val;
            nvsSave(); break;
        case 6: // PWM Modus
            if (isCrsf) {
                if (val == 1) {
                    pwm_wert[x] = 200 + x; // MWprop
                    einkanal_Data |= (1 << x); // Ausgang einschalten
                } else if (getPwmModus(x) == 1) {
                    pwm_wert[x] = 255; // zurueck auf Festwert
                }
                nvsSave();
            }
            break;
        case 7: // PWM Festwert
            if (getPwmModus(x) == 0) { pwm_wert[x] = val; nvsSave(); }
            break;
        case 8: // Test
            if (val == 1) einkanal_Data |=  (1 << x);
            else          einkanal_Data &= ~(1 << x);
            break;
        }
    } else if (idx == 78) {
        // NEU v2.02: rein persistierend, wie beim Soundmodul (dort Feld
        // 175) - kein sofortiges Ein-/Ausschalten, wirkt erst beim naechsten
        // Timeout-Ablauf bzw. Bootvorgang.
        g_wifi_auto = (val != 0); nvsSave();
    } else if (idx == 79) {
        // NEU v2.02, analog Soundmodul Feld 176.
        g_wifi_auto_timeout = constrain((uint16_t)val, (uint16_t)5, (uint16_t)240);
        nvsSave();
    } else if (idx == 80) {
        // NEU v2.02: rein persistierend wie 78/79, KEIN sofortiger Aufruf
        // von webui_enableAP() (anders als der Sofort-Schalter "WLAN
        // jetzt" ueber /api/wifi). Wirkt erst ab dem naechsten Bootvorgang -
        // bewusst so (Lehre aus dem Soundmodul-Projekt seit v7.15): ein
        // WLAN-Start zur Laufzeit direkt aus dem CRSF-Parameter-Handler
        // heraus ist unnoetiges Risiko, da der AP durch den Default "an"
        // beim Booten ohnehin schon laeuft.
        g_wifi_always_on = (val != 0); nvsSave();
    } else if (idx == 82) {
        // NEU v4.04: wirkt erst nach Neustart (UARTs werden in setup() belegt)
        if (val < UROLE_ROLE_COUNT) { g_portb_role = val; nvsSave(); }
    } else if (idx == 83) {
#if defined(UART_C_SERIAL)
        if (val < UROLE_ROLE_COUNT) { g_portc_role = val; nvsSave(); }
#endif
    } else if (idx == 84) {
        if (val < GPS_BAUD_COUNT) { g_gps_baud = GPS_BAUD_LIST[val]; nvsSave(); }
    }
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
static void wifiFailsafeCheck() {
    static unsigned long busLostSinceMs = 0;
    static bool          busLostSincePending = false;

    // NEU v2.02: WLAN dauerhaft an ueberstimmt den Auto-Failsafe komplett -
    // der AP laeuft in diesem Fall bereits seit dem Booten (siehe
    // webui_init() in webui.h), ein Timer hier waere wirkungslos.
    if (g_wifi_always_on) {
        busLostSincePending = false;
        return;
    }

    if (BUS_OK) {
        busLostSincePending = false;
        return;
    }
    if (!g_wifi_auto) {
        busLostSincePending = false;
        return;
    }
    if (webui_isApActive()) {
        // WLAN laeuft bereits (manuell oder schon vom Failsafe gestartet) -
        // nichts zu tun (siehe Kommentar oben, "nie automatisch wieder aus").
        busLostSincePending = false;
        return;
    }
    if (!busLostSincePending) {
        busLostSincePending = true;
        busLostSinceMs = millis();
        return;
    }
    unsigned long timeoutMs = (unsigned long)g_wifi_auto_timeout * 1000UL;
    if (millis() - busLostSinceMs >= timeoutMs) {
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

// ======== UART B/C starten (NEU v4.04) ===========================
// CRSF startet der Router in setup(), hier nur S.Port und GPS.
static void startPort(const char* name, int role, HardwareSerial* ser, uint8_t rx, uint8_t tx) {
    if (role == UROLE_SPORT)    sportLipoInit(ser, rx, tx, SPORT_BAUD);
    else if (role == UROLE_GPS) gpsSpeedInit(ser, rx, tx, g_gps_baud);
    const bool idle = (role == UROLE_CRSF && RC_System_boot != 4);
    Serial.printf("UART %s: %s%s (RX=%d TX=%d)\n", name, portRoleName(role),
                  idle ? " - nur bei RC-System CRSF aktiv" : "", rx, tx);
}

// Telemetrie an den Empfaenger (nur CRSF): Zellspannungen (0x0E) und GPS
// (0x02), wie beim Soundmodul alle 100 ms. Kein Batterie-Frame (0x08),
// sonst wuerde die vom Empfaenger gemessene RxBt ueberschrieben.
static void sendPortTelemetry() {
    static unsigned long lastTelem = 0;
    if (millis() - lastTelem < 100) return;
    lastTelem = millis();
    if (sportLipoIsActive()) {
        if (lipoSensor[0].online) sportSendCellsTelemetry(0);
        if (lipoSensor[1].online) sportSendCellsTelemetry(1);
    }
    if (gpsIsActive()) {
        uint16_t gspd = (uint16_t)(gpsGetSpeedKmh() * 10.0f);   // CRSF: km/h * 10
        crsf.send_tele_GPS(gpsGetLatE7(), gpsGetLonE7(), gspd,
                           gpsGetHeadingCdeg(), gpsGetAltMeters(), gpsGetSatellites());
    }
}

// ======== Setup =================================================
void setup() {
    Serial.begin(115200);
    nvsLoad();

    RC_System_boot = RC_System;

    // v4.04: Rollen UART B/C fuer diesen Lauf festhalten. S.Port und GPS
    // haben nur einen Zustand und laufen deshalb nur auf einem Port: bei
    // doppelter Wahl bleibt UART C aus.
    g_portb_role_boot = g_portb_role;
#if defined(UART_C_SERIAL)
    g_portc_role_boot = g_portc_role;
    if (g_portc_role_boot == g_portb_role_boot &&
        (g_portc_role_boot == UROLE_SPORT || g_portc_role_boot == UROLE_GPS)) {
        g_portc_role_boot = UROLE_OFF;
        g_portc_duplicate = true;
    }
#else
    g_portc_role_boot = UROLE_OFF;
#endif

    if (RC_System_boot == 4) {
        // v4.00: CRSF auf UART A (UART0), Pins siehe board_config.h
        crsf.init_crsf(&UART_A_SERIAL, UART_A_RX, UART_A_TX);
        crsf.setDeviceAddress(crsfAddrFromWM());   // eindeutige CRSF-Adresse aus WM-Adresse
        Serial.printf("CRSF gestartet (Board %s, RX=%d, TX=%d, 420000 Bd)  Geraeteadresse 0x%02X  Slot %d\n",
                      BOARD_NAME, UART_A_RX, UART_A_TX,
                      crsfAddrFromWM(), (uint8_t)constrain(modul_adress,0,15)*2);
        // v4.01: Router fuer die Modulkette, v4.04: nur auf den UARTs mit
        // Rolle CRSF (B, beim S3 auch C)
        HardwareSerial* rb = (g_portb_role_boot == UROLE_CRSF) ? &UART_B_SERIAL : nullptr;
#if defined(UART_C_SERIAL)
        HardwareSerial* rc = (g_portc_role_boot == UROLE_CRSF) ? &UART_C_SERIAL : nullptr;
        crsf.init_router(rb, UART_B_RX, UART_B_TX, rc, UART_C_RX, UART_C_TX);
#else
        crsf.init_router(rb, UART_B_RX, UART_B_TX, nullptr, 0, 0);
#endif
    } else {
        sbus_rx.Begin();   // v4.00: ebenfalls UART A
        Serial.printf("SBUS gestartet (Board %s, RX=%d, RC-System=%d)\n",
                      BOARD_NAME, UART_A_RX, RC_System_boot);
    }

    // v4.04: S.Port und GPS auf UART B/C (CRSF startet oben im Router)
    startPort("B", g_portb_role_boot, &UART_B_SERIAL, UART_B_RX, UART_B_TX);
#if defined(UART_C_SERIAL)
    startPort("C", g_portc_role_boot, &UART_C_SERIAL, UART_C_RX, UART_C_TX);
    if (g_portc_duplicate)
        Serial.println("UART C aus: gleiche Rolle wie UART B (S.Port/GPS nur einmal)");
#endif

    for (int i = 0; i < 8; i++) {
        if (OutPwm[i]) {
            ledcAttach(OutPin[i], PWM_FREQ, PWM_RESOLUTION);
            ledcWrite(OutPin[i], 0);
        } else {
            pinMode(OutPin[i], OUTPUT);   // C3: Ausgang 7/8 ohne LEDC-Kanal
            digitalWrite(OutPin[i], LOW);
        }
    }

    statusLedWrite(false);
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
    sportLipoUpdate();   // NEU v4.04 - tut nichts, wenn kein Port S.Port hat
    gpsSpeedUpdate();    // NEU v4.04 - tut nichts, wenn kein Port GPS hat

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
        sendPortTelemetry();   // v4.04: S.Port/GPS an den Empfaenger

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
            crsf.send_device_info(devName, CRSF_PARAM_COUNT, (uint8_t)(Version & 0xFF));
        }
        // ELRS LUA Script: Parameter lesen
        if (crsf.getDeviceReadReplyPending()) {
            crsf.setDeviceReadReplyPending(false);
            crsfSendParam(crsf.getParamReadIndex());
        }
        // ELRS LUA Script: Parameter schreiben + Read-Back (CRSF Standard)
        if (crsf.getDeviceWriteReplyPending()) {
            uint8_t widx = crsf.getParamWriteIndex();
            uint8_t wval = crsf.getParamWriteValue();
            crsf.setDeviceWriteReplyPending(false);
            crsfWriteParam(widx, wval);
            // Read-Back: geschriebenen Parameter direkt zuruecksenden
            crsfSendParam(widx);
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
