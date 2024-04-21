/**
  ******************************************************************************
  * @file           : LILYGO_T-PICOC3_GHLBD_KEYPAD_HOOKER.ino
  * @brief          : for hooking numeric keypad of GHLBD Desktop Calculator
  * @link           : https://github.com/TerraIncognitaKR/GHLBD_KeypadHooker
  ******************************************************************************
  * @attention
  *
  *  2024 TeIn
  *  https://blog.naver.com/bieemiho92
  *
  *  Target Device :
  *     RP2040 of LILYGO T-PICOC3
  *               (=> Raspberry Pi RP2040 + ESP32-C3 + ST7789 LCD)
  *
  *  IDE           :
  *     Arduino IDE 2.3.2
  *
  *  Dependancy    :
  *     https://github.com/Xinyuan-LilyGO/T-PicoC3
  *
  * @note
  *     https://youtu.be/bl15rUi-c0s?si=jf0vgedSBFEMNGt8
  *
  *   Ver.01 (2024/04) :
  *     - Start from default example source
  *       @ref firmware.ino of official release
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "TFT_eSPI.h"       // https://github.com/Bodmer/TFT_eSPI
#include "OneButton.h"      // https://github.com/mathertel/OneButton

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"


#include "pin_config.h"     // Pin definition
#include "picoImage.h"      // "Raspberry Pi" Splash Logo
#include "GHLBD_Keycode.h"  // GHLBD Calculator Key Code

#include <Keyboard.h>       // for HID Keyboard



/* Defines -------------------------------------------------------------------*/
/**
 * @note Current Firmware Version & Board Name
 */
#define FW_VER              1

/**
 * @note Keycode Input
 */
#define KEYCODE_BUF_SIZE    16

/**
 * @note Debug/Function Enable Purpose
 */
// #define DBG_PRINTF_EN     1

// #define VBAT_MON_EN       1
  #ifdef VBAT_MON_EN
  #include "hardware/adc.h"
  #endif

// #define ESP32_EN          1
  #ifdef ESP32_EN
  #include <WiFiEspAT.h>      // https://github.com/jandrassy/WiFiEspAT
  #endif

// #define IO7_KEY_USE       1

// #define LILYGO_ORG_SRC_EN 1

/* Macros --------------------------------------------------------------------*/

/* Types ---------------------------------------------------------------------*/

/* Constructor ---------------------------------------------------------------*/

/**
 * @note Keys
 */
OneButton button_IO6(PIN_BOTTON1, true);
OneButton button_IO7(PIN_BOTTON2, true);

/**
 * @note to use TFT LCD,
 *       must be select "Setup137_LilyGo_TDisplay_RP2040.h"
 *       on "TFT_eSPI/User_Setup_Select.h".
 *       otherwise, TFT LCD is not working properly.
 */
TFT_eSPI tft = TFT_eSPI(135, 240);

/* Variables -----------------------------------------------------------------*/
/**
 * @note Board Name String
 */
const char *my_board_name = "GHLBD HOOKER";

#ifdef VBAT_MON_EN
/**
 * @note Battery Voltage Monitoring
 */
const float conversion_factor = 3.3f / (1 << 12);
#define VOLT_COMPENSATOR 0.05
#endif

/**
 * @note Keycode Input
 */
char bRxBuf[KEYCODE_BUF_SIZE];
char bPressedKey = 0;
volatile int  dwRecvBufIdx = 0;
volatile int  dwFreeBufIdx = KEYCODE_BUF_SIZE;
volatile bool isKeyPadRecvDone = false;
char bCurrentInputMode = KEYPAD_MODE_WIN_PC_CALC;

/**
 * @note TFT-LCD Display position
 */
int wPosY = 50;
int wPosX = 0;



/* Function prototypes -------------------------------------------------------*/
void    on_uart0_rx(void);
void    GHLBD_Keystroke_send(char INPUT_MODE, char KEYSTROKE);
void    update_disp_curr_method(char METHOD);


/* User code -----------------------------------------------------------------*/

/**
 * @brief      Update display of current input method
 * @param      METHOD @ref bCurrentInputMode
 * @return     none
 * @note       TextSize=1 => 40 Chars
 */
