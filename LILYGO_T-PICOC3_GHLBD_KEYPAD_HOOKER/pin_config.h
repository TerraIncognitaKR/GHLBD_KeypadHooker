/**
  ******************************************************************************
  * @file           : pin_config.h
  * @brief          : IO pin configuration
  *                   for "LILYGO_T-PICOC3_GHLBD_KEYPAD_HOOKER.ino"
  ******************************************************************************
  * @attention
  *
  *  2024 TeIn
  *  https://blog.naver.com/bieemiho92
  *
  * @note
  *  https://github.com/Xinyuan-LilyGO/T-PicoC3/blob/main/Schematic/T-PicoC3.pdf
  *
  ******************************************************************************
  */

#pragma once

/* Defines -------------------------------------------------------------------*/

/**
 * @note do not use this section
 */
#if 0
    #define TFT_MISO    -1
    #define TFT_MOSI    3
    #define TFT_SCLK    2
    #define TFT_CS      5  // Chip select control pin
    #define TFT_DC      1  // Data Command control pin
    #define TFT_RST     0 // Reset pin (could connect to RST pin)
#endif

/**
 * @note LILYGO_T-PICOC3 board specific
 */
#define PIN_TFT_BL      4
#define PIN_PWR_ON      22
#define PIN_BOTTON1     6
#define PIN_BOTTON2     7
#define PIN_RED_LED     25
#define PIN_BAT_VOLT    26

/**
 * @note UART1 of RP2040 == Serial2
 */
#define ESP32C3_RX_PIN  9
#define ESP32C3_TX_PIN  8

/**
 * @note UART0 of RP2040 == Serial1
 */
#define PIN_UART_RX     13
#define PIN_UART_TX     12

#ifdef LILYGO_ORG_SRC_EN
const char ssid[] = "Your ssid";
const char pass[] = "password";
#endif /** LILYGO_ORG_SRC_EN **/
