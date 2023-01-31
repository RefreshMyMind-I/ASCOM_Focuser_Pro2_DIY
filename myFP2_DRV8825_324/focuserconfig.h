//-----------------------------------------------------------------------
// myFocuserPro2 Focuser Config File DRV8825
// (c) R, Brown, 2014-2022, All rights reserved.
//-----------------------------------------------------------------------
#ifndef focuserconfig_h
#define focuserconfig_h

#include <Arduino.h>
#include "myBoardDefs.h"


//-----------------------------------------------------------------------
// SPECIFY FOCUS CONTROLLER OPTIONS HERE
//-----------------------------------------------------------------------
// Caution: Do not enable a feature if the associated hardware circuits are 
// not fitted on the board. Enable or disable the specific hardware below


// -----------------------------------------------------------------------------
// SPECIFY DRIVER BOARD HERE
// -----------------------------------------------------------------------------
// DRIVER BOARDS - Please specify your driver board here, only 1 can be defined, see #define DRVBRD lines
// Set DRVBRD to the correct driver board, ONLY ONE!!!! - uncomment the correct line for your controller
#define DRVBRD   DRV8825
//#define DRVBRD   DRV8825RE
//#define DRVBRD   DRV8825TFT22        // Wiring details in myBoards.h


//-----------------------------------------------------------------------
// INPUT DEVICES
// HOME POSITION SWITCH, STEPPER POWER DETECT, TEMPERATURE PROBE, PUSH BUTTONS
//-----------------------------------------------------------------------
// do NOT uncomment HOMEPOSITIONSWITCH if you do not have the switch fitted
// To enable the HOMEPOSITION SWITCH, uncomment the nextline
//#define HOMEPOSITIONSWITCH 1

// This prevents the stepper motor moving when 12V to the stepper is OFF 
// and needs special circuitry or has no effect. To enable the 12V power 
// detect to the stepper motor, uncomment the next line (only available on some boards)
//#define STEPPERPWRDETECT 1

// To enable the temperature probe, uncomment next line
//#define TEMPERATUREPROBE 1

// To enable a Rotary encoder. uncomment the next line (only available on some boards)
//#define ROTARYENCODER 1

// To enable the Push Buttons for manual focusing, uncomment the next line
//#define PUSHBUTTONS 1

// To specify the number of motor steps to move for 1 push button press
// change the 1 value for PB_STEPS below. The max value should be 1/2 the
// focuser step size, [1-255 is the range]. If you set PB_STEPS to 0
// then the push buttons will not move the focuser. 
//#define PB_STEPS 1   


//-----------------------------------------------------------------------
// OUTPUT DEVICES
// BUZZER AND LEDS
//-----------------------------------------------------------------------
// Buzzer is used as a power on boot test, and with push-buttons as a
// feedback for push button operation
// To enable the buzzer, uncomment the next line
//#define BUZZER 1

// To enable the IN-OUT LEDS, uncomment the next line
//#define INOUTLEDS 1


//-----------------------------------------------------------------------
// CONTROLLER CONNECTION OPTIONS: BLUETOOTH, CONTROLLERISAMICRO
//-----------------------------------------------------------------------
// To enable bluetooth, uncomment the next line  (only available on some boards)
//#define BLUETOOTH 1

// provided by IL, enables the serialEventRun function for a Micro
// To enable support when usng a Micro instead of a Nano, uncomment the next line
//#define CONTROLLERISAMICRO 1


//-----------------------------------------------------------------------
// SOFTWARE OPTIONS
//-----------------------------------------------------------------------
// To enable the super slow jogging, uncomment the next line
//#define SUPERSLOWJOG 1

// To enable the start boot screen showing startup messages, uncomment the next line
//#define SHOWBOOTMSGS 1

// This option helps when using pushbuttons
// To disable the update of the display till after a move has finished, uncomment the next line
//#define DELAYEDDISPLAYUPDATE 1


//-----------------------------------------------------------------------
// LCD DISPLAY
//-----------------------------------------------------------------------
// To enable the LCD DISPLAY uncomment the next line (LCD1602, I2C)
//#define LCDDISPLAY 1

// only uncomment one of the following LCDxxxx lines depending upon your lcd type
#define LCD1602             1             // 16 character, 2 lines
//#define LCD1604           2             // 16 character, 4 lines
//#define LCD2004           3             // 20 character, 4 lines


//-----------------------------------------------------------------------
// OLED DISPLAY
//-----------------------------------------------------------------------
// To enable the OLED DISPLAY uncomment the next line
//#define OLEDDISPLAY 1

// only uncomment one of the following USE_SSxxxx lines depending upon your lcd type
// For the OLED 128x64 0.96" display using the SSD1306 driver, uncomment the following line
#define USE_SSD1306   1

// For the OLED 128x64 1.3" display using the SSH1106 driver, uncomment the following line
//#define USE_SSH1106   2


//-----------------------------------------------------------------------
// TFT22 DISPLAY
//-----------------------------------------------------------------------
// To enable a TFT22 display on the TFT22 stripboard, uncomment the next line
//#define TFTDISPLAY 3


//-----------------------------------------------------------------------
// USER CONFIGURATION END: DO NOT EDIT BELOW THIS LINE
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
// HARDWARE OPTIONS CHECKS: DO NOT CHANGE
//-----------------------------------------------------------------------
// DO NOT CHANGE

#ifndef PB_STEPS
#define PB_STEPS 1
#endif

#ifndef DRVBRD
#halt // ERROR you must have DRVBRD defined
#endif

#ifdef LCDDISPLAY
#ifdef OLEDDISPLAY
#Halt // You cannot have both LCDDISPLAY and OLEDDISPLAY defined at the same time - one or the other
#endif
#ifdef TFTDISPLAY
#Halt // You cannot have both LCDDISPLAY and TFTDISPLAY defined at the same time - one or the other
#endif
#endif

#ifdef OLEDDISPLAY
#ifdef TFTDISPLAY
#Halt // You cannot have both OLEDDISPLAY and TFTDISPLAY defined at the same time - one or the other
#endif
#endif

#endif