void    update_disp_curr_method(char METHOD)
{
  // wPosY = tft.getCursorY();
  // wPosX = tft.getCursorX();
  // Serial.printf(">> X %03d Y %03d\r\n", wPosX, wPosY);

  /* Clear only refreshing area */
  tft.fillRect(wPosX, wPosY, 240, 135, TFT_BLACK);
  tft.setCursor(wPosX,wPosY);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.printf("> ");

  switch(METHOD)
  {
    case KEYPAD_MODE_DISABLE :
      Serial.printf(">> [%s] KEYPAD_MODE_DISABLE\r\n", __FUNCTION__);
      tft.setTextColor(TFT_RED);
      tft.printf("Disable\r\n");

      tft.setTextSize(1);   tft.setTextColor(TFT_WHITE);
      tft.printf("                                        ");
      tft.printf("                                        ");
      tft.printf("         Key Inputs are Ignored.        ");
      tft.printf("                                        ");
      tft.printf("      to change Mode, Push IO6 Key.     ");
      tft.printf("                                        ");
      tft.printf("                                        ");
    break;

    case KEYPAD_MODE_WIN_PC_CALC :
      Serial.printf(">> [%s] KEYPAD_MODE_WIN_PC_CALC\r\n", __FUNCTION__);
      tft.setTextColor(TFT_BLUE);
      tft.printf("PC Numeric Keypad\r\n");

      tft.setTextSize(1);   tft.setTextColor(TFT_WHITE);
      tft.printf("------------ [ KEY ASSIGN ] ------------");
      tft.printf(" 1                           ENG   PROG ");
      tft.printf(" 2   NLCK  TAB    MC    MR    M-    M+  ");
      tft.printf(" 3   SIGN   7     8     9    o/o   SQRT ");
      tft.printf(" 4   BKSP   4     5     6     x     /   ");
      tft.printf(" 5    CE    1     2     3     +     -   ");
      tft.printf(" 6     C    0     00    .     +     =   ");

    break;

    case KEYPAD_MODE_HIPER_CALC :
      Serial.printf(">> [%s] KEYPAD_MODE_HIPER_CALC\r\n", __FUNCTION__);
      tft.setTextColor(TFT_GREEN);
      tft.printf("HiPER Calc App\r\n");

      tft.setTextSize(1);   tft.setTextColor(TFT_WHITE);
      tft.printf("------------ [ KEY ASSIGN ] ------------");
      tft.printf(" 1                            <-    ->  ");
      tft.printf(" 2    (      )   EXP   x^y   x^-1  #/#  ");
      tft.printf(" 3   SIGN   7     8     9    o/o   SQRT ");
      tft.printf(" 4   BKSP   4     5     6     x     /   ");
      tft.printf(" 5    CE    1     2     3     +     -   ");
      tft.printf(" 6     C    0     00    .     +     =   ");
    break;

    case KEYPAD_MODE_NAVKEY :
      Serial.printf(">> [%s] KEYPAD_MODE_NAVKEY\r\n", __FUNCTION__);
      tft.setTextColor(TFT_GOLD);
      tft.printf("Navigation Key\r\n");

      tft.setTextSize(1);   tft.setTextColor(TFT_WHITE);
      tft.printf("------------ [ KEY ASSIGN ] ------------");
      tft.printf(" 1                           SAVE  EXIT ");
      tft.printf(" 2   ESC   WIN1  WIN2  WIN3  WIN4  WIN5 ");
      tft.printf(" 3   <TSK  Home   UP   PgUp  NOTI  LOCK ");
      tft.printf(" 4   TSK>  LEFT  ENTR  RIGT  DEL   HSCR ");
      tft.printf(" 5   MENU  End   DOWN  PgDn  WIN+  BKSP ");
      tft.printf(" 6   WIN   <TAB  TAB>  ____  TAB    =   ");
    break;

    default :
    break;
  }

  return;
}


/**
 * @brief      Send Keystroke to Host
 * @param      INPUT_MODE   @ref bCurrentInputMode
 * @param      KEYSTROKE    Hexadecimal Keycode value. @ref GHLBD_Keycode.h
 * @return     none
 * @note       @ref Keyboard.h
 */
