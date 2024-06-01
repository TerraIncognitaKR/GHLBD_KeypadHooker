# GHLBD_KeypadHooker

Hacking numeric keypad of GHLBD, a Chinese-made Android 9 calculator by Hardware method.

This calculator's numeric keypad has an MCU. Each time you press the keypad, a unique data packet is sent to the Allwinner AP via UART.

I passed this unique data packet through a wire trick to another MCU --- RP2040 --- and then implemented the functionality I wanted.




See Video :
  https://www.youtube.com/watch?v=Qw5ag-F0t1g

Target Board :
  LILYGO T-PicoC3 (RP2040 Based)
  https://github.com/Xinyuan-LilyGO/T-PicoC3

IDE :
  Arduino IDE 2.3.2 or latest

  
