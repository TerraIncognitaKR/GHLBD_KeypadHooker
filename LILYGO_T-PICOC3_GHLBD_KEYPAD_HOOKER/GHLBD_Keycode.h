/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : GHLBD_Keycode.h
  * @brief          : Keypad code definition
  *
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GHLBD_KEYCODE_H__
#define __GHLBD_KEYCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Defines -------------------------------------------------------------------*/

/**
 * @brief Input Mode
 *
 */
#define KEYPAD_MODE_DISABLE         0
#define KEYPAD_MODE_WIN_PC_CALC     1
#define KEYPAD_MODE_HIPER_CALC      2
#define KEYPAD_MODE_NAVKEY          3
#define KEYPAD_MODE_HALFKEY         4
#define KEYPAD_MODE_HALFKEY_HOLD    5
#define KEYPAD_MODE_MAX_LIMIT       KEYPAD_MODE_HALFKEY_HOLD

#define KEYPAD_MULTIPRESS_DELAY    delay(100)


/**
 * @brief Keypad code [ Keypad MCU -> (UART) -> Allwinner AP ]
 * @note  Packet structure :
 *          4 Byte Hexadecimal value
 *          [0xAA] [Keycode] [!Keycode] [0x55]
 */

#define GHLBD_KEYPAD_HEAD            0xAA
#define GHLBD_KEYPAD_TAIL            0x55 // ~GHLBD_KEYPAD_HEAD
/********* Function   Keys *********/
#define GHLBD_KEYPAD_FULL            0x01 // (upper key)        // 1
#define GHLBD_KEYPAD_CUT             0x02 // (upper key)        // 2
#define GHLBD_KEYPAD_GT              0x0F // GT                 // 15
#define GHLBD_KEYPAD_SIGN            0x10 // ±                  // 16
#define GHLBD_KEYPAD_BKSPACE         0x11 // →                  // 17
#define GHLBD_KEYPAD_CE              0x12 // CE                 // 18
#define GHLBD_KEYPAD_C               0x13 // C                  // 19

#define GHLBD_KEYPAD_MARKUP          0x14 // MU                 // 20
#define GHLBD_KEYPAD_MEM_CLEAR       0x15 // CM                 // 21
#define GHLBD_KEYPAD_MEM_RECALL      0x16 // RM                 // 22
#define GHLBD_KEYPAD_M_MINUS         0x17 // M-                 // 23
#define GHLBD_KEYPAD_M_PLUS          0x18 // M+                 // 24

#define GHLBD_KEYPAD_PLUS            0x1B // +                  // 27
#define GHLBD_KEYPAD_EQUAL           0x1F // =                  // 31
#define GHLBD_KEYPAD_MINUS           0x1E // -                  // 30
#define GHLBD_KEYPAD_MULTIPLY        0x1A // ×                  // 26
#define GHLBD_KEYPAD_DIVIDE          0x1D // ÷                  // 29
#define GHLBD_KEYPAD_PERCENTAGE      0x19 // %                  // 25
#define GHLBD_KEYPAD_SQRT            0x1C // √                  // 28
/********* Num        Keys *********/
#define GHLBD_KEYPAD_9               0x0E // 9                  // 14
#define GHLBD_KEYPAD_8               0x0D // 8                  // 13
#define GHLBD_KEYPAD_7               0x0C // 7                  // 12
#define GHLBD_KEYPAD_6               0x0B // 6                  // 11
#define GHLBD_KEYPAD_5               0x0A // 5                  // 10
#define GHLBD_KEYPAD_4               0x09 // 4                  // 9
#define GHLBD_KEYPAD_3               0x08 // 3                  // 8
#define GHLBD_KEYPAD_2               0x07 // 2                  // 7
#define GHLBD_KEYPAD_1               0x06 // 1                  // 6
#define GHLBD_KEYPAD_0               0x05 // 0                  // 5
#define GHLBD_KEYPAD_00              0x04 // 00                 // 4
#define GHLBD_KEYPAD_POINT           0x03 // .(dot)             // 3

/* Macros --------------------------------------------------------------------*/

/* Types ---------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Functions prototypes ------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __GHLBD_KEYCODE_H__ */