void    GHLBD_Keystroke_send(char INPUT_MODE, char KEYSTROKE)
{
#ifdef DBG_PRINTF_EN
  Serial.printf(">> [%s] INPUT_MODE %02X    KEYSTROKE %02X\r\n", \
                    __FUNCTION__, INPUT_MODE, KEYSTROKE);
#endif
  /***************************************************************************/
  if(KEYPAD_MODE_DISABLE == INPUT_MODE)
  {
    /**
     * @note  Do nothing at this mode
     */
  }
  /***************************************************************************/
  else if(KEYPAD_MODE_WIN_PC_CALC == INPUT_MODE)
  {
    switch(KEYSTROKE)
    {
      /********* Function   Keys *********/
      case GHLBD_KEYPAD_FULL :
      // no corresponding function
      //    -> switch to engineer mode (ALT+2)
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press('2');
        Keyboard.release('2');
        Keyboard.release(KEY_LEFT_ALT);
      break;

      case GHLBD_KEYPAD_CUT :
      // no corresponding function
      //    -> switch to programmer mode (ALT+4)
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press('4');
        Keyboard.release('4');
        Keyboard.release(KEY_LEFT_ALT);
      break;

      case GHLBD_KEYPAD_GT :
      // no corresponding function
      //    -> assign to NumLock
        Keyboard.press(KEY_NUM_LOCK);
      break;

      case GHLBD_KEYPAD_SIGN :
        Keyboard.press(KEY_F9);
      break;

      case GHLBD_KEYPAD_BKSPACE :
        Keyboard.press(KEY_BACKSPACE);
      break;

      case GHLBD_KEYPAD_CE :
        Keyboard.press(KEY_DELETE);
      break;

      case GHLBD_KEYPAD_C :
        Keyboard.press(KEY_ESC);
      break;

      case GHLBD_KEYPAD_MARKUP :
      // no corresponding function
      //    -> assign to Tab
        Keyboard.press(KEY_TAB);
      break;

      case GHLBD_KEYPAD_MEM_CLEAR :
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('l');
        Keyboard.release('l');
        Keyboard.release(KEY_LEFT_CTRL);
      break;

      case GHLBD_KEYPAD_MEM_RECALL :
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('r');
        Keyboard.release('r');
        Keyboard.release(KEY_LEFT_CTRL);
      break;

      case GHLBD_KEYPAD_M_MINUS :
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('q');
        Keyboard.release('q');
        Keyboard.release(KEY_LEFT_CTRL);
      break;

      case GHLBD_KEYPAD_M_PLUS :
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('p');
        Keyboard.release('p');
        Keyboard.release(KEY_LEFT_CTRL);
      break;

      case GHLBD_KEYPAD_PLUS :
        Keyboard.press(KEY_KP_PLUS);
      break;

      case GHLBD_KEYPAD_EQUAL :
        Keyboard.press(KEY_KP_ENTER);
      break;

      case GHLBD_KEYPAD_MINUS :
        Keyboard.press(KEY_KP_MINUS);
      break;

      case GHLBD_KEYPAD_MULTIPLY :
        Keyboard.press(KEY_KP_ASTERISK);
      break;

      case GHLBD_KEYPAD_DIVIDE :
        Keyboard.press(KEY_KP_SLASH);
      break;

      case GHLBD_KEYPAD_PERCENTAGE :
        Keyboard.press('%');
      break;

      case GHLBD_KEYPAD_SQRT :
        Keyboard.press('@');
      break;

      /********* Num        Keys *********/
      /* use numeric keypad code to support numlock */
      //    Numlock ON              Numlock OFF
      case GHLBD_KEYPAD_0 :     // Insert
        Keyboard.press(KEY_KP_0);
      break;

      case GHLBD_KEYPAD_1 :     // End
        Keyboard.press(KEY_KP_1);
      break;

      case GHLBD_KEYPAD_2 :     // Down
        Keyboard.press(KEY_KP_2);
      break;

      case GHLBD_KEYPAD_3 :     // PgDn
        Keyboard.press(KEY_KP_3);
     break;

      case GHLBD_KEYPAD_4 :     // Left
        Keyboard.press(KEY_KP_4);
      break;

      case GHLBD_KEYPAD_5 :     // n/a
        Keyboard.press(KEY_KP_5);
      break;

      case GHLBD_KEYPAD_6 :     // Right
        Keyboard.press(KEY_KP_6);
      break;

      case GHLBD_KEYPAD_7 :     // Home
        Keyboard.press(KEY_KP_7);
      break;

      case GHLBD_KEYPAD_8 :     // Up
        Keyboard.press(KEY_KP_8);
      break;

      case GHLBD_KEYPAD_9 :     // PgUp
        Keyboard.press(KEY_KP_9);
      break;

      case GHLBD_KEYPAD_00 :    // n/a
        Keyboard.press(KEY_KP_0);    // 1st press
        Keyboard.releaseAll();
        Keyboard.press(KEY_KP_0);    // 2nd press
      break;

      case GHLBD_KEYPAD_POINT : // Del
        Keyboard.press(KEY_KP_DOT);
      break;

      /***********************************/
      default :
#ifdef DBG_PRINTF_EN
        Serial.printf(">> Press Key : %02X\r\n", KEYSTROKE);
#endif
      break;
    }
  }
  /***************************************************************************/
  else if(KEYPAD_MODE_HIPER_CALC == INPUT_MODE)
  {
    switch(KEYSTROKE)
    {
      /********* Function   Keys *********/
      case GHLBD_KEYPAD_FULL :
      // no corresponding function
      //    -> switch to (left)
      Keyboard.press(KEY_LEFT_ARROW);
      break;

      case GHLBD_KEYPAD_CUT :
      // no corresponding function
      //    -> switch to (right)
      Keyboard.press(KEY_RIGHT_ARROW);
      break;

      case GHLBD_KEYPAD_GT :
      // no corresponding function or can't find mapping
      //    -> switch to '('
        Keyboard.press('(');
      break;

      case GHLBD_KEYPAD_SIGN :
        Keyboard.press('n');
      break;

      case GHLBD_KEYPAD_BKSPACE :
        Keyboard.press(KEY_BACKSPACE);
      break;

      case GHLBD_KEYPAD_CE :
        Keyboard.press(KEY_DELETE);
      break;

      case GHLBD_KEYPAD_C :
        Keyboard.press(KEY_ESC);
      break;

      case GHLBD_KEYPAD_MARKUP :
      // no corresponding function or can't find mapping
      //    -> switch to ')'
        Keyboard.press(')');
      break;

      case GHLBD_KEYPAD_MEM_CLEAR :
      // no corresponding function or can't find mapping
      //    -> switch to 'x10^()'
        Keyboard.press('e');
      break;

      case GHLBD_KEYPAD_MEM_RECALL :
      // no corresponding function or can't find mapping
      //    -> switch to '()^()'
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('y');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('y');
        Keyboard.release(KEY_LEFT_CTRL);
      break;

      case GHLBD_KEYPAD_M_MINUS :
      // no corresponding function or can't find mapping
      //    -> switch to '()^-1'
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('i');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('i');
        Keyboard.release(KEY_LEFT_CTRL);
      break;

      case GHLBD_KEYPAD_M_PLUS :
      // no corresponding function or can't find mapping
      //    -> switch to '()/()'
        Keyboard.press('f');
      break;

      case GHLBD_KEYPAD_PLUS :
        Keyboard.press(KEY_KP_PLUS);
      break;

      case GHLBD_KEYPAD_EQUAL :
        Keyboard.press(KEY_KP_ENTER);
      break;

      case GHLBD_KEYPAD_MINUS :
        Keyboard.press(KEY_KP_MINUS);
      break;

      case GHLBD_KEYPAD_MULTIPLY :
        Keyboard.press(KEY_KP_ASTERISK);
      break;

      case GHLBD_KEYPAD_DIVIDE :
        Keyboard.press(KEY_KP_SLASH);
      break;

      case GHLBD_KEYPAD_PERCENTAGE :
        Keyboard.press('%');
      break;

      case GHLBD_KEYPAD_SQRT :
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press('2');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('2');
        Keyboard.release(KEY_LEFT_ALT);
      break;

      /********* Num        Keys *********/
      /* use numeric keypad code to support numlock */
      //    Numlock ON              Numlock OFF
      case GHLBD_KEYPAD_0 :     // Insert
        Keyboard.press(KEY_KP_0);
      break;

      case GHLBD_KEYPAD_1 :     // End
        Keyboard.press(KEY_KP_1);
      break;

      case GHLBD_KEYPAD_2 :     // Down
        Keyboard.press(KEY_KP_2);
      break;

      case GHLBD_KEYPAD_3 :     // PgDn
        Keyboard.press(KEY_KP_3);
     break;

      case GHLBD_KEYPAD_4 :     // Left
        Keyboard.press(KEY_KP_4);
      break;

      case GHLBD_KEYPAD_5 :     // n/a
        Keyboard.press(KEY_KP_5);
      break;

      case GHLBD_KEYPAD_6 :     // Right
        Keyboard.press(KEY_KP_6);
      break;

      case GHLBD_KEYPAD_7 :     // Home
        Keyboard.press(KEY_KP_7);
      break;

      case GHLBD_KEYPAD_8 :     // Up
        Keyboard.press(KEY_KP_8);
      break;

      case GHLBD_KEYPAD_9 :     // PgUp
        Keyboard.press(KEY_KP_9);
      break;

      case GHLBD_KEYPAD_00 :    // n/a
        Keyboard.press(KEY_KP_0);    // 1st press
        Keyboard.releaseAll();
        Keyboard.press(KEY_KP_0);    // 2nd press
      break;

      case GHLBD_KEYPAD_POINT : // Del
        Keyboard.press(KEY_KP_DOT);
      break;

      /***********************************/
      default :
#ifdef DBG_PRINTF_EN
        Serial.printf(">> Press Key : %02X\r\n", KEYSTROKE);
#endif
      break;
    }

  }
  /***************************************************************************/
  else if(KEYPAD_MODE_NAVKEY == INPUT_MODE)
  {
    switch(KEYSTROKE)
    {
      /********* Function   Keys *********/
      case GHLBD_KEYPAD_FULL :
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press('s');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('s');
        Keyboard.release(KEY_LEFT_CTRL);
      break;

      case GHLBD_KEYPAD_CUT :
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_F4);
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release(KEY_F4);
        Keyboard.release(KEY_LEFT_ALT);
      break;

      case GHLBD_KEYPAD_GT :          // ESC
        Keyboard.press(KEY_ESC);
      break;

      case GHLBD_KEYPAD_SIGN :        // Alt+Shift+Tab
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_TAB);
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release(KEY_TAB);
        Keyboard.release(KEY_LEFT_SHIFT);
        Keyboard.release(KEY_LEFT_ALT);
      break;

      case GHLBD_KEYPAD_BKSPACE :     // Alt+Tab
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_TAB);
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release(KEY_TAB);
        Keyboard.release(KEY_LEFT_ALT);
      break;

      case GHLBD_KEYPAD_CE :
        Keyboard.press(KEY_MENU);     // Context Menu Key

      break;

      case GHLBD_KEYPAD_C :
        Keyboard.press(KEY_LEFT_GUI); // LWin key
      break;

      case GHLBD_KEYPAD_MARKUP :      // LWin+1
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('1');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('1');
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_MEM_CLEAR :   // LWin+2
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('2');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('2');
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_MEM_RECALL :  // LWin+3
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('3');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('3');
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_M_MINUS :     // LWin+4
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('4');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('4');
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_M_PLUS :      // LWin+5
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('5');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('5');
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_PLUS :        // LWin + Tab
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press(KEY_TAB);
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release(KEY_TAB);
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_EQUAL :       // Enter
        Keyboard.press('=');
      break;

      case GHLBD_KEYPAD_MINUS :       // Backspace
        Keyboard.press(KEY_BACKSPACE);
      break;

      case GHLBD_KEYPAD_MULTIPLY :    // Delete
        Keyboard.press(KEY_DELETE);
      break;

      case GHLBD_KEYPAD_DIVIDE :      // (Android Home Screen)  LWin + Enter
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press(KEY_RETURN);
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release(KEY_RETURN);
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_PERCENTAGE :  // (Android Notification) LWin + N
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('n');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('n');
        Keyboard.release(KEY_LEFT_GUI);
      break;

      case GHLBD_KEYPAD_SQRT :        // (Screen Lock)  LWin + L
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('l');
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release('l');
        Keyboard.release(KEY_LEFT_GUI);
      break;

      /********* Num        Keys *********/
      case GHLBD_KEYPAD_0 :   // Reverse Tab
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_TAB);
        KEYPAD_MULTIPRESS_DELAY;
        Keyboard.release(KEY_TAB);
        Keyboard.release(KEY_LEFT_SHIFT);
      break;

      case GHLBD_KEYPAD_1 :   // End
        Keyboard.press(KEY_END);
      break;

      case GHLBD_KEYPAD_2 :   // Down
        Keyboard.press(KEY_DOWN_ARROW);
      break;

      case GHLBD_KEYPAD_3 :   // Page Down
        Keyboard.press(KEY_PAGE_DOWN);
     break;

      case GHLBD_KEYPAD_4 :   // Left
        Keyboard.press(KEY_LEFT_ARROW);
      break;

      case GHLBD_KEYPAD_5 :   // Enter
        Keyboard.press(KEY_RETURN);
      break;

      case GHLBD_KEYPAD_6 :   // Right
        Keyboard.press(KEY_RIGHT_ARROW);
      break;

      case GHLBD_KEYPAD_7 :   // Home
        Keyboard.press(KEY_HOME);
      break;

      case GHLBD_KEYPAD_8 :   // Up
        Keyboard.press(KEY_UP_ARROW);
      break;

      case GHLBD_KEYPAD_9 :   // Page Up
        Keyboard.press(KEY_PAGE_UP);
      break;

      case GHLBD_KEYPAD_00 :  // Tab
        Keyboard.press(KEY_TAB);
      break;

      case GHLBD_KEYPAD_POINT : // SpaceBar
        Keyboard.press(' ');
      break;

      /***********************************/
      default :
