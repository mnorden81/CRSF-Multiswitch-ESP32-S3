/*
 * board_config.h  -  Pin- und Board-Konfiguration RC-Multiswitch v4.02
 *
 * Gleicher Quellcode fuer beide Platinen. Das Board wird automatisch aus dem
 * in der Arduino IDE gewaehlten Chip erkannt:
 *   ESP32C3 Dev Module -> BOARD_C3 (ESP32-C3 SuperMini)
 *   ESP32S3 Dev Module -> BOARD_S3 (Waveshare ESP32-S3-Zero)
 * Alternativ BOARD_C3 bzw. BOARD_S3 vor dem Include selbst definieren.
 *
 * Arduino-IDE-Einstellungen (beide Boards):
 *   USB CDC On Boot: Enabled  (Debug-Ausgaben ueber USB, UART0 = CRSF)
 *   Partition Scheme: mit OTA (z.B. "Default 4MB with spiffs")
 *
 * Pinbelegung laut docs/Hardware-Beschreibung_C3_S3.docx und
 * docs/Firmware-Spezifikation.md, Kapitel 2.
 */
#pragma once
#include <Arduino.h>

#if !defined(BOARD_C3) && !defined(BOARD_S3)
  #if defined(CONFIG_IDF_TARGET_ESP32C3)
    #define BOARD_C3
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    #define BOARD_S3
  #else
    #error "Unbekanntes Board: bitte ESP32C3 Dev Module oder ESP32S3 Dev Module waehlen"
  #endif
#endif

#if !ARDUINO_USB_CDC_ON_BOOT
  #error "Bitte 'USB CDC On Boot: Enabled' einstellen - UART0 wird fuer CRSF (UART A) gebraucht"
#endif

#if defined(BOARD_C3)
  #define BOARD_NAME "C3"

  // Ausgaenge S0..S7 (Kanal 1..8)
  static constexpr uint8_t OutPin[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  // ESP32-C3 hat nur 6 LEDC-Kanaele: Ausgang 7/8 schalten nur digital
  // (an ab 50 % Duty, Blinken weiter per Software), siehe outWrite().
  static constexpr bool OutPwm[8] = {true, true, true, true, true, true, false, false};

  // UART A (Eingang vom Empfaenger) = UART0
  #define UART_A_SERIAL Serial0
  static constexpr uint8_t UART_A_RX = 20;
  static constexpr uint8_t UART_A_TX = 21;
  // UART B (weiter zum naechsten Modul) = UART1
  #define UART_B_SERIAL Serial1
  static constexpr uint8_t UART_B_RX = 8;    // zugleich Onboard-LED des SuperMini
  static constexpr uint8_t UART_B_TX = 10;
  // UART C: nicht vorhanden

  static constexpr uint8_t BOOT_PIN = 9;
  // Keine steuerbare Status-LED (Onboard-LED IO8 ist UART-B-RX)
  static constexpr int STATUS_LED_PIN = -1;
  #define STATUS_LED_WS2812 0

#elif defined(BOARD_S3)
  #define BOARD_NAME "S3"

  static constexpr uint8_t OutPin[8] = {11, 4, 5, 6, 10, 9, 8, 7};
  static constexpr bool    OutPwm[8] = {true, true, true, true, true, true, true, true};

  #define UART_A_SERIAL Serial0
  static constexpr uint8_t UART_A_RX = 44;
  static constexpr uint8_t UART_A_TX = 43;
  #define UART_B_SERIAL Serial1
  static constexpr uint8_t UART_B_RX = 12;
  static constexpr uint8_t UART_B_TX = 13;
  #define UART_C_SERIAL Serial2
  static constexpr uint8_t UART_C_RX = 1;
  static constexpr uint8_t UART_C_TX = 2;

  static constexpr uint8_t BOOT_PIN = 0;
  // WS2812 auf dem S3-Zero
  static constexpr int STATUS_LED_PIN = 21;
  #define STATUS_LED_WS2812 1
  static constexpr uint8_t STATUS_LED_BRIGHTNESS = 40;  // von 255, ca. 15 %
#endif
