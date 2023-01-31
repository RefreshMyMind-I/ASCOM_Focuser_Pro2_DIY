//-----------------------------------------------------------------------
// myFocuserPro2 Focuser Config File A4998
// (c) R, Brown, 2014-2022, All rights reserved.
//-----------------------------------------------------------------------
#ifndef focuserconfig_h
#define focuserconfig_h

#include <Arduino.h>
#include "myBoardDefs.h"


//-----------------------------------------------------------------------
// SPECIFY DRIVER BOARD HERE
//-----------------------------------------------------------------------
#define DRVBRD  A4998


//-----------------------------------------------------------------------
// SPECIFY FOCUS CONTROLLER OPTIONS HERE
//-----------------------------------------------------------------------
// Caution: Do not enable a feature if the associated hardware circuits are
// not fitted on the board. Enable or disable the specific hardware below


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
// USER CONFIGURATION END: DO NOT EDIT BELOW THIS LINE
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
// HARDWARE OPTIONS CHECKS: DO NOT CHANGE
//-----------------------------------------------------------------------
// DO NOT CHANGE

#ifndef DRVBRD
#halt // ERROR you must have DRVBRD defined
#endif

#ifndef PB_STEPS
#define PB_STEPS 1
#endif

#ifdef LCDDISPLAY
#ifdef LCD1602
#ifdef LCD1604
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif

#ifdef LCDDISPLAY
#ifdef LCD1602
#ifdef LCD2004
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif

#ifdef LCDDISPLAY
#ifdef LCD1604
#ifdef LCD2004
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif

#ifdef LCDDISPLAY
#ifdef LCD1602
#ifdef LCD2004
#ifdef LCD1604
#halt //Error - you can only have one LCD type defined, one of LCD1602, LCD1604 or LCD2004.
#endif
#endif
#endif
#endif

#ifdef LCDDISPLAY
#ifndef LCD2004
#ifndef LCD1604
#ifndef LCD1602
#halt //Error - you can must define either LCD1602 or LCD2004 or LCD1604.
#endif
#endif
#endif
#endif

#ifdef OLEDDISPLAY
#if defined(USE_SSD1306) && defined(USE_SSH1106)
#halt //Error - you can must define either USE_SSD1306 or USE_SSH1106 if using an OLEDDISPLAY
#endif
#endif

#ifndef USE_SSD1306
#ifndef USE_SSH1106
#halt //Error - you  must define either USE_SSD1306 or USE_SSH1106 if using an OLEDDISPLAY
#endif
#endif

#ifdef LCDDISPLAY
#ifdef OLEDDISPLAY
#Halt // You cannot have both LCDDISPLAY and OLEDDISPLAY defined at the same time - one or the other
#endif
#endif


#endif