#ifdef DBG_PRINTF_EN
        Serial.printf(">> Press Key : %02X\r\n", KEYSTROKE);
#endif
      break;
    }
  }

  /* Release Keypress */
  Keyboard.releaseAll();

  return;
}



/**
 * @brief      UART0 Receive IRQ
 * @param      none
 * @return     none
 * @note
 */
void    on_uart0_rx(void)
{
  if(dwFreeBufIdx)
  {
    /* Accept all characters */
    bRxBuf[dwRecvBufIdx] = uart_getc(uart0);

    /* Buffer control */
    dwRecvBufIdx++;
    dwFreeBufIdx--;

    /* Set flag to receive 1 packet done */
    if (dwRecvBufIdx >= 4)
    {
      isKeyPadRecvDone = true;
    }

    /* Buffer control */
    if(dwRecvBufIdx >= KEYCODE_BUF_SIZE)
    {
      dwRecvBufIdx = 0;
      dwFreeBufIdx = KEYCODE_BUF_SIZE;
    }
  }
}



/**
  * @brief      arduino setup()
  * @param      none
  * @return     none
  * @note       put your setup code here, to run once
  */
void    setup()
{
  /* Open Serial for Monitor */
  Serial.begin(115200);
  delay(1000);

#ifdef ESP32_EN
  /* Configure pins for communication with ESP32C3-AT */
  Serial2.setTX(ESP32C3_TX_PIN);
  Serial2.setRX(ESP32C3_RX_PIN);
  Serial2.begin(115200);
  delay(1000);
#endif

  /* Splash */
  Serial.printf ("\r\n\r\n\r\n");
  Serial.println("*******************************************************************************");
  Serial.println();
  Serial.printf ("\t[ %s ]", my_board_name);
  Serial.println();
  Serial.printf ("\tFW VER x%02X\t(Build @  %s %s)\r\n", FW_VER, __TIME__, __DATE__);
  // Serial.printf("\tMAC Addr :\t");
  // Serial.println(WiFi.macAddress());
  Serial.println();
  Serial.println("*******************************************************************************");
  Serial.println();

  /* Open the power supply (of TFT-LCD) */
  Serial.println(">> Power On...");
  pinMode(PIN_PWR_ON, OUTPUT);
  digitalWrite(PIN_PWR_ON, HIGH);

  /* Configure pins for communication with GHLBD */
  Serial.println(">> Initialize UART0 for GHLBD...");

  /* use Raspberry Pi Pico C SDK Style to use UART IRQ */
  uart_init(uart0, 2400);
  gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
  gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
  uart_set_baudrate(uart0, 115200);
  uart_set_hw_flow(uart0, false, false);
  uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
  uart_set_fifo_enabled(uart0, false);
  /* UART0 Receive Interrupt */
  Serial.println(">> Initialize UART0 IRQ...");
  irq_set_exclusive_handler(20, on_uart0_rx);
  irq_set_enabled(20, true);
  uart_set_irq_enables(uart0, true, false);

#ifdef VBAT_MON_EN
  /* Configure battery voltage detection pin */
  Serial.println(">> Initialize ADC...");
  adc_init();
  adc_gpio_init(PIN_BAT_VOLT);
  adc_select_input(0);
#endif

  /* Bind keystroke callback event */
  Serial.println(">> Initialize Button...");
  button_IO6.attachClick(io6_click_event_cb);
#ifdef IO7_KEY_USE
  button_IO7.attachClick(io7_click_event_cb);
#endif

  /* Configure LED GPIO pins */
  Serial.println(">> Initialize GPIOs...");
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, 0);

  /* Initialize the TFT-LCD */
  Serial.println(">> Initialize LCD...");
  tft.init();
  tft.fillScreen(TFT_BLACK); // Clear the screen junk
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 240, 135, pico);

  /* Gradually lighten up */
  analogWrite(PIN_TFT_BL, 0);
  for (int i = 0x00; i < 0xFF; i++)
  {
    delay(5);
    analogWrite(PIN_TFT_BL, i);
  }
  delay(1000);

  /* Keyboard */
  Serial.println(">> Initialize Keyboard System...");
  Keyboard.begin();

  /* Display */
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0,0);
  tft.setTextColor(TFT_ORANGE);
  tft.printf("[   %s   ] \r\n", my_board_name);
  tft.setTextColor(TFT_DARKGREY);
  tft.setTextSize(1);
  tft.printf("VER x%02X (Build @ %s %s)\r\n", FW_VER, __TIME__, __DATE__);
  analogWrite(PIN_TFT_BL, 64); // decrease backlight brightness
  update_disp_curr_method(bCurrentInputMode);

}


/**
  * @brief      arduino loop()
  * @param      none
  * @return     none
  * @note       put your main code here, to run repeatedly:
  */
void    loop()
{
  /* LED Blink */
  blink();

  /* for Key Event Handling */
  button_IO6.tick();
  button_IO7.tick();

  /* for Calculator Keypad Event Handling */
  if(isKeyPadRecvDone)
  {
#ifdef DBG_PRINTF_EN
    for(int i=0; i<dwRecvBufIdx; i++)
      Serial.printf("%02X ", bRxBuf[i]);
    Serial.println();
#endif

    /* Send Key to Host */
    GHLBD_Keystroke_send(bCurrentInputMode, bRxBuf[1]);

    isKeyPadRecvDone = false;
    memset(bRxBuf, 0, KEYCODE_BUF_SIZE);
    dwRecvBufIdx = 0;
    dwFreeBufIdx = KEYCODE_BUF_SIZE;
  }
}



/**
 * @brief      IO6 Button click event callback
 * @param      none
 * @return     none
 * @note       switch Input Mode @ref bCurrentInputMode
 *              KEYPAD_MODE_WIN_PC_CALC -> KEYPAD_MODE_HIPER_CALC
 *              -> KEYPAD_MODE_NAVKEY -> KEYPAD_MODE_DISABLE -> (..)
 */
void io6_click_event_cb()
{
#if 0
  // Scaning Wi-Fi Networks
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  Serial.println();
  Serial.println("Scanning available networks...");
  tft.println("Scanning available networks...");
  listNetworks();
#endif

  bCurrentInputMode++;
  if(KEYPAD_MODE_MAX_LIMIT < bCurrentInputMode)
    bCurrentInputMode = 0;

  update_disp_curr_method(bCurrentInputMode);
}


#ifdef IO7_KEY_USE
/**
 * @brief      IO7 Button click event callback
 * @param      none
 * @return     none
 * @note       (tbd)
 */
void io7_click_event_cb()
{
#if 0
  #ifdef VBAT_MON_EN
  // Display battery voltage
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  float volt = (2 * adc_read() * conversion_factor) + VOLT_COMPENSATOR;
  tft.printf("Battery volt: %0.2f\n", volt);
  Serial.printf("Battery volt: %0.2f\n", volt);
  tft.setTextSize(1);
  #endif
#endif

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0);
  tft.setTextColor(TFT_RED);
  tft.println("IO7 Key Pressed");
  Serial.println(">> IO7 Key Pressed");
}
#endif /** IO7_KEY_USE **/

/**
 * @brief      Red LED Blink (IO25 / RED / Bottom)
 * @param      none
 * @return     none
 * @note       it is random...
 */
void blink(void)
{
  static uint32_t MILLIS, rad;
  if (millis() - MILLIS > rad)
  {
    digitalWrite(PIN_RED_LED, !digitalRead(PIN_RED_LED));
    MILLIS = millis();
    rad = random(1, 500);
  }
}


/* Unused codes from example */
#ifdef LILYGO_ORG_SRC_EN
void wifi_test()
{
  uint32_t overtime = 0;
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(0, 0);
  tft.println("Hello Pico and ESP32C3");
  Serial.println("Hello Pico and ESP32C3");
  bool isAT_Program = Send_AT_CMD("AT");
  if (isAT_Program)
  {
    Serial.println("AT success");
    tft.println("Scanning available networks...");
    /* Scanning the WIFI */
    WiFi.init(Serial2);

    if (WiFi.status() == WL_NO_MODULE)
    {
      Serial.println();
      Serial.println("Communication with WiFi module failed!");
      // don't continue
      while (true)
        ;
    }
    WiFi.disconnect();    // to clear the way. not persistent
    WiFi.setPersistent(); // set the following WiFi connection as persistent
    WiFi.endAP();         // to disable default automatic start of persistent AP at startup

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    printMacAddress(mac);

    // scan for existing networks:
    Serial.println();
    Serial.println("Scanning available networks...");
    listNetworks();

    Serial.println("Connecting to the network...");
    tft.println("Connecting to the network...");

    Serial.print("Attempting to connect to WPA SSID: ");
    tft.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    tft.println(ssid);

    int status;
    overtime = millis();
    while (status != WL_CONNECTED)
    {
      status = WiFi.begin(ssid, pass);
      Serial.print(".");
      tft.print(".");
      delay(500);

      if (millis() - overtime > 5000)
      {
        overtime = 0;
        break;
      }
    }

    Serial.println("You're connected to the network");
    printWifiData();
    Serial.println("Try to connect \"https://www.baidu.com\"");
    tft.println("Try to connect \"https://www.baidu.com\"");

    Serial2.println("AT+HTTPCGET=\"https://www.baidu.com/\"");
    overtime = millis();
    while (!Serial2.available())
    {
      if (millis() - overtime > 2000)
      {
        overtime = 0;
        break;
      }
    }

    String acc = "OK\r\n";
    String ack = "";
    overtime = millis();
    do
    {
      ack += Serial2.readString();
      if (millis() - overtime > 2000)
      {
        overtime = 0;
        break;
      }
    } while (!ack.endsWith(acc));

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println(ack.c_str());
  }
  else
  {
    Serial.println("AT fail");
    tft.println("The ESP32C3 communication fails..");
  }
}

bool Send_AT_CMD(const char *cmd)
{
  Serial2.println(cmd);
  String acc = "OK\r\n";
  String ack = "";
  uint32_t smap = millis() + 100;
  while (millis() < smap)
  {
    if (Serial2.available() > 0)
    {
      ack += Serial2.readString();
      if (ack.endsWith(acc))
      {
        return true;
      }
    }
  }
  return false;
}

void printWifiData()
{
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  tft.print("IP Address: ");
  tft.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void listNetworks()
{
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
  {
    Serial.println("Couldn't get a WiFi connection");
    while (true)
      ;
  }

  // print the list of networks seen:
  Serial.print("number of available networks: ");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++)
  {
    Serial.print(thisNet + 1);
    Serial.print(") ");
    Serial.print("Signal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tChannel: ");
    Serial.print(WiFi.channel(thisNet));
    byte bssid[6];
    Serial.print("\t\tBSSID: ");
    printMacAddress(WiFi.BSSID(thisNet, bssid));
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
    Serial.print("\t\tSSID: ");
    Serial.println(WiFi.SSID(thisNet));
    Serial.flush();

    tft.printf("[%d]RSSI: %ddBm SSID: %s", thisNet, WiFi.RSSI(thisNet), WiFi.SSID(thisNet));
    tft.println();
  }
  Serial.println();
  tft.println("end of scan");
}

void printEncryptionType(int thisType)
{
  // read the encryption type and print out the name:
  switch (thisType)
  {
  case ENC_TYPE_WEP:
    Serial.print("WEP");
    break;
  case ENC_TYPE_TKIP:
    Serial.print("WPA");
    break;
  case ENC_TYPE_CCMP:
    Serial.print("WPA2");
    break;
  case ENC_TYPE_NONE:
    Serial.print("None");
    break;
  case ENC_TYPE_AUTO:
    Serial.print("Auto");
    break;
  case ENC_TYPE_UNKNOWN:
  default:
    Serial.print("Unknown");
    break;
  }
}

void printMacAddress(byte mac[])
{
  for (int i = 5; i >= 0; i--)
  {
    if (mac[i] < 16)
    {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0)
    {
      Serial.print(":");
    }
  }
  Serial.println();
}
#endif /** LILYGO_ORG_SRC_EN **/

/* end of file */
